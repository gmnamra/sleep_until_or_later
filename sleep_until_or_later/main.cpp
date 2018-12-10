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

bool run_test (std::pair<uint64_t,uint32_t>& test_case, float better_by, bool verbose) {
 
    stats<float> sleepUntilOrLater, thisThreadSleepFor;
    
   
    {
        if(verbose)std::cout << "Testing sleep_until_or_later " << std::endl;
        std::thread sleep_thread (&test_sleep_until_or_later<std::chrono::microseconds>, test_case.first, test_case.second, 10, verbose, std::ref(sleepUntilOrLater));
        sleep_thread.join ();

    }
    
    {
        if(verbose)std::cout << "Testing sleep_for  " << std::endl;;
        std::thread sleep_thread (&test_std_this_thread_sleep_for<std::chrono::microseconds>, test_case.first, test_case.second,  10, verbose, std::ref(thisThreadSleepFor));
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
    if(verbose)std::cout << "sleep_until_or_later was on or later at most by " << -sleepUntilOrLater.median() << std::endl;
    
    ok &= (thisThreadSleepFor.minimum() <= epsilon);
    if(verbose)std::cout << "this_thread::sleep_f0r was on or later at most by " << -thisThreadSleepFor.median() << std::endl;
    
    auto min_better = thisThreadSleepFor.minimum() / (sleepUntilOrLater.minimum() + epsilon);
    ok &= min_better >= better_by;
    
    auto verdict =  ok? "Passed" : "Failed";
    std::cout << "======================" << test_case.first << " microseconds =========" << verdict << std::endl;
    
    return ok;
};

void print_usage (){
    std::cout << "sleep_until_or_later is run and performance compared with thisthread::sleep_for\n";
    std::cout << "Usage: " << " [ optional:-[vV] verbose optional:count <microseconds>\n";
    std::cout << "\tcommandline is empty, runs standard test with verbose set to off\n";
    std::cout << "\tIf count is 0, runs built in test cases\n";
    std::cout << "\telse sleep_until_or_later is run and performance compared with thisthread::sleep_for\n";
}


int main (int argc, char* argv [] ) {
   
    std::this_thread::sleep_for(1s);
    
    std::vector<std::string> verbose_cmds {"-v","-V"};
    
    if(argc < 2 || argc > 4){
        print_usage();
        return 0;
    }
    bool verbose = argv[1] == verbose_cmds[0] || argv[1] == verbose_cmds[1];
    if ((argc == 2 && verbose) || (argc == 3 && ! verbose)){
        print_usage();
        return 1;
    }

    typedef std::pair<uint64_t,uint32_t> u64_pair_t;
    float better_by = 2.0f;
    bool valid_input = false;

    uint64_t val = 0;

    if(verbose)
        std::cout << "sleep_until_or_later is run and performance compared with thisthread::sleep_for\n";
    
    try{
        val = std::stoul(argv[argc-1]);
        valid_input = true;
    }
    catch (std::exception &e) {
            if(verbose)
                std::cerr << "Error: " << e.what() << std::endl << std::endl << val << std::endl;
            return 1;
    }

    switch(val){
        case 0:{
                std::vector<u64_pair_t> builtin_tests {{1023u,1u},{10023u, 10u},{100023u, 100u},{1002323u, 100u}};
                for(auto test_case : builtin_tests){
                    run_test(test_case, better_by, false);
                }
                break;
        }
        default:
            u64_pair_t test_case (val,1);
            run_test(test_case, better_by, verbose);
            break;
    }
}
