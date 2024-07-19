#include <WinSerialPort.hpp>

#include <iostream>

using namespace win;

SerialPort::SerialPort(const std::wstring& portName)
    : common::EventEmitter(), portName(portName), hSerial(INVALID_HANDLE_VALUE), hCompletionPort(NULL), running(false) {}

SerialPort::~SerialPort() {
    stopAsyncRead();
    close();
}

enum class OperationType {
    Read,
    Write
};

struct CustomOverlapped : public OVERLAPPED {
    OperationType operationType;
};

bool SerialPort::open() {
    hSerial = CreateFileW(
        portName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED, // 使用重疊 I/O
        0
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening serial port" << std::endl;
        return false;
    }

    hCompletionPort = CreateIoCompletionPort(hSerial, NULL, 0, 0);
    if (hCompletionPort == NULL) {
        std::cerr << "Error creating IO completion port" << std::endl;
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return false;
    }

    return true;
}

void SerialPort::close() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
    }
    if (hCompletionPort != NULL) {
        CloseHandle(hCompletionPort);
        hCompletionPort = NULL;
    }
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

bool SerialPort::startAsyncRead() {
    if (running) return false;

    running = true;
    readThread = std::thread(&SerialPort::asyncReadThread, this);
    return true;
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
    CustomOverlapped overlapped = {};
    char buffer[BUFFER_SIZE];
    DWORD bytesRead;

    while (running) {
        ZeroMemory(&overlapped, sizeof(overlapped));
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        overlapped.operationType = OperationType::Read;

        if (!ReadFile(hSerial, buffer, BUFFER_SIZE, &bytesRead, &overlapped)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                std::cerr << "Error reading from serial port" << std::endl;
                break;
            }
        }

        DWORD numberOfBytesTransferred;
        ULONG_PTR completionKey;
        LPOVERLAPPED lpOverlapped;

        //query_iocp_status:
        if (GetQueuedCompletionStatus(hCompletionPort, &numberOfBytesTransferred, &completionKey, &lpOverlapped, INFINITE)) {
            auto* customOverlapped = reinterpret_cast<CustomOverlapped*>(lpOverlapped);

            if (customOverlapped->operationType == OperationType::Read && numberOfBytesTransferred > 0) {
                std::vector<char> buffer2send(buffer, buffer + numberOfBytesTransferred);

                std::vector<std::any> emitArgs = { buffer2send };

                emit(SerialPortEvent::ON_DATA, emitArgs);
            }
            // else if(customOverlapped->operationType == OperationType::Write && numberOfBytesTransferred > 0) {
            //     // 處理掉 write 事件
            //     CloseHandle(customOverlapped->hEvent);

            //     delete customOverlapped;
                
            //     //goto query_iocp_status;
            // }
        } else {
            std::cerr << "Error in IO completion" << std::endl;
            break;
        }

        CloseHandle(overlapped.hEvent);
    }
}

bool SerialPort::write(const std::string& data) {
    auto* overlapped = new CustomOverlapped();
    ZeroMemory(overlapped, sizeof(CustomOverlapped));
    overlapped->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    overlapped->operationType = OperationType::Write;

    DWORD bytesWritten = 0;
    if (!WriteFile(hSerial, data.c_str(), data.size(), &bytesWritten, overlapped)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            std::wcerr << L"Error writing to serial port" << std::endl;
            CloseHandle(overlapped->hEvent);
            delete overlapped;
            return false;
        }
        
        DWORD numberOfBytesTransferred;
        if (!GetOverlappedResult(hSerial, overlapped, &numberOfBytesTransferred, TRUE)) {
            std::wcerr << L"Error getting overlapped result" << std::endl;
            CloseHandle(overlapped->hEvent);
            delete overlapped;
            return false;
        }
        bytesWritten = numberOfBytesTransferred;
    }

    CloseHandle(overlapped->hEvent);
    delete overlapped;
    return bytesWritten == data.size();
}

int main() {
    SerialPort serial(L"COM2");

    if (!serial.open()) {
        return 1;
    }

    if (!serial.configure(CBR_9600, 8, ONESTOPBIT, NOPARITY)) {
        return 1;
    }

    if (!serial.setTimeouts(50, 50, 50)) {
        return 1;
    }

    serial.on(SerialPortEvent::ON_DATA, [](const std::vector<std::any>& args) {
        auto& data = std::any_cast<const std::vector<char>&>(args[0]);
        std::string str(data.begin(), data.end());
        std::cout << str << std::endl;
    });

    if (!serial.startAsyncRead()) {
        return 1;
    }

    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "exit") break;

        if (!serial.write(input)) {
            std::cerr << "Failed to write data to serial port" << std::endl;
        }
    }

    serial.stopAsyncRead();
    return 0;
}