#include "simple-multithreader.h"
#include <assert.h>
#include <limits>

int main(int argc, char** argv) {
  try {
    // Validate command line arguments
    if (argc > 1 && (atoi(argv[1]) <= 0 || atoi(argv[1]) > 64)) {
      throw std::runtime_error("Number of threads must be between 1 and 64");
    }
    if (argc > 2 && (atoi(argv[2]) <= 0 || atoi(argv[2]) > std::numeric_limits<int>::max())) {
      throw std::runtime_error("Vector size must be positive and within integer limits");
    }

    // Initialize problem size
    int numThread = argc > 1 ? atoi(argv[1]) : 2;
    int size = argc > 2 ? atoi(argv[2]) : 48000000;  

    // Allocate vectors with error checking
    int *A = nullptr, *B = nullptr, *C = nullptr;
    try {
      A = new int[size];
      B = new int[size];
      C = new int[size];
    } catch (const std::bad_alloc& e) {
      std::cerr << "Memory allocation failed: " << e.what() << std::endl;
      delete[] A; delete[] B; delete[] C;
      return 1;
    }

    // Initialize the vectors
    std::fill(A, A + size, 1);
    std::fill(B, B + size, 1);
    std::fill(C, C + size, 0);

    // Start the parallel addition of two vectors
    parallel_for(0, size, [&](int i) {
      C[i] = A[i] + B[i];
    }, numThread);

    // Verify the result vector with bounds checking
    try {
      for(int i = 0; i < size; i++) {
        if (C[i] != 2) {
          throw std::runtime_error("Verification failed at index " + std::to_string(i));
        }
      }
      printf("Test Success\n");
    } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
      delete[] A; delete[] B; delete[] C;
      return 1;
    }

    // Cleanup memory
    delete[] A; delete[] B; delete[] C;
    return 0;

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
