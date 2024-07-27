#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import BatteryState

class BatteryStateListener(Node):
    def __init__(self):
        super().__init__('battery_state_listener')
        self.subscription = self.create_subscription(
            BatteryState,
            'battery_state',
            self.listener_callback,
            10)

    def listener_callback(self, msg):
        self.get_logger().info(f'Battery charge: {msg.charge}')
        self.get_logger().info(f'Battery percentage: {msg.percentage}')
        self.get_logger().info(f'Battery voltage: {msg.voltage}')
        self.get_logger().info(f'Design capacity: {msg.design_capacity}')
        self.get_logger().info(f'Status: {msg.power_supply_status}')
        self.get_logger().info(f'Health: {msg.power_supply_health}')
        self.get_logger().info(f'Technology: {msg.power_supply_technology}')
        self.get_logger().info(f'Present: {msg.present}')

def main(args=None):
    rclpy.init(args=args)
    node = BatteryStateListener()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
