import pytest
import subprocess
import time
from async_pyserial import SerialPort, SerialPortOptions, set_async_worker
import os

from async_pyserial.common import SerialPortEvent
from tests.test_util import get_port_pair, mock_receieve_data
import threading

from concurrent.futures import Future

# Fixture to set up and tear down a pair of virtual serial ports using socat
@pytest.fixture(scope="module")
def virtual_serial_ports():
    port1, port2 = get_port_pair()

    # Create virtual serial ports using socat
    socat_process = subprocess.Popen([
        'socat', '-d', '-d', f'PTY,link={port1},raw,echo=0', f'PTY,link={port2},raw,echo=0'
    ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # Give socat some time to set up the ports
    time.sleep(2)

    set_async_worker('none')

    yield port1, port2

    # Terminate the socat process and clean up
    socat_process.terminate()
    socat_process.wait()

    if os.path.exists(port1):
        os.remove(port1)
    if os.path.exists(port2):
        os.remove(port2)

# Test case for initializing SerialPort
def test_serialport_init(virtual_serial_ports):
    port1, _ = virtual_serial_ports
    options = SerialPortOptions()
    serial = SerialPort(port1, options)
    assert serial is not None

# Test case for opening and closing SerialPort
def test_serialport_open_close(virtual_serial_ports):
    port1, _ = virtual_serial_ports
    options = SerialPortOptions()
    serial = SerialPort(port1, options)

    serial.open()

    assert serial.is_open() == True

    serial.close()

    assert serial.is_open() == False

# Test case for writing to SerialPort
def test_serialport_write(virtual_serial_ports):
    port1, port2 = virtual_serial_ports
    options = SerialPortOptions()
    serial_port = SerialPort(port1, options)
    serial_port.open()
    test_data = b'Hello, world!'
    
    future = Future()

    def on_complete(err):
        if err is not None:
            future.set_exception(err)
            return
        
        future.set_result(None)
    
    serial_port.write(test_data, on_complete)
    
    # wait for write
    future.result()

    with open(port2, 'rb') as f:
        written_data = f.read(len(test_data))
    
    assert written_data == test_data
    
    serial_port.close()


def test_serialport_read(virtual_serial_ports):
    port1, port2 = virtual_serial_ports
    options = SerialPortOptions()
    options.read_bufsize = 512
    serial = SerialPort(port1, options)
    serial.open()

    test_data = b'Hello, world!'

    with open(port2, 'wb') as f:
        f.write(test_data)

    future = Future()

    def on_receieved(data):
        future.set_result(data)
        

    serial.read(callback=on_receieved)

    buf = future.result(timeout=2)

    assert buf == test_data

    serial.close()

def test_serialport_read_with_delay_write(virtual_serial_ports):
    port1, port2 = virtual_serial_ports
    options = SerialPortOptions()
    options.read_bufsize = 512
    serial = SerialPort(port1, options)
    serial.open()

    test_data = b'Hello, world!'

    def write_data():
        time.sleep(0.5)

        with open(port2, 'wb') as f:
            f.write(test_data)

    t = threading.Thread(target=write_data)

    t.daemon = True

    t.start()

    future = Future()

    def on_receieved(data):
        future.set_result(data)
        

    serial.read(callback=on_receieved)

    buf = future.result(timeout=2)

    assert buf == test_data

    serial.close()

def test_serialport_read_with_mock_receieve(virtual_serial_ports):
    port1, port2 = virtual_serial_ports
    options = SerialPortOptions()
    options.read_bufsize = 512
    serial = SerialPort(port1, options)
    serial.open()

    test_data = b'Hello, world!'

    future = Future()

    def on_receieved(data):
        future.set_result(data)

    serial.read(callback=on_receieved)

    mock = mock_receieve_data(serial)

    mock(test_data)

    buf = future.result(timeout=2)

    assert buf == test_data

    serial.close()

def test_serialport_read_with_mock_receieve2(virtual_serial_ports):
    port1, port2 = virtual_serial_ports
    options = SerialPortOptions()
    options.read_bufsize = 512
    serial = SerialPort(port1, options)
    serial.open()

    test_data = b'Hello, world!'

    future = Future()

    def on_receieved(data):
        future.set_result(data)

    mock = mock_receieve_data(serial)

    mock(test_data)

    serial.read(callback=on_receieved)

    buf = future.result(timeout=2)

    assert buf == test_data

    serial.close()

def test_serialport_read_with_mock_receieve3(virtual_serial_ports):
    port1, port2 = virtual_serial_ports
    options = SerialPortOptions()
    serial = SerialPort(port1, options)
    serial.open()

    test_data = b'Hello, world!'

    future = Future()

    def on_receieved(data):
        future.set_result(data)

    serial.read(callback=on_receieved)

    mock = mock_receieve_data(serial)

    mock(test_data)

    buf = future.result(timeout=2)

    assert buf == test_data

    serial.close()