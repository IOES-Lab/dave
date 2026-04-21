#!/usr/bin/env python3
import asyncio
import json

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy
import websockets


class JoyWebSocketServer(Node):
    """Bridge browser websocket joystick payloads to sensor_msgs/Joy."""

    def __init__(self):
        super().__init__("joy_ws_server")

        self.declare_parameter("ws_host", "0.0.0.0")
        self.declare_parameter("ws_port", 8765)
        self.declare_parameter("output_topic", "/joy")

        ws_host_value = self.get_parameter("ws_host").value
        ws_port_value = self.get_parameter("ws_port").value
        output_topic_value = self.get_parameter("output_topic").value

        self.ws_host = str(ws_host_value or "0.0.0.0")
        self.ws_port = int(ws_port_value)
        self.output_topic = str(output_topic_value or "/joy")

        self.pub = self.create_publisher(Joy, self.output_topic, 10)

        self.get_logger().info(
            f"WebSocket listening on ws://{self.ws_host}:{self.ws_port} -> " f"{self.output_topic}"
        )

    def publish_payload(self, payload):
        msg = Joy()
        msg.header.stamp = self.get_clock().now().to_msg()

        payload_id = str(payload.get("id", "")).strip() or "unknown"
        msg.header.frame_id = f"browser_gamepad:{payload_id}"

        msg.axes = [float(value) for value in payload.get("axes", [])]
        msg.buttons = [int(value) for value in payload.get("buttons", [])]

        self.pub.publish(msg)


async def main_async():
    rclpy.init()
    node = JoyWebSocketServer()

    async def handler(websocket):
        node.get_logger().info("Web joystick client connected")

        async for message in websocket:
            try:
                payload = json.loads(message)
            except Exception as exc:
                node.get_logger().warn(f"Invalid websocket JSON payload: {exc}")
                continue

            try:
                node.publish_payload(payload)
            except Exception as exc:
                node.get_logger().error(f"Failed to publish Joy payload: {exc}")

    async with websockets.serve(handler, node.ws_host, node.ws_port):
        while rclpy.ok():
            rclpy.spin_once(node, timeout_sec=0.01)
            await asyncio.sleep(0.01)

    node.destroy_node()
    if rclpy.ok():
        rclpy.shutdown()


def main():
    asyncio.run(main_async())


if __name__ == "__main__":
    main()
