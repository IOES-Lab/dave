import threading
from functools import partial

class Battery:
    def __init__(self, name, voltage):
        self.name = name
        self.init_voltage = voltage
        self.real_voltage = max(0.0, voltage)
        self.power_loads = {}  # Map of consumer ID to power loads in watts
        self.power_load_counter = 0
        self.update_func = self.update_default  # Default update function
        self.lock = threading.Lock()

    def init(self):
        """Initialize the battery."""
        self.reset_voltage()
        self.init_consumers()

    def reset_voltage(self):
        """Reset the battery's real voltage to its initial value."""
        self.real_voltage = max(0.0, self.init_voltage)

    def init_consumers(self):
        """Initialize the power load consumers."""
        with self.lock:
            self.power_loads.clear()

    def add_consumer(self):
        """Add a new power load consumer and return its unique ID."""
        with self.lock:
            new_id = self.power_load_counter
            self.power_load_counter += 1
            self.power_loads[new_id] = 0.0  # Initially zero power load
            return new_id

    def remove_consumer(self, consumer_id):
        """Remove a power load consumer by its ID."""
        with self.lock:
            if consumer_id in self.power_loads:
                del self.power_loads[consumer_id]
                return True
            else:
                print(f"Invalid battery consumer id[{consumer_id}]")
                return False

    def set_power_load(self, consumer_id, power_load):
        """Set the power load for a specific consumer."""
        with self.lock:
            if consumer_id in self.power_loads:
                self.power_loads[consumer_id] = power_load
                return True
            else:
                print(f"Invalid consumer ID: {consumer_id}")
                return False

    def get_power_load(self, consumer_id):
        """Get the power load for a specific consumer."""
        with self.lock:
            if consumer_id in self.power_loads:
                return self.power_loads[consumer_id]
            else:
                print(f"Invalid consumer ID: {consumer_id}")
                return None

    def power_loads(self):
        """Return all power loads."""
        with self.lock:
            return self.power_loads.copy()

    def voltage(self):
        """Return the current voltage of the battery."""
        return self.real_voltage

    def set_update_func(self, update_func):
        """Set the function used to update the real voltage."""
        self.update_func = update_func

    def reset_update_func(self):
        """Reset the update function to the default."""
        self.set_update_func(self.update_default)

    def update(self):
        """Update the battery's voltage using the current update function."""
        self.real_voltage = max(0.0, self.update_func(self))

    def update_default(self, battery):
        """Default update function, simply returns the current voltage."""
        if battery:
            return battery.voltage()
        else:
            return 0.0

    def __repr__(self):
        return f"Battery(name={self.name}, voltage={self.real_voltage}, power_loads={self.power_loads})"
