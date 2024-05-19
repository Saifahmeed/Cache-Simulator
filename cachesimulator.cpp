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
    unsigned int totalCycles;
    unsigned int totalAccesses;
    vector<Simulator> instructionCache;
    vector<Simulator> dataCache;
};

int AMAT(const CacheHierarchy &cacheHierarchy) { 
	return cacheHierarchy.totalCycles / cacheHierarchy.totalAccesses; 
}

void displayCacheStatus(const Simulator &ctrl, const CacheHierarchy &cacheHierarchy) {
	cout << std::fixed << std::setprecision(2);

    cout << "Cache Entry Info:" << endl;
    cout << "=================" << endl;
    cout << "Cache Statistics:" << endl;
    cout << "=================" << endl;
    for (auto &entry : ctrl.memory) {
    cout << "Valid bit: " << entry.validBit << "\t";
    cout << "Tag: " << entry.tag << endl;
}
    cout << "Total Accesses: " << ctrl.accessCount << endl;
    cout << "Hits: " << ctrl.hits << endl;
    cout << "Misses: " << ctrl.misses << endl;
    cout << "Hit Ratio: " << ctrl.hits/ctrl.accessCount * 100 << "%" << endl;
    cout << "Miss Ratio: " << ctrl.misses/ctrl.accessCount * 100 << "%" << endl;
    cout << "AMAT (Average Memory Access Time): " << AMAT(cacheHierarchy) << " cycles" << endl;
}


bool isPowerOfTwo(int x) {
    return (x > 0) && ((x & (x - 1)) == 0);
}
int main() {
    // input from user
    vector<unsigned int> blockSizes, cacheSizes, cycleTimes;
    unsigned int numLevels;
    do{ cout << "Specify the number of cache levels: ";
        cin >> numLevels;
        if (numLevels < 1){
        cout << "Error: Number of levels must be at least 1.\n";
    }}while (numLevels < 1) ;
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
    do{ cout << "Access cycles: ";
        cin >> cycles;
        if (cycles < 1 || cycles > 10) {
            cout << "Error: Access cycles must be between 1 and 10.\n";
        }} 
        while (cycles < 1 || cycles > 10);
        cacheSizes.push_back(size);
        blockSizes.push_back(block);
        cycleTimes.push_back(cycles);
    }

    //initializing Cache Hierarchy
    Simulator ctrl;
    CacheHierarchy cacheHierarchy;
    for (int i = 0; i < cacheSizes.size(); ++i) {
       	//Simulator ctrl;
        ctrl.accessCount = ctrl.hits = ctrl.misses = 0;
        ctrl.lineCount = cacheSizes[i] / blockSizes[i] ;
        ctrl.indexLength = log2(ctrl.lineCount); // assigns the number of bits required to represent the index part of the memory address in a cache line.
        ctrl.offsetLength = log2(blockSizes[i]);
    for (unsigned int j = 0; j < ctrl.lineCount; j++) {
         ctrl.memory.push_back(Line{});
    }// making size of memory equal to the number of lines in the cache. 
        ctrl.tagLength = 32 - ctrl.indexLength - ctrl.offsetLength; // assuming 32 bit memory address.
        for (auto &entry : ctrl.memory) {
        entry.validBit = false;
        entry.tag = 0;
    }
        cacheHierarchy.instructionCache.push_back(ctrl);
        cacheHierarchy.dataCache.push_back(ctrl);
    }
    cacheHierarchy.totalCycles = 0;
    cacheHierarchy.totalAccesses = 0;

    //reading Input File
    string fileName = "sequence.txt";
    ifstream inputFile(fileName);
    unsigned int address;
    char accessType;
   while (inputFile >> accessType >> address) {
    cacheHierarchy.totalAccesses++;
    int currentCycleTime = 0;
    bool foundInCache = false;
    int cacheLevel;  // to store what level you are in to calcualte miss penalty  
    vector<Simulator> &currentCaches = (accessType == 'I') ? cacheHierarchy.instructionCache : cacheHierarchy.dataCache;
        for (int i = 0; i < currentCaches.size(); ++i) {
        currentCycleTime += cycleTimes[i];
        unsigned int offsetMask = (1 << currentCaches[i].offsetLength) - 1;
        unsigned int addressWithoutOffset = address >> currentCaches[i].offsetLength;
        unsigned int indexMask = (1 << currentCaches[i].indexLength) - 1;
        unsigned int index = addressWithoutOffset & indexMask;
        unsigned int tag = address >> (currentCaches[i].indexLength + currentCaches[i].offsetLength);
        currentCaches[i].accessCount++;
        if (currentCaches[i].memory[index].validBit && currentCaches[i].memory[index].tag == tag) {
            currentCaches[i].hits++;
            cacheLevel = i;
            break;
        } else {
            currentCaches[i].misses++;
            currentCaches[i].memory[index].validBit = true;
            currentCaches[i].memory[index].tag = tag;
            // if you reach the main memory and still no hits, add 100 to cycle time
          if(i == currentCaches.size()-1) {
          	currentCycleTime += 100; 
		  }
        }
    }
    // total cycles is the access time of each input * the cache level you're in 
    cacheHierarchy.totalCycles += currentCycleTime*(cacheLevel+1); 
    displayCacheStatus(ctrl, cacheHierarchy);
    }
    inputFile.close();
    
    return 0;
}

