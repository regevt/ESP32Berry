#include "EventManager.h"
#include <ESP32Berry_Network.hpp>

EventManager GlobalEventBus;

void EventManager::on(const String &name, EventCallback cb)
{
    listeners[name].push_back(cb);
}
void EventManager::on(const String &name, NetworkEventCallback cb)
{
    network_listeners[name].push_back(cb);
}

void EventManager::emit(const String &name, const String &data)
{
    auto it = listeners.find(name);
    if (it != listeners.end())
    {
        for (auto &cb : it->second)
        {
            cb(data);
        }
    }
}

void EventManager::emit(const String &name, int eventId, void *param)
{
    auto it = network_listeners.find(name);
    if (it != network_listeners.end())
    {
        for (auto &cb : it->second)
        {
            cb(eventId, param);
        }
    }
}
