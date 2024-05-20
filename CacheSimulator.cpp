#include "CacheSimulator.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>
#include <cmath>
#include <limits>

CacheSimulator::CacheSimulator(QWidget *parent) : QWidget(parent)
{
    QLabel *cacheSizeLabel = new QLabel("Cache size (bytes):");
    cacheSizeEdit = new QLineEdit;

    QLabel *blockSizeLabel = new QLabel("Block size (bytes):");
    blockSizeEdit = new QLineEdit;

    QLabel *cycleTimesLabel = new QLabel("Access cycles:");
    cycleTimesEdit = new QLineEdit;

    QLabel *saveLoadLabel = new QLabel("% of instructions that are load/store:");
    saveLoadEdit = new QLineEdit;

    startButton = new QPushButton("Start Simulation");
    connect(startButton, &QPushButton::clicked, this, &CacheSimulator::startSimulation);

    outputTextEdit = new QTextEdit;
    outputTextEdit->setReadOnly(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *inputLayout1 = new QHBoxLayout;
    inputLayout1->addWidget(cacheSizeLabel);
    inputLayout1->addWidget(cacheSizeEdit);

    QHBoxLayout *inputLayout2 = new QHBoxLayout;
    inputLayout2->addWidget(blockSizeLabel);
    inputLayout2->addWidget(blockSizeEdit);

    QHBoxLayout *inputLayout3 = new QHBoxLayout;
    inputLayout3->addWidget(cycleTimesLabel);
    inputLayout3->addWidget(cycleTimesEdit);

    QHBoxLayout *inputLayout4 = new QHBoxLayout;
    inputLayout4->addWidget(saveLoadLabel);
    inputLayout4->addWidget(saveLoadEdit);

    mainLayout->addLayout(inputLayout1);
    mainLayout->addLayout(inputLayout2);
    mainLayout->addLayout(inputLayout3);
    mainLayout->addLayout(inputLayout4);
    mainLayout->addWidget(startButton);
    mainLayout->addWidget(outputTextEdit);

    setLayout(mainLayout);
    setWindowTitle("Cache Simulator");
}

bool CacheSimulator::isPowerOfTwo(int x)
{
    return (x > 0) && (x != 1) && ((x & (x - 1)) == 0);
}

void CacheSimulator::startSimulation()
{
    bool ok;
    unsigned int cacheSize = cacheSizeEdit->text().toUInt(&ok);
    if (!ok || !isPowerOfTwo(cacheSize)) {
        QMessageBox::critical(this, "Error", "Cache size must be a power of 2.");
        return;
    }

    unsigned int blockSize = blockSizeEdit->text().toUInt(&ok);
    if (!ok || !isPowerOfTwo(blockSize)) {
        QMessageBox::critical(this, "Error", "Block size must be a power of 2.");
        return;
    }

    unsigned int cycleTimes = cycleTimesEdit->text().toUInt(&ok);
    if (!ok || cycleTimes < 1 || cycleTimes > 10) {
        QMessageBox::critical(this, "Error", "Access cycles must be between 1 and 10.");
        return;
    }

    unsigned int saveLoad = saveLoadEdit->text().toUInt(&ok);
    if (!ok || saveLoad < 0 || saveLoad > 100) {
        QMessageBox::critical(this, "Error", "Percentage should be between 0 and 100.");
        return;
    }

    // Cache hierarchy initialization
    cacheHierarchy = {};

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
    cacheHierarchy.totalCycles = 0;
    cacheHierarchy.totalAccesses = 0;
    cacheHierarchy.instructionCount = 0;
    cacheHierarchy.dataCount = 0;

    int miss_penalty = 400;
    // Check current working directory
    QString currentDir = QDir::currentPath();
  //  outputTextEdit->append("Current working directory: " + currentDir);

    // Use absolute path for the file
    QString fileName = currentDir + "/sequence.txt";
    QFile inputFile(fileName);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
     //   outputTextEdit->append("Error: Unable to open file " + fileName);
        QMessageBox::critical(this, "Error", "Unable to open sequence.txt");
        return;
    }

    QTextStream in(&inputFile);
    QString line;
    unsigned int lineNumber = 1;
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.isEmpty()) {
            continue;
        }
        QChar accessType = line.at(0);
        unsigned int address = line.mid(2).toUInt(&ok, 10); // Assume decimal input
        if (!ok) {
            outputTextEdit->append("Error parsing address: " + line);
            continue;
        }

        cacheHierarchy.totalAccesses++;
        unsigned int currentCycleTime = 0;
        outputTextEdit->append("Total number of accesses: " + QString::number(cacheHierarchy.totalAccesses));

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
                outputTextEdit->append("Instruction Cache - Line " + QString::number(lineNumber) + ": "
                                      + (instructionCache.memory[index].validBit ? "Valid" : "Invalid")
                                      + ", Tag: " + QString::number(instructionCache.memory[index].tag));
                double hitRatio_ins = static_cast<double>(instructionCache.hits) / instructionCache.accessCount;
                double missRatio_ins = static_cast<double>(instructionCache.misses) / instructionCache.accessCount;
                outputTextEdit->append("Hit ratio: " + QString::number(hitRatio_ins * 100) + "%");
                outputTextEdit->append("Miss ratio: " + QString::number(missRatio_ins * 100) + "%");
                double amat = cycleTimes + (missRatio_ins * miss_penalty);
                outputTextEdit->append("Average Memory Access Time (AMAT): " + QString::number(amat) + " cycles");
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
                outputTextEdit->append("Data Cache - Line " + QString::number(lineNumber) + ": "
                                      + (dataCache.memory[index].validBit ? "Valid" : "Invalid")
                                      + ", Tag: " + QString::number(dataCache.memory[index].tag));
                double hitRatio_data = static_cast<double>(dataCache.hits) / dataCache.accessCount;
                double missRatio_data = static_cast<double>(dataCache.misses) / dataCache.accessCount;
                outputTextEdit->append("Hit ratio: " + QString::number(hitRatio_data * 100) + "%");
                outputTextEdit->append("Miss ratio: " + QString::number(missRatio_data * 100) + "%");
                double amat = cycleTimes + ((static_cast<double>(saveLoad) / 100) * missRatio_data * miss_penalty);
                outputTextEdit->append("Average Memory Access Time (AMAT): " + QString::number(amat) + " cycles");
            }
        }
        lineNumber++;
    }
}

