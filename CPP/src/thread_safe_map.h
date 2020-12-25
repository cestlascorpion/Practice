#ifndef SPIDER_THREAD_SAFE_MAP_H
#define SPIDER_THREAD_SAFE_MAP_H

#include <algorithm>
#include <boost/thread/pthread/shared_mutex.hpp>
#include <list>
#include <map>
#include <vector>

template <typename K, typename V, typename Hash = std::hash<K>>
class thread_safe_map {
private:
    using bucket_value = std::pair<K, V>;
    using bucket_data = std::list<std::pair<K, V>>;
    using bucket_iter = typename std::list<std::pair<K, V>>::iterator;

    class bucket {
    private:
        bucket_data data;
        mutable boost::shared_mutex mutex;

        bucket_iter find_entry_for(const K &key) const {
            return std::find_if(data.begin(), data.end(), [&](const bucket_value &item) { return item.first == key; });
        }

    public:
        V value_for(const K &key, const V &default_value) const {
            boost::shared_lock<boost::shared_mutex> lock(mutex);
            const bucket_iter found_entry = find_entry_for(key);
            if (found_entry != data.end()) {
                return found_entry->second;
            }
            return default_value;
        }

        void add_or_update_mapping(const K &key, const V &value) {
            std::unique_lock<boost::shared_mutex> lock(mutex);
            const bucket_iter found_entry = find_entry_for(key);
            if (found_entry != data.end()) {
                found_entry->second = value;
                return;
            }
            data.push_back(bucket_value(key, value));
        }

        void remove_mapping(const K &key) {
            std::unique_lock<boost::shared_mutex> lock(mutex);
            const bucket_iter found_entry = find_entry_for(key);
            if (found_entry != data.end()) {
                data.erase(found_entry);
            }
        }
    };

    std::vector<std::unique_ptr<bucket>> buckets;
    Hash hash_func;

    bucket &get_bucket(const K &key) const {
        const std::size_t bucket_index = hash_func(key) % buckets.size();
        return *buckets[bucket_index];
    }

public:
    explicit thread_safe_map(unsigned num_buckets = 19, const Hash &hash = Hash())
        : buckets(num_buckets)
        , hash_func(hash) {
        for (unsigned i = 0; i < num_buckets; ++i) {
            buckets[i].reset(new bucket);
        }
    }

    thread_safe_map(thread_safe_map const &) = delete;
    thread_safe_map &operator=(thread_safe_map const &) = delete;

    V value_for(const K &key, const V &default_value) const {
        return get_bucket(key).value_for(key, default_value);
    }

    void add_or_update_mapping(const K &key, const V &value) {
        get_bucket(key).add_or_update_mapping(key, value);
    }

    void remove_mapping(const K &key) {
        get_bucket(key).remove_mapping(key);
    }

    std::map<K, V> get_map() const {
        std::vector<std::unique_lock<boost::shared_mutex>> locks;
        for (unsigned i = 0; i < buckets.size(); ++i) {
            locks.push_back(std::unique_lock<boost::shared_mutex>(buckets[i].mutex));
        }
        std::map<K, V> res;
        for (unsigned i = 0; i < buckets.size(); ++i) {
            for (bucket_iter it = buckets[i].data.begin(); it != buckets[i].data.end(); ++it) {
                res.insert(*it);
            }
        }
        return res;
    }
};

#endif // SPIDER_THREAD_SAFE_MAP_H