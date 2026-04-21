#!/usr/bin/env python3
import os
import select
import sys
import termios
import time
import tty

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy

AXIS_SWAY = 0
AXIS_FWD = 1
AXIS_YAW = 4
AXIS_HEAVE = 5

BTN_Z_HOLD_OFF = 0
BTN_Z_HOLD_ON = 3
BTN_ARM_OFF = 8
BTN_ARM_ON = 9

NUM_AXES = 6
NUM_BUTTONS = 17
PUBLISH_HZ = 30.0
KEY_TIMEOUT_SEC = 0.5


class KeyboardJoyPublisher(Node):
    """Publish keyboard input as Joy messages for ArduSub manual control."""

    def __init__(self):
        super().__init__("keyboard_joy_publisher")

        self.declare_parameter("output_topic", "/keyboard/joy")
        output_topic = (
            self.get_parameter("output_topic").get_parameter_value().string_value
            or "/keyboard/joy"
        )

        self.pub = self.create_publisher(Joy, output_topic, 10)
        self.axes = [0.0] * NUM_AXES
        self.pending_buttons = set()
        self.button_release_pending = False
        self.last_key_time = 0.0

        self.input_stream = self._open_input_stream()
        self.fd = None
        self.old_term = None

        if self.input_stream is not None:
            self.fd = self.input_stream.fileno()
            self.old_term = termios.tcgetattr(self.fd)
            tty.setcbreak(self.fd)

        self.timer = self.create_timer(1.0 / PUBLISH_HZ, self.on_timer)

        self.get_logger().info(
            "Keyboard teleop ready on "
            f"{output_topic} | "
            "c: arm, x: disarm, w/s: forward/back, "
            "a/d: yaw, r/f: ascend/descend, "
            "h: ALT_HOLD, j: STABILIZE, space: stop, q: quit"
        )

    def _open_input_stream(self):
        if sys.stdin.isatty():
            return sys.stdin

        try:
            # Use unbuffered binary mode for reliable single-key reads.
            return open("/dev/tty", "rb", buffering=0)
        except OSError as exc:
            self.get_logger().warn(
                "No interactive TTY for keyboard teleop " f"({exc}). Run launch from a terminal."
            )
            return None

    def destroy_node(self):
        try:
            if self.fd is not None and self.old_term is not None:
                termios.tcsetattr(self.fd, termios.TCSADRAIN, self.old_term)
        except Exception:
            pass

        try:
            if self.input_stream not in (None, sys.stdin):
                self.input_stream.close()
        except Exception:
            pass

        super().destroy_node()

    def read_key_nonblocking(self):
        if self.fd is None:
            return None

        if select.select([self.fd], [], [], 0.0)[0]:
            raw = os.read(self.fd, 1)
            if not raw:
                return None
            try:
                return raw.decode("utf-8")
            except UnicodeDecodeError:
                return None

        return None

    def publish_joy(self):
        msg = Joy()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = "keyboard"
        msg.axes = list(self.axes)
        msg.buttons = [0] * NUM_BUTTONS

        for idx in self.pending_buttons:
            if 0 <= idx < NUM_BUTTONS:
                msg.buttons[idx] = 1

        self.pending_buttons.clear()
        self.pub.publish(msg)

    def zero_axes(self):
        self.axes = [0.0] * NUM_AXES

    def on_timer(self):
        if self.button_release_pending:
            self.zero_axes()
            self.publish_joy()
            self.button_release_pending = False

        key = self.read_key_nonblocking()
        now = time.time()

        if key is not None:
            self.last_key_time = now

            if key == "q":
                self.zero_axes()
                self.pending_buttons.add(BTN_ARM_OFF)
                self.publish_joy()
                rclpy.shutdown()
                return

            if key == " ":
                self.zero_axes()
                self.publish_joy()
                return

            self.zero_axes()

            if key == "c":
                self.pending_buttons.add(BTN_ARM_ON)
                self.button_release_pending = True
            elif key == "x":
                self.pending_buttons.add(BTN_ARM_OFF)
                self.button_release_pending = True
            elif key == "w":
                self.axes[AXIS_FWD] = -0.8
            elif key == "s":
                self.axes[AXIS_FWD] = 0.8
            elif key == "a":
                self.axes[AXIS_YAW] = 2.0
            elif key == "d":
                self.axes[AXIS_YAW] = -2.0
            elif key == "r":
                self.axes[AXIS_HEAVE] = -0.8
            elif key == "f":
                self.axes[AXIS_HEAVE] = 0.8
            elif key == "h":
                self.pending_buttons.add(BTN_Z_HOLD_ON)
                self.button_release_pending = True
            elif key == "j":
                self.pending_buttons.add(BTN_Z_HOLD_OFF)
                self.button_release_pending = True

            self.publish_joy()
            return

        if now - self.last_key_time > KEY_TIMEOUT_SEC and any(self.axes):
            self.zero_axes()
            self.publish_joy()


def main():
    rclpy.init()
    node = KeyboardJoyPublisher()

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
