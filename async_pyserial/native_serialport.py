from async_pyserial.common import SerialPortOptions, SerialPortEvent, SerialPortBase

<<<<<<< HEAD
from typing import Callable
=======
import sys

def on_write(err):
    print(f'error: {err}')
>>>>>>> 6f2edc6151fb6c5fadd37a3a74579e133383a68c

class SerialPort(SerialPortBase):
    def __init__(self, portName: str, options: SerialPortOptions) -> None:
        
        super().__init__(portName, options)

        from async_pyserial import async_pyserial_core
        
        self.internal = async_pyserial_core.SerialPort(portName, self.internal_options)
            
        def on_data(data):
            self.emit(SerialPortEvent.ON_DATA, data)
        
        self.internal.set_data_callback(on_data)
    
    def read(self, bufsize: int, callback: Callable | None = None):
        if callback is None:
            pass
        else:
            pass
        
    def write(self, data: bytes, callback: Callable | None = None):
        if callback is None:
            # without callback logic
            self.internal.write(data)
        else:
            # with callback logic
            self.internal.write(data)
        
    def open(self):
        self.internal.open()
        
    def close(self):
        self.internal.close()