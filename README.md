# Polyphase Sort Implementation
A C++ implementation of the Polyphase Merge Sort algorithm, designed for efficient external sorting of records. This project demostrates how to sort large datasets that do not fit into RAM by using file-based storage and memory buffering.

[See the detailed project report](report/Sprawozdanie.pdf)

## Features
- **External Sorting**: Designed to handle data sets larger than available RAM by using file-based "tapes"
- **Fibonacci Distribution**: Automatically calculates the optimal initial distribution of runs
- **Dummy Run Handling**: Correctly manages cases where the initial number of runs isn't a perfect Fibonacci number by utilizing "dummy" runs
- **Performance Tracking**: Detailed statistics for
    - Total Disk Reads (**R**)
    - Total Disk Writes (**W**)
    - Number of Merge Phases (**k**) 

## Key Concepts
To understand how this algorithm works, you need to know two terms:
- **Run (or Series)**: A sequence of records that are already sorted.
    - **Example (Ascending)**: In the sequence **10, 25, 5, 8, 12, 3**, there are 3 runs: **[10, 25]**, **[5, 8, 12]**, and **[3]**
    - The algorithm ends when all runs are merged into exactly one
- **Tape**: A file used as temporary external storage. The name refers to the sequential nature of the data access—much like historical magnetic tapes, the algorithm reads and writes records linearly without random access.

## How It Works
1. **Initial Distribution**

   The program counts the initial runs in the input file and calculates the nearest Fibonacci numbers. It then distributes these runs across two files (tapes) such that the ratio of runs follows the golden ratio.

2. **Merging Phases**

   In each phase, the algorithm merges runs from the two active source tapes into a single destination tape. The number of runs merged is determined by the "shorter" tape. This process repeats until only one sorted run remains.

3. **I/O Optimization**

   The implementation uses a Blocking Factor (**b**), allowing the program to read/write multiple records in a single disk operation, significantly reducing overhead.

## Theoretical Model vs. Results
   The project includes a mathematical analysis of the algorithm's complexity:
   - **Number of Phases (k)**: $\approx 1.44 \log_{2}r$ (where **r** is the numberof initial runs)
   - **Disk Operations**: $$\frac{2N}{b}(\alpha k+1)$$ (where **$\alpha \approx 0.72$** represents the average proportion of records participating in each merge phase)

**Experiment Result**: For datasets exceeding 1 million records, the experimental results achieved a 99% match with the theoretical model.
   
## Usage
**Prerequisites**

- C++17 or higher
- The project is designed to work with a custom **Record** type (if the right methods are implemented). Default type is a **Circular Sector** which has two values: angle and radius and is sorted by its area.

**Running the Program**

1. **Blocking Factor**: Enter how many records to process per block
2. **Data Source**: Choose to generate random data or enter records manually
3. **Sorting Order**: Select Ascending or Descending
4. **Logging**: Toggle phase-by-phase visualization of tape contents

```bash
g++ -O3 main.cpp -o main
./main
```

**Autor:** Kacper Grzelakowski  
**GitHub:** [github.com/Kacp00rek](https://github.com/Kacp00rek)
