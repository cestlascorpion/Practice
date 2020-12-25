#ifndef SPIDER_ANY_TYPE_H
#define SPIDER_ANY_TYPE_H

#include <memory>
#include <typeindex>

class Any {
public:
    Any(void)
        : m_ty_idx(std::type_index(typeid(void))) {}
    Any(Any &that)
        : m_ptr(that.Clone())
        , m_ty_idx(that.m_ty_idx) {}
    Any(Any &&that)
        : m_ptr(std::move(that.m_ptr))
        , m_ty_idx(that.m_ty_idx) {}

    template <typename U,
              class = typename std::enable_if<!std::is_same<typename std::decay<U>::type, Any>::value, U>::type>
    Any(U &&value)
        : m_ptr(new Derived<typename std::decay<U>::type>(std::forward<U>(value)))
        , m_ty_idx(std::type_index(typeid(typename std::decay<U>::type))) {}

    bool isNull() const {
        return !bool(m_ptr);
    }

    template <typename U>
    [[nodiscard]] bool Is() const {
        return m_ty_idx == std::type_index(typeid(U));
    }

    template <typename U>
    U &AnyCast() {
        if (!Is<U>()) {
            throw std::bad_cast();
        }
        auto derived = dynamic_cast<Derived<U> *>(m_ptr.get());
        return derived->m_value;
    }

    Any &operator=(const Any &a) {
        if (m_ptr == a.m_ptr) {
            return *this;
        }

        m_ptr = a.Clone();
        m_ty_idx = a.m_ty_idx;
        return *this;
    }

private:
    struct Base;
    using base_ptr = std::unique_ptr<Base>;

    struct Base {
        virtual ~Base() = default;
        virtual base_ptr Clone() const = 0;
    };

    template <typename T>
    struct Derived : public Base {
        T m_value;

        template <typename U>
        Derived(U &&value)
            : m_value(std::forward<U>(value)) {}

        base_ptr Clone() const override {
            return base_ptr(new Derived<T>(m_value));
        }
    };

    base_ptr Clone() const {
        if (m_ptr != nullptr) {
            return m_ptr->Clone();
        }
        return nullptr;
    }

    base_ptr m_ptr;
    std::type_index m_ty_idx;
};

#endif // #ifndef SPIDER_ANY_TYPE_H