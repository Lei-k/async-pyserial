#ifndef ASYNC_PYSERIAL_BASE_SERIALPORT_H
#define ASYNC_PYSERIAL_BASE_SERIALPORT_H

namespace async_pyserial {
    namespace base {
        struct SerialPortOptions
        {
            unsigned long baudrate;
            unsigned char bytesize;
            unsigned char stopbits;
            unsigned char parity;
            unsigned long read_timeout = 50;
            unsigned long write_timeout = 50;
        };
    }
}


#endif