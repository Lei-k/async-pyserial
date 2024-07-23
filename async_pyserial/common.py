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