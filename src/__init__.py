import async_pyserial_core

import sys

class EventEmitter:
    def __init__(self) -> None:
        self.listeners: dict[str, list] = {}
        
    def emit(self, evt: str, *args, **kwargs):
        if evt not in self.listeners:
            return
        
        listeners = self.listeners[evt]
        
        for listener in listeners:
            listener(*args, **kwargs)
        
    def on(self, evt: str, listener):
        if evt not in self.listeners:
            self.listeners[evt] = []
            
        self.listeners[evt].append(listener)
        
class PlatformNotSupported(Exception):
    def __init__(self, *args: object) -> None:
        super().__init__(*args)
        
class SerialPortOptions:
    def __init__(self) -> None:
        self.baudrate = 9600
        self.bytesize = 8
        self.stopbits = 1
        self.parity = 0
        self.write_timeout = 50
        self.read_timeout = 50

class SerialPortEvent:
    ON_DATA = 'data'
        
class SerialPort(EventEmitter):
    def __init__(self, portName: str, options: SerialPortOptions) -> None:
        
        super().__init__()
        
        self.options = options
        self.internal_options = async_pyserial_core.SerialPortOptions()
        
        self.internal_options.baudrate = options.baudrate
        self.internal_options.bytesize = options.bytesize
        self.internal_options.stopbits = options.stopbits
        self.internal_options.parity = options.parity
        self.internal_options.write_timeout = options.write_timeout
        self.internal_options.read_timeout = options.read_timeout
        
        sys_platform = sys.platform
        if sys_platform  == 'win32':
            
            self.internal = async_pyserial_core.WinSerialPort(portName, self.internal_options)
            
            def on_data(data):
                self.emit(SerialPortEvent.ON_DATA, data)
            
            self.internal.set_data_callback(on_data)
        elif sys_platform == 'linux':
            raise PlatformNotSupported()
        elif sys_platform == 'darwin':
            raise PlatformNotSupported()
        else:
            raise PlatformNotSupported()
        
    def write(self, data: bytes):
        self.internal.write(data)
        
    def open(self):
        self.internal.open()
        
    def close(self):
        self.internal.close()

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