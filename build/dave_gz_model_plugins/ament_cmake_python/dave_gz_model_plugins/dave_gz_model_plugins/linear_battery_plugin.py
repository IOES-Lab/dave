import rclpy
from rclpy.node import Node
from sensor_msgs.msg import BatteryState
from gz.sim8.systems import LinearBatteryPlugin
from gz.sim8 import Model, Link, Plugin

class LinearBatteryGzSimPlugin(LinearBatteryPlugin):

    def __init__(self):
        super().__init__()
        self.node = None
        self.battery_state_pub = None
        self.update_timer = None
        self.robot_namespace = ""
        self.update_rate = 2

    def configure(self, entity, sdf, ecm, event_mgr):
        try:
            self.model = Model(entity)
            self.link = Link(self.model.link_by_name("link_name"))
        except Exception as e:
            print(f"Error loading plugin: {e}")
            return

        if sdf.HasElement("namespace"):
            self.robot_namespace = sdf.Get("namespace")

        if sdf.HasElement("update_rate"):
            self.update_rate = sdf.Get("update_rate")

        if self.update_rate <= 0.0:
            print(f"Invalid update rate, setting it to 2 Hz, rate={self.update_rate}")
            self.update_rate = 2

        rclpy.init()
        self.node = rclpy.create_node('battery_plugin', namespace=self.robot_namespace)
        self.battery_state_pub = self.node.create_publisher(BatteryState, 'battery_state', 10)
        self.update_timer = self.node.create_timer(1.0 / self.update_rate, self.publish_battery_state)

        self.node.get_logger().info(f"Battery Plugin for link <{self.link.name}> initialized")
        self.node.get_logger().info(f"Update rate [Hz]={self.update_rate}")

    def publish_battery_state(self):
        battery_state_msg = BatteryState()
        battery_state_msg.header.stamp = self.node.get_clock().now().to_msg()
        battery_state_msg.header.frame_id = self.link.name

        battery_state_msg.charge = self.q
        battery_state_msg.percentage = self.q / self.q0
        battery_state_msg.voltage = self.voltage
        battery_state_msg.design_capacity = self.q0

        battery_state_msg.power_supply_status = BatteryState.POWER_SUPPLY_STATUS_DISCHARGING
        battery_state_msg.power_supply_health = BatteryState.POWER_SUPPLY_HEALTH_GOOD
        battery_state_msg.power_supply_technology = BatteryState.POWER_SUPPLY_TECHNOLOGY_UNKNOWN
        battery_state_msg.present = True

        self.battery_state_pub.publish(battery_state_msg)

    def init(self):
        super().init()

    def reset(self):
        super().reset()

def get_system():
    return LinearBatteryGzSimPlugin()


# GZ_REGISTER_SYSTEM_PLUGIN(LinearBatteryGzSimPlugin)
