#ifndef COMMON_EVENT_HPP
#define COMMON_EVENT_HPP

#include <map>
#include <vector>
#include <functional>
#include <any>

namespace common {
    typedef unsigned int EventType;
    typedef unsigned int ListenerHandle;

    class EventEmitter {
        public:
            ListenerHandle addListener(EventType eventType, std::function<void(const std::vector<std::any>&)> listener);
            ListenerHandle on(EventType eventType, std::function<void(const std::vector<std::any>&)> listener);
            void removeListener(EventType eventType, ListenerHandle listenerHandle);
            void emit(EventType eventType, const std::vector<std::any>& args);

        private:
            std::map<EventType, std::map<ListenerHandle, std::function<void(const std::vector<std::any>&)>>> listeners;
            ListenerHandle nextListenerHandle = 0;
    };
}


#endif