#ifdef Win32

#include <win32/serialport.h>

#include <sstream>

#include <iostream>

using namespace async_pyserial;
using namespace async_pyserial::internal;

void WriteCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

SerialPort::SerialPort(const std::wstring& portName, const base::SerialPortOptions& options)
    : common::EventEmitter(), portName(portName), hSerial(INVALID_HANDLE_VALUE), options(options), hCompletionPort(NULL), _is_open(false), running(false) {}

SerialPort::~SerialPort() {
    close();
}

enum class OperationType {
    Read,
    Write
};

struct CustomOverlapped : public OVERLAPPED {
    OperationType operationType;
    std::function<void(unsigned long)> callback;
};

void SerialPort::open() {
    hSerial = CreateFileW(
        portName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED, // 使用重疊 I/O
        0
    );

    std::ostringstream exMessage;

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening serial port" << std::endl;

        exMessage << "Error opening serial port " << common::wstring_to_string(portName);

        throw common::SerialPortException(exMessage.str());
    }

    hCompletionPort = CreateIoCompletionPort(hSerial, NULL, 0, 0);
    if (hCompletionPort == NULL) {
        exMessage << "Error creating IO completion port";

        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;

        throw common::SerialPortException(exMessage.str());
    }

    bool success = configure(options.baudrate, options.bytesize, options.parity, options.stopbits) && 
        setTimeouts(50, options.read_timeout, options.write_timeout);

    if (!success)
    {
        exMessage << "Error configure" << common::wstring_to_string(portName);

        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;

        throw common::SerialPortException(exMessage.str());
    }

    startAsyncRead();

    _is_open = true;
}

bool SerialPort::is_open() {
    return _is_open;
}

void SerialPort::close() {
    if(!_is_open) return;

    stopAsyncRead();

    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
    }

    if (hCompletionPort != NULL) {
        CloseHandle(hCompletionPort);
        hCompletionPort = NULL;
    }

    _is_open = false;
}

bool SerialPort::configure(DWORD baudRate, BYTE byteSize, BYTE stopBits, BYTE parity) {
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting serial port state" << std::endl;
        return false;
    }

    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = byteSize;
    dcbSerialParams.StopBits = stopBits;
    dcbSerialParams.Parity = parity;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting serial port state" << std::endl;
        return false;
    }
    return true;
}

bool SerialPort::setTimeouts(DWORD readInterval, DWORD readTotal, DWORD writeTotal) {
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = readInterval;
    timeouts.ReadTotalTimeoutConstant = readTotal;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = writeTotal;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Error setting serial port timeouts" << std::endl;
        return false;
    }
    return true;
}

void SerialPort::startAsyncRead() {
    if (running) return;

    running = true;
    readThread = std::thread(&SerialPort::asyncReadThread, this);
}

void SerialPort::stopAsyncRead() {
    if (running) {
        running = false;
        if (readThread.joinable()) {
            PostQueuedCompletionStatus(hCompletionPort, 0, 0, NULL);
            readThread.join();
        }
    }
}

void SerialPort::asyncReadThread() {
    char buffer[BUFFER_SIZE];
    DWORD bytesRead;

    while (running) {
        CustomOverlapped overlapped = {};
        ZeroMemory(&overlapped, sizeof(overlapped));
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        overlapped.operationType = OperationType::Read;

        if (!ReadFile(hSerial, buffer, BUFFER_SIZE, &bytesRead, &overlapped)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                std::cerr << "Error reading from serial port" << std::endl;
                break;
            }
        }

        query_iocp_status:

        DWORD numberOfBytesTransferred;
        ULONG_PTR completionKey;
        LPOVERLAPPED lpOverlapped;

        if (GetQueuedCompletionStatus(hCompletionPort, &numberOfBytesTransferred, &completionKey, &lpOverlapped, INFINITE)) {

            auto* customOverlapped = reinterpret_cast<CustomOverlapped*>(lpOverlapped);

            if (customOverlapped->operationType == OperationType::Read && numberOfBytesTransferred > 0) {
                std::string buffer2send(buffer, buffer + numberOfBytesTransferred);

                std::vector<std::any> emitArgs = { buffer2send };

                emit(SerialPortEvent::ON_DATA, emitArgs);
            }
            else if(customOverlapped->operationType == OperationType::Write) {
                // 處理掉 write 事件

                // std::cout << "check " << GetLastError() << std::endl;

                // if(GetLastError() == ERROR_IO_PENDING) {
                //     goto query_iocp_status;
                // }

                

                if(numberOfBytesTransferred > 0) {
                    WriteCompletionRoutine(0, numberOfBytesTransferred, lpOverlapped);
                } else {
                    WriteCompletionRoutine(1, numberOfBytesTransferred, lpOverlapped);
                }
                
                goto query_iocp_status;
            }
        } else {
            std::cerr << "Error in IO completion" << std::endl;
            break;
        }

        CloseHandle(overlapped.hEvent);
    }
}

void WriteCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {
    auto* customOverlapped = reinterpret_cast<CustomOverlapped*>(lpOverlapped);

    try {
        if(customOverlapped->callback != nullptr) {
            auto c = std::move(customOverlapped->callback);

            CloseHandle(customOverlapped->hEvent);



            // 清理 OVERLAPPED 结构的内存
            delete customOverlapped;

            c(dwErrorCode);
        } else {
            // 清理 OVERLAPPED 结构的内存
            CloseHandle(customOverlapped->hEvent);
            delete customOverlapped;
        }
    } catch(std::exception ex) {
        std::cerr << ex.what() << std::endl;
    }

    
}

void SerialPort::write(const std::string& data, const std::function<void(unsigned long)>& callback) {
    auto* overlapped = new CustomOverlapped();
    ZeroMemory(overlapped, sizeof(CustomOverlapped));
    overlapped->operationType = OperationType::Write;
    overlapped->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    if(callback) {
        overlapped->callback = callback;
    }

    DWORD bytesWritten = 0;

    if (!WriteFile(hSerial, data.c_str(), data.size(), &bytesWritten, overlapped)) {
        auto lasterr= GetLastError();
        if(lasterr != ERROR_IO_PENDING) {
            std::cerr << "Failed to write file, error: " << GetLastError() << std::endl;
            CloseHandle(overlapped->hEvent);
            delete overlapped;

            if(callback) {
                callback(lasterr);
            }
        }
    }

}

int main() {
    base::SerialPortOptions options;
    options.baudrate = CBR_9600;
    options.bytesize = 8;
    options.stopbits = ONESTOPBIT;
    options.parity = NOPARITY;
    options.read_timeout = 50;
    options.write_timeout = 50;

    SerialPort serial(L"COM2", options);

    serial.on(SerialPortEvent::ON_DATA, [](const std::vector<std::any>& args) {
        auto& data = std::any_cast<const std::vector<char>&>(args[0]);
        std::string str(data.begin(), data.end());
        std::cout << str << std::endl;
    });

    serial.open();

    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "exit") break;

        auto callback = [](int err) {

        };

        serial.write(input, callback);
    }

    return 0;
}

#endif