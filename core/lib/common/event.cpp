#include <common/event.h>

using namespace async_pyserial::common;


ListenerHandle EventEmitter::addListener(EventType eventType, std::function<void(const std::vector<std::any>&)> listener) {
    ListenerHandle handle = nextListenerHandle++;
    listeners[eventType][handle] = std::move(listener);
    return handle;
}

ListenerHandle EventEmitter::on(EventType eventType, std::function<void(const std::vector<std::any>&)> listener) {
    return addListener(eventType, std::move(listener));
}

void EventEmitter::emit(EventType eventType, const std::vector<std::any>& args) {
    const auto& listenerMap = listeners[eventType];
    for (const auto& [handle, listener] : listenerMap) {
        listener(args);
    }
}

void EventEmitter::removeListener(EventType eventType, ListenerHandle listenerHandle) {
    auto& listenerMap = listeners[eventType];
    listenerMap.erase(listenerHandle);
    if (listenerMap.empty()) {
        listeners.erase(eventType);
    }
}