ASYNC PYSERIAL
===============

Python bindings for a C++ serial port library providing asynchronous serial communication support.

Features
--------
- Non-blocking read operations.
- Blocking write operations.
- Cross-platform support for Windows, Linux, macOS, and FreeBSD.
- Event-driven architecture for handling data events.

Installation
------------
You can install the `async-pyserial` package using `poetry`:

1. Install `poetry` if you haven't already:

    .. code-block:: shell

        curl -sSL https://install.python-poetry.org | python3 -

2. Add `async-pyserial` to your project:

    .. code-block:: shell

        poetry add async-pyserial

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

        poetry build

5. Install the package:

    .. code-block:: shell

        pip install dist/*.whl

Usage
-----
Here's a simple example of how to use `async-pyserial`:

.. code-block:: python

    from async_pyserial import SerialPort, SerialPortOptions, SerialPortEvent

    def on_data(data):
        print(f"Received: {data}")

    options = SerialPortOptions()
    options.baudrate = 9600
    options.bytesize = 8
    options.stopbits = 1
    options.parity = 0
    options.read_timeout = 50
    options.write_timeout = 50

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
- `def write(self, data: bytes)`: Writes `data` to the serial port (blocking operation).
- `def open(self)`: Opens the serial port.
- `def close(self)`: Closes the serial port.
- `def on(self, event: SerialPortEvent, callback: Callable[[bytes], None])`: Registers a callback for the specified event.

### SerialPortOptions
A class for specifying serial port options.

#### Attributes

- `baudrate: int`: The baud rate for the serial port.
- `bytesize: int`: The number of data bits.
- `stopbits: int`: The number of stop bits.
- `parity: int`: The parity checking (0: None, 1: Odd, 2: Even).
- `read_timeout: int`: The read timeout in milliseconds.
- `write_timeout: int`: The write timeout in milliseconds.

### SerialPortEvent
An enumeration for serial port events.

- `ON_DATA`: Event triggered when data is received.

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
