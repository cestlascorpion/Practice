#include <iostream>
#include <vector>

using namespace std;

int main() {
    vector<int> vec(10); // 10 zero-initialized elements

    for (auto i = 0u; i < vec.size(); i++)
        vec[i] = i;

    cout << "vec contains:";
    for (auto i = 0u; i < vec.size(); i++)
        cout << ' ' << vec[i];
    cout << '\n';

    return 0;
}