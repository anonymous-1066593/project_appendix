#include <stdio.h>
#include <iostream>
#include <random>
#include <locale>
#include <string>
#include <codecvt>
#include <functional>

#include "graph.hpp"
#include "type.hpp"
#include "type_generator.hpp"
#include "subtyping.hpp"
#include "unfold.hpp"
#include "ast.hpp"
#include "parse.hpp"

// Timer
#include <chrono>

std::chrono::time_point<std::chrono::steady_clock> start_time;

void start_timer() {
    start_time = std::chrono::steady_clock::now();
}

long long end_timer() {
    auto end_time = std::chrono::steady_clock::now();
    auto diff = end_time - start_time;
    return std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
}

#include <thread>
#include <mutex>
#include <condition_variable>

// https://stackoverflow.com/questions/40550730/how-to-implement-timeout-for-function-in-c
template <typename T>
bool run_with_timeout(std::function<T(bool &h)> f, long long timeout, T& result, long long &time_taken) {
    std::mutex m;
    std::condition_variable cv;
    bool handler = false;
    std::thread t([&]() {
        start_timer();
        result = f(handler);
        time_taken = end_timer();
        cv.notify_one();
    });
    std::unique_lock<std::mutex> lk(m);
    bool has_timeout = (cv.wait_for(lk, std::chrono::nanoseconds(timeout)) == std::cv_status::timeout);
    if(has_timeout) {
        handler = true;
    }
    t.join();
    return !has_timeout;
}

const long long ONE_SECOND = 1'000'000'000;

using namespace ast;

int main() {
    // Set up UTF-8 output https://stackoverflow.com/questions/50053386/wcout-does-not-output-as-desired
    std::ios_base::sync_with_stdio(false);
    std::locale utf8( std::locale(), new std::codecvt_utf8_utf16<wchar_t> );
    std::wcout.imbue(utf8);

    std::random_device rd;
    std::mt19937 rng(rd());

    /*** BENCHMARKS ***/

    // Exponential tests
    for(int k = 1; k <= 10; k++) {
        Type t1 = generate_exponential_counterexample(k);
        Type t2 = generate_exponential_counterexample(k+1);
        // std::wcout << t1.to_string() << std::endl;
        // std::wcout << t2.to_string() << std::endl;
        bool res = false;
        long long time_taken = 0;
        bool success;
        success = run_with_timeout<bool>([&](bool &h){return inductive_sub::subtype(t1, t2, h);}, 10 * ONE_SECOND, res, time_taken);
        std::cout << "inductive," << k << ',' << success << ',' << time_taken << std::endl;
        success = run_with_timeout<bool>([&](bool &h){return coinductive_sub::subtype(t1, t2, h);}, 10 * ONE_SECOND, res, time_taken);
        std::cout << "coinductive," << k << ',' << success << ',' << time_taken << std::endl;
    }

    // Random tests
    for(int k = 1; k <= 100; k++) {
        Type t1 = generate_random_isomorphic_type(k, rng);
        Type t2 = generate_random_isomorphic_type(k, rng);
        // std::wcout << t1.to_string() << std::endl;
        // std::wcout << t2.to_string() << std::endl;
        bool res = false;
        long long time_taken = 0;
        bool success;
        success = run_with_timeout<bool>([&](bool &h){return inductive_sub::subtype(t1, t2, h);}, 10 * ONE_SECOND, res, time_taken);
        std::cout << "inductive," << k << ',' << success << ',' << time_taken << std::endl;
        success = run_with_timeout<bool>([&](bool &h){return coinductive_sub::subtype(t1, t2, h);}, 10 * ONE_SECOND, res, time_taken);
        std::cout << "coinductive," << k << ',' << success << ',' << time_taken << std::endl;
    }

    // Idempotent
    auto idemp_rng = std::mt19937(42);
    for(int i = 0; i < 100; i++) {
        Type t1 = generate_random_type(10000, 4, idemp_rng, true, 2);
        bool res = false;
        long long time_taken = 0;
        bool success;
        const int ITERS = 10000;
        success = run_with_timeout<bool>([&](bool &h){volatile int x = 0; for(int i = 0; i < ITERS; i++) { x += inductive_sub::subtype(t1, t1, h);} return x;}, 10 * ONE_SECOND, res, time_taken);
        std::cout << "inductive," << i << ',' << success << ',' << time_taken << std::endl;
        success = run_with_timeout<bool>([&](bool &h){volatile int x = 0; for(int i = 0; i < ITERS; i++) { x += coinductive_sub::subtype(t1, t1, h);} return x;}, 10 * ONE_SECOND, res, time_taken);
        std::cout << "coinductive," << i << ',' << success << ',' << time_taken << std::endl;
    }

    // Unfolded
    auto unfold_rng = std::mt19937(42);
    for(int i = 0; i < 100; i++) {
        Type t1, t2;
        do {
            t1 = generate_random_type(1000, 4, unfold_rng, true, 2);
            // t1 = generate_random_isomorphic_type(100, unfold_rng);
            t2 = unfold_once(t1);
        } while (t2.nodes.size() == t1.nodes.size());
        // std::wcout << t1.to_string() << std::endl;

        bool res = false;
        long long time_taken = 0;
        bool success;
        const int ITERS = 10000;
        success = run_with_timeout<bool>([&](bool &h){volatile int x = 0; for(int i = 0; i < ITERS; i++) { x += inductive_sub::subtype(t1, t2, h);} return x; }, 10 * ONE_SECOND, res, time_taken);
        std::cout << "inductive," << i << ',' << success << ',' << time_taken << std::endl;
        success = run_with_timeout<bool>([&](bool &h){volatile int x = 0; for(int i = 0; i < ITERS; i++) { x += coinductive_sub::subtype(t1, t2, h);} return x; }, 10 * ONE_SECOND, res, time_taken);
        std::cout << "coinductive," << i << ',' << success << ',' << time_taken << std::endl;
    }
}
