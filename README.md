# sleep_until_or_later
repeatability test for sleep_for

Summary
—————
Sleep_until_or_later is always the behavior. That is observed sleep time greater than expected is an assertion. If it is not, that is a bug. 

Predictability is influenced by OS’s scheduler priority choice for the sleep_thread ( in main.cpp ). To test this using std::thread API, it allows access to the native thread object. For macOS that is posix. I almost never set thread priority in my work. In this case, it applies only to a thread whose life is limited. In this case what is important is how far worst can the observed be relative to expected. The regression result of expected versus the observed, shows a very good correlation implying high degree of prediction that we can trust sleep_until_or later will not return earlier or take more than roughly expected + 20 percent. 


