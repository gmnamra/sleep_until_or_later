//
//  main.cpp
//  sleep_until_or_later
//
//  Created by Arman Garakani on 11/8/18.
//  Copyright © 2018 darisallc. All rights reserved.
//
#include <iostream>
#include <cmath>
#include <string>
#include <chrono>
#include <iomanip>
#include <thread>
#include <thread>
#include <mutex>
#include <cstring>
#include <pthread.h>
#include "stats.hpp"

std::mutex iomutex;

/*
 TT is must be a duration<long long, "unit">
 O and M are offset and multiplier for generating varying sleep tim
 N is number of iterations to run
 */
template<typename TT, int O, int M, int N = 10>
void thread_func ()
{
    std::cout << "thread started " << std::endl;
    stats<float> info;
    for (int i = 0; i < N; ++i)
    {
        auto sleep_time = TT { O + i * M};
        auto mark = std::chrono::steady_clock::now ();
        std::this_thread::sleep_for (sleep_time);
        auto duration = std::chrono::duration_cast<TT>(std::chrono::steady_clock::now () - mark);
        sched_param sch;
        int policy;
        pthread_getschedparam(pthread_self(), &policy, &sch);
        std::lock_guard<std::mutex> lk(iomutex);
        
        float diff = duration.count () - sleep_time.count ();
        info.add(diff);
        std::cout << i << " out of " << N << "\t Priority " << sch.sched_priority;
        std::cout << "\t Expected \t" << sleep_time.count () << " units " <<
        "\t Observed \t" << duration.count () << " units " <<
        "\t Difference \t" << duration.count () - sleep_time.count () << " units " << std::endl;
    }
    stats<float>::PrintTo(info, &std::cout);
}

void set_priority (std::thread& thread, int num){
    if (num < 0) return;
    sched_param sch;
    int policy;
    pthread_getschedparam(thread.native_handle(), &policy, &sch);
    sch.sched_priority = num;
    if (pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &sch)) {
        std::cout << "Failed to setschedparam: " << std::strerror(errno) << '\n';
    }
}

int get_priority (std::thread& thread){
    sched_param sch;
    int policy;
    pthread_getschedparam(thread.native_handle(), &policy, &sch);
    return sch.sched_priority;
}

int main(int argc, const char * argv[]) {
    
    int thread_priority = 0;
    {
        std::cout << "unit = milliseconds ";
        std::thread sleep_thread (&thread_func<std::chrono::milliseconds, 1, 1>);
        set_priority(sleep_thread,thread_priority);
        sleep_thread.join ();
    }
    
    {
        std::cout << "unit = microseconds ";
        std::thread sleep_thread (&thread_func<std::chrono::microseconds, 1, 1>);
        set_priority(sleep_thread,thread_priority);
        sleep_thread.join ();
    }
    {
        std::cout << "unit = nanoseconds ";
        std::thread sleep_thread (&thread_func<std::chrono::nanoseconds, 100, 10>);
        set_priority(sleep_thread,thread_priority);
        sleep_thread.join ();
    }
    
    return 0;
};
