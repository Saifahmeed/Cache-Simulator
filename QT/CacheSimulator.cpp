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
    double hits;
    double misses;
    unsigned int cycleTime;
    unsigned int tagLength;
    unsigned int accessCount;
};

struct CacheHierarchy {
    unsigned int totalCycles;
    unsigned int totalAccesses;
    double totalHits;
    double totalMisses; 
    vector<Simulator> instructionCache;
    vector<Simulator> dataCache;
};

int AMAT(const CacheHierarchy &cacheHierarchy) { 
    return cacheHierarchy.totalCycles / cacheHierarchy.totalAccesses; 
}

void displayCacheStatus(const CacheHierarchy &cacheHierarchy) {
    cout << std::fixed << std::setprecision(2);
    cout << "Instruction Cache: " << endl;
    cout << "Cache Entry Info:" << endl;
    cout << "=================" << endl;
    for (const auto &sim : cacheHierarchy.instructionCache) {			// for every level of instruction cache
        for (const auto &entry : sim.memory) {							// for every entry in memory
           if (entry.validBit) {										// display only if valid bit is 1
                cout << "Valid bit: " << entry.validBit << "\t";
                cout << "Tag: " << entry.tag << endl;
            }
        }
    }
    cout << endl;
    cout << "Data Cache: " << endl;
    cout << "Cache Entry Info:" << endl;
    cout << "=================" << endl;
    for (const auto &sim : cacheHierarchy.dataCache) {
        for (const auto &entry : sim.memory) {
            if (entry.validBit) {
                cout << "Valid bit: " << entry.validBit << "\t";
                cout << "Tag: " << entry.tag << endl;
            }
        }
    }
  	cout << endl;
    cout << "Total Accesses: " << cacheHierarchy.totalAccesses << endl;
    cout << "Hit ratio " << cacheHierarchy.totalHits/cacheHierarchy.totalAccesses * 100 << "%" << endl;
    cout << "Miss ratio " << cacheHierarchy.totalMisses/cacheHierarchy.totalAccesses * 100 << "%" <<  endl;
    cout << "Cache Statistics:" << endl;
    cout << "=================" << endl;
    cout << "AMAT (Average Memory Access Time): " << AMAT(cacheHierarchy) << " cycles" << endl;
    cout << "==================\n" << endl;
}

bool isPowerOfTwo(int x) {
    return (x > 0) && ((x & (x - 1)) == 0);
}

int main() {
    vector<unsigned int> blockSizes, cacheSizes, cycleTimes;
    unsigned int numLevels;

    do { 
        cout << "Specify the number of cache levels: ";
        cin >> numLevels;
        if (numLevels < 1) {
            cout << "Error: Number of levels must be at least 1.\n";
        }
    } while (numLevels < 1);

    for (int i = 0; i < numLevels; ++i) {
        cout << "Enter details for level " << i + 1 << " Cache:\n";
        unsigned int size, block, cycles;
        do {
            cout << "Cache size (bytes): ";
            cin >> size;
            if (!isPowerOfTwo(size)) {
                cout << "Error: Cache size must be a power of 2.\n";
            }
        } while (!isPowerOfTwo(size));

        do {
            cout << "Block size (bytes): ";
            cin >> block;
            if (!isPowerOfTwo(block)) {
                cout << "Error: Block size must be a power of 2.\n";
            }
        } while (!isPowerOfTwo(block));

        do { 
            cout << "Access cycles: ";
            cin >> cycles;
            if (cycles < 1 || cycles > 10) {
                cout << "Error: Access cycles must be between 1 and 10.\n";
            }
        } while (cycles < 1 || cycles > 10);

        cacheSizes.push_back(size);
        blockSizes.push_back(block);
        cycleTimes.push_back(cycles);
    }

    CacheHierarchy cacheHierarchy;

    for (int i = 0; i < cacheSizes.size(); ++i) {
        Simulator sim;
        sim.accessCount = sim.hits = sim.misses = 0;
        sim.lineCount = cacheSizes[i] / blockSizes[i];
        sim.indexLength = log2(sim.lineCount);
        sim.offsetLength = log2(blockSizes[i]);
        sim.memory.resize(sim.lineCount, {false, 0});
        sim.tagLength = 32 - sim.indexLength - sim.offsetLength;

        cacheHierarchy.instructionCache.push_back(sim);
        cacheHierarchy.dataCache.push_back(sim);
    }

    cacheHierarchy.totalCycles = 0;
    cacheHierarchy.totalAccesses = 0;

    string fileName = "sequence.txt";
    ifstream inputFile(fileName);
    unsigned int address;
    char accessType;

    while (inputFile >> accessType >> address) {
        cacheHierarchy.totalAccesses++;
        int currentCycleTime = 0;
        int cacheLevel = 0;
        vector<Simulator> &currentCaches = (accessType == 'I') ? cacheHierarchy.instructionCache : cacheHierarchy.dataCache;

        for (int i = 0; i < currentCaches.size(); ++i) {
            currentCycleTime += cycleTimes[i];
            unsigned int offsetMask = (1 << currentCaches[i].offsetLength) - 1;
            unsigned int addressWithoutOffset = address >> currentCaches[i].offsetLength;
            unsigned int indexMask = (1 << currentCaches[i].indexLength) - 1;
            unsigned int index = addressWithoutOffset & indexMask;
            unsigned int tag = address >> (currentCaches[i].indexLength + currentCaches[i].offsetLength);

            cacheHierarchy.totalAccesses++;

            if (currentCaches[i].memory[index].validBit && currentCaches[i].memory[index].tag == tag) {
                  cacheHierarchy.totalHits++;
                cacheLevel = i;
                break;
            } else {
                 cacheHierarchy.totalMisses++;
                currentCaches[i].memory[index].validBit = true;
                currentCaches[i].memory[index].tag = tag;
                cacheLevel = i;
                if (i == currentCaches.size() - 1) {
                    currentCycleTime += 100;
                }
            }
        }
      //  cacheHierarchy.totalHits += currentCaches[cacheLevel].hits;					// get total number of hits from 
      //  cacheHierarchy.totalMisses += currentCaches[cacheLevel].misses;
        cacheHierarchy.totalCycles += currentCycleTime * (cacheLevel + 1);
        displayCacheStatus(cacheHierarchy);
    }

    inputFile.close();

    
   // displayCacheStatus(cacheHierarchy.dataCache, cacheHierarchy);

    return 0;
}

