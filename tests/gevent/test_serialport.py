import pytest
import subprocess
import time
from async_pyserial import SerialPort, SerialPortOptions, set_async_worker
import os

import gevent
from gevent.event import AsyncResult

from tests.test_util import get_port_pair, mock_receieve_data

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

    set_async_worker('gevent')

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
    serial = SerialPort(port1, options)
    serial.open()
    test_data = b'Hello, world!'

    def run():
        serial.write(test_data)

        with open(port2, 'rb') as f:
            written_data = f.read(len(test_data))
        
        assert written_data == test_data

    t = gevent.spawn(run)

    t.join()
    
    serial.close()


def test_serialport_read(virtual_serial_ports):
    port1, port2 = virtual_serial_ports
    options = SerialPortOptions()
    options.read_bufsize = 512
    serial = SerialPort(port1, options)
    serial.open()

    test_data = b'Hello, world!'

    def run():

        with open(port2, 'wb') as f:
            f.write(test_data)

        buf = serial.read()

        assert buf == test_data

    t = gevent.spawn(run)

    t.join()

    serial.close()

def test_serialport_read(virtual_serial_ports):
    port1, port2 = virtual_serial_ports
    options = SerialPortOptions()
    options.read_bufsize = 512
    serial = SerialPort(port1, options)
    serial.open()

    test_data = b'Hello, world!'

    def run():

        with open(port2, 'wb') as f:
            f.write(test_data)

        buf = serial.read()

        assert buf == test_data

    t = gevent.spawn(run)

    mock = mock_receieve_data(serial)

    mock(test_data)

    t.join()

    serial.close()