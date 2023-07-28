#ifndef TESTVTK_H
#define TESTVTK_H

#include <QWidget>
#include <QLocale>

#include <vtkObject.h>
#include <vtkSmartPointer.h>

#include "src/core/selected_actor_mgr.h"

/* clang-format off */
QT_BEGIN_NAMESPACE
namespace Ui { class TestVtk; }
QT_END_NAMESPACE
/* clang-format on */

class vtkRenderer;
class vtkEventQtSlotConnect;
class vtkGenericOpenGLRenderWindow;
class vtkMinimalStandardRandomSequence;

class TestVtk final : public QWidget
{
    Q_OBJECT

    typedef TestVtk Self;

public:
    explicit TestVtk(QWidget *parent = nullptr);
    ~TestVtk() final;

    void AddCubeAt(double x, double y, double z);
    void RemoveCubeAt(vtkActor *actor);
    void ClearCubeAt(vtkActor *actor);

    void onSingleActorClicked(vtkObject *, unsigned long, void *);
    void onActorMoveDelta(vtkObject *, unsigned long, void *);
    void onSelectedArea(vtkObject *, unsigned long, void *);
    void onRightButtonUp(vtkObject *, unsigned long, void *);

private slots:
    void on_add_cube_clicked();
    void on_delete_cube_clicked();
    void on_load_pcl_clicked();

    void onPclLoadFinished(void *p);

private:
    void handlePclFile(QString file_path);

private:
    Ui::TestVtk *ui;

    int _render_timer;
    lingxi::vtk::SelectedActorMgr _selected_actor_mgr;

    vtkSmartPointer<vtkMinimalStandardRandomSequence> _random_sequence;
    vtkSmartPointer<vtkRenderer> _renderer;
    vtkSmartPointer<vtkEventQtSlotConnect> _vtk_connect;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> _render_window;
};
#endif  // TESTVTK_H
