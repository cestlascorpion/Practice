#ifndef SPIDER_IOC_CONTAINER_H
#define SPIDER_IOC_CONTAINER_H

#include "any_type.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
template <typename T>
class ioc_container {
public:
    ioc_container() = default;
    ~ioc_container() = default;

public:
    template <typename Drived>
    void RegisterType(const std::string &key) {
        std::function<T *()> function = [] { return new Drived(); };
        registerType(key, function);
    }

    T *Resolve(const std::string &key) {
        if (m_map.find(key) == m_map.end()) {
            return nullptr;
        }
        return m_map[key]();
    }

    std::shared_ptr<T> ResolveShared(const std::string &key) {
        T *ptr = Resolve(key);
        return std::shared_ptr<T>(ptr);
    }

private:
    void registerType(const std::string &key, std::function<T *()> function) {
        m_map[key] = function;
    }

private:
    std::map<std::string, std::function<T *()>> m_map;
};

class ioc_container_any {
public:
    ioc_container_any() = default;
    ~ioc_container_any() = default;

public:
    template <typename T, typename Depend>
    void RegisterType(const std::string &key) {
        std::function<T *()> function = [] { return new T(new Depend()); };
        registerType(key, function);
    }
    template <typename T>
    void RegisterType(const std::string &key) {
        std::function<T *()> function = [] { return new T(); };
        registerType(key, function);
    }

    template <typename T>
    T *Resolve(const std::string &key) {
        if (m_map.find(key) == m_map.end()) {
            return nullptr;
        }
        Any resolver = m_map[key];

        std::function<T *()> function = resolver.AnyCast<std::function<T *()>>();
        return function();
    }

    template <typename T>
    std::shared_ptr<T> ResolveShared(const std::string &key) {
        T *ptr = Resolve<T>(key);
        return std::shared_ptr<T>(ptr);
    }

private:
    void registerType(const std::string &key, Any constructor) {
        m_map.emplace(key, constructor);
    }

private:
    std::map<std::string, Any> m_map;
};

#endif // SPIDER_IOC_CONTAINER_H