#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <string>
#include <chrono>

class ScopedTimer
{
public:
    ScopedTimer(const std::string &msg = "")
        : message(msg)
    {
        start_timer();
    }

    void start_timer()
    {
        start = std::chrono::high_resolution_clock::now();
    }

    void stop_timer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << message << duration.count() << " ms\n";
    }

private:
    std::string message;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

#endif // TIMER_H
