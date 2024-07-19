#ifndef COMMON_EXCEPTION_HPP
#define COMMON_EXCEPTION_HPP

#include <string>

namespace pybind {
  class OSException: public std::exception {
    public:
      explicit OSException(const std::string& message): msg(message) {}

      virtual const char* what() const noexcept override {
        return msg.c_str();
      }

      private:
          std::string msg;
  };
}

#endif