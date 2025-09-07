#pragma once

#include <functional>
#include <map>
#include <vector>
#include <Arduino.h>

using EventCallback = std::function<void(const String &)>;
using NetworkEventCallback = std::function<void(int, void *)>;

class EventManager
{
public:
    void on(const String &name, EventCallback cb);
    void on(const String &name, NetworkEventCallback cb);
    void emit(const String &name, const String &data = String());
    void emit(const String &name, int eventId, void *param);

private:
    std::map<String, std::vector<EventCallback>> listeners;
    std::map<bool, std::vector<EventCallback>> bool_listeners;
    std::map<String, std::vector<NetworkEventCallback>> network_listeners;
};

extern EventManager GlobalEventBus;
