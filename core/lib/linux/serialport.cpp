#ifdef LINUX

#include <linux/serialport.h>

#include <sstream>

#include <iostream>

using namespace async_pyserial;
using namespace async_pyserial::internal;

SerialPort::SerialPort(const std::wstring& portName, const base::SerialPortOptions& options)
    : common::EventEmitter(), portName(portName), options(options), _is_open(false) {}

SerialPort::~SerialPort() {
    close();
}


void SerialPort::open() {
    _is_open = true;
}

bool SerialPort::is_open() {
    return true;
}

void SerialPort::close() {
    if(!_is_open) return;

    _is_open = false;
}


void SerialPort::write(const std::string& data) {
    
}

#endif