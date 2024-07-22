#ifdef LINUX

#include <linux/serialport.h>

#endif

#include <base/serialport.h>

using namespace async_pyserial;

int main() {
    base::SerialPortOptions options;
    options.baudrate = 9600;
    options.bytesize = 8;
    options.stopbits = 1;
    options.parity = 0;
    options.read_timeout = 50;
    options.write_timeout = 50;

    internal::SerialPort serial(L"/dev/pts/8", options);

    serial.on(internal::SerialPortEvent::ON_DATA, [](const std::vector<std::any>& args) {
        auto& data = std::any_cast<const std::vector<char>&>(args[0]);
        std::string str(data.begin(), data.end());
        std::cout << str << std::endl;
    });

    serial.open();

    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "exit") break;

        serial.write(input);
    }

    return 0;
}