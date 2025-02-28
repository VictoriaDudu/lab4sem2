#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>

#include "support.hpp"

#define N 5
#define numIter 100

using namespace std;

class Semaphore {
public:
    Semaphore(int count = 1) : count(count) {}

    void acquire() {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this]() { return count > 0; });
        --count;
    }

    void release() {
        unique_lock<mutex> lock(mtx);
        ++count;
        cv.notify_one();
    }

private:
    mutex mtx;
    condition_variable cv;
    int count;
};

Semaphore semaphore(1);

void run() {
    auto start = chrono::steady_clock::now();
    semaphore.acquire(); // захватывает семафор
    for (int i = 0; i < numIter; i++) {
        cout << generateRandom() << " ";
    }
    cout << endl;
    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;
    semaphore.release(); // освобождает семафор
    cout << "Elapsed time: " << elapsed.count() << " seconds" << endl;
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