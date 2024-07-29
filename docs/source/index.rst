.. async-pyserial documentation master file, created by
   sphinx-quickstart on Wed Jul 24 01:50:11 2024.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Async PySerial
==============

Welcome to the Async PySerial documentation!
--------------------------------------------

Python bindings for a C++ serial port library providing asynchronous serial communication support.

Features
--------
- Non-blocking and blocking read/write operations.
- Cross-platform support for Windows, Linux, macOS, and FreeBSD.
- Event-driven architecture for handling data events.
- Support for `gevent`, `eventlet`, `asyncio`, `callback`, and synchronous operations.
- Uses `epoll`, `iocp`, and `kqueue` for efficient, scalable I/O operations based on the underlying operating system.

Links
-----
- `Documentation <https://lei-k.github.io/async-pyserial/>`_
- `PyPI page <https://pypi.org/project/async-pyserial/>`_

Installation
------------
You can install the `async-pyserial` package using either `poetry` or `pip`.

### Using Poetry

1. Install `poetry` if you haven't already:

    .. code-block:: shell

        curl -sSL https://install.python-poetry.org | python3 -

2. Add `async-pyserial` to your project:

    .. code-block:: shell

        poetry add async-pyserial

### Using pip

1. Install the package from PyPI:

    .. code-block:: shell

        pip install async-pyserial

### FreeBSD Installation

For FreeBSD, you need to build the package manually:

1. Clone the repository:

    .. code-block:: shell

        git clone https://github.com/Lei-k/async-pyserial.git

2. Navigate to the project directory:

    .. code-block:: shell

        cd async-pyserial

3. Install the dependencies using `poetry`:

    .. code-block:: shell

        poetry install

4. Build the package:

    .. code-block:: shell

        python -m build

5. Install the package:

    .. code-block:: shell

        pip install dist/*.whl

Usage
-----
Here's a simple example of how to use `async-pyserial`:

.. code-block:: python

    from async_pyserial import SerialPort, SerialPortOptions, SerialPortEvent, SerialPortParity

    def on_data(data):
        print(f"Received: {data}")

    options = SerialPortOptions()
    options.baudrate = 9600
    options.bytesize = 8
    options.stopbits = 1
    options.parity = SerialPortParity.NONE # NONE, ODD, EVEN

    serial_port = SerialPort('/dev/ttyUSB0', options)
    serial_port.on(SerialPortEvent.ON_DATA, on_data)
    serial_port.open()

    try:
        while True:
            data_to_send = input("Enter data to send (or 'exit' to quit): ")
            if data_to_send.lower() == 'exit':
                break
            serial_port.write(data_to_send.encode('utf-8'))
    finally:
        serial_port.close()

API
---
### SerialPort
A class for serial communication.

#### Methods

- `__init__(self, port: str, options: SerialPortOptions)`: Initializes the serial port with the specified parameters.
- `def write(self, data: bytes, callback: Callable | None = None)`: Writes `data` to the serial port. Can be blocking or non-blocking. If a callback is provided, the write will be asynchronous. Supports `gevent`, `eventlet`, `asyncio`, `callback`, and synchronous operations.
- `def read(self, bufsize: int = 512, callback: Callable | None = None)`: Reads data from the serial port. Can be blocking or non-blocking. If a callback is provided, the read will be asynchronous. Supports `gevent`, `eventlet`, `asyncio`, `callback`, and synchronous operations.
- `def open(self)`: Opens the serial port.
- `def close(self)`: Closes the serial port.
- `def on(self, event: SerialPortEvent, callback: Callable[[bytes], None])`: Registers a callback for the specified event.
- `def emit(self, evt: str, *args, **kwargs)`: Emits an event, triggering all registered callbacks for that event.
- `def remove_all_listeners(self, evt: str)`: Removes all listeners for the specified event.
- `def remove_listener(self, evt: str, listener: Callable)`: Removes a specific listener for the specified event.
- `def off(self, evt: str, listener: Callable)`: Alias for `remove_listener`.

### SerialPortOptions
A class for specifying serial port options.

#### Attributes

- `baudrate: int`: The baud rate for the serial port.
- `bytesize: int`: The number of data bits.
- `stopbits: int`: The number of stop bits.
- `parity: int`: The parity checking (0: None, 1: Odd, 2: Even).
- `read_timeout: int`: The read timeout in milliseconds.
- `write_timeout: int`: The write timeout in milliseconds.
- `read_bufsize: int`: The read buffer size. Default is 0. When `read_bufsize` is 0, the internal buffer is not used, and only data received after the read call will be returned. If `read_bufsize` is not 0, both buffered and new data will be returned.

### SerialPortEvent
An enumeration for serial port events.

- `ON_DATA`: Event triggered when data is received.

### SerialPortError
An exception class for handling serial port errors.

- `__init__(self, *args: object)`: Initializes the SerialPortError with the specified arguments.

### PlatformNotSupported
An exception class for handling unsupported platforms.

- `__init__(self, *args: object)`: Initializes the PlatformNotSupported exception with the specified arguments.

### set_async_worker
A function for setting the asynchronous worker.

- `def set_async_worker(w: str, loop = None)`: Sets the asynchronous worker to `gevent`, `eventlet`, or `asyncio`. Optionally, an event loop can be provided for `asyncio`.

Examples
--------

The `examples` directory contains sample scripts demonstrating how to use `async-pyserial` for various operations. Below are a few examples to help you get started.

- Basic read and write operations.
- Non-blocking read with asyncio.
- Using gevent and eventlet for asynchronous operations.

Example scripts included in the `examples` directory:

- `interval_write.py`: Demonstrates periodic writing to the serial port.
- `serialport_read.py`: Demonstrates reading from the serial port with different async workers.
- `serialport_terminal.py`: A terminal interface for interacting with the serial port.

Platform Support
----------------
Supports Windows, Linux, macOS, and FreeBSD.

Development
-----------
To contribute to the project, follow these steps:

1. Clone the repository:

    .. code-block:: shell

        git clone https://github.com/Lei-k/async-pyserial.git

2. Navigate to the project directory:

    .. code-block:: shell

        cd async-pyserial

3. Install the dependencies using `poetry`:

    .. code-block:: shell

        poetry install

4. Run the tests:

    .. code-block:: shell

        poetry run pytest

License
-------
This project is licensed under the MIT License. See the `LICENSE` file for more details.

Contact
-------

If you have any questions or need help, please contact the project maintainer: Neil Lei (qwe17235@gmail.com)

Indices and tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
