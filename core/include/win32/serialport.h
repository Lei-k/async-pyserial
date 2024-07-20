#ifdef Win32

#ifndef ASYNC_PYSERIAL_WIN32_SERIALPORT_H
#define ASYNC_PYSERIAL_WIN32_SERIALPORT_H

#include <windows.h>

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
            void asyncReadThread();

            bool configure(DWORD baudRate, BYTE byteSize, BYTE stopBits, BYTE parity);
            bool setTimeouts(DWORD readInterval, DWORD readTotal, DWORD writeTotal);

            void startAsyncRead();
            void stopAsyncRead();

            std::wstring portName;
            HANDLE hSerial;
            HANDLE hCompletionPort;
            std::thread readThread;
            bool running;

            base::SerialPortOptions options;

            bool _is_open;

            static const int BUFFER_SIZE = 1024;
        };
    }
}

#endif

#endif
