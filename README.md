# SimpleMultithreader Design Document

## 1. Project Structure
```
./
├── simple-multithreader.h    # Main implementation header
├── vector.cpp                # 1D parallel example
├── matrix.cpp                # 2D parallel example
└── Makefile                 
```

## 2. Implementation Details

### 2.1 Key Components

**Data Structures:**
```cpp
// For 1D parallel operations
struct ThreadData1D {
    int thread_id;
    int num_threads;
    int low, high;
    std::function<void(int)>* lambda;
};

// For 2D parallel operations
struct ThreadData2D {
    int thread_id;
    int num_threads;
    int low1, high1, low2, high2;
    std::function<void(int, int)>* lambda;
};
```

**Core Functions:**
1. `parallel_for` (1D): Parallelizes single-dimension operations
2. `parallel_for` (2D): Parallelizes two-dimension operations

### 2.2 Key Features
- Creates new threads for each parallel_for call (no thread pooling)
- Main thread participates in computation
- Even work distribution among threads
- Built-in execution time measurement
- Error handling for invalid inputs

## 3. Setup & Usage

### 3.1 Requirements
- Linux OS
- G++ with C++11
- pthread library

### 3.2 Build & Run
```bash
# Build both examples
make

# Run vector example
./vector [num_threads] [size]
Example: ./vector 4 1000000

# Run matrix example
./matrix [num_threads] [size]
Example: ./matrix 4 1024
```

## 4. Implementation Choices

1. **Thread Management:**
   - New threads created per call
   - Main thread used as worker
   - Automatic cleanup after execution

2. **Work Distribution:**
   - 1D: Chunks divided by thread count
   - 2D: Outer loop divided among threads

3. **Error Handling:**
   - Range validation
   - Thread creation checks
   - Resource cleanup on failure

## 5. Testing
- Vector addition (1D parallel test)
- Matrix multiplication (2D parallel test)
- Both examples include correctness verification