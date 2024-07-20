import async_pyserial_core

import sys

from common import *

__version__ = '3.8'

VERSION = __version__

sys_platform = sys.platform

if sys_platform  == 'win32':
            
    from native_serialport import SerialPort
    
elif sys_platform == 'linux':
    from native_serialport import SerialPort
elif sys_platform == 'darwin':
    raise PlatformNotSupported()
else:
    raise PlatformNotSupported()

if __name__ == "__main__":
    import sys
    
    args = sys.argv[1:]
    
    if len(args) < 1:
        raise Exception('please give a comport')
    
    def on_data(data):
        
        print(data)
    
    options = SerialPortOptions()
    options.baudrate = 9600
    options.bytesize = 8
    options.stopbits = 1
    options.parity = 0
    options.write_timeout = 50
    options.read_timeout = 50

    serial = SerialPort(args[0], options)
    serial.on(SerialPortEvent.ON_DATA, on_data)
    serial.open()
    
    try:
        while True:
            data_to_send = input("Enter to send data or ('exit' to quit): ")
            if data_to_send.lower() == 'exit':
                break
            
            size = 5 * 1024

            # # 生成指定大小的字节对象
            large_bytes = bytes([42] * size)
            
            #serial.write(large_bytes)
            
            serial.write(data_to_send.encode('utf-8'))
    except KeyboardInterrupt:
        pass
    except Exception as ex:
        print(ex)
    finally:
        serial.close()