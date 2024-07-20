#ifdef LINUX

#ifndef ASYNC_PYSERIAL_LINUX_SERIALPORT_H
#define ASYNC_PYSERIAL_LINUX_SERIALPORT_H

#include <iostream>
#include <string>
#include <thread>

#include <base/serialport.h>
#include <common/event.h>
#include <common/exception.h>

namespace async_pyserial
{
    namespace internal
    {
        enum SerialPortEvent : common::EventType
        {
            ON_DATA = 1
        };

        class SerialPort : public common::EventEmitter
        {
        public:
            SerialPort(const std::wstring &portName, const base::SerialPortOptions& options);
            ~SerialPort();

            void open();

            void close();
            
            void write(const std::string &data);

            bool is_open();

        private:
            std::wstring portName;

            base::SerialPortOptions options;

            bool _is_open;
        };
    }
}

#endif

#endif
