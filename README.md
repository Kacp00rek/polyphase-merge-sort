**Polyphase Merge Sort**

A C++ implementation of the Polyphase Merge Sort algorithm, designed for efficient external sorting of records. This project demostrates how to sort large datasets that do not fit into RAM by using file-based storage and memory buffering.


**Project Structure**

The project is designed to work with a custom **Record** type (if the right methods are implemented). Default type is a **Circular Sector** which has two values: angle and radius and is sorted by its area.


**Fibonacci Strategy**

This algorithm calculates the required number of runs (non-decreasing sequences) for tapes (files) using the Fibonacci sequence. This ensures that at each phase, one tape is emptied while the others provide the perfect number of runs to continue the merge process without unnecessary phases.


**Performance Tracking**
After execution, the program outputs:
- READS: Total number of blocks read from disk,
- WRITES: Total number of blocks written to disk,
- PHASES: Number of merge passes required to sort the file


**Autor:** Kacper Grzelakowski  
**GitHub:** [github.com/Kacp00rek](https://github.com/Kacp00rek)