#ifdef Win32

#include <win32/serialport.h>

#endif

#ifdef LINUX

#include <linux/serialport.h>

#endif

#include <base/serialport.h>
#include <common/exception.h>
#include <common/util.h>
#include <any>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace async_pyserial
{
    namespace pybind
    {
        class SerialPort
        {
        public:
            SerialPort(const std::wstring &portName, const base::SerialPortOptions &options);
            ~SerialPort();
            void open();
            void close();
            void write(const std::string &data);
            // 只設定一個 data callback 以減少 python-c++ 交互調用
            void set_data_callback(const std::function<void(const pybind11::bytes &)> &callback);

        private:
            std::wstring portName;

            const base::SerialPortOptions &options;

            internal::SerialPort *serial;

            std::function<void(const pybind11::bytes &)> data_callback;

            void call(const std::vector<std::any> &args);
        };
    }

}

using namespace async_pyserial;
using namespace async_pyserial::pybind;

SerialPort::SerialPort(const std::wstring &portName, const base::SerialPortOptions &options) : portName(portName), options(options)
{
    serial = new internal::SerialPort(portName, options);

    // 預設註冊一個 ON_DATA listener
    serial->on(internal::SerialPortEvent::ON_DATA, [this](const std::vector<std::any> &args)
               { this->call(args); });
}

SerialPort::~SerialPort()
{
    if (serial != nullptr)
    {
        close();

        delete serial;

        serial = nullptr;
    }
}

void SerialPort::open()
{
    serial->open();
}

void SerialPort::close()
{
    serial->close();
}

void SerialPort::write(const std::string &data)
{
    serial->write(data);
}

void SerialPort::set_data_callback(const std::function<void(const pybind11::bytes &)> &callback)
{
    data_callback = callback;
}

namespace py = pybind11;

void SerialPort::call(const std::vector<std::any> &args)
{
    if (data_callback)
    {
        auto &data = std::any_cast<const std::vector<char> &>(args[0]);
        data_callback(py::bytes(data.data(), data.size()));
    }
}

PYBIND11_MODULE(async_pyserial_core, m)
{
    py::class_<base::SerialPortOptions>(m, "SerialPortOptions")
        .def(py::init<>())
        .def_readwrite("baudrate", &base::SerialPortOptions::baudrate)
        .def_readwrite("bytesize", &base::SerialPortOptions::bytesize)
        .def_readwrite("stopbits", &base::SerialPortOptions::stopbits)
        .def_readwrite("parity", &base::SerialPortOptions::parity)
        .def_readwrite("read_timeout", &base::SerialPortOptions::read_timeout)
        .def_readwrite("write_timeout", &base::SerialPortOptions::write_timeout);

    py::class_<pybind::SerialPort>(m, "SerialPort")
        .def(py::init<const std::wstring &, const base::SerialPortOptions &>())
        .def("open", &pybind::SerialPort::open)
        .def("close", &pybind::SerialPort::close)
        .def("write", &pybind::SerialPort::write)
        .def("set_data_callback", &pybind::SerialPort::set_data_callback);
}