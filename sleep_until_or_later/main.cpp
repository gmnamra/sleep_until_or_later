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

long double getDifference(const std::chrono::steady_clock::time_point& tp1,const std::chrono::steady_clock::time_point& tp2){
    auto diff= tp2- tp1;
    auto res= std::chrono::duration <double, std::milli> (diff).count();
    return res;
}

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

// measure execution time of a void()
// @todo template or passed in with clock and resolution as arguments
namespace benchmark {
    auto measure = [](auto f, unsigned repetitions = 500) {
        std::chrono::time_point<l_clock_t> start, stop;
        start = l_clock_t::now();
        for (auto i = repetitions; i > 0; --i) {
            f();
        }
        stop = l_clock_t::now();
        auto dur = stop - start;
        auto time = std::chrono::duration_cast<std::chrono::duration<float>>(
                                                                             dur / repetitions
                                                                             );
        auto took = std::chrono::duration_cast<std::chrono::nanoseconds>(time);
        return took;
    };
}

#define SINGLETONPTR(Typename)                     \
static Typename* instance() {                 \
static Typename e;                          \
return &e;                                  \
}


class timeMeasureCalibration
{
public:
    timeMeasureCalibration ()
    {
        m_took = benchmark::measure(&l_clock_t::now);
    }
    
    const std::chrono::nanoseconds execution_time() const { return m_took; }
    
private:
    std::chrono::nanoseconds m_took;
};

SINGLETONPTR(timeMeasureCalibration);

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
}//


/*
 * sleep_until_or_later
 */

/*
 *  void sleep_until_or_later (unsigned int seconds_later)
 *  @todo: add support for exceptions
 */
void sleep_until_or_later (unsigned int seconds_later){
    using CK = l_clock_t;
    std::chrono::duration<double> dur (static_cast<double>(seconds_later));
    // sleep time in microseconds
    auto us_dur = std::chrono::duration_cast<std::chrono::microseconds>(dur);
    // this_thread::sleep_for may sleep sleep_time +/- small number of milliseconds
    // To produce a more repeatable sleep functionality:
    // 1. Choose a sleep period, chunk, smaller than requested sleep_time to use with sleep_for
    //      a. sleep_for(chunk) sleeps for maximum of sleep_time
    //      b. small enough that microsecond accurate sleep does not call now() too many times
    // 2. sleep_for (chunk)
    // 3. measure actual sleep time
    // 4. compute sleep_time remaining
    // 5. use microsecond sleep function
    auto remainder = std::chrono::microseconds(10000);
    auto chunk = us_dur - remainder;

    // use sleep_until for all the way until the last millisecond.
    auto mark = CK::now ();
    std::this_thread::sleep_for(chunk);
    auto read = CK::now ();
    auto took = std::chrono::duration_cast<std::chrono::microseconds>(read - mark);
    remainder = us_dur - took;
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
        auto in_secs = std::chrono::duration_cast<std::chrono::seconds> (sleep_time);
        auto mark = l_clock_t::now ();
        sleep_until_or_later( in_secs.count());
        // initialize duration for now ()
        auto took = std::chrono::duration_cast<std::chrono::microseconds>(l_clock_t::now () - mark);
        
        auto time = std::chrono::duration_cast<std::chrono::duration<float>>(took);
        auto diff = std::chrono::duration_cast<std::chrono::duration<float>>(sleep_time - time);

        
        auto diff_count = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
        info.add(diff_count);
        std::lock_guard<std::mutex> lk(iomutex);
        
        std::cout << i << " out of " << N;
        std::cout << "\t Expected \t" << sleep_time.count() << " units " << "\t Observed  \t" <<
            std::chrono::duration_cast<std::chrono::microseconds>(time).count() / 1000000 << " with " << diff_count << " microseconds " << std::endl;
    }
    stats<float>::PrintTo(info, &std::cout);
}

int main  (int argc, const char * argv[]) {
 
    auto now_took = instance()->execution_time().count();
    std::cout << "::now () takes " << now_took << " nanoseconds " << std::endl;
    std::cout << "===============================================================" << std::endl;
    
    {
        std::cout << "sleep_until_or_later unit = seconds ";
        std::thread sleep_thread (&thread_func_3<std::chrono::seconds, 1, 1>);
        sleep_thread.join ();
    }
    
#if 0
    {
        std::cout << "unit = nanoseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::nanoseconds, 100, 100>);
        sleep_thread.join ();
    }

    {
        std::cout << "unit = microseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::microseconds, 1, 1>);
        sleep_thread.join ();
    }

    {
        std::cout << "unit = milliseconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::milliseconds, 1, 1>);
        sleep_thread.join ();
    }
 
    {
        std::cout << "unit = seconds ";
        std::thread sleep_thread (&thread_func_2<std::chrono::seconds, 1, 1>);
        sleep_thread.join ();
    }
#endif
 
    return 0;
};
