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

class Semaphore {
public:
    explicit Semaphore(int count = 0) : count(count) {}

    void acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return count > 0; });
        --count;
    }

    void release() {
        std::lock_guard<std::mutex> lock(mtx);
        ++count;
        cv.notify_one();
    }

private:
    int count;
    std::mutex mtx;
    std::condition_variable cv;
};

Semaphore semaphore(1); // Инициализируем наш семафор с единичным счетчиком
mutex cout_mutex;

void run() {
    auto start = chrono::steady_clock::now();

    semaphore.acquire();

    string output; // буфер вывода
    for (int i = 0; i < numIter; i++) {
        output += generateRandom();
        output += " ";
    }

    // Синхронизированный вывод
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << output << endl;
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;

    semaphore.release();

    // Синхронизированный вывод времени выполнения
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Elapsed time: " << elapsed.count() << " seconds" << endl;
    }
}

template <typename T>
void primitive(T func) {
    vector<thread> threads;
    for (int i = 0; i < N; i++) {
        threads.emplace_back(func);
    }

    for (auto& th : threads) {
        th.join();
    }
}

int main() {
    primitive(run);
    return 0;
}