#include "testvtk.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TestVtk w;
    w.show();
    return a.exec();
}
