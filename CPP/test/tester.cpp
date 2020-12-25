#include <boost/thread.hpp>
#include <iostream>

using namespace std;

void func1(const int &id) {
    cout << "thread #" << id << " start..." << endl;
    boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(3));
    cout << "thread #" << id << " end" << endl;
}

void func2(const int &id) {
    cout << "thread #" << id << " start..." << endl;
    boost::thread::yield();
    cout << "thread #" << id << " end" << endl;
}

void func3(const int &id) {
    cout << "thread #" << id << " start..." << endl;
    boost::this_thread::interruption_point();
    cout << "thread #" << id << " end" << endl;
}

int main() {
    boost::thread t1(func1, 11);
    t1.interrupt();
    boost::thread t2(func2, 22);
    t2.interrupt();
    boost::thread t3(func3, 33);
    t3.interrupt();

    t1.join();
    t2.join();
    t3.join();

    return 0;
}