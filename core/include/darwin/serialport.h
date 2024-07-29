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
#include <functional>
#include <deque>
#include <common/event.h>
#include <common/common.h>
#include <mutex>

#include <common/util.h>
#include <common/exception.h>
#include <iostream>

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

        struct IOEvent {
            std::string data;
            size_t bytes_written;
            std::function<void(unsigned long)> callback;
        };

        class SerialPort : public common::EventEmitter
        {
        public:
            SerialPort(const std::wstring &portName, const base::SerialPortOptions& options);
            ~SerialPort();

            void open();

            void close();
            
            void write(const std::string &data, const std::function<void(unsigned long)>& callback);

            bool is_open();

        private:
            void configure(unsigned long baudRate, unsigned char byteSize, unsigned char stopBits, unsigned char parity);

            void startKqueueWorker();
            void stopKqueueWorker();

            void kqueueWorker();

            std::wstring portName;

            base::SerialPortOptions options;

            struct kevent serial_evt;
            int notify_fd;

            std::thread readThread;

            int serial_fd;
            int kqueue_fd;

            bool _is_open;
            bool running;

            std::deque<IOEvent> w_queue;
            std::mutex w_mutex;

        };
    }
}

#endif

#endif
