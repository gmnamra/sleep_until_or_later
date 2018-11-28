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
#include <ratio>
#include <iomanip>
#include <thread>
#include <mutex>
#include <cstring>
#include <pthread.h>
#include "stats.hpp"

std::mutex iomutex;
typedef std::chrono::steady_clock  l_clock_t;


template
<
typename CK = l_clock_t,
typename TT = std::chrono::microseconds,
typename FC,
typename... FCArgs
>
auto measure_execution_time( FC&& f, FCArgs&&... fargs )
{
    auto time_begin = CK::now();
    f( std::forward<FCArgs>( fargs )... );
    auto time_end = CK::now();
    return std::chrono::duration_cast<TT>( time_end - time_begin );
}

/*
 TT is must be a duration<long long, "unit">
 O and M are offset and multiplier for generating varying sleep tim
 N is number of iterations to run
 */

/*
 Sleep for a target sleep time, return pair (actual sleep time, count of calls to ::now ()
 */

template<typename TT, typename CK = l_clock_t>
std::pair<TT,int> sleep(TT sleep_time){
    auto mark = CK::now ();
    // initialize duration for now ()
    auto duration = std::chrono::duration_cast<TT>(CK::now () - mark);
    
    // Find the number of microseconds. Remainder
    // update duration as long as we have spent less time than sleep time
    static int i = 0;
    while (duration.count() < sleep_time.count()){
        duration = std::chrono::duration_cast<TT>(CK::now () - mark);
//        std::this_thread::yield();
        i++;
    }
    return std::make_pair(duration,i);
}



/*
 * sleep_until_or_later
 */

/*
 *  void sleep_until_or_later (unsigned int seconds_later)
 *  @todo: add support for exceptions
 */
void sleep_until_or_later (unsigned int microseconds_later){
    using CK = l_clock_t;
    using namespace std::chrono;
    if (microseconds_later == 0) return;
    
    auto mark = CK::now ();
    // sleep time in microseconds
    microseconds dur (microseconds_later);
    auto logx = std::lround(std::log(microseconds_later)) + 1;
    // this_thread::sleep_for may sleep sleep_time +/- small number of milliseconds
    // To produce a more repeatable sleep functionality:
    // 1. Choose a sleep period, chunk, smaller than requested sleep_time to use with sleep_for
    //      a. sleep_for(chunk) sleeps for maximum of sleep_time
    //      b. small enough that microsecond accurate sleep does not call now() too many times
    //         use log of microseconds to proportionally set remainder.
    // 2. sleep_for (chunk)
    // 3. measure actual sleep time
    // 4. compute sleep_time remaining
    // 5. use microsecond sleep function
    auto reminder_counts = (logx < 3) ? 1 : logx < 4 ? 100 : logx < 6 ? 1000 : 10000;
    auto remainder = std::chrono::microseconds(reminder_counts);
    auto chunk = dur - remainder;
    // use sleep_until for all the way until the last millisecond.
 
    std::this_thread::sleep_for(chunk);
    auto took = std::chrono::duration_cast<std::chrono::microseconds>(CK::now () - mark);
    remainder = dur - took;
    sleep(remainder);

    
}


/*
TT is must be a duration<long long, "unit">
O and M are offset and multiplier for generating varying sleep tim
N is number of iterations to run
*/

template<typename TT, int O, int M, int N = 10>
void thread_func_3 ()
{
    std::cout << "thread started " << std::endl;
    stats<float> info;
    for (int i = 0; i < N; ++i)
    {
        int rnd = 0; // 1 + std::rand()/((RAND_MAX + 1u)/7);
        auto sleep_time = TT { O + rnd + i * M};
        auto in_microsecs = std::chrono::duration_cast<std::chrono::microseconds> (sleep_time);
        auto mark = l_clock_t::now ();
        sleep_until_or_later( static_cast<unsigned int>(in_microsecs.count()));
        // initialize duration for now ()
        auto took = std::chrono::duration_cast<std::chrono::microseconds>(l_clock_t::now () - mark);
        
        auto time = std::chrono::duration_cast<std::chrono::duration<float>>(took);
        auto diff = std::chrono::duration_cast<std::chrono::duration<float>>(sleep_time - time);

        
        auto diff_count = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
        info.add(diff_count);
        {
            std::lock_guard<std::mutex> lk(iomutex);
            
            std::cout << i << " out of " << N;
            std::cout << "\t Expected \t" << sleep_time.count() << " units " << "\t Observed  \t" <<
                std::chrono::duration_cast<std::chrono::microseconds>(time).count() << " with " << diff_count << " microseconds " << std::endl;
        }
    }
    stats<float>::PrintTo(info, &std::cout);
}


template<typename TT, int O, int M, int N = 10>
void thread_func_2 ()
{
    std::cout << "thread started " << std::endl;
    stats<float> info;
    for (int i = 0; i < N; ++i)
    {
        int rnd = 0; // 1 + std::rand()/((RAND_MAX + 1u)/7);
        auto sleep_time = TT { O + rnd + i * M};
        auto in_microsecs = std::chrono::duration_cast<std::chrono::microseconds> (sleep_time);
        auto mark = l_clock_t::now ();
        std::this_thread::sleep_for(in_microsecs);
        // initialize duration for now ()
        auto took = std::chrono::duration_cast<std::chrono::microseconds>(l_clock_t::now () - mark);
        
        auto time = std::chrono::duration_cast<std::chrono::duration<float>>(took);
        auto diff = std::chrono::duration_cast<std::chrono::duration<float>>(sleep_time - time);
        
        
        auto diff_count = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
        info.add(diff_count);
        {
            std::lock_guard<std::mutex> lk(iomutex);
            
            std::cout << i << " out of " << N;
            std::cout << "\t Expected \t" << sleep_time.count() << " units " << "\t Observed  \t" <<
            std::chrono::duration_cast<std::chrono::microseconds>(time).count() << " with " << diff_count << " microseconds " << std::endl;
        }
    }
    stats<float>::PrintTo(info, &std::cout);
}

int main  (int argc, const char * argv[]) {
 
    std::cout << "===============================================================" << std::endl;
    std::cout << "===============================================================" << std::endl;
    std::cout << "===============================================================" << std::endl;
    {
        std::cout << "sleep_until_or_later unit = microseconds ";
        std::thread sleep_thread (&thread_func_3<std::chrono::microseconds, 1023, 1>);
        sleep_thread.join ();
    }
    
    {
        std::cout << "sleep_for unit = microseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::microseconds, 1023, 1>);
        sleep_thread.join ();
    }
    
    {
        std::cout << "sleep_for unit = milliseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::milliseconds, 1, 1>);
        sleep_thread.join ();
    }
    
    {
        std::cout << "sleep_until_or_later unit =  microseconds ";
        std::thread sleep_thread (&thread_func_3<std::chrono::microseconds, 10023, 10>);
        sleep_thread.join ();
    }
    
    {
        std::cout << "sleep_for unit = microseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::microseconds, 10023, 1>);
        sleep_thread.join ();
    }
    
    {
        std::cout << "sleep_for unit = milliseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::milliseconds, 10, 1>);
        sleep_thread.join ();
    }
    
    {
        std::cout << "sleep_until_or_later unit =  microseconds ";
        std::thread sleep_thread (&thread_func_3<std::chrono::microseconds, 100023, 100>);
        sleep_thread.join ();
    }
    
    {
        std::cout << "sleep_for unit = microseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::microseconds, 100023, 1>);
        sleep_thread.join ();
    }
    
    {
        std::cout << "sleep_for unit = milliseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::milliseconds, 100, 1>);
        sleep_thread.join ();
    }
    {
        std::cout << "sleep_until_or_later unit =  microseconds ";
        std::thread sleep_thread (&thread_func_3<std::chrono::microseconds, 100002323, 1000>);
        sleep_thread.join ();
    }
    {
        std::cout << "sleep_for unit = microseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::microseconds, 100002323, 1>);
        sleep_thread.join ();
    }
    {
        std::cout << "sleep_for unit = milliseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::milliseconds, 100000, 1>);
        sleep_thread.join ();
    }
 
    return 0;
};
