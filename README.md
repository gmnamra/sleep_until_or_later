# sleep_until_or_later


## Overview

Is std library's sleep_for accurate to a microsecond ? Put another way, what should be our expectation of the difference between **expected_count** of miroceconds sleep and **actual_count**. It is required that **actual_count** be equal or greater than **expected_count**. Initial tests showed that ```std::this_thread::sleep_for*```  is accurate to *a millisecond or so*. 

```sleep_until_or_later```  is a *header-only* microsecond accurate alternative to ```std::this_thread::sleep_for```. 

## Design
### Requirements

- OS’s scheduler priority choice can not be altered.
- Runs on the thread it is called on. Does not create any threads. 
- Minimal work above comparable ```std::this_thread::sleep_for```.
- Simple API ```void sleep_until_or_later (unsigned int microseconds)```

### Design approach: Coarse duration & Fine duration
Divide the **expected_count** in to larger coarse and fine duration counts. Use ```std::this_thread::sleep_for``` for coarse_duration, followed by call sleeping through fine_duration be repeated calls to ```::now()```.

*1.  ```Initial_Mark = ::now()```

*2.  ```std::this_thread::sleep_for``` for a ```chunk_count =  expected_count - delta_count```

*3. Calculate  ```remainder_count = ::now() - Initial_Mark``` 

*4. ```Rem_Mark = ::now()```

*5. Initialize ```duration_count = ::now() - Rem_Mark``` 

*6. While ```(duration_count < remainder_count) {duration_count = ::now() - Rem_Mark}``` 


#### Choice of fine_duration count ( microseconds)
Fine_duration count is smaller than expected_count and is set proportional to the expected_count but never larger than 10,000 ( or 10 milliseconds)

## Example
We'll now see how ```sleep_until_or_later```  can be used. The header file can be fetched from the repository. 

```c++
#include "sleep_util_or_later.hpp"

unsigned int in_microsecs = 1234567u;
auto mark = std::chrono::steady_clock ::now ();

sleep_until_or_later(in_microsecs.count);

auto took = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock ::now () - mark);
auto time = std::chrono::duration_cast<std::chrono::duration<float>>(took);
auto diff = std::chrono::duration_cast<std::chrono::duration<float>>(sleep_time - time);

```

## Executable Demo: sleep_until_or_later

### Building
```git clone https://github.com/gmnamra/sleep_until_or_later```
```cd sleep_until_or_later```
```open sleep_until_or_later.xcodeproj```
Set Team and build

### Running
```
$ ./sleep_until_or_later
    Measures performance of sleep_until_or_later versus this_thread::sleep_for
    Usage:  [ optional:-[vV] verbose optional:count <microseconds>
    commandline is empty, runs standard test with verbose set to off
    If count is 0, runs built in test cases
    else sleep_until_or_later is run and performance compared with this_thread::sleep_for

```
```
$ ./sleep_until_or_later -v 10000000
sleep_until_or_later is run and performance compared with thisthread::sleep_for
Testing sleep_until_or_later 
0 out of 10     Expected     10000000 units      Observed      10000003 with -3 microseconds 
1 out of 10     Expected     10000001 units      Observed      10000002 with -1 microseconds 
2 out of 10     Expected     10000002 units      Observed      10000003 with -1 microseconds 
3 out of 10     Expected     10000003 units      Observed      10000004 with -1 microseconds 
4 out of 10     Expected     10000004 units      Observed      10000005 with -1 microseconds 
5 out of 10     Expected     10000005 units      Observed      10000007 with -2 microseconds 
6 out of 10     Expected     10000006 units      Observed      10000008 with -2 microseconds 
7 out of 10     Expected     10000007 units      Observed      10000008 with -1 microseconds 
8 out of 10     Expected     10000008 units      Observed      10000009 with -1 microseconds 
9 out of 10     Expected     10000009 units      Observed      10000011 with -2 microseconds 
Testing sleep_for  
0 out of 10     Expected     10000000 units      Observed      10005030 with -5030 microseconds 
1 out of 10     Expected     10000001 units      Observed      10001864 with -1863 microseconds 
2 out of 10     Expected     10000002 units      Observed      10000586 with -584 microseconds 
3 out of 10     Expected     10000003 units      Observed      10004994 with -4991 microseconds 
4 out of 10     Expected     10000004 units      Observed      10005036 with -5032 microseconds 
5 out of 10     Expected     10000005 units      Observed      10005004 with -4999 microseconds 
6 out of 10     Expected     10000006 units      Observed      10001017 with -1011 microseconds 
7 out of 10     Expected     10000007 units      Observed      10001751 with -1744 microseconds 
8 out of 10     Expected     10000008 units      Observed      10004987 with -4979 microseconds 
9 out of 10     Expected     10000009 units      Observed      10001330 with -1321 microseconds 
===============================================================
10 Test Cases
sleep_until_or_later was on or later at most by 1
this_thread::sleep_f0r was on or later at most by 3421
======================10000000 microseconds =========Passed

```



