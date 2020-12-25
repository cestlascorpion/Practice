#ifndef SPIDER_FUNCTION_WRAPPER_H
#define SPIDER_FUNCTION_WRAPPER_H

#include <memory>

class function_wrapper {
public:
    template <typename F>
    function_wrapper(F &&f)
        : impl(new impl_type<F>(std::forward<F>(f))) {}

    void operator()() {
        impl->call();
    }

    function_wrapper() = default;

    function_wrapper(function_wrapper &&other) noexcept
        : impl(std::move(other.impl)) {}

    function_wrapper &operator=(function_wrapper &&other) noexcept {
        impl = std::move(other.impl);
        return *this;
    }

    function_wrapper(function_wrapper &) = delete;
    function_wrapper(const function_wrapper &) = delete;
    function_wrapper &operator=(const function_wrapper &) = delete;

private:
    struct impl_base {
        virtual void call() = 0;

        virtual ~impl_base() = default;
    };

    template <typename F>
    struct impl_type : impl_base {
        F f;

        explicit impl_type(F &&_f)
            : f(std::move(_f)) {}

        void call() override {
            f();
        }
    };

    std::unique_ptr<impl_base> impl;
};

#endif // SPIDER_FUNCTION_WRAPPER_H
