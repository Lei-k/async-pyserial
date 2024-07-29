from __future__ import annotations
__all__ = ['SerialPort', 'SerialPortOptions']
class SerialPort:
    def __init__(self, arg0: str, arg1: SerialPortOptions) -> None:
        ...
    def close(self) -> None:
        ...
    def open(self) -> None:
        ...
    def write(self, data: str, callback: function) -> None:
        ...
    def set_data_callback(self, callback: function) -> None:
        ...
class SerialPortOptions:
    baudrate: int
    bytesize: int
    parity: int
    stopbits: int
    read_timeout: int
    write_timeout: int
    def __init__(self) -> None:
        ...
