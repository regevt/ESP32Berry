// Centralized SPI bus lock to coordinate shared-bus peripherals (TFT, SD, Radio)
#pragma once

#include <Arduino.h>

// Initialize the lock; safe to call multiple times
void spi_bus_init();

// Block until the lock is acquired
void spi_bus_lock();

// Try to acquire the lock within ticks; returns true on success
bool spi_bus_try_lock(TickType_t ticks);

// Release the lock
void spi_bus_unlock();
