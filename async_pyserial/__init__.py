import sys

from async_pyserial.common import *

__version__ = '3.8'

VERSION = __version__

__all__ = ["SerialPort", "SerialPortOptions", "SerialPortEvent", "SerialPortParity"]

sys_platform = sys.platform
    
if sys_platform  == 'win32' or \
    sys_platform == 'linux' or \
    sys_platform == 'darwin' or \
    'bsd' in sys_platform.lower():

    from async_pyserial.native_serialport import SerialPort

else:
    raise PlatformNotSupported()