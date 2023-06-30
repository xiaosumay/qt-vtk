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
class vtkGenericOpenGLRenderWindow;

class vtkCylinderSource;
class vtkPolyDataMapper;

class TestVtk : public QWidget
{
    Q_OBJECT

public:
    TestVtk(QWidget *parent = nullptr);
    ~TestVtk();

private slots:
    void on_open_clicked();

private:
    Ui::TestVtk *ui;

    vtkSmartPointer<vtkActor> m_cylinderActor;
    vtkSmartPointer<vtkCylinderSource> m_cylinderSource;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
};
#endif  // TESTVTK_H
