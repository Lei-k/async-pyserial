import pytest
import subprocess
import time
from async_pyserial import SerialPort, SerialPortOptions, SerialPortEvent
import os
import threading

# Fixture to set up and tear down a pair of virtual serial ports using socat
@pytest.fixture(scope="module")
def virtual_serial_ports():
    port1 = "/tmp/ttyV0"
    port2 = "/tmp/ttyV1"

    # Create virtual serial ports using socat
    socat_process = subprocess.Popen([
        'socat', '-d', '-d', f'PTY,link={port1},raw,echo=0', f'PTY,link={port2},raw,echo=0'
    ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # Give socat some time to set up the ports
    time.sleep(2)

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
    port1, port2 = virtual_serial_ports
    options = SerialPortOptions()
    serial_port = SerialPort(port1, options)
    assert serial_port is not None

# Test case for opening and closing SerialPort
def test_serialport_open_close(virtual_serial_ports):
    port1, port2 = virtual_serial_ports
    options = SerialPortOptions()
    serial_port = SerialPort(port1, options)
    serial_port.open()
    serial_port.close()

# Test case for writing to SerialPort
def test_serialport_write(virtual_serial_ports):
    port1, port2 = virtual_serial_ports
    options = SerialPortOptions()
    serial_port = SerialPort(port1, options)
    serial_port.open()
    test_data = b'Hello, world!'
    serial_port.write(test_data)

    with open(port2, 'rb') as f:
        written_data = f.read(len(test_data))
    
    assert written_data == test_data
    
    serial_port.close()


def test_serialport_on_data_event(virtual_serial_ports):
    port1, port2 = virtual_serial_ports
    options = SerialPortOptions()
    serial_port = SerialPort(port1, options)
    serial_port.open()

    test_data = b'Hello, world!'
    event = threading.Event()

    def on_data(data):
        assert data == test_data
        event.set()

    serial_port.on(SerialPortEvent.ON_DATA, on_data)

    with open(port2, 'wb') as f:
        f.write(test_data)

    event_triggered = event.wait(timeout=2)
    assert event_triggered
    serial_port.close()
