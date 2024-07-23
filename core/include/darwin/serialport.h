#ifdef __darwin__

#ifndef ASYNC_PYSERIAL_DARWIN_SERIALPORT_H
#define ASYNC_PYSERIAL_DARWIN_SERIALPORT_H

#include <string>
#include <vector>
#include <any>
#include <thread>
#include <atomic>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <common/event.h>

#include <base/serialport.h>

namespace async_pyserial
{
    namespace internal
    {
        #define MAX_KEVENTS 8

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

            struct kevent serial_evt;
            struct kevent notify_evt;
            int notify_fd;

            std::thread readThread;

            int serial_fd;
            int kqueue_fd;

            bool _is_open;
            bool running;
        };
    }
}

#endif

#endif
