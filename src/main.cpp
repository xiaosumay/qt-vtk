#include "testvtk.h"

#include <QApplication>
#include <QSurfaceFormat>

#include <QVTKOpenGLNativeWidget.h>

int main(int argc, char *argv[])
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication a(argc, argv);

    TestVtk w;
    w.show();
    return a.exec();
}
