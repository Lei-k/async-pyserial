#ifdef LINUX

#include <linux/serialport.h>

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>


#include <common/util.h>
#include <common/exception.h>
#include <sys/eventfd.h>

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
        {230400, B230400},
        {460800, B460800},
        {500000, B500000},
        {576000, B576000},
        {921600, B921600},
        {1000000, B1000000},
        {1152000, B1152000},
        {1500000, B1500000},
        {2000000, B2000000},
        {2500000, B2500000},
        {3000000, B3000000},
        {3500000, B3500000},
        {4000000, B4000000}
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

    notify_fd = eventfd(0, EFD_NONBLOCK);
    if (notify_fd == -1) {
        throw std::runtime_error("Failed to create eventfd");
    }

    epoll_fd = epoll_create1(0);

    if(epoll_fd == -1) {
        ::close(serial_fd);
        serial_fd = -1;

        ::close(notify_fd);

        notify_fd = -1;
        throw common::SerialPortException("open serial port failure");
    }

    notify_evt.events = EPOLLIN;
    notify_evt.data.fd = notify_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, notify_fd, &notify_evt) == -1) {
        perror("epoll_ctl");

        ::close(notify_fd);

        notify_fd = -1;

        ::close(serial_fd);

        serial_fd = -1;

        ::close(epoll_fd);

        epoll_fd = -1;

        throw common::SerialPortException("open serial port failure");
    }

    event.events = EPOLLIN;
    event.data.fd = serial_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serial_fd, &event) == -1) {
        perror("epoll_ctl");

        ::close(notify_fd);

        notify_fd = -1;

        ::close(serial_fd);

        serial_fd = -1;

        ::close(epoll_fd);

        epoll_fd = -1;

        throw common::SerialPortException("open serial port failure");
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
    tty.c_iflag &= ~IGNBRK; // 禁用忽略断开连接
    tty.c_lflag = 0; // 非规范模式
    tty.c_oflag = 0; // 禁用输出处理

    tty.c_cc[VMIN]  = 1; // 最小读取字符数
    tty.c_cc[VTIME] = 0; // 读取超时

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // 禁用软件流控制
    tty.c_cflag |= (CLOCAL | CREAD); // 启用接收器，设置本地模式
    tty.c_cflag &= ~(PARENB | PARODD); //清除奇偶校验
    if (parity == 0) {
        // 无奇偶校验
    } else if (parity == 1) {
        tty.c_cflag |= PARENB | PARODD; // 启用奇校验
    } else if (parity == 2) {
        tty.c_cflag |= PARENB; // 启用偶校验
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
    struct epoll_event epoll_evts[EPOLL_MAX_EVENTS];

    char buffer[BUFFER_SIZE];

    while(running) {
        

        int n = epoll_wait(epoll_fd, epoll_evts, EPOLL_MAX_EVENTS, -1);

        if(n == -1) {
            if (errno == EINTR) {
                // epoll_wait was interrupted by a signal, continue the loop
                continue;
            } else {
                std::cerr << "epoll_wait error: " << strerror(errno) << std::endl;
                goto exit;
            }
        }

        for(int i = 0; i < n; i++) {
            auto evt = epoll_evts[i];

            if(evt.data.fd == notify_fd) {
                ::read(notify_fd, buffer, BUFFER_SIZE);
                goto exit;
            }

            if(evt.events & EPOLLIN) {

                int bytes_read = ::read(evt.data.fd, buffer, BUFFER_SIZE);

                if(bytes_read > 0) {
                    std::string buffer2send(buffer, buffer + bytes_read);

                    std::vector<std::any> emitArgs = { buffer2send };

                    emit(SerialPortEvent::ON_DATA, emitArgs);
                }

                
            }else if(evt.events & (EPOLLERR | EPOLLHUP)) {
                fprintf(stderr, "Epoll error on fd %d\n", evt.data.fd);
                
                goto exit;
            }
        }
    }

    exit:

    running = false;

    std::cout << "exit" << std::endl;
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

    if(epoll_fd != -1) {
        ::close(epoll_fd);

        epoll_fd = -1;
    }
    
    

    _is_open = false;
}


void SerialPort::write(const std::string &data, const std::function<void(unsigned long)>& callback) {
    if (!is_open()) {
        throw common::SerialPortException("Serial port is not open");
    }

    ssize_t total_bytes_written = 0;
    ssize_t bytes_to_write = data.size();

    while (total_bytes_written < bytes_to_write) {
        ssize_t bytes_written = ::write(serial_fd, data.c_str() + total_bytes_written, bytes_to_write - total_bytes_written);

        if (bytes_written < 0) {
            if (errno == EINTR) {
                // Retry write if it was interrupted by a signal
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