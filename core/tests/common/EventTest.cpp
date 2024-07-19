#include <Event.hpp>

#include <iostream>

#include <vector>

#include <any>

using namespace common;

int main() {

  EventType ECHO_EVENT = 1;

  EventEmitter emitter;


  auto handle = emitter.on(ECHO_EVENT, [](const std::vector<std::any>& args) {
    std::cout << "Received event with value: " << std::any_cast<int>(args[0]) << std::endl;
  });

  std::vector<std::any> args = { 22 };

  emitter.emit(ECHO_EVENT, args);

  emitter.removeListener(ECHO_EVENT, handle);
}