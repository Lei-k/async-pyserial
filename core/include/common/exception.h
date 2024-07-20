#ifndef ASYNC_PYSERIAL_COMMON_EXCEPTION_H
#define ASYNC_PYSERIAL_COMMON_EXCEPTION_H

#include <string>

namespace async_pyserial
{
    namespace common
    {
        class SerialPortException : public std::exception
        {
        public:
            explicit SerialPortException(const std::string &message) : msg(message) {}

            virtual const char *what() const noexcept override
            {
                return msg.c_str();
            }

        private:
            std::string msg;
        };
        
        class OSException : public std::exception
        {
        public:
            explicit OSException(const std::string &message) : msg(message) {}

            virtual const char *what() const noexcept override
            {
                return msg.c_str();
            }

        private:
            std::string msg;
        };
    }
}

#endif