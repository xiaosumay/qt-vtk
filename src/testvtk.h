#ifndef TESTVTK_H
#define TESTVTK_H

#include <QWidget>

#include <vtkObject.h>
#include <vtkSmartPointer.h>

#include "my_interactor_style.h"

/* clang-format off */
QT_BEGIN_NAMESPACE
namespace Ui { class TestVtk; }
QT_END_NAMESPACE
/* clang-format on */

class vtkRenderer;
class vtkEventQtSlotConnect;
class vtkGenericOpenGLRenderWindow;
class vtkMinimalStandardRandomSequence;

namespace lingxi::vtk
{
class MyInteractorStyle;
}

class TestVtk final : public QWidget
{
    Q_OBJECT

public:
    explicit TestVtk(QWidget *parent = nullptr);
    ~TestVtk() final;

    void AddCubeAt(double x, double y, double z);
    void RemoveCubeAt(vtkActor *actor);
    void ClearCubeAt(vtkActor *actor);

protected:
    void timerEvent(QTimerEvent *event) override;

private slots:
    void on_add_cube_clicked();
    void on_delete_cube_clicked();
    void on_load_pcl_clicked();

    void onStatusRenderer(bool);
    void onVtkLeftButtonPress(vtkObject *);
    void onVtkLeftButtonRelease(vtkObject *);
    void onVtkRightButtonPress(vtkObject *);
    void onVtkRightButtonRelease(vtkObject *);
    void onVtkMouseMove(vtkObject *);

private:
    Ui::TestVtk *ui;

    int _render_timer;
    lingxi::vtk::MyInteractorStyle _my_interactor_style;

    vtkSmartPointer<vtkMinimalStandardRandomSequence> _random_sequence;
    vtkSmartPointer<vtkRenderer> _renderer;
    vtkSmartPointer<vtkEventQtSlotConnect> _vtk_connect;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> _render_window;
};
#endif  // TESTVTK_H
