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


#if 0
template <class T, class F>
T timeout(const F &bind, long sleep) {
    T ret;
    std::thread thrd(boost::lambda::var(ret) = bind);
    if(thrd.timed_join(std::chrono::milliseconds(sleep))) {
        return ret;
    }
    else throw std::runtime_error("timeout");
}
#endif


void print_time(std::chrono::system_clock::time_point t)
{
    using namespace std::chrono;
    time_t c_time = system_clock::to_time_t(t);
    std::tm* tmptr = std::localtime(&c_time);
    system_clock::duration d = t.time_since_epoch();
    std::cout << tmptr->tm_hour << ':' << tmptr->tm_min << ':' << tmptr->tm_sec
    << '.' << (d - duration_cast<seconds>(d)).count();
}

template
<
typename CK = std::chrono::high_resolution_clock,
typename TT = std::chrono::milliseconds,
typename FC,
typename... FCArgs
>
long long measure_execution_time( FC&& f, FCArgs&&... fargs )
{
    auto time_begin = CK::now();
    f( std::forward<FCArgs>( fargs )... );
    auto time_end = CK::now();
    return std::chrono::duration_cast<TT>( time_end - time_begin ).count();
}

// measure execution time of a void()
// @todo template or passed in with clock and resolution as arguments
namespace benchmark {
    auto measure = [](auto f, unsigned repetitions = 500) {
        std::chrono::time_point<std::chrono::steady_clock> start, stop;
        start = std::chrono::steady_clock::now();
        for (auto i = repetitions; i > 0; --i) {
            f();
        }
        stop = std::chrono::steady_clock::now();
        auto dur = stop - start;
        auto time = std::chrono::duration_cast<std::chrono::duration<float>>(
                                                                             dur / repetitions
                                                                             );
        auto took = std::chrono::duration_cast<std::chrono::nanoseconds>(time);
        return took.count();
    };
}

/*
 TT is must be a duration<long long, "unit">
 O and M are offset and multiplier for generating varying sleep tim
 N is number of iterations to run
 */

/*
 Sleep for a target sleep time, return pair (actual sleep time, count of calls to ::now ()
 */

template<typename TT, typename CK = std::chrono::high_resolution_clock>
std::pair<TT,int> sleep(TT sleep_time){
    auto mark = CK::now ();
    // initialize duration for now ()
    auto duration = std::chrono::duration_cast<TT>(CK::now () - mark);
    
    // Find the number of microseconds. Remainder
    // update duration as long as we have spent less time than sleep time
    static int i = 0;
    while (duration.count() < sleep_time.count()){
        duration = std::chrono::duration_cast<TT>(CK::now () - mark);
        std::this_thread::yield();
        i++;
    }
    return std::make_pair(duration,i);
}



// @todo add nanosleep

/*
 TT is must be a duration<long long, "unit">
 O and M are offset and multiplier for generating varying sleep tim
 N is number of iterations to run
 */

template<typename TT, int O, int M, int N = 10>
void thread_func_2 ()
{
    std::cout << "thread started " << std::endl;
    stats<float> info;
    for (int i = 0; i < N; ++i)
    {
        auto sleep_time = TT { O + i * M};
        auto duration_loop_pair = sleep(sleep_time);
        auto diff = duration_loop_pair.first.count() - sleep_time.count();
        std::lock_guard<std::mutex> lk(iomutex);
        
        info.add(diff);
        std::cout << i << " out of " << N << "\t Loop Count " << duration_loop_pair.second ;
        std::cout << "\t Expected \t" << sleep_time.count () << " units " <<
        "\t Observed \t" << duration_loop_pair.first.count () << " units " <<
        "\t Difference \t" << duration_loop_pair.first.count () - sleep_time.count () << " units " << std::endl;
    }
    stats<float>::PrintTo(info, &std::cout);
}


/*
 * sleep_until_or_later
 */

void sleep_until_or_later (unsigned int seconds_later){
    using CK = std::chrono::high_resolution_clock;
    std::chrono::duration<double> dur (static_cast<double>(seconds_later));
    auto mark = CK::now ();
    auto us_dur = std::chrono::duration_cast<std::chrono::microseconds>(dur);
    if (us_dur < dur)
        ++us_dur;

    std::chrono::microseconds pad (1000);
    // always number of seconds. Less than pad, use our microsecond accurate sleep
    
    auto st = mark + us_dur - pad;
    
    // use sleep_until for all the way until the last millisecond.
    std::this_thread::sleep_until(st);
    
    // use our microsecond accurate sleep for the last millisecond.
    sleep(pad);
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
        int rnd = 1 + std::rand()/((RAND_MAX + 1u)/7);
        auto sleep_time = TT { O + rnd + i * M};
        auto in_secs = std::chrono::duration_cast<std::chrono::seconds> (sleep_time);
        auto sleep_time_ms = std::chrono::milliseconds(sleep_time).count();
        auto took = measure_execution_time(sleep_until_or_later, in_secs.count());

        auto diff = sleep_time_ms - took;
        std::lock_guard<std::mutex> lk(iomutex);
        
        info.add(diff);
        std::cout << i << " out of " << N << "\t Loop Count " ;
        std::cout << "\t Expected \t" << sleep_time_ms << " units " << took <<
        "\t Difference \t" << diff << " units " << std::endl;
    }
    stats<float>::PrintTo(info, &std::cout);
}

int main  (int argc, const char * argv[]) {
 
    auto now_took = benchmark::measure(&std::chrono::steady_clock::now);
    std::cout << now_took << " nanoseconds " << std::endl;
    
    
    {
        std::cout << "unit = seconds ";
        std::thread sleep_thread (&thread_func_3<std::chrono::seconds, 1, 10>);
        sleep_thread.join ();
    }
    
    {
        std::cout << "unit = milliseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::milliseconds, 1, 1>);
        sleep_thread.join ();
    }
 
    {
        std::cout << "unit = microseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::microseconds, 1, 1>);
        sleep_thread.join ();
    }
    {
        std::cout << "unit = nanoseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::nanoseconds, 100, 100>);
        sleep_thread.join ();
    }
  
 
    return 0;
};
