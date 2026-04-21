#!/usr/bin/env python3
import math

import rclpy
from mavros_msgs.msg import ManualControl, State
from mavros_msgs.srv import CommandBool, SetMode
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile, ReliabilityPolicy
from sensor_msgs.msg import Joy
from std_msgs.msg import Bool

AXIS_SWAY = 0
AXIS_FWD = 1
AXIS_YAW = 4
AXIS_HEAVE = 5

BTN_Z_HOLD_OFF = 0
BTN_SCALE_UP = 1
BTN_SCALE_DN = 2
BTN_Z_HOLD_ON = 3
BTN_ARM_OFF = 8
BTN_ARM_ON = 9

DEADZONE = 0.08
DEADZONE_HEAVE = 0.18
RATE_HZ = 20.0
TIMEOUT_SEC = 0.3

STABILIZE_MODE = "STABILIZE"
DEPTH_HOLD_MODE = "ALT_HOLD"
MAX_MANUAL = 1000.0
THROTTLE_NEUTRAL = 500.0
THROTTLE_RANGE = 500.0

THROTTLE_LEVELS = [0.25, 0.50, 0.75, 1.00]
THROTTLE_DEFAULT_INDEX = 1


def dz(value, deadzone):
    return 0.0 if abs(value) < deadzone else value


def clamp(value, lo, hi):
    return lo if value < lo else hi if value > hi else value


def resolve_namespace(value):
    cleaned = str(value).strip()
    if not cleaned:
        return ""

    cleaned = cleaned.strip("/")
    return f"/{cleaned}" if cleaned else ""


class ArduSubManualControl(Node):
    """Convert Joy messages to MAVROS ManualControl for ArduSub SITL."""

    def __init__(self):
        super().__init__("ardusub_manual_control")

        self.declare_parameter("model_name", "bluerov2")
        self.declare_parameter("joystick_topic", "/joy")
        self.declare_parameter("keyboard_topic", "/keyboard/joy")
        self.declare_parameter("mavros_namespace", "mavros")

        self.model_name = (
            self.get_parameter("model_name").get_parameter_value().string_value or "bluerov2"
        )
        self.joystick_topic = (
            self.get_parameter("joystick_topic").get_parameter_value().string_value or "/joy"
        )
        self.keyboard_topic = (
            self.get_parameter("keyboard_topic").get_parameter_value().string_value
            or "/keyboard/joy"
        )

        mavros_namespace = (
            self.get_parameter("mavros_namespace").get_parameter_value().string_value or "mavros"
        )
        mavros_ns = resolve_namespace(mavros_namespace)

        manual_control_topic = f"{mavros_ns}/manual_control/send"
        state_topic = f"{mavros_ns}/state"
        arming_service = f"{mavros_ns}/cmd/arming"
        mode_service = f"{mavros_ns}/set_mode"
        armed_topic = f"/model/{self.model_name}/control/armed"

        state_qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=1,
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
        )

        self.create_subscription(Joy, self.joystick_topic, self.cb_joy, 10)
        self.create_subscription(Joy, self.keyboard_topic, self.cb_keyboard_joy, 10)
        self.create_subscription(State, state_topic, self.cb_state, state_qos)

        self.pub_manual = self.create_publisher(ManualControl, manual_control_topic, 10)
        self.pub_armed = self.create_publisher(Bool, armed_topic, state_qos)

        self.arm_client = self.create_client(CommandBool, arming_service)
        self.mode_client = self.create_client(SetMode, mode_service)

        self.input_state = {
            "joystick": {
                "axes": [],
                "buttons": [],
                "activity_time": None,
            },
            "keyboard": {
                "axes": [],
                "buttons": [],
                "activity_time": None,
            },
        }
        self.last_axes = []
        self.last_buttons = []

        self.connected = False
        self.armed = False
        self.current_mode = ""
        self.last_requested_mode = ""

        self.throttle_index = THROTTLE_DEFAULT_INDEX
        self.throttle_scale = THROTTLE_LEVELS[self.throttle_index]

        self._last_arm_on = 0
        self._last_arm_off = 0
        self._last_hold_on = 0
        self._last_hold_off = 0
        self._last_scale_up = 0
        self._last_scale_dn = 0

        self._last_warn_sec = {}
        self._mode_future = None
        self._arm_future = None

        self._publish_armed_state()
        self.timer = self.create_timer(1.0 / RATE_HZ, self.tick)

        self.get_logger().info(
            "ardusub_manual_control started | "
            f"model={self.model_name}, joy={self.joystick_topic}, "
            f"keyboard={self.keyboard_topic}, mavros_ns={mavros_ns or '/'} | "
            "mode control: external/QGC respected (no forced STABILIZE)"
        )

    def cb_joy(self, msg):
        self._update_input_state("joystick", msg)

    def cb_keyboard_joy(self, msg):
        self._update_input_state("keyboard", msg)

    def cb_state(self, msg):
        was_connected = self.connected
        previous_mode = self.current_mode
        previous_armed = self.armed

        self.connected = bool(msg.connected)
        self.armed = bool(msg.armed)
        self.current_mode = msg.mode

        if self.connected and not was_connected:
            self.get_logger().info("MAVROS connected to ArduSub")
        if previous_mode != self.current_mode and self.current_mode:
            self.get_logger().info(f"ArduSub mode: {self.current_mode}")
        if previous_armed != self.armed:
            self.get_logger().info("ArduSub armed" if self.armed else "ArduSub disarmed")

        self._publish_armed_state()

    def _update_input_state(self, source, msg):
        state = self.input_state[source]
        state["axes"] = list(msg.axes)
        state["buttons"] = list(msg.buttons)

        if any(abs(axis) > 1e-6 for axis in state["axes"]) or any(state["buttons"]):
            state["activity_time"] = self.get_clock().now()

    def _select_active_input(self):
        now = self.get_clock().now()
        active_state = None
        active_time_ns = -1

        for state in self.input_state.values():
            activity_time = state["activity_time"]
            if activity_time is None:
                continue

            age = (now - activity_time).nanoseconds * 1e-9
            if age > TIMEOUT_SEC:
                continue

            if activity_time.nanoseconds > active_time_ns:
                active_state = state
                active_time_ns = activity_time.nanoseconds

        return active_state

    def _get_axis(self, idx):
        return float(self.last_axes[idx]) if idx < len(self.last_axes) else 0.0

    def _get_btn(self, idx):
        return int(self.last_buttons[idx]) if idx < len(self.last_buttons) else 0

    def _publish_armed_state(self):
        msg = Bool()
        msg.data = self.armed
        self.pub_armed.publish(msg)

    def _publish_manual(self, x, y, z, r):
        msg = ManualControl()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.x = float(x)
        msg.y = float(y)
        msg.z = float(z)
        msg.r = float(r)
        msg.buttons = 0
        msg.buttons2 = 0
        msg.enabled_extensions = 0
        self.pub_manual.publish(msg)

    def _warn_throttled(self, key, message, period_sec=2.0):
        now_sec = self.get_clock().now().nanoseconds * 1e-9
        last_sec = self._last_warn_sec.get(key, -math.inf)
        if now_sec - last_sec >= period_sec:
            self.get_logger().warn(message)
            self._last_warn_sec[key] = now_sec

    def _call_set_mode(self, mode):
        self.last_requested_mode = mode

        if not self.mode_client.service_is_ready():
            self._warn_throttled("set_mode", "Waiting for MAVROS set_mode service")
            return
        if self._mode_future is not None and not self._mode_future.done():
            return

        request = SetMode.Request()
        request.custom_mode = mode
        self._mode_future = self.mode_client.call_async(request)
        self._mode_future.add_done_callback(self._on_mode_response)

    def _on_mode_response(self, future):
        try:
            response = future.result()
        except Exception as exc:
            self.get_logger().warn(f"Set mode request failed: {exc}")
            return

        if response.mode_sent:
            self.get_logger().info(f"Requested ArduSub mode: {self.last_requested_mode}")
        else:
            self.get_logger().warn(f"ArduSub rejected mode request: {self.last_requested_mode}")

    def _call_arm(self, arm):
        if not self.arm_client.service_is_ready():
            self._warn_throttled("arming", "Waiting for MAVROS arming service")
            return
        if self._arm_future is not None and not self._arm_future.done():
            return

        request = CommandBool.Request()
        request.value = arm
        self._arm_future = self.arm_client.call_async(request)
        self._arm_future.add_done_callback(self._on_arm_response)

    def _on_arm_response(self, future):
        try:
            response = future.result()
        except Exception as exc:
            self.get_logger().warn(f"Arm request failed: {exc}")
            return

        if not response.success:
            self.get_logger().warn("ArduSub rejected arm/disarm request")

    def _update_throttle_scale(self):
        scale_up = self._get_btn(BTN_SCALE_UP)
        scale_dn = self._get_btn(BTN_SCALE_DN)

        if scale_up == 1 and self._last_scale_up == 0:
            if self.throttle_index < len(THROTTLE_LEVELS) - 1:
                self.throttle_index += 1
                self.throttle_scale = THROTTLE_LEVELS[self.throttle_index]
                self.get_logger().info(f"manual XY/yaw scale = {self.throttle_scale:.2f}")

        if scale_dn == 1 and self._last_scale_dn == 0:
            if self.throttle_index > 0:
                self.throttle_index -= 1
                self.throttle_scale = THROTTLE_LEVELS[self.throttle_index]
                self.get_logger().info(f"manual XY/yaw scale = {self.throttle_scale:.2f}")

        self._last_scale_up = scale_up
        self._last_scale_dn = scale_dn

    def _handle_mode_buttons(self):
        hold_on = self._get_btn(BTN_Z_HOLD_ON)
        hold_off = self._get_btn(BTN_Z_HOLD_OFF)

        if hold_on == 1 and self._last_hold_on == 0:
            self._call_set_mode(DEPTH_HOLD_MODE)

        if hold_off == 1 and self._last_hold_off == 0:
            self._call_set_mode(STABILIZE_MODE)

        self._last_hold_on = hold_on
        self._last_hold_off = hold_off

    def _handle_arm_buttons(self):
        arm_on = self._get_btn(BTN_ARM_ON)
        arm_off = self._get_btn(BTN_ARM_OFF)

        if arm_on == 1 and self._last_arm_on == 0:
            self._call_arm(True)

        if arm_off == 1 and self._last_arm_off == 0:
            self._call_arm(False)

        self._last_arm_on = arm_on
        self._last_arm_off = arm_off

    def tick(self):
        active_input = self._select_active_input()
        if active_input is None:
            self.last_axes = []
            self.last_buttons = []
            self._publish_manual(0.0, 0.0, THROTTLE_NEUTRAL, 0.0)
            return

        self.last_axes = active_input["axes"]
        self.last_buttons = active_input["buttons"]

        self._handle_arm_buttons()
        self._handle_mode_buttons()
        self._update_throttle_scale()

        forward = -dz(self._get_axis(AXIS_FWD), DEADZONE) * MAX_MANUAL
        forward *= self.throttle_scale

        sway = -dz(self._get_axis(AXIS_SWAY), DEADZONE) * MAX_MANUAL
        sway *= self.throttle_scale

        heave = THROTTLE_NEUTRAL
        heave += -dz(self._get_axis(AXIS_HEAVE), DEADZONE_HEAVE) * THROTTLE_RANGE

        yaw = -dz(self._get_axis(AXIS_YAW), DEADZONE) * MAX_MANUAL
        yaw *= self.throttle_scale

        self._publish_manual(
            clamp(forward, -MAX_MANUAL, MAX_MANUAL),
            clamp(sway, -MAX_MANUAL, MAX_MANUAL),
            clamp(heave, 0.0, MAX_MANUAL),
            clamp(yaw, -MAX_MANUAL, MAX_MANUAL),
        )


def main():
    rclpy.init()
    node = ArduSubManualControl()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()
