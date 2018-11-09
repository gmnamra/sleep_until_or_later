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
    m_count(0)
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
        if (val < m_min)
            m_min = val;
        if (val > m_max)
            m_max = val;
        m_total += val;
        ++m_count;
        m_squared_total += val*val;
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
    stats& operator+=(const stats& s)
    {
        if (s.m_min < m_min)
            m_min = s.m_min;
        if (s.m_max > m_max)
            m_max = s.m_max;
        m_total += s.m_total;
        m_squared_total += s.m_squared_total;
        m_count += s.m_count;
        return *this;
    }
    
    static void PrintTo(const stats<T>& stinst, std::ostream* stream_ptr)
    {
        if(! stream_ptr) return;
        *stream_ptr  << "Count: " << stinst.count ()              << std::endl;
        *stream_ptr << "Min  : " << stinst.minimum ()             << std::endl;
        *stream_ptr << "Max  : " << stinst.maximum ()             << std::endl;
        *stream_ptr << "Mean : " << stinst.mean ()                << std::endl;
        *stream_ptr << "Std  : " << std::sqrt(stinst.variance ()) << std::endl;
        *stream_ptr << "RMS  : " << stinst.rms ()                 << std::endl;
    }
    
private:
    T m_min, m_max, m_total, m_squared_total;
    uintmax_t m_count;
};


#endif /* stats_h */
