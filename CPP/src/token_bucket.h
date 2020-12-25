#ifndef SPIDER_TOKEN_BUCKET_H
#define SPIDER_TOKEN_BUCKET_H

#include <algorithm>
#include <ctime>

class token_bucket {
public:
    bool grant() {
        // 这段时间产生的令牌
        auto now = time(nullptr);
        int in = (int)(now - m_ts) * m_token_rate;
        // 此刻桶内可以取得的令牌总数
        m_token_num = std::min(m_size, m_token_num + in);
        m_ts = now;
        // 限制流量
        if (m_token_num > 0) {
            --m_token_num;
            return true; // passed
        } else {
            return false; // denied
        }
    }

private:
    time_t m_ts;
    int m_size;
    int m_token_rate;
    int m_token_num;
};

#endif // SPIDER_TOKEN_BUCKET_H
