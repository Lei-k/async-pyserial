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
    def __init__(self) -> None:
        self.baudrate = 9600
        self.bytesize = 8
        self.stopbits = 1
        self.parity = 0
        self.write_timeout = 50
        self.read_timeout = 50

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