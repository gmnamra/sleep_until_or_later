//
//  main.cpp
//  sleep_until_or_later
//
//  Created by Arman Garakani on 11/8/18.
//  Copyright © 2018 darisallc. All rights reserved.
//
#include <iostream>
#include <map>
#include <cmath>
#include <string>
#include <chrono>
#include <ratio>
#include <iomanip>
#include <thread>
#include <mutex>
#include <cstring>
#include "stats.hpp"


/*
 *  void sleep_until_or_later (unsigned int  microseconds_later)
 *  @todo: add support for exceptions

  To produce a more repeatable sleep functionality:
  1. Choose a sleep period, chunk, smaller than requested sleep_time to use with sleep_for
       a. sleep_for(chunk) sleeps for maximum of sleep_time
       b. small enough that microsecond accurate sleep does not call now() too many times
          use log of microseconds to proportionally set remainder.
  2. sleep_for (chunk)
  3. measure actual sleep time
  4. compute sleep_time remaining
  5. use microsecond sleep function
 */
void sleep_until_or_later (unsigned int microseconds_later){
    using namespace std::chrono;
    using CK = std::chrono::steady_clock;

    if (microseconds_later == 0) return;
    
    auto mark = CK::now ();
    // sleep time in microseconds
    microseconds dur (microseconds_later);
    
    // log10 of sleep time in microseconds.
    auto logx = std::lround(std::log(microseconds_later)) + 1;
  
    auto reminder_counts = (logx < 3) ? 1 : logx < 4 ? 100 : logx < 6 ? 1000 : 10000;
    auto remainder = std::chrono::microseconds(reminder_counts);
    auto chunk = dur - remainder;
    // use sleep_until for all the way until the last millisecond.
 
    std::this_thread::sleep_for(chunk);
    auto took = std::chrono::duration_cast<std::chrono::microseconds>(CK::now () - mark);
    remainder = dur - took;
    mark = CK::now ();
    // initialize duration for now ()
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(CK::now () - mark);
    
    // Find the number of microseconds. Remainder
    // update duration as long as we have spent less time than sleep time
    while (duration.count() < remainder.count()){
        duration = std::chrono::duration_cast<std::chrono::microseconds>(CK::now () - mark);
    }
}


/*
 *      Testing Support ( @todo use gtest )
 */


/*
 * mutex for regulating outpuyt to std::cout
 */
std::mutex iomutex;

typedef std::chrono::steady_clock l_clock_t;
stats<float> sleepUntilOrLater, thisThreadSleepFor;

/*
TT is must be a duration<long long, "unit">
O and M are offset and multiplier for generating varying sleep tim
N is number of iterations to run
*/

template<typename TT, int O, int M, int N = 10, bool PRNT = false>
void test_sleep_until_or_later ()
{
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
        sleepUntilOrLater.add(diff_count);
        
        if(PRNT)
        {
            std::lock_guard<std::mutex> lk(iomutex);
            
            std::cout << i << " out of " << N;
            std::cout << "\t Expected \t" << sleep_time.count() << " units " << "\t Observed  \t" <<
                std::chrono::duration_cast<std::chrono::microseconds>(time).count() << " with " << diff_count << " microseconds " << std::endl;
        }
    }

}


template<typename TT, int O, int M, int N = 10, bool PRNT = false>
void test_std_this_thread_sleep_for ()
{
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
        thisThreadSleepFor.add(diff_count);
        
        if(PRNT)
        {
            std::lock_guard<std::mutex> lk(iomutex);
            
            std::cout << i << " out of " << N;
            std::cout << "\t Expected \t" << sleep_time.count() << " units " << "\t Observed  \t" <<
            std::chrono::duration_cast<std::chrono::microseconds>(time).count() << " with " << diff_count << " microseconds " << std::endl;
        }
    }
}

int main  (int argc, const char * argv[]) {
 
    std::cout << "===============================================================" << std::endl;
    {
        std::cout << "Testing sleep_until_or_later unit 1023 microseconds ";
        std::thread sleep_thread (&test_sleep_until_or_later<std::chrono::microseconds, 1023, 1>);
        sleep_thread.join ();
    }
    
    {
        std::cout << "Testing sleep_for unit 1023 microseconds ";
        std::thread sleep_thread (&test_std_this_thread_sleep_for<std::chrono::microseconds, 1023, 1>);
        sleep_thread.join ();
    }
    
    std::cout << "===============================================================" << std::endl;
    {
        std::cout << "Testing sleep_until_or_later unit 10023  microseconds ";
        std::thread sleep_thread (&test_sleep_until_or_later<std::chrono::microseconds, 10023, 10>);
        sleep_thread.join ();
    }
    
    {
        std::cout << "Testing sleep_for unit 10023 microseconds ";
        std::thread sleep_thread (&test_std_this_thread_sleep_for<std::chrono::microseconds, 10023, 10>);
        sleep_thread.join ();
    }
    
    std::cout << "===============================================================" << std::endl;
    {
        std::cout << "Testing sleep_until_or_later unit 100023 microseconds ";
        std::thread sleep_thread (&test_sleep_until_or_later<std::chrono::microseconds, 100023, 100>);
        sleep_thread.join ();
    }
    
    {
        std::cout << "Testing sleep_for unit 100023 microseconds ";
        std::thread sleep_thread (&test_std_this_thread_sleep_for<std::chrono::microseconds, 100023, 100>);
        sleep_thread.join ();
    }

  

    std::cout << "===============================================================" << std::endl;

    {
        std::cout << "Testing sleep_until_or_later unit 10002323 microseconds ";
        std::thread sleep_thread (&test_sleep_until_or_later<std::chrono::microseconds, 1002323, 100>);
        sleep_thread.join ();
    }
    {
        std::cout << "Testing sleep_for unit 10002323 microseconds ";
        std::thread sleep_thread (&test_std_this_thread_sleep_for<std::chrono::microseconds, 1002323, 100>);
        sleep_thread.join ();
    }
 
    std::cout << "===============================================================" << std::endl;
    
    stats<float>::PrintTo(sleepUntilOrLater , &std::cout);
    stats<float>::PrintTo(thisThreadSleepFor , &std::cout);
    
    // Check neither sleeps for more than was asked
    auto epsilon = 1e-6; // 1/1000000 of microseconds or 1/1000 of a nanosecond
    assert(sleepUntilOrLater.minimum() <= epsilon);
    assert(thisThreadSleepFor.minimum() <= epsilon);
    auto min_better = thisThreadSleepFor.minimum() / (sleepUntilOrLater.minimum() + epsilon);
    assert(min_better >= 100);
    auto std_better = std::sqrt(thisThreadSleepFor.variance()) / std::sqrt((sleepUntilOrLater.variance() + epsilon));
    assert(std_better >= 100);
    std::cout << min_better << "," << std_better << std::endl;
    
    
    return 0;
};
