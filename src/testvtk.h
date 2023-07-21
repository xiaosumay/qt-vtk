#ifndef TESTVTK_H
#define TESTVTK_H

#include <QWidget>
#include <vtkSmartPointer.h>

/* clang-format off */
QT_BEGIN_NAMESPACE
namespace Ui { class TestVtk; }
QT_END_NAMESPACE
/* clang-format on */

class vtkActor;
class vtkRenderer;
class vtkJPEGReader;
class vtkImageActor;
class vtkPolyDataMapper;
class vtkGenericOpenGLRenderWindow;

class TestVtk : public QWidget
{
    Q_OBJECT

public:
    explicit TestVtk(QWidget *parent = nullptr);
    ~TestVtk();

protected:
    void timerEvent(QTimerEvent *event) override;

private slots:
    void on_open_clicked();

private:
    Ui::TestVtk *ui;

    int _render_timer;

    vtkSmartPointer<vtkJPEGReader> _jpeg_reader;
    vtkSmartPointer<vtkImageActor> _image_actor;
    vtkSmartPointer<vtkPolyDataMapper> _poly_data_mapper;
    vtkSmartPointer<vtkRenderer> _renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> _render_window;
};
#endif  // TESTVTK_H
