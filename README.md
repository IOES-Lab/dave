# DAVE

[![Publish Lyrical / Jetty Docker image (AMD64)](https://github.com/IOES-Lab/dave/actions/workflows/docker-amd64.yml/badge.svg?branch=ros2)](https://github.com/IOES-Lab/dave/actions/workflows/docker-amd64.yml)
[![Publish Lyrical / Jetty Docker image (ARM64)](https://github.com/IOES-Lab/dave/actions/workflows/docker-arm64v8.yml/badge.svg?branch=ros2)](https://github.com/IOES-Lab/dave/actions/workflows/docker-arm64v8.yml)

## DAVE’s migration to ROS 2 and Gazebo Sim: POSIM

> **POSIM is the result of the substantial engineering effort to migrate DAVE to the latest LTS releases of ROS 2 and Gazebo Sim.**
>
> This modernization effort is now recognized under the name **[POSIM](https://github.com/IOES-Lab/POSIM)**. Visit the POSIM repository to explore the migrated platform.

Documentation is currently available at [dave-ros2.notion.site](http://dave-ros2.notion.site).

The current `ros2` stack targets **ROS 2 Lyrical on Ubuntu 26.04** and
**Gazebo Jetty** (provided through the ROS `ros_gz` vendor packages).

Before committing a contribution, run:

```sh
pip3 install pre-commit && pre-commit install && pre-commit run --all-files
