//
//  sleep_util_or_later.hpp
//  sleep_until_or_later
//
//  Created by Arman Garakani on 12/8/18.
//  Copyright © 2018 darisallc. All rights reserved.
//

#ifndef sleep_util_or_later_hpp
#define sleep_util_or_later_hpp

// For log
#include <cmath>
// For chrono
#include <chrono>
#include <ratio>

// For this ::this_tread
#include <thread>


namespace sleep_util{
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
    
    auto reminder_counts = (logx < 2) ? 1 : logx < 3 ? 10 : logx < 4 ? 100 : logx < 6 ? 1000 : 10000;
    auto remainder = std::chrono::microseconds(reminder_counts);
    auto chunk = dur - remainder;

    // Find the number of microseconds. Remainder
    std::this_thread::sleep_for(chunk);
    auto took = std::chrono::duration_cast<std::chrono::microseconds>(CK::now () - mark);
    remainder = dur - took;
    mark = CK::now ();
    // initialize duration for now ()
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(CK::now () - mark);
    

    // update duration as long as we have spent less time than sleep time
    while (duration.count() < remainder.count()){
        duration = std::chrono::duration_cast<std::chrono::microseconds>(CK::now () - mark);
    }
}

}

#endif /* sleep_util_or_later_hpp */
