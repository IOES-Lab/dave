from setuptools import setup

package_name = 'dave_gz_model_plugins'

setup(
    name=package_name,
    version='0.0.1',
    packages=[package_name],
    package_dir={package_name: 'src'},
    install_requires=['setuptools'],
    zip_safe=True,
    author='Abhimanyu Bhowmik',
    author_email='bhowmikabhimanyu@gmail.com',
    description='GzSim battery plugin python implementation',
    license='Apache License 2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'linear_battery_plugin = dave_gz_model_plugins.linear_battery_plugin:LinearBatteryGzSimPlugin',
        ],
        'gz.system_plugins': [
            'linear_battery_plugin = dave_gz_model_plugins.linear_battery_plugin:get_system',
        ],
    },
)
