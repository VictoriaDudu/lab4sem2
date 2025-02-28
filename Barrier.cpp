#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include "support.hpp"

#define N 5
#define numIter 100

using namespace std;

class Barrier {
public:
    Barrier(int count) : threshold(count), count(0), generation(0) {}

    void arrive_and_wait() {
        unique_lock<mutex> lock(mtx);
        auto gen = generation;

        if (++count == threshold) {
            ++generation;
            count = 0;
            cv.notify_all(); // Будим всех ожидающих
        }
        else {
            cv.wait(lock, [this, gen] { return gen != generation; });
        }
    }

private:
    mutex mtx;
    condition_variable cv;
    int threshold;
    int count;
    int generation;
};

Barrier point(N);
mutex mtx;

void run() {
    auto start = chrono::steady_clock::now();

    // Ожидание на барьере
    point.arrive_and_wait();

    vector<char> random_symbols;
    for (int i = 0; i < numIter; i++) {
        random_symbols.push_back(generateRandom());
    }

    // Синхронизированный вывод
    {
        lock_guard<mutex> lock(mtx);
        for (auto num : random_symbols) {
            cout << num << " ";
        }
        cout << endl;
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;

    // Вывод времени выполнения
    {
        lock_guard<std::mutex> lock(mtx);
        cout << "Elapsed time: " << elapsed.count() << " seconds" << endl;
    }
}

template <typename T>
void primitive(T func) {
    vector<thread> threads;
    for (int i = 0; i < N; i++) {
        threads.emplace_back(func);
    }

    for (auto& t : threads) {
        t.join();
    }
}

int main() {
    primitive(run);
    return 0;
}