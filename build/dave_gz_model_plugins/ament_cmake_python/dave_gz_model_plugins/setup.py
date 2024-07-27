from setuptools import find_packages
from setuptools import setup

setup(
    name='dave_gz_model_plugins',
    version='0.0.0',
    packages=find_packages(
        include=('dave_gz_model_plugins', 'dave_gz_model_plugins.*')),
)
