#include <WinSerialPort.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace pybind {
    struct SerialPortOptions {
        unsigned long baudrate;
        unsigned char bytesize;
        unsigned char stopbits;
        unsigned char parity;
        unsigned long read_timeout = 50;
        unsigned long write_timeout = 50;
    };

    class WinSerialPort {
        public:
            WinSerialPort(const std::wstring& portName, const SerialPortOptions& options);
            ~WinSerialPort();
            void open();
            void close();
            void write(const std::string& data);
            // 只設定一個 data callback 以減少 python-c++ 交互調用
            void set_data_callback(const std::function<void(const pybind11::bytes&)>& callback);

        private:
            std::wstring portName;

            const SerialPortOptions &options;

            win::SerialPort* serial;

            std::function<void(const pybind11::bytes&)> data_callback;

            void call(const std::vector<std::any>& args);
    };
}

using namespace pybind;

WinSerialPort::WinSerialPort(const std::wstring& portName, const SerialPortOptions& options): options(options) {
    serial = new win::SerialPort(portName);

    // 預設註冊一個 ON_DATA listener
    serial->on(win::SerialPortEvent::ON_DATA, [this](const std::vector<std::any>& args){
        this->call(args);
    });
}

WinSerialPort::~WinSerialPort() {
    if(serial != nullptr) {
        close();

        delete serial;

        serial = nullptr;
    }
}

void WinSerialPort::open() {
    serial->open();
    serial->configure(options.baudrate, options.bytesize, options.parity, options.stopbits);
    serial->setTimeouts(50, options.read_timeout, options.write_timeout);
    serial->startAsyncRead();
}

void WinSerialPort::close() {
    serial->stopAsyncRead();
    serial->close();
}

void WinSerialPort::write(const std::string& data) {
    serial->write(data);
}

void WinSerialPort::set_data_callback(const std::function<void(const pybind11::bytes&)>& callback) {
    data_callback = callback;
}

void WinSerialPort::call(const std::vector<std::any>& args) {
    if(data_callback) {
        auto& data = std::any_cast<const std::vector<char>&>(args[0]);
        data_callback(pybind11::bytes(data.data(), data.size()));
    }
}

namespace py = pybind11;

PYBIND11_MODULE(async_pyserial_core, m) {
    py::class_<pybind::SerialPortOptions>(m, "SerialPortOptions")
        .def(py::init<>())
        .def_readwrite("baudrate", &pybind::SerialPortOptions::baudrate)
        .def_readwrite("bytesize", &pybind::SerialPortOptions::bytesize)
        .def_readwrite("stopbits", &pybind::SerialPortOptions::stopbits)
        .def_readwrite("parity", &pybind::SerialPortOptions::parity)
        .def_readwrite("read_timeout", &pybind::SerialPortOptions::read_timeout)
        .def_readwrite("write_timeout", &pybind::SerialPortOptions::write_timeout);

    py::class_<pybind::WinSerialPort>(m, "WinSerialPort")
        .def(py::init<const std::wstring&, const pybind::SerialPortOptions&>())
        .def("open", &pybind::WinSerialPort::open)
        .def("close", &pybind::WinSerialPort::close)
        .def("write", &pybind::WinSerialPort::write)
        .def("set_data_callback", &pybind::WinSerialPort::set_data_callback);
}