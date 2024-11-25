#ifndef SIMPLE_MULTITHREADER_H
#define SIMPLE_MULTITHREADER_H

#include <pthread.h>
#include <functional>
#include <chrono>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstring>

// Structure to track thread timing
struct ThreadTiming {
    int thread_id;
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
};

// Structure for 1D parallel execution
struct ThreadData1D {
    int thread_id;
    int num_threads;
    int low;
    int high;
    std::function<void(int)>* lambda;
    ThreadTiming* timing;
};

// Structure for 2D parallel execution
struct ThreadData2D {
    int thread_id;
    int num_threads;
    int low1;
    int high1;
    int low2;
    int high2;
    std::function<void(int, int)>* lambda;
    ThreadTiming* timing;
};

// Enhanced thread function for 1D parallel_for
void* execute_1d(void* arg) {
    if (!arg) {
        std::cerr << "Error: Null thread data" << std::endl;
        return nullptr;
    }
    ThreadData1D* data = (ThreadData1D*)arg;
    
    try {
        // Start timing for this thread
        data->timing->start_time = std::chrono::high_resolution_clock::now();
        
        if (data->lambda == nullptr) {
            throw std::runtime_error("Null lambda function");
        }
        
        int range = data->high - data->low;
        if (range <= 0 || data->num_threads <= 0) {
            throw std::runtime_error("Invalid range or thread count");
        }
        
        int chunk = range / data->num_threads;
        int start = data->low + data->thread_id * chunk;
        int end = (data->thread_id == data->num_threads - 1) ? data->high : start + chunk;
        
        for(int i = start; i < end; i++) {
            (*(data->lambda))(i);
        }
        
        // End timing for this thread
        data->timing->end_time = std::chrono::high_resolution_clock::now();
    } catch (const std::exception& e) {
        std::cerr << "Thread " << data->thread_id << " error: " << e.what() << std::endl;
    }
    return nullptr;
}

// Thread function for 2D parallel_for
void* execute_2d(void* arg) {
    ThreadData2D* data = (ThreadData2D*)arg;
    
    // Start timing for this thread
    data->timing->start_time = std::chrono::high_resolution_clock::now();
    
    int range1 = data->high1 - data->low1;
    int chunk1 = range1 / data->num_threads;
    int start1 = data->low1 + data->thread_id * chunk1;
    int end1 = (data->thread_id == data->num_threads - 1) ? data->high1 : start1 + chunk1;
    
    for(int i = start1; i < end1; i++) {
        for(int j = data->low2; j < data->high2; j++) {
            (*(data->lambda))(i, j);
        }
    }
    
    // End timing for this thread
    data->timing->end_time = std::chrono::high_resolution_clock::now();
    return nullptr;
}

// Function to print thread timings
void print_thread_timings(ThreadTiming* timings, int num_threads) {
    for(int i = 0; i < num_threads; i++) {
        auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(
            timings[i].end_time - timings[i].start_time);
        std::cout << "Execution time of Thread " << i << " : " 
                  << duration.count() << " seconds" << std::endl;
    }
}

// Enhanced 1D parallel_for implementation
void parallel_for(int low, int high, std::function<void(int)>&& lambda, int numThreads) {
    if (numThreads <= 0 || numThreads > 64) {
        throw std::runtime_error("Number of threads must be between 1 and 64");
    }
    if (low >= high) {
        throw std::runtime_error("Invalid range: low must be less than high");
    }
    if (high - low > std::numeric_limits<int>::max() / 2) {
        throw std::runtime_error("Range too large");
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    // Create thread handles and timing data
    pthread_t* threads = new pthread_t[numThreads - 1];
    ThreadData1D* thread_data = new ThreadData1D[numThreads];
    ThreadTiming* thread_timings = new ThreadTiming[numThreads];

    // Initialize timing data
    for(int i = 0; i < numThreads; i++) {
        thread_timings[i].thread_id = i;
    }

    // Create and launch threads
    for(int i = 0; i < numThreads - 1; i++) {
        thread_data[i] = {i, numThreads, low, high, &lambda, &thread_timings[i]};
        int err = pthread_create(&threads[i], nullptr, execute_1d, &thread_data[i]);
        if(err != 0) {
            std::string error_msg = "Thread creation failed: ";
            error_msg += std::strerror(err);
            delete[] threads;
            delete[] thread_data;
            delete[] thread_timings;
            throw std::runtime_error(error_msg);
        }
    }

    // Main thread work
    thread_data[numThreads-1] = {numThreads-1, numThreads, low, high, &lambda, &thread_timings[numThreads-1]};
    execute_1d(&thread_data[numThreads-1]);

    // Wait for all threads
    for(int i = 0; i < numThreads - 1; i++) {
        int err = pthread_join(threads[i], nullptr);
        if(err != 0) {
            std::cerr << "Warning: Thread " << i << " join failed: " 
                      << std::strerror(err) << std::endl;
        }
    }

    // Print individual thread timings
    print_thread_timings(thread_timings, numThreads);

    // Print total execution time
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time);
    std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;

    delete[] threads;
    delete[] thread_data;
    delete[] thread_timings;
}

// 2D parallel_for implementation
void parallel_for(int low1, int high1, int low2, int high2, 
                 std::function<void(int, int)>&& lambda, int numThreads) {
    if(numThreads <= 0 || low1 >= high1 || low2 >= high2) {
        std::cerr << "Error: Invalid arguments for parallel_for" << std::endl;
        return;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    // Create thread handles and timing data
    pthread_t* threads = new pthread_t[numThreads - 1];
    ThreadData2D* thread_data = new ThreadData2D[numThreads];
    ThreadTiming* thread_timings = new ThreadTiming[numThreads];

    // Initialize timing data
    for(int i = 0; i < numThreads; i++) {
        thread_timings[i].thread_id = i;
    }

    // Create and launch threads
    for(int i = 0; i < numThreads - 1; i++) {
        thread_data[i] = {i, numThreads, low1, high1, low2, high2, &lambda, &thread_timings[i]};
        if(pthread_create(&threads[i], nullptr, execute_2d, &thread_data[i]) != 0) {
            std::cerr << "Error creating thread " << i << std::endl;
            delete[] threads;
            delete[] thread_data;
            delete[] thread_timings;
            return;
        }
    }

    // Main thread work
    thread_data[numThreads-1] = {numThreads-1, numThreads, low1, high1, low2, high2, &lambda, &thread_timings[numThreads-1]};
    execute_2d(&thread_data[numThreads-1]);

    // Wait for all threads
    for(int i = 0; i < numThreads - 1; i++) {
        pthread_join(threads[i], nullptr);
    }

    // Print individual thread timings
    print_thread_timings(thread_timings, numThreads);

    // Print total execution time
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time);
    std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;

    delete[] threads;
    delete[] thread_data;
    delete[] thread_timings;
}

#endif // SIMPLE_MULTITHREADER_H