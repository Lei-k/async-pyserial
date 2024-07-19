import os
import sys
import glob
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

def get_pybind_include(user=False):
    """Helper class to determine the pybind11 include path

    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the `get_include()`
    method can be invoked. """

    import pybind11
    return pybind11.get_include(user=user)

sources = glob.glob('core/lib/**/*.cpp', recursive=True)

ext_modules = [
    Extension(
        'async_pyserial_core',
        sources,
        include_dirs=[
            'core/include/common',
            'core/include/win',
            get_pybind_include(),
            get_pybind_include(user=True)
        ],
        language='c++',
        extra_compile_args=['/std:c++17'] if sys.platform == 'win32' else ['-std=c++17'],
        extra_link_args=[],
    ),
]

setup(
    name='async_pyserial',
    version='0.1.0',
    author='Neil Lei',
    author_email='qwe17235@gmail.com',
    description='Python bindings for a C++ serial port library',
    long_description='',
    ext_modules=ext_modules,
    install_requires=['pybind11>=2.6.1'],
    cmdclass={'build_ext': build_ext},
    zip_safe=False,
)