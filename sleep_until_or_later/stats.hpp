//
//  stats.hpp
//  sleep_until_or_later
//
//  Created by Arman Garakani on 11/9/18.
//  Copyright © 2018 darisallc. All rights reserved.
//

#ifndef stats_h
#define stats_h


#include <stdint.h>
#include <limits>
#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

template <class T>
class stats
{
public:
    stats()
    : m_min(std::numeric_limits<T>::max()),
    m_max(std::numeric_limits<T>::min() ),
    m_total(0),
    m_squared_total(0),
    m_count(0),
    m_median_cached(T(0),false)
    {}
    
    stats(const T& total, const T& total_squared, uint32_t count, const T& minv, const T& maxv):
    m_min(minv),
    m_max(maxv),
    m_total(total),
    m_squared_total(total_squared),
    m_count(count)
    
    {}
    
    void add(const T& val)
    {
        // invalidate the median cache.
        // use case does not require incremental median calculation
        m_median_cached.second = false;
        if (val < m_min)
            m_min = val;
        if (val > m_max)
            m_max = val;
        m_total += val;
        ++m_count;
        m_squared_total += val*val;
        m_copy.push_back(val);
    }
    T minimum()const { return m_min; }
    T maximum()const { return m_max; }
    T total()const { return m_total; }
    T mean()const { return m_total / static_cast<T>(m_count); }
    uintmax_t count()const { return m_count; }
    T variance()const
    {
        T t = m_squared_total - m_total * m_total / m_count;
        t /= (m_count - 1);
        return t;
    }
    T rms()const
    {
        return sqrt(m_squared_total / static_cast<T>(m_count));
    }
    

    /*
     * Partial_sort_copy copies the smallest Nelements from the range [first, last) to the range [result_first, result_first + N)
     * where N is the smaller of last - first and result_last - result_first .
     * The elements in [result_first, result_first + N) will be in ascending order.
     */

    T median()const
    {
        if(! m_median_cached.second){
            auto length =  m_copy.size();
            if ( length == 0 )
                return 0.0;
            
            const bool is_odd = length % 2;
            auto array_length = (length / 2) + 1;
            std::vector<T> array (array_length);
            partial_sort_copy(m_copy.begin(), m_copy.end(), array.begin(), array.end());

            if (is_odd){
                m_median_cached.first = array [array_length-1];
                m_median_cached.second = true;
            }
            else{
                 m_median_cached.first = (array [array_length - 2] + array [array_length-1]) / T(2);
                m_median_cached.second = true;
            }
        }
        return m_median_cached.first;
    }
    static void PrintTo(const stats<T>& stinst, std::ostream* stream_ptr)
    {
        if(! stream_ptr) return;
        *stream_ptr  << "Count: " << stinst.count ()              << std::endl;
        *stream_ptr << "Min  : " << stinst.minimum ()             << std::endl;
        *stream_ptr << "Max  : " << stinst.maximum ()             << std::endl;
        *stream_ptr << "Median : " << stinst.median ()            << std::endl;
        *stream_ptr << "Mean : " << stinst.mean ()                << std::endl;
        *stream_ptr << "Std  : " << std::sqrt(stinst.variance ()) << std::endl;
        *stream_ptr << "RMS  : " << stinst.rms ()                 << std::endl;
    }
    
private:
    T m_min, m_max, m_total, m_squared_total;
    uintmax_t m_count;
    std::vector<T> m_copy;
    mutable std::pair<T, std::atomic<bool>> m_median_cached;
};




#endif /* stats_h */
