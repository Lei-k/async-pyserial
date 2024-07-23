#ifdef __darwin__

#include <darwin/serialport.h>
#include <common/util.h>
#include <common/exception.h>
#include <iostream>

#define BUFFER_SIZE 256

using namespace async_pyserial;
using namespace async_pyserial::internal;

SerialPort::SerialPort(const std::wstring& portName, const base::SerialPortOptions& options)
    : common::EventEmitter(), portName(portName), options(options), _is_open(false),serial_fd(-1), notify_fd(-1), running(false) {}

SerialPort::~SerialPort() {
    close();
}

speed_t convert_baud_rate(unsigned long baudRate) {
    static std::map<unsigned long, speed_t> baud_map = {
        {0, B0},
        {50, B50},
        {75, B75},
        {110, B110},
        {134, B134},
        {150, B150},
        {200, B200},
        {300, B300},
        {600, B600},
        {1200, B1200},
        {1800, B1800},
        {2400, B2400},
        {4800, B4800},
        {9600, B9600},
        {19200, B19200},
        {38400, B38400},
        {57600, B57600},
        {115200, B115200},
        {230400, B230400}
    };

    auto it = baud_map.find(baudRate);
    if (it != baud_map.end()) {
        return it->second;
    } else {
        throw common::ConvertException("Error convert baudrate");
    }
}

tcflag_t convert_byte_size(unsigned char byteSize) {
    switch (byteSize) {
        case 5: return CS5;
        case 6: return CS6;
        case 7: return CS7;
        case 8: return CS8;
        default:
            throw common::ConvertException("Error convert bytesize");
    }
}

void SerialPort::open() {
    serial_fd = ::open(common::wstring_to_string(portName).c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

    if(serial_fd < 0) {
        perror("open filure");
        ::close(serial_fd);
        serial_fd = -1;
        throw common::SerialPortException("open serial port failure");
    }

    try {
        configure(options.baudrate, options.bytesize, options.stopbits, options.parity);
    } catch(std::exception& err) {
        ::close(serial_fd);
        serial_fd = -1;
        throw err;
    }

    // notify_fd = eventfd(0, EFD_NONBLOCK);
    // if (notify_fd == -1) {
    //     throw std::runtime_error("Failed to create eventfd");
    // }

    kqueue_fd = kqueue();

    if(kqueue_fd == -1) {
        ::close(serial_fd);
        serial_fd = -1;

        ::close(notify_fd);

        notify_fd = -1;
        throw common::SerialPortException("open serial port failure");
    }

    EV_SET(&serial_evt, serial_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

    if (kevent(kqueue_fd, &serial_evt, 1, NULL, 0, NULL) == -1) {

        ::close(serial_fd);

        serial_fd = -1;

        ::close(kqueue_fd);

        kqueue_fd = -1;

        throw common::SerialPortException("Failed to add event to kqueue");
    }

    // Create a pipe for notification
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        ::close(serial_fd);

        serial_fd = -1;

        ::close(kqueue_fd);

        kqueue_fd = -1;

        throw common::SerialPortException("Failed to create pipe");
    }

    notify_fd = pipe_fd[1];

    EV_SET(&serial_evt, pipe_fd[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(kqueue_fd, &serial_evt, 1, NULL, 0, NULL) == -1) {

        ::close(notify_fd);

        notify_fd = -1;

        ::close(serial_fd);

        serial_fd = -1;

        ::close(kqueue_fd);

        kqueue_fd = -1;

        throw common::SerialPortException("Failed to add pipe event to kqueue");
    }

    startAsyncRead();

    _is_open = true;
}

void SerialPort::configure(unsigned long baudRate, unsigned char byteSize, unsigned char stopBits, unsigned char parity) {
    struct termios tty;
    if (tcgetattr(serial_fd, &tty) != 0) {
        perror("tcgetattr");

        throw common::SerialPortException("configure serial port failure");
    }

    cfsetospeed(&tty, convert_baud_rate(baudRate));
    cfsetispeed(&tty, convert_baud_rate(baudRate));

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | convert_byte_size(byteSize);
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;

    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 0;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    if (parity == 0) {
        // None
    } else if (parity == 1) {
        tty.c_cflag |= PARENB | PARODD; // odd
    } else if (parity == 2) {
        tty.c_cflag |= PARENB; // even
    } else {
        throw common::SerialPortException("configure serial port failure");
    }

    if (stopBits == 1) {
        tty.c_cflag &= ~CSTOPB; // 1 停止位
    } else if (stopBits == 2) {
        tty.c_cflag |= CSTOPB;  // 2 停止位
    } else {
        throw common::SerialPortException("configure serial port failure");
    }

    tty.c_cflag &= ~CRTSCTS; // 禁用硬件流控制

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        throw common::SerialPortException("configure serial port failure");
    }
}

void SerialPort::asyncReadThread() {
    struct kevent event;

    char buffer[BUFFER_SIZE];

    while(running) {
        

        int n = kevent(kqueue_fd, NULL, 0, &event, 1, NULL);

        if(n == -1) {
            if (errno == EINTR) {
                // kevent was interrupted by a unix signal, continue the loop
                continue;
            } else {
                std::cerr << "kevent error: " << strerror(errno) << std::endl;
                break;
            }
        }

        if(n > 0) {
            if (event.filter == EVFILT_READ) {
                if(event.ident == serial_fd) {
                    ssize_t bytes_read = ::read(serial_fd, buffer, BUFFER_SIZE);

                    // maybe need to process errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR
                    // when bytes_read is negative
                    if (bytes_read > 0) {
                        std::vector<std::any> args;
                        args.emplace_back(std::string(reinterpret_cast<char*>(buffer), bytes_read));
                        emit(SerialPortEvent::ON_DATA, args);
                    }
                } else if(event.ident == notify_fd) {
                    ::read(notify_fd, &buffer, sizeof(buffer));

                    break;
                }
            }
        }
    }

    running = false;
}

void SerialPort::startAsyncRead() {
    if(running) {
        return;
    }

    running = true;

    readThread = std::thread(&SerialPort::asyncReadThread, this);
}

void SerialPort::stopAsyncRead() {
    if(!running) {
        return;
    }

    uint64_t notify_val = 1;
    ::write(notify_fd, &notify_val, sizeof(notify_val));

    running = false;

    if(readThread.joinable()) {
        readThread.join();
    }
}

bool SerialPort::is_open() {
    return _is_open;
}

void SerialPort::close() {
    stopAsyncRead();

    if(!_is_open) return;

    if(notify_fd != -1) {
        ::close(notify_fd);

        notify_fd = -1;
    }

    _is_open = false;
    if(serial_fd != -1) {
        ::close(serial_fd);

        serial_fd = -1;
    }

    if(kqueue_fd != -1) {
        ::close(kqueue_fd);

        kqueue_fd = -1;
    }
    
    

    _is_open = false;
}


void SerialPort::write(const std::string& data) {
    if (!is_open()) {
        throw common::SerialPortException("Serial port is not open");
    }

    ssize_t total_bytes_written = 0;
    ssize_t bytes_to_write = data.size();

    while (total_bytes_written < bytes_to_write) {
        ssize_t bytes_written = ::write(serial_fd, data.c_str() + total_bytes_written, bytes_to_write - total_bytes_written);

        if (bytes_written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                // Retry write if it was interrupted by a signal
                // or when Resource temporarily unavailable occure
                continue;
            } else {
                std::cerr << "Error: " << strerror(errno) << std::endl;

                throw common::SerialPortException("Write to serial port failure");
            }
        }
        
        total_bytes_written += bytes_written;
    }
}

#endif