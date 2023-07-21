#include "testvtk.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QTranslator>
#include <QLocale>
#include <QDebug>

#include <QVTKOpenGLNativeWidget.h>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingContextOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

static void OutputRedirection(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray _msg = msg.toLocal8Bit();
    switch (type)
    {
    case QtDebugMsg:
        fprintf(stdout, "Debug: %s (%s:%u, %s)\n", _msg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stdout, "Info: %s (%s:%u, %s)\n", _msg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stdout, "Warning: %s (%s:%u, %s)\n", _msg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stdout, "Critical: %s (%s:%u, %s)\n", _msg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stdout, "Fatal: %s (%s:%u, %s)\n", _msg.constData(), context.file, context.line, context.function);
        abort();
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(OutputRedirection);

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    vtkObject::GlobalWarningDisplayOff();

    QApplication a(argc, argv);

    QTranslator translator;
    QLocale locale;
    if (locale.language() == QLocale::Chinese)
    {
        bool load = translator.load(QStringLiteral("test-vtk_zh_CN.qm"));
        if (load)
        {
            qDebug() << QStringLiteral("load success");
            QApplication::installTranslator(&translator);
        }
        else
        {
            qDebug() << QStringLiteral("load failed!");
        }
    }

    TestVtk w;
    w.show();

    return a.exec();
}
