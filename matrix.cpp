#include "simple-multithreader.h"
#include <assert.h>
#include <limits>

int main(int argc, char** argv) {
  try {
    // Validate command line arguments
    if (argc > 1 && (atoi(argv[1]) <= 0 || atoi(argv[1]) > 64)) {
      throw std::runtime_error("Number of threads must be between 1 and 64");
    }
    if (argc > 2 && (atoi(argv[2]) <= 0 || atoi(argv[2]) > 16384)) {
      throw std::runtime_error("Matrix size must be between 1 and 16384");
    }

    // Initialize problem size
    int numThread = argc > 1 ? atoi(argv[1]) : 2;
    int size = argc > 2 ? atoi(argv[2]) : 1024;  

    // Allocate matrices with error checking
    int **A = nullptr, **B = nullptr, **C = nullptr;
    try {
      A = new int*[size];
      B = new int*[size];
      C = new int*[size];
      
      // Initialize matrices in parallel with error checking
      parallel_for(0, size, [=](int i) {
        try {
          A[i] = new int[size];
          B[i] = new int[size];
          C[i] = new int[size];
        } catch (const std::bad_alloc& e) {
          throw std::runtime_error("Memory allocation failed for row " + std::to_string(i));
        }
        for(int j = 0; j < size; j++) {
          // Initialize the matrices
          std::fill(A[i], A[i] + size, 1);
          std::fill(B[i], B[i] + size, 1);
          std::fill(C[i], C[i] + size, 0);
        }
      }, numThread);

    } catch (const std::exception& e) {
      // Cleanup on allocation failure
      if (A) {
        for(int i = 0; i < size; i++) delete[] A[i];
        delete[] A;
      }
      if (B) {
        for(int i = 0; i < size; i++) delete[] B[i];
        delete[] B;
      }
      if (C) {
        for(int i = 0; i < size; i++) delete[] C[i];
        delete[] C;
      }
      throw;
    }

    // Start the parallel multiplication of two matrices
    parallel_for(0, size, 0, size, [&](int i, int j) {
      for(int k = 0; k < size; k++) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }, numThread);

    // Verify results with bounds checking
    try {
      for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
          if(C[i][j] != size) {
            throw std::runtime_error("Verification failed at position (" + 
                                   std::to_string(i) + "," + std::to_string(j) + ")");
          }
        }
      }
      printf("Test Success.\n");
    } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
      // Cleanup memory
      parallel_for(0, size, [=](int i) {
        delete [] A[i];
        delete [] B[i];
        delete [] C[i];
      }, numThread);
      delete[] A;
      delete[] B;
      delete[] C;
      return 1;
    }

    // Cleanup memory
    parallel_for(0, size, [=](int i) {
      delete [] A[i];
      delete [] B[i];
      delete [] C[i];
    }, numThread);
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
