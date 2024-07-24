Async PySerial Project
======================

Overview
--------

This project provides an asynchronous serial communication library, combining the strengths of Python and C++ to achieve high-performance serial communication.

Environment Setup
-----------------

Creating a Virtual Environment
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

First, ensure you have Python and `virtualenv` installed.

.. code-block:: bash

   python -m venv .venv

Activating the Virtual Environment
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Activate the virtual environment to ensure you use the project-specific dependencies.

.. code-block:: bash

   # Linux and macOS
   source .venv/bin/activate

   # Windows
   .venv\Scripts\activate

Installing Dependencies
^^^^^^^^^^^^^^^^^^^^^^^

Install all necessary dependencies within the virtual environment.

.. code-block:: bash

   poetry install

Common Commands
---------------

Building and Installing the Extension Module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Use the following command to build and install the C++ extension module.

.. code-block:: bash

   python setup.py build_ext --inplace

Running Tests
^^^^^^^^^^^^^

Ensure all unit tests pass.

.. code-block:: bash

   pytest --cov=async_pyserial --cov-report=term-missing tests/

Generating Coverage Report
^^^^^^^^^^^^^^^^^^^^^^^^^^

Generate an HTML coverage report.

.. code-block:: bash

   pytest --cov=async_pyserial --cov-report=html tests/
   open htmlcov/index.html  # Open the HTML report in your browser

Running the Example
^^^^^^^^^^^^^^^^^^^

Use the following command to run the example script, replacing `<port>` with your actual serial port.

.. code-block:: bash

   python examples/serialport_terminal.py <port>

Development Guide
-----------------

Code Style
^^^^^^^^^^

Please follow these code style guidelines:

- Use PEP 8 for Python code style.
- For C++ code, follow the Google C++ Style Guide.

Commit Guidelines
^^^^^^^^^^^^^^^^^

When committing code, please ensure:

- All unit tests pass.
- Code is formatted according to the style guidelines.
- Commit messages are clear and concise.

Contribution Guide
^^^^^^^^^^^^^^^^^^

If you want to contribute code, please:

1. Fork this repository.
2. Create a new branch (`git checkout -b feature-branch`).
3. Commit your changes (`git commit -am 'Add some feature'`).
4. Push to the branch (`git push origin feature-branch`).
5. Create a Pull Request.

Contact
-------

If you have any questions or need help, please contact the project maintainer: Neil Lei (qwe17235@gmail.com)
