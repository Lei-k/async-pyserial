import os
import sys
import glob
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

with open("README.rst", "r", encoding="utf-8") as fh:
    long_description = fh.read()
    

if sys.platform.startswith('win32'):
    os_specific_macros = [('Win32', None)]
elif sys.platform.startswith('darwin'):
    os_specific_macros = [('MACOS', None)]
elif sys.platform.startswith('linux'):
    os_specific_macros = [('LINUX', None)]
else:
    raise RuntimeError("Unsupported platform")

def get_pybind_include(user=False):
    """Helper class to determine the pybind11 include path

    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the `get_include()`
    method can be invoked. """

    import pybind11
    return pybind11.get_include(user=user)

sources = glob.glob('./core/lib/**/*.cpp', recursive=True)

ext_modules = [
    Extension(
        'async_pyserial.async_pyserial_core',
        sources,
        include_dirs=[
            './core/include',
            get_pybind_include(),
            get_pybind_include(user=True)
        ],
        language='c++',
        extra_compile_args=['/std:c++17'] if sys.platform == 'win32' else ['-std=c++17'],
        define_macros=os_specific_macros,
        extra_link_args=[],
    ),
]

setup(
    name='async_pyserial',
    version='0.1.1',
    author='Neil Lei',
    author_email='qwe17235@gmail.com',
    description='Python bindings for a C++ serial port library',
    packages=['async_pyserial'],
    ext_modules=ext_modules,
    install_requires=[],
    long_description=long_description,
    cmdclass={'build_ext': build_ext},
    zip_safe=False,
)