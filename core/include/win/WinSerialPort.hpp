#ifndef WIN_SERIAL_PORT_HPP
#define WIN_SERIAL_PORT_HPP

#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <Event.hpp>

namespace win {
    enum SerialPortEvent: common::EventType {
        ON_DATA = 1
    };

    class SerialPort: public common::EventEmitter {
        public:
            SerialPort(const std::wstring& portName);
            ~SerialPort();
            bool open();
            void close();
            bool configure(DWORD baudRate, BYTE byteSize, BYTE stopBits, BYTE parity);
            bool setTimeouts(DWORD readInterval, DWORD readTotal, DWORD writeTotal);
            bool write(const std::string& data);
            bool startAsyncRead();
            void stopAsyncRead();

        private:
            void asyncReadThread();
            std::wstring portName;
            HANDLE hSerial;
            HANDLE hCompletionPort;
            std::thread readThread;
            bool running;

            static const int BUFFER_SIZE = 1024;
    };
}

#endif

