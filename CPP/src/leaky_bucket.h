#ifndef SPIDER_LEAKY_BUCKET_H
#define SPIDER_LEAKY_BUCKET_H

#include <algorithm>
#include <ctime>

class leaky_bucket {
public:
    bool grant() {
        // 这段时间流失的水
        auto now = time(nullptr);
        int out = (int)(now - m_ts) * m_rate; // 出水的速率恒定
        // 此刻桶内剩余的水
        m_water = std::max(0, m_water - out);
        m_ts = now;
        // 限制流量
        if (m_water + 1 < m_size) {
            ++m_water;
            return true; // passed
        } else {
            return false; // denied
        }
    }

private:
    time_t m_ts; // last ts
    int m_water; // 漏桶里现有的水
    int m_size;  // 漏桶大小
    int m_rate;  // 出水的速度
};

#endif // SPIDER_LEAKY_BUCKET_H
