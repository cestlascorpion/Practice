#include "any_type.h"
#include "ioc_container.h"
#include <iostream>

using namespace std;

class ICar {
public:
    virtual ~ICar() = default;
    virtual void test() = 0;
};

class Bus : public ICar {
public:
    Bus() = default;
    void test() override {
        cout << "test bus" << endl;
    }
};

class Car : public ICar {
public:
    Car() = default;
    void test() override {
        cout << "test car" << endl;
    }
};

class cool {
public:
    explicit cool(ICar *ptr)
        : m_ptr(ptr) {}

    ~cool() {
        if (m_ptr != nullptr) {
            delete m_ptr;
            m_ptr = nullptr;
        }
    }

    void test() {
        m_ptr->test();
    }

private:
    ICar *m_ptr;
};

void test_ioc() {
    ioc_container<ICar> ioc;
    ioc.RegisterType<Bus>("bus");
    ioc.RegisterType<Car>("car");

    std::shared_ptr<ICar> bus = ioc.ResolveShared("bus");
    std::shared_ptr<ICar> car = ioc.ResolveShared("car");

    bus->test();
    car->test();
}

void test_any() {
    Any n;
    auto r = n.isNull();
    cout << boolalpha << r << endl;

    string s = "hi";
    n = s;

    try {
        n.AnyCast<int>();
    } catch (...) {
        cout << "bad cast" << endl;
    }

    Any x = 1;
    cout << boolalpha << x.Is<int>() << endl;
}

class apple {
public:
    void test() {
        cout << "test apple" << endl;
    }
};

class banana {
public:
    void test() {
        cout << "test banana" << endl;
    }
};

void test_ioc_any() {
    ioc_container_any ioc;
    ioc.RegisterType<apple>("apple");
    ioc.RegisterType<banana>("banana");
    shared_ptr<apple> a = ioc.ResolveShared<apple>("apple");
    a->test();
    shared_ptr<banana> b = ioc.ResolveShared<banana>("banana");
    b->test();

    ioc.RegisterType<Bus>("bus");
    ioc.RegisterType<Car>("car");
    shared_ptr<Bus> bus = ioc.ResolveShared<Bus>("bus");
    bus->test();
    shared_ptr<Bus> car = ioc.ResolveShared<Bus>("car");
    car->test();

    ioc.RegisterType<cool, Bus>("coolBus");
    auto cb = ioc.ResolveShared<cool>("coolBus");
    cb->test();
}

int main() {
    test_ioc_any();
    return 0;
}