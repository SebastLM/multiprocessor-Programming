#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <string>
#include <stdexcept>


std::vector<std::mutex> chopsticks;
std::atomic<bool> running(true);


std::pair<int, int> chopstick_indices(int table_pos, int n) {
    int left  = table_pos;
    int right = table_pos + 1;
    // so we dont try acessing a position out of bounds
    if (right == n) right = 0;

    return {left, right};
}

void philosopher_thread(int table_pos, int n) {
    auto [left, right] = chopstick_indices(table_pos, n);

    // a good way to prevent deadlocks is locking the lower index first
    int first  = std::min(left, right);
    int second = std::max(left, right);

    while (running) {
        std::cout << "Philosopher" << table_pos << " is thinking..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        std::lock(chopsticks[first], chopsticks[second]);
        std::lock_guard<std::mutex> lk1(chopsticks[first],  std::adopt_lock);
        std::lock_guard<std::mutex> lk2(chopsticks[second], std::adopt_lock);

        std::cout << "Philosopher" << table_pos << " is now eating using chopsticks " 
                  << left << " and " << right << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::cout << "Philosopher" << table_pos << " finished eating.." << std::endl;

        chopsticks[second].unlock();
        chopsticks[first].unlock();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Should be" << argv[0] << " <number_of_philosophers>" << std::endl;
        return 1;
    }

    int n = 0;
    try {
        n = std::stoi(argv[1]);
    } catch (const std::exception&) {
        std::cerr << "Error converting String number_of_philosophers to int";
        return 1;
    }

    chopsticks = std::vector<std::mutex>(n);

    std::vector<std::thread> philosophers;
    philosophers.reserve(n); // so the vector as at least n elements
    for (int i = 0; i < n; ++i) {
        philosophers.emplace_back(philosopher_thread, i, n);
    }

    std::cin.get();
    running = false;

    for (auto& t : philosophers) {
        t.join();
    }
    return 0;
}