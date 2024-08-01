# Publishes depths and velocities read from the csv database

# Depths
float32[] depths

# Velocities
geometry_msgs/Vector3[] velocities

# Tide time (GMT)
int16[] timegmtyear
int16[] timegmtmonth
int16[] timegmtday
int16[] timegmthour
int16[] timegmtminute

# Tide velocities
float32[] tidevelocities

# Tide constituents
bool tideconstituents
float32 m2amp
float32 m2phase
float32 m2speed
float32 s2amp
float32 s2phase
float32 s2speed
float32 n2amp
float32 n2phase
float32 n2speed

# Tide direction
float32 ebbdirection
float32 flooddirection

# World start time (GMT)
int16 worldstarttimeyear
int16 worldstarttimemonth
int16 worldstarttimeday
int16 worldstarttimehour
int16 worldstarttimeminute

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
