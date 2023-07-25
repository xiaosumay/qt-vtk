#ifndef TESTVTK_H
#define TESTVTK_H

#include <QWidget>
#include "vtkActor.h"
#include <vtkSmartPointer.h>

/* clang-format off */
QT_BEGIN_NAMESPACE
namespace Ui { class TestVtk; }
QT_END_NAMESPACE
/* clang-format on */

class vtkRenderer;
class vtkGenericOpenGLRenderWindow;
class vtkMinimalStandardRandomSequence;

namespace lingxi::vtk
{
class MyInteractorStyle;
}

class TestVtk : public QWidget
{
    Q_OBJECT

public:
    explicit TestVtk(QWidget *parent = nullptr);
    ~TestVtk();

protected:
    void timerEvent(QTimerEvent *event) override;

private slots:
    void on_add_cube_clicked();
    void on_delete_cube_clicked();

    void onStatusRenderer(bool);

private:
    Ui::TestVtk *ui;

    int _render_timer;

    vtkSmartPointer<vtkMinimalStandardRandomSequence> _random_sequence;
    vtkSmartPointer<vtkRenderer> _renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> _render_window;
    vtkSmartPointer<lingxi::vtk::MyInteractorStyle> _my_interactor_style;
};
#endif  // TESTVTK_H
