from typing import Callable

class EventEmitter:
    def __init__(self) -> None:
        self.listeners: dict[str, list[Callable]] = {}
        
    def emit(self, evt: str, *args, **kwargs):
        if evt not in self.listeners:
            return
        
        listeners = self.listeners[evt]
        
        for listener in listeners:
            listener(*args, **kwargs)
        
    def on(self, evt: str, listener: Callable | None = None):

        def decorator(listener: Callable):
            if evt not in self.listeners:
                self.listeners[evt] = []
            
            self.listeners[evt].append(listener)

        if listener is None:
            return decorator
        
        decorator(listener=listener)

    def remove_all_listeners(self, evt: str):
        if evt not in self.listeners:
            return
        
        del self.listeners[evt]

    def remove_listener(self, evt: str, listener: Callable):
        if evt not in self.listeners:
            return
        
        self.listeners[evt].remove(listener)

        if len(self.listeners[evt]) == 0:
            del self.listeners[evt]
    
    def off(self, evt: str, listener: Callable):
        """ alias for remove_listener
        """
        self.remove_listener(evt, listener=listener)
        
class PlatformNotSupported(Exception):
    def __init__(self, *args: object) -> None:
        super().__init__(*args)
        
class SerialPortOptions:
    """
    SerialPortOptions class defines the configuration options for a serial port connection.

    Attributes:
        `baudrate` (int): The baud rate for the serial communication. Default is 9600.
        `bytesize` (int): The number of data bits in each character. Default is 8.
        `stopbits` (int): The number of stop bits. Default is 1.
        `parity` (SerialPortParity): The parity check setting. Default is SerialPortParity.NONE.
                                   Options are SerialPortParity.NONE (0), SerialPortParity.ODD (1), 
                                   SerialPortParity.EVEN (2).
        `write_timeout` (int): The write timeout in milliseconds. Default is 50.
        `read_timeout` (int): The read timeout in milliseconds. Default is 50.
        `read_bufsize` (int): The read buffer size. Default is 0. When read_bufsize is 0, the internal buffer 
                            is not used, and the user will only get the data received after the read call. 
                            If read_bufsize is not 0, the user will get the data present in the internal buffer
                            as well as any new data received after the read call.
    """
    def __init__(self) -> None:
        self.baudrate = 9600
        self.bytesize = 8
        self.stopbits = 1
        self.parity = SerialPortParity.NONE # NONE: 0, ODD: 1, EVEN: 2
        self.write_timeout = 50
        self.read_timeout = 50
        self.read_bufsize = 0

class SerialPortEvent:
    ON_DATA = 'data'
    
class SerialPortParity:
    NONE = 0
    ODD = 1
    EVEN = 2
        
class SerialPortBase(EventEmitter):
    def __init__(self, portName: str, options: SerialPortOptions) -> None:
        
        super().__init__()

        from async_pyserial import async_pyserial_core
        
        self.portName = portName
        
        self.options = options
        self.internal_options = async_pyserial_core.SerialPortOptions()
        
        self.internal_options.baudrate = options.baudrate
        self.internal_options.bytesize = options.bytesize
        self.internal_options.stopbits = options.stopbits
        self.internal_options.parity = options.parity
        self.internal_options.write_timeout = options.write_timeout
        self.internal_options.read_timeout = options.read_timeout