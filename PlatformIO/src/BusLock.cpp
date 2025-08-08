#include "BusLock.hpp"

static SemaphoreHandle_t s_spi_mutex = nullptr;

void spi_bus_init()
{
    if (!s_spi_mutex)
    {
        s_spi_mutex = xSemaphoreCreateMutex();
    }
}

void spi_bus_lock()
{
    spi_bus_init();
    xSemaphoreTake(s_spi_mutex, portMAX_DELAY);
}

bool spi_bus_try_lock(TickType_t ticks)
{
    spi_bus_init();
    return xSemaphoreTake(s_spi_mutex, ticks) == pdTRUE;
}

void spi_bus_unlock()
{
    if (s_spi_mutex)
    {
        xSemaphoreGive(s_spi_mutex);
    }
}
