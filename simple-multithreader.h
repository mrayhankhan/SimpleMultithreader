#ifndef SIMPLE_MULTITHREADER_H
#define SIMPLE_MULTITHREADER_H

#include <pthread.h>
#include <functional>
#include <chrono>
#include <iostream>

// Structure for 1D parallel execution
struct ThreadData1D {
    int thread_id;
    int num_threads;
    int low;
    int high;
    std::function<void(int)>* lambda;
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
};

// Thread function for 1D parallel_for
void* execute_1d(void* arg) {
    ThreadData1D* data = (ThreadData1D*)arg;
    int range = data->high - data->low;
    int chunk = range / data->num_threads;
    int start = data->low + data->thread_id * chunk;
    int end = (data->thread_id == data->num_threads - 1) ? data->high : start + chunk;
    
    for(int i = start; i < end; i++) {
        (*(data->lambda))(i);
    }
    return nullptr;
}

// Thread function for 2D parallel_for
void* execute_2d(void* arg) {
    ThreadData2D* data = (ThreadData2D*)arg;
    int range1 = data->high1 - data->low1;
    int chunk1 = range1 / data->num_threads;
    int start1 = data->low1 + data->thread_id * chunk1;
    int end1 = (data->thread_id == data->num_threads - 1) ? data->high1 : start1 + chunk1;
    
    for(int i = start1; i < end1; i++) {
        for(int j = data->low2; j < data->high2; j++) {
            (*(data->lambda))(i, j);
        }
    }
    return nullptr;
}

// 1D parallel_for implementation
void parallel_for(int low, int high, std::function<void(int)>&& lambda, int numThreads) {
    if(numThreads <= 0 || low >= high) {
        std::cerr << "Error: Invalid arguments for parallel_for" << std::endl;
        return;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    // Create thread handles
    pthread_t* threads = new pthread_t[numThreads - 1];  // -1 as main thread will be used
    ThreadData1D* thread_data = new ThreadData1D[numThreads];

    // Create and launch threads
    for(int i = 0; i < numThreads - 1; i++) {
        thread_data[i] = {i, numThreads, low, high, &lambda};
        if(pthread_create(&threads[i], nullptr, execute_1d, &thread_data[i]) != 0) {
            std::cerr << "Error creating thread " << i << std::endl;
            delete[] threads;
            delete[] thread_data;
            return;
        }
    }

    // Main thread work
    thread_data[numThreads-1] = {numThreads-1, numThreads, low, high, &lambda};
    execute_1d(&thread_data[numThreads-1]);

    // Wait for all threads
    for(int i = 0; i < numThreads - 1; i++) {
        pthread_join(threads[i], nullptr);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Parallel execution time: " << duration.count() << " microseconds" << std::endl;

    delete[] threads;
    delete[] thread_data;
}

// 2D parallel_for implementation
void parallel_for(int low1, int high1, int low2, int high2, 
                 std::function<void(int, int)>&& lambda, int numThreads) {
    if(numThreads <= 0 || low1 >= high1 || low2 >= high2) {
        std::cerr << "Error: Invalid arguments for parallel_for" << std::endl;
        return;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    // Create thread handles
    pthread_t* threads = new pthread_t[numThreads - 1];  // -1 as main thread will be used
    ThreadData2D* thread_data = new ThreadData2D[numThreads];

    // Create and launch threads
    for(int i = 0; i < numThreads - 1; i++) {
        thread_data[i] = {i, numThreads, low1, high1, low2, high2, &lambda};
        if(pthread_create(&threads[i], nullptr, execute_2d, &thread_data[i]) != 0) {
            std::cerr << "Error creating thread " << i << std::endl;
            delete[] threads;
            delete[] thread_data;
            return;
        }
    }

    // Main thread work
    thread_data[numThreads-1] = {numThreads-1, numThreads, low1, high1, low2, high2, &lambda};
    execute_2d(&thread_data[numThreads-1]);

    // Wait for all threads
    for(int i = 0; i < numThreads - 1; i++) {
        pthread_join(threads[i], nullptr);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Parallel execution time: " << duration.count() << " microseconds" << std::endl;

    delete[] threads;
    delete[] thread_data;
}

#endif // SIMPLE_MULTITHREADER_H