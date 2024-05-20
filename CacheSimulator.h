#ifndef CACHESIMULATOR_H
#define CACHESIMULATOR_H

#include <QWidget>
#include <vector>

class QLabel;
class QLineEdit;
class QPushButton;
class QTextEdit;

struct Line {
    bool validBit;
    unsigned int tag;
};

struct Simulator {
    std::vector<Line> memory;
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
    std::vector<Simulator> instructionCache;
    std::vector<Simulator> dataCache;
    unsigned int totalCycles;
    unsigned int totalAccesses;
    unsigned int instructionCount;
    unsigned int dataCount;
};

class CacheSimulator : public QWidget
{
    Q_OBJECT

public:
    CacheSimulator(QWidget *parent = nullptr);

private slots:
    void startSimulation();

private:
    bool isPowerOfTwo(int x);

    QLineEdit *cacheSizeEdit;
    QLineEdit *blockSizeEdit;
    QLineEdit *cycleTimesEdit;
    QLineEdit *saveLoadEdit;
    QPushButton *startButton;
    QTextEdit *outputTextEdit;

    CacheHierarchy cacheHierarchy;
};

#endif // CACHESIMULATOR_H
