from setuptools import setup, find_packages


import os
os.environ['SETUPTOOLS_ENABLE_FEATURES'] = 'legacy-editable'

setup(
    name="cross-ipc",
    version="0.1.0",
    packages=find_packages(),
    package_data={
        "cross_ipc": ["*.dll"],  
    },
    description="Python bindings for cross-ipc library",
    author="HMNT",
    python_requires=">=3.6",
    
    zip_safe=False,
    include_package_data=True,
)