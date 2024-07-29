
from async_pyserial.async_pyserial_core import SerialPort
from async_pyserial.common import SerialPortEvent


port_counter = 0

def get_port_pair():
    global port_counter

    first = port_counter
    second = port_counter + 1

    port_counter += 2
    
    return f'/tmp/ttyV{first}', f'/tmp/ttyV{second}'

def mock_receieve_data(serial: SerialPort):
    def on_receieved(data):
        serial._SerialPort__read_buf = data
            
        serial.emit(SerialPortEvent.ON_DATA, data)

    return on_receieved