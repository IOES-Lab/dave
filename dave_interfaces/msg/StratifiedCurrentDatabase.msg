# Publishes depths and velocities read from the csv database

# Depths
float32[] depths

# Velocities
geometry_msgs/Vector3[] velocities

# Tide time (GMT)
int16[] time_gmt_year
int16[] time_gmt_month
int16[] time_gmt_day
int16[] time_gmt_hour
int16[] time_gmt_minute

# Tide velocities
float32[] tidevelocities

# Tide constituents
bool tideconstituents
float32 m2_amp
float32 m2_phase
float32 m2_speed
float32 s2_amp
float32 s2_phase
float32 s2_speed
float32 n2_amp
float32 n2_phase
float32 n2_speed

# Tide direction
float32 ebb_direction
float32 flood_direction

# World start time (GMT)
int16 world_start_time_year
int16 world_start_time_month
int16 world_start_time_day
int16 world_start_time_hour
int16 world_start_time_minute

# Everything is written in lowercase according to ROS 2 conventions old(v)

# # Publishes depths and velocities read from the csv database

# # Depths
# float32[] depths

# # Velocities
# geometry_msgs/Vector3[] velocities

# # Tide time (GMT)
# int16[] timeGMTYear
# int16[] timeGMTMonth
# int16[] timeGMTDay
# int16[] timeGMTHour
# int16[] timeGMTMinute

# # Tide velocities
# float32[] tideVelocities

# # Tide constituents
# bool tideConstituents
# float32 M2amp
# float32 M2phase
# float32 M2speed
# float32 S2amp
# float32 S2phase
# float32 S2speed
# float32 N2amp
# float32 N2phase
# float32 N2speed

# # Tide direction
# float32 ebbDirection
# float32 floodDirection

# # World start time (GMT)
# int16 worldStartTimeYear
# int16 worldStartTimeMonth
# int16 worldStartTimeDay
# int16 worldStartTimeHour
# int16 worldStartTimeMinute