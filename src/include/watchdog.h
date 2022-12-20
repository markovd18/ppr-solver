#pragma once

#include <chrono>
#include <thread>
#include <atomic>

class Watchdog {
private:
    std::thread m_watchdog_thread;
    std::chrono::milliseconds m_timeout;
    std::atomic_bool m_guarding = false;

    std::atomic<std::size_t> m_total_processed_items = 0;
public:
    explicit Watchdog(std::chrono::milliseconds timeout);
    void Start();
    void Stop();
    void Kick(std::size_t processsed_items);

private:
    void Run();
};