import rclpy
from sensor_msgs.msg import BatteryState
from gz.sim8 import Model, Link, Joint
from .Battery import Battery
import math
import time
from collections import deque
import threading


class LinearBatteryGzSimPlugin:

    def __init__(self):
        super().__init__()
        self.node = None  # Gazebo communication node
        self.battery_state_pub = None  # Battery state of charge publisher
        self.update_timer = None  # Update timer for plugin
        self.robot_namespace = ""  # Namespace for the robot
        self.update_rate = 2  # Update rate in Hz
        self.ecm = None  # Entity component manager

        # Name of model and battery, used for debugging and identification
        self.modelName = ""
        self.batteryName = ""

        # Battery-related attributes
        self.battery = None  # Pointer to battery object
        self.drainPrinted = False  # Flag for printing battery drained warning
        self.consumerId = -1  # Battery consumer identifier
        self.batteryEntity = None  # Battery entity
        self.e0 = 0.0  # Open-circuit voltage parameter e0
        self.e1 = 0.0  # Open-circuit voltage parameter e1
        self.q0 = 0.0  # Initial battery charge in Ah
        self.c = 0.0  # Battery capacity in Ah
        self.r = 0.0  # Internal resistance in Ohms
        self.tau = 1.0  # Low-pass filter time constant
        self.iraw = 0.0  # Raw battery current in A
        self.ismooth = 0.0  # Smoothed battery current in A
        self.q = 0.0  # Instantaneous battery charge in Ah
        self.soc = 1.0  # State of charge [0, 1]
        self.startCharging = False  # Flag for recharging status
        self.tCharge = 0.0  # Hours to fully charge battery
        self.fixIssue225 = False  # Flag for specific battery fixes

        # Historic current and time interval lists (used for smoothing)
        self.iList = deque(maxlen=100)  # Battery current history
        self.dtList = deque(maxlen=100)  # Time interval history

        # Simulation-related attributes
        self.stepSize = 0.0  # Time step size during update
        self.startDraining = False  # Flag for whether battery should start draining
        self.drainStartTime = -1  # Time when battery starts draining
        self.lastPrintTime = -1  # Last time a debug message was printed
        self.initialPowerLoad = 0.0  # Initial power load from config

        # Transport node and message publishing
        self.node = None  # Placeholder for transport node (communication)
        self.statePub = None  # Placeholder for state publisher

    def reset(self):
        # Reset the battery plugin to initial state
        self.q = self.q0  # Reset the charge to the initial value
        self.soc = 1.0  # Reset state of charge
        self.startCharging = False  # Stop charging
        self.startDraining = False  # Stop draining
        self.drainPrinted = False  # Reset drain printed flag
        self.iList.clear()  # Clear the current history
        self.dtList.clear()  # Clear the time interval history

    def state_of_charge(self):
        # Return the current state of charge
        return self.soc

    def on_enable_recharge(self, req):
        # Start recharging the battery
        if req.data:  # Assuming req is an object with a `data` attribute
            self.startCharging = True

    def on_disable_recharge(self, req):
        # Stop recharging the battery
        if req.data:
            self.startCharging = False

    def on_battery_draining_msg(self, data, size, info):
        # Handle messages to start battery draining
        self.startDraining = True
        self.drainStartTime = time.time()

    def on_battery_stop_draining_msg(self, data, size, info):
        # Handle messages to stop battery draining
        self.startDraining = False
        self.drainStartTime = -1

    def configure(self, entity, sdf, ecm, event_mgr):

        self.model = Model(entity)
        self.model.link_by_name(ecm=ecm, name="base_link")
        self.link = Link(entity)
        self.ecm = ecm
        self.robot_namespace = self.model.name(ecm)
        
        if not self.link:
            self.node.get_logger().error("Link not initialized properly.")
            return

        namespace = sdf.get_string("namespace", "default_model/default_link")[0]
        if namespace == "default_model/default_link":
            self.robot_namespace = self.model.name(ecm)
        else:
            self.robot_namespace = namespace + "/" + self.model.name(ecm)
        self.update_rate = sdf.get_int("update_rate", 2)[0]

        if self.update_rate <= 0.0:
            print(f"Invalid update rate, setting it to 2 Hz, rate={self.update_rate}")
            self.update_rate = 2

        # Initialize battery parameters from SDF
        self.q0 = sdf.get_float("capacity", 5.0)[0]
        self.q = sdf.get_float("initial_charge", self.q0)[0]
        self.voltage = sdf.get_float("voltage", 12.0)[0]

        rclpy.init()
        self.node = rclpy.create_node("battery_plugin", namespace=self.robot_namespace)
        self.battery_state_pub = self.node.create_publisher(BatteryState, "battery_state", 10)

        # Add logging before initializing the timer
        self.node.get_logger().info("Initializing the update timer")
        # self.update_timer = self.node.create_timer(
        #     1.0 / self.update_rate, self.publish_battery_state_callback
        # )

        # self.update_connection = event_mgr.connect(events.Update, self.publish_battery_state_callback)
        self.node.get_logger().info(f"Timer initialized with rate {self.update_rate} Hz")

        self.node.get_logger().info(f"Battery initial charge: {self.q} Ah")
        self.node.get_logger().info(f"Battery capacity: {self.q0} Ah")
        self.node.get_logger().info(f"Battery voltage: {self.voltage} V")

        self.node.get_logger().info(f"Battery Plugin for <{self.robot_namespace}> initialized")
        self.node.get_logger().info(f"Update rate [Hz]={self.update_rate}")





        if sdf.has_element("open_circuit_voltage_constant_coef"):
            self.e0 = sdf.get_float("open_circuit_voltage_constant_coef")

        if sdf.has_element("open_circuit_voltage_linear_coef"):
            self.e1 = sdf.get_float("open_circuit_voltage_linear_coef")

        if sdf.has_element("capacity"):
            self.c = sdf.get_float("capacity")

        if self.c <= 0:
            self.node.get_logger().error("No <capacity> or incorrect value specified. Capacity should be greater than 0.")
            return

        self.q0 = self.c
        if sdf.has_element("initial_charge"):
            self.q0 = sdf.get_float("initial_charge")
            if self.q0 > self.c or self.q0 < 0:
                self.node.get_logger().error("<initial_charge> value should be between [0, <capacity>].")
                self.q0 = max(0.0, min(self.q0, self.c))
                self.node.get_logger().error(f"Setting <initial_charge> to [{self.q0}] instead.")

        self.q = self.q0

        if sdf.has_element("resistance"):
            self.r = sdf.get_float("resistance")

        if sdf.has_element("smooth_current_tau"):
            self.tau = sdf.get_float("smooth_current_tau")
            if self.tau <= 0:
                self.node.get_logger().error("<smooth_current_tau> value should be positive. Using [1] instead.")
                self.tau = 1

        if sdf.has_element("fix_issue_225"):
            self.fix_issue_225 = sdf.get_bool("fix_issue_225")

        if sdf.has_element("battery_name") and sdf.has_element("voltage"):
            self.batteryName = sdf.get_string("battery_name")
            self.init_voltage = sdf.get_string("voltage")

            self.battery = Battery(self.batteryName, float(self.init_voltage))
            self.battery.init()
            self.battery.set_update_func(self.on_update_voltage)
        else:
            self.node.get_logger().error("Error: No <battery_name> or <voltage> specified. Both are required.")
            return


        if sdf.has_element("enable_recharge"):
            isCharging = sdf.get_bool("enable_recharge")
            if isCharging:
                if sdf.has_element("charging_time"):
                    self.tCharge = sdf.get_float("charging_time")
                    self.node.get_logger().info(f"charging_time={self.tCharge}")
                else:
                    self.node.get_logger().error("No <charging_time> specified. Parameter required to enable recharge.")
                    return

        #         enableRechargeTopic = f"/model/{self.modelName}/battery/{sdf.get_str('battery_name')}/recharge/start"
        #         disableRechargeTopic = f"/model/{self.modelName}/battery/{sdf.get_str('battery_name')}/recharge/stop"

        #         validEnableRechargeTopic = transport13.TopicUtils.AsValidTopic(enableRechargeTopic)
        #         validDisableRechargeTopic = transport13.TopicUtils.AsValidTopic(disableRechargeTopic)

        #         if not validEnableRechargeTopic or not validDisableRechargeTopic:
        #             self.node.get_logger().error(f"Failed to create valid topics. Not valid: [{enableRechargeTopic}] and [{disableRechargeTopic}]")
        #             return

        #         self.node.Advertise(validEnableRechargeTopic, self.OnEnableRecharge)
        #         self.node.Advertise(validDisableRechargeTopic, self.OnDisableRecharge)

        #         if sdf.has_element("recharge_by_topic"):
        #             self.node.Subscribe(validEnableRechargeTopic, self.OnEnableRecharge)
        #             self.node.Subscribe(validDisableRechargeTopic, self.OnDisableRecharge)

        if sdf.has_element("power_load"):
            self.initialPowerLoad = sdf.get_float("power_load")
            self.consumerId = self.battery.AddConsumer()
            success = self.battery.SetPowerLoad(self.consumerId, self.initialPowerLoad)
            if not success:
                self.node.get_logger().error("Failed to set consumer power load.")
        else:
            self.node.get_logger().warning("Required attribute power_load missing in LinearBatteryPlugin SDF")

        if sdf.has_element("start_draining"):
            self.startDraining = sdf.get_bool("start_draining")

        # if sdf.has_element("power_draining_topic"):
        #     sdfElem = sdf.FindElement("power_draining_topic")
        #     while sdfElem:
        #         topic = sdfElem.get()
        #         self.node.SubscribeRaw(topic, self.OnBatteryDrainingMsg)
        #         self.node.get_logger().info(f"LinearBatteryPlugin subscribes to power draining topic [{topic}].")
        #         sdfElem = sdfElem.getNextElement("power_draining_topic")

        # if sdf.has_element("stop_power_draining_topic"):
        #     sdfElem = sdf.FindElement("stop_power_draining_topic")
        #     while sdfElem:
        #         topic = sdfElem.get()
        #         self.node.SubscribeRaw(topic, self.OnBatteryStopDrainingMsg)
        #         self.node.get_logger().info(f"LinearBatteryPlugin subscribes to stop power draining topic [{topic}].")
        #         sdfElem = sdfElem.getNextElement("stop_power_draining_topic")

        self.node.get_logger().info(f"LinearBatteryPlugin configured. Battery name: {self.battery.name}")
        self.node.get_logger().info(f"Battery initial voltage: {self.battery.init_voltage}")

        self.soc = self.q / self.c
        # ecm.CreateComponent(self.batteryEntity, components.BatterySoC(self.soc))

        # stateTopic = f"/model/{self.model.Name(ecm)}/battery/{self.battery.name}/state"
        # validStateTopic = transport13.TopicUtils.AsValidTopic(stateTopic)

        # if not validStateTopic:
        #     self.node.get_logger().error(f"Failed to create valid state topic [{stateTopic}]")
        #     return

        # opts = transport13.AdvertiseMessageOptions()
        # opts.SetMsgsPerSec(50)
        # self.statePub = self.node.Advertise(msgs10.BatteryState, validStateTopic, opts)




    def pre_update(self, info,ecm):
        

        if not self.startDraining:
            
            self.startDraining = True

            # joints = ecm.children_by_components(
            #     self.model.entity(),
            #     Joint()
            # )

            # for joint_entity in joints:
            #     joint_velocity_cmd = ecm.component(JointVelocityCmd, joint_entity)
            #     if joint_velocity_cmd:
            #         for joint_vel in joint_velocity_cmd.data():
            #             if abs(float(joint_vel)) > 0:
            #                 self.startDraining = True
            #                 return

            #     joint_force_cmd = ecm.component(JointForceCmd, joint_entity)
            #     if joint_force_cmd:
            #         for joint_force in joint_force_cmd.data():
            #             if abs(float(joint_force)) > 0:
            #                 self.startDraining = True
            #                 return
                        
    def update(self,info, ecm):

        if not self.startDraining and not self.startCharging:
            return

        # Find the time at which battery starts to drain
        sim_time = int(info.sim_time.total_seconds())
        if self.drainStartTime == -1:
            self.drainStartTime = sim_time

        # Print drain time in minutes
        drain_time = (sim_time - self.drainStartTime) // 60
        if drain_time != self.lastPrintTime:
            self.lastPrintTime = drain_time
            self.node.get_logger().info("[Battery Plugin] Battery drain: {} minutes passed.".format(drain_time))

        # Update actual battery
        self.stepSize = info.dt

        # Sanity check: tau should be between [dt, +inf)
        dt = self.stepSize.total_seconds()
        if self.tau < dt:
            self.node.get_logger().error("<smooth_current_tau> should be in the range [dt, +inf) but is "
                "configured with [{}]. We'll be using [{}] instead".format(self.tau, dt))
            self.tau = dt

        if self.battery:
            self.battery.update()

            # # Update component
            # battery_comp = ecm.component(BatterySoC, self.batteryEntity)
            # battery_comp.data = self.state_of_charge()



    def post_update(self,info,ecm):
        self.publish_battery_state()

    def publish_battery_state(self):

        self.node.get_logger().info("Publishing Battery State:...")
        battery_state_msg = BatteryState()
        battery_state_msg.header.stamp = self.node.get_clock().now().to_msg()
        battery_state_msg.header.frame_id = self.link.name(self.ecm) if self.link else "unknown_link"

        self.node.get_logger().info(f"Publishing with frame_id: {battery_state_msg.header.frame_id}")
        self.node.get_logger().info(f"Timestamp: {battery_state_msg.header.stamp.sec}.{battery_state_msg.header.stamp.nanosec}")

        battery_state_msg.charge = self.q
        battery_state_msg.percentage = self.q / self.q0
        battery_state_msg.voltage = self.voltage
        battery_state_msg.design_capacity = self.q0

        battery_state_msg.power_supply_status = BatteryState.POWER_SUPPLY_STATUS_DISCHARGING
        battery_state_msg.power_supply_health = BatteryState.POWER_SUPPLY_HEALTH_GOOD
        battery_state_msg.power_supply_technology = BatteryState.POWER_SUPPLY_TECHNOLOGY_UNKNOWN
        battery_state_msg.present = True

        self.battery_state_pub.publish(battery_state_msg)
        self.node.get_logger().info("Battery state published")

    def update_battery(self):
        """Function to regularly update the battery state."""
        voltage = self.on_update_voltage(self.battery)
        # Here you can add logic to update the rest of your simulation based on the new voltage

        # Call this function again after the update interval
        self.update_timer = threading.Timer(1.0 / self.update_rate, self.update_battery)
        self.update_timer.start()


    def on_update_voltage(self, battery):
        assert battery is not None, "Battery is null."

        if abs(battery.voltage()) < 1e-3 and not self.startCharging:
            return 0.0
        if self.state_of_charge() < 0 and not self.startCharging:
            return battery.voltage()

        prev_soc_int = int(self.state_of_charge() * 100)

        # Seconds
        dt = self.stepSize.total_seconds()
        total_power = 0.0
        k = dt / self.tau

        if self.startDraining:
            for power_load in battery.power_loads:
                total_power += power_load[1]

        self.iraw = total_power / battery.voltage()

        # Compute charging current
        i_charge = self.c / self.tCharge

        # Add charging current to battery
        if self.startCharging and self.state_of_charge() < 0.9:
            self.iraw -= i_charge

        self.ismooth = self.ismooth + k * (self.iraw - self.ismooth)

        if not self.fix_issue_225:
            if len(self.iList) >= 100:
                self.iList.popleft()
                self.dtList.popleft()
            self.iList.append(self.ismooth)
            self.dtList.append(dt)

        # Convert dt to hours
        self.q -= (dt * self.ismooth) / 3600.0

        # Open circuit voltage
        voltage = (self.e0 + self.e1 * (1 - self.q / self.c)
                - self.r * self.ismooth)

        # Estimate state of charge
        if self.fix_issue_225:
            self.soc = self.q / self.c
        else:
            isum = sum(self.iList[i] * self.dtList[i] / 3600.0
                    for i in range(len(self.iList)))
            self.soc -= isum / self.c

        # Throttle debug messages
        soc_int = int(self.state_of_charge() * 100)
        if soc_int % 10 == 0 and soc_int != prev_soc_int:
            print(f"Battery: {self.battery.name()}")
            print(f"PowerLoads().size(): {len(battery.power_loads())}")
            print(f"Charging status: {self.startCharging}")
            print(f"Charging current: {i_charge}")
            print(f"Voltage: {voltage}")
            print(f"State of charge: {self.state_of_charge()} (q {self.q})\n")

        if self.state_of_charge() < 0 and not self.drainPrinted:
            print(f"Model {self.modelName} out of battery.")
            self.drainPrinted = True

        return voltage


    def init(self):
        super().init()

    def reset(self):
        super().reset()


    


def get_system():
    return LinearBatteryGzSimPlugin()


# GZ_REGISTER_SYSTEM_PLUGIN(LinearBatteryGzSimPlugin)
