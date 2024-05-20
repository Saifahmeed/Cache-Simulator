#include <QApplication>
#include "CacheSimulator.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    CacheSimulator cacheSim;
    cacheSim.show();
    return app.exec();
}
