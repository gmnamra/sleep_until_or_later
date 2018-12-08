//
//  main.cpp
//  sleep_until_or_later
//
//  Created by Arman Garakani on 11/8/18.
//  Copyright © 2018 darisallc. All rights reserved.
//
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <chrono>
#include <ratio>
#include <iomanip>
#include <thread>
#include <mutex>
#include <cstring>
#include "sleep_util_or_later.hpp"

#include "stats.hpp"

using namespace sleep_util;


/*
 *      Testing Support ( @todo use gtest )
 *      Generate varying sleep time with an offset and a multiplier
 */


/*
 * mutex for regulating outpuyt to std::cout
 */
std::mutex iomutex;

typedef std::chrono::steady_clock l_clock_t;



/*
TT is must be a duration<long long, "unit">
O and M are offset and multiplier for generating varying sleep tim
N is number of iterations to run
*/

template<typename TT>
void test_sleep_until_or_later (int offset, int multiplier, int N , bool PRNT, stats<float>& stats_holder )
{
    for (int i = 0; i < N; ++i)
    {
        int rnd = 0; // 1 + std::rand()/((RAND_MAX + 1u)/7);
        auto sleep_time = TT { offset + rnd + i * multiplier};
        auto in_microsecs = std::chrono::duration_cast<std::chrono::microseconds> (sleep_time);
        auto mark = l_clock_t::now ();
        sleep_until_or_later( static_cast<unsigned int>(in_microsecs.count()));
        // initialize duration for now ()
        auto took = std::chrono::duration_cast<std::chrono::microseconds>(l_clock_t::now () - mark);
        
        auto time = std::chrono::duration_cast<std::chrono::duration<float>>(took);
        auto diff = std::chrono::duration_cast<std::chrono::duration<float>>(sleep_time - time);

        
        auto diff_count = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
        stats_holder.add(diff_count);
        
        if(PRNT)
        {
            std::lock_guard<std::mutex> lk(iomutex);
            
            std::cout << i << " out of " << N;
            std::cout << "\t Expected \t" << sleep_time.count() << " units " << "\t Observed  \t" <<
                std::chrono::duration_cast<std::chrono::microseconds>(time).count() << " with " << diff_count << " microseconds " << std::endl;
        }
    }

}


template<typename TT>
void test_std_this_thread_sleep_for (int offset, int multiplier, int N , bool PRNT, stats<float>& stats_holder)
{
    stats<float> info;
    for (int i = 0; i < N; ++i)
    {
        int rnd = 0; // 1 + std::rand()/((RAND_MAX + 1u)/7);
        auto sleep_time = TT { offset + rnd + i * multiplier};
        auto in_microsecs = std::chrono::duration_cast<std::chrono::microseconds> (sleep_time);
        auto mark = l_clock_t::now ();
        std::this_thread::sleep_for(in_microsecs);
        // initialize duration for now ()
        auto took = std::chrono::duration_cast<std::chrono::microseconds>(l_clock_t::now () - mark);
        
        auto time = std::chrono::duration_cast<std::chrono::duration<float>>(took);
        auto diff = std::chrono::duration_cast<std::chrono::duration<float>>(sleep_time - time);
        
        
        auto diff_count = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
        stats_holder.add(diff_count);
        
        if(PRNT)
        {
            std::lock_guard<std::mutex> lk(iomutex);
            
            std::cout << i << " out of " << N;
            std::cout << "\t Expected \t" << sleep_time.count() << " units " << "\t Observed  \t" <<
            std::chrono::duration_cast<std::chrono::microseconds>(time).count() << " with " << diff_count << " microseconds " << std::endl;
        }
    }
}

bool run_test (std::pair<uint32_t,uint32_t>& test_case, float better_by, bool verbose) {
 
    stats<float> sleepUntilOrLater, thisThreadSleepFor;
    
   
    {
        if(verbose)std::cout << "Testing sleep_until_or_later " << std::endl;
        std::thread sleep_thread (&test_sleep_until_or_later<std::chrono::microseconds>, test_case.first, test_case.second, 10, false, std::ref(sleepUntilOrLater));
        sleep_thread.join ();
    }
    
    {
        if(verbose)std::cout << "Testing sleep_for  " << std::endl;;
        std::thread sleep_thread (&test_std_this_thread_sleep_for<std::chrono::microseconds>, test_case.first, test_case.second,  10, false, std::ref(thisThreadSleepFor));
        sleep_thread.join ();
    }


    if(verbose)std::cout << "===============================================================" << std::endl;
    auto epsilon = 1e-6; // 1/1000000 of microseconds or 1/1000 of a nanosecond
    
    bool ok = sleepUntilOrLater.count() == thisThreadSleepFor.count();
    
    if(verbose)std::cout << sleepUntilOrLater.count() << " Test Cases" << std::endl;
    
    // Uncomment to see all stats
    //stats<float>::PrintTo(sleepUntilOrLater , &std::cout);
    //stats<float>::PrintTo(thisThreadSleepFor , &std::cout);
    
    // Check neither sleeps for more than was asked
 
    ok &= (sleepUntilOrLater.minimum() <= epsilon);
    if(verbose)std::cout << "sleep_until_or_later was on or later by " << -sleepUntilOrLater.minimum() << std::endl;
    
    ok &= (thisThreadSleepFor.minimum() <= epsilon);
    if(verbose)std::cout << "this_thread::sleep_fpr was on or later by " << -thisThreadSleepFor.minimum() << std::endl;
    
    auto min_better = thisThreadSleepFor.minimum() / (sleepUntilOrLater.minimum() + epsilon);
    ok &= min_better >= better_by;
    
    return ok;
};



int main (int argc, char* argv [] ) {
    typedef std::pair<uint32_t,uint32_t> u32_pair_t;
    std::vector<u32_pair_t> tests {{1023u,1u},{10023u, 10u},{100023u, 100u},{1002323u, 100u}};
    float better_by = 100.0f;
    for(auto test_case : tests){
        auto res = run_test(test_case, better_by, false);
        auto verdict =  res ? "Passed" : "Failed";
        std::cout << "======================" << test_case.first << " microseconds =========" << verdict << std::endl;

    }

    
}

