from async_pyserial.common import SerialPortOptions, SerialPortEvent, SerialPortBase

import sys

def on_write(err):
    print(f'error: {err}')

class SerialPort(SerialPortBase):
    def __init__(self, portName: str, options: SerialPortOptions) -> None:
        
        super().__init__(portName, options)

        from async_pyserial import async_pyserial_core
        
        self.internal = async_pyserial_core.SerialPort(portName, self.internal_options)
            
        def on_data(data):
            self.emit(SerialPortEvent.ON_DATA, data)
        
        self.internal.set_data_callback(on_data)
        
    def write(self, data: bytes):
        if sys.platform == 'win32':
            self.internal.write(data, on_write)
        else:
            self.internal.write(data)
        
    def open(self):
        self.internal.open()
        
    def close(self):
        self.internal.close()