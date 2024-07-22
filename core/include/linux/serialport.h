#ifdef LINUX

#ifndef ASYNC_PYSERIAL_LINUX_SERIALPORT_H
#define ASYNC_PYSERIAL_LINUX_SERIALPORT_H

#include <iostream>
#include <string>
#include <thread>

#include <base/serialport.h>
#include <common/event.h>
#include <common/exception.h>

#include <sys/epoll.h>

namespace async_pyserial
{
    namespace internal
    {
        #define EPOLL_MAX_EVENTS 8

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
            void configure(unsigned long baudRate, unsigned char byteSize, unsigned char stopBits, unsigned char parity);

            void startAsyncRead();
            void stopAsyncRead();

            void asyncReadThread();

            std::wstring portName;

            base::SerialPortOptions options;

            struct epoll_event event;
            struct epoll_event notify_evt;
            int notify_fd;

            std::thread readThread;

            int serial_fd;
            int epoll_fd;

            bool _is_open;
            bool running;
        };
    }
}

#endif

#endif
