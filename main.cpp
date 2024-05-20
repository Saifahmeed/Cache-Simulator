#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <limits>

using namespace std;

struct Line {
    bool validBit;
    unsigned int tag;
};

struct Simulator {
    vector<Line> memory;
    unsigned int lineCount;
    unsigned int indexLength;
    unsigned int offsetLength;
    unsigned int cycleTime;
    unsigned int hits;
    unsigned int misses;
    unsigned int tagLength;
    unsigned int accessCount;
};

struct CacheHierarchy {
    vector<Simulator> instructionCache;
    vector<Simulator> dataCache;
    unsigned int totalAccesses;
    unsigned int instructionCount;
    unsigned int dataCount;
};


bool isPowerOfTwo(int x) {
    return (x > 0) && (x != 1) && ((x & (x - 1)) == 0);
}
int main() {
    int miss_penalty = 400;
    // clock rate = 4GHz (Clock Period = 0.25 ns) 
    // Main memory access time = 100ns
    // Mis penalty = 100 ns / 0.25 ns = 400 cycles
    unsigned int cacheSize, blockSize, cycleTimes, save_load;

    // Cache size input
    do {
        cout << "Cache size (bytes): ";
        cin >> cacheSize;
        if (!isPowerOfTwo(cacheSize)) {
            cout << "Error: Cache size must be a power of 2.\n";
        }
    } while (!isPowerOfTwo(cacheSize));

    // Block size input
    do {
        cout << "Block size (bytes): ";
        cin >> blockSize;
        if (!isPowerOfTwo(blockSize)) {
            cout << "Error: Block size must be a power of 2.\n";
        }
    } while (!isPowerOfTwo(blockSize));

    // Cycle time input
    do {
        cout << "Access cycles: ";
        cin >> cycleTimes;
        if (cycleTimes < 1 || cycleTimes > 10) {
            cout << "Error: Access cycles must be between 1 and 10.\n";
        }
    } while (cycleTimes < 1 || cycleTimes > 10);

    // Percentage of instructions that are load/store input
    do {
        cout << "% of instructions that are load/store: ";
        cin >> save_load;
        if (save_load < 0 || save_load > 100) {
            cout << "Error: Percentage should be between 0 and 100.\n";
        }
    } while (save_load < 0 || save_load > 100);

    // Cache hierarchy initialization
    CacheHierarchy cacheHierarchy;

    // Initialize instruction cache simulator
    Simulator instructionCache;
    instructionCache.accessCount = instructionCache.hits = instructionCache.misses = 0;
    instructionCache.lineCount = cacheSize / blockSize;
    instructionCache.indexLength = log2(instructionCache.lineCount);
    instructionCache.offsetLength = log2(blockSize);
    instructionCache.tagLength = 32 - instructionCache.indexLength - instructionCache.offsetLength;
    for (unsigned int j = 0; j < instructionCache.lineCount; j++) {
        instructionCache.memory.push_back(Line{});
    }

    // Initialize data cache simulator
    Simulator dataCache = instructionCache;

    cacheHierarchy.instructionCache.push_back(instructionCache);
    cacheHierarchy.dataCache.push_back(dataCache);
    cacheHierarchy.totalAccesses = 0;
    cacheHierarchy.instructionCount = 0;
    cacheHierarchy.dataCount = 0;

    // Process input file
string fileName = "sequence.txt";
ifstream inputFile(fileName);
unsigned int address;
char accessType;
int LineNumber = 1; 
while (inputFile >> accessType >> address) {
    cacheHierarchy.totalAccesses++;
    unsigned int currentCycleTime = 0;
    cout << "Total number of accesses: " << cacheHierarchy.totalAccesses << endl; 
    if (accessType == 'I') {
        cacheHierarchy.instructionCount++;
        for (auto& instructionCache : cacheHierarchy.instructionCache) {
            currentCycleTime = cycleTimes;
            unsigned int offsetMask = (1 << instructionCache.offsetLength) - 1;
            unsigned int addressWithoutOffset = address >> instructionCache.offsetLength;
            unsigned int indexMask = (1 << instructionCache.indexLength) - 1;
            unsigned int index = addressWithoutOffset & indexMask;
            unsigned int tag = address >> (instructionCache.indexLength + instructionCache.offsetLength);
            instructionCache.accessCount++;
            if (instructionCache.memory[index].validBit && instructionCache.memory[index].tag == tag) {
                instructionCache.hits++;
            } else {
                instructionCache.misses++;
                instructionCache.memory[index].validBit = true;
                instructionCache.memory[index].tag = tag;
            }
            // Display cache status after each access
            cout << "Instruction Cache - Line " << LineNumber << ": "
                 << (instructionCache.memory[index].validBit ? "Valid" : "Invalid")
                 << ", Tag: " << instructionCache.memory[index].tag << "\n";
            double hitRatio_ins = static_cast<double>(instructionCache.hits) / instructionCache.accessCount;
            double missRatio_ins = static_cast<double>(instructionCache.misses) / instructionCache.accessCount;
            cout << "Hit ratio: " << hitRatio_ins *100 << "%\n";
            cout << "Miss ratio: " << missRatio_ins *100 << "%\n";
            double amat = cycleTimes + (missRatio_ins* miss_penalty);
            cout << "Average Memory Access Time (AMAT): " << amat << " cycles\n";
            // Average Memory Access Time =  Hit time + (# instructions x (instruction miss rate x miss penalty) + # data x (% of instructions that are loads/save x Data miss rate x miss penalty).
        }
    } else {
        cacheHierarchy.dataCount++;
        for (auto& dataCache : cacheHierarchy.dataCache) {
            currentCycleTime = cycleTimes;
            unsigned int offsetMask = (1 << dataCache.offsetLength) - 1;
            unsigned int addressWithoutOffset = address >> dataCache.offsetLength;
            unsigned int indexMask = (1 << dataCache.indexLength) - 1;
            unsigned int index = addressWithoutOffset & indexMask;
            unsigned int tag = address >> (dataCache.indexLength + dataCache.offsetLength);
            dataCache.accessCount++;
            if (dataCache.memory[index].validBit && dataCache.memory[index].tag == tag) {
                dataCache.hits++;
            } else {
                dataCache.misses++;
                dataCache.memory[index].validBit = true;
                dataCache.memory[index].tag = tag;
            }
            // Display cache status after each access
            cout << "Data Cache - Slot " << LineNumber << ": "
                 << (dataCache.memory[index].validBit ? "Valid" : "Invalid")
                 << ", Tag: " << dataCache.memory[index].tag << "\n";
            double hitRatio_data = static_cast<double>(dataCache.hits) / dataCache.accessCount;
            double missRatio_data = static_cast<double>(dataCache.misses) / dataCache.accessCount;
            cout << "Hit ratio: " << hitRatio_data *100 << "%\n";
            cout << "Miss ratio: " << missRatio_data*100 << "%\n";
            double amat = cycleTimes + ((static_cast<double>(save_load) / 100 )*missRatio_data* miss_penalty);
            cout << "Average Memory Access Time (AMAT): " << amat << " cycles\n";
        }
    }
    LineNumber++;
}

    
    return 0;
}
