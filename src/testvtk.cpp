#include "testvtk.h"
#include "qnamespace.h"
#include "ui_testvtk.h"

#include "version_internal.h"

#include <chrono>

#include <QFileDialog>
#include <QDebug>
#include <QStandardPaths>
#include <QMenu>

#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include <vtkPointData.h>

#include <vtkPolyLine.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkCamera.h>
#include <vtkAxesActor.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPointPicker.h>

#include "selected_actor_mgr.h"

using namespace std::chrono_literals;
using namespace lingxi::vtk;

TestVtk::TestVtk(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TestVtk{})
    , _render_timer(-1)
{
    ui->setupUi(this);
    setWindowTitle(tr("Test VTK") + QStringLiteral(" " TEST_VTK_VERSION_FULL));

    _random_sequence = vtkSmartPointer<vtkMinimalStandardRandomSequence>::New();
    _random_sequence->SetSeed(8775070);

    _renderer = vtkSmartPointer<vtkRenderer>::New();
    _renderer->SetBackground(0.1, 0.2, 0.4);

    _render_window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    _render_window->AddRenderer(_renderer);

    ui->vtkWidget->SetRenderWindow(_render_window);

    auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    style->SetDefaultRenderer(_renderer);

    auto inter = ui->vtkWidget->GetInteractor();
    inter->SetInteractorStyle(style);

    _my_interactor_style.SetInteractor(inter);

    inter->RemoveObservers(vtkCommand::LeftButtonPressEvent);
    inter->RemoveObservers(vtkCommand::LeftButtonReleaseEvent);
    inter->RemoveObservers(vtkCommand::RightButtonPressEvent);
    inter->RemoveObservers(vtkCommand::RightButtonReleaseEvent);
    inter->RemoveObservers(vtkCommand::MouseMoveEvent);

    _vtk_connect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    _vtk_connect->Connect(inter, vtkCommand::LeftButtonPressEvent, this, SLOT(onVtkLeftButtonPress(vtkObject*)));
    _vtk_connect->Connect(inter, vtkCommand::LeftButtonReleaseEvent, this, SLOT(onVtkLeftButtonRelease(vtkObject*)));
    _vtk_connect->Connect(inter, vtkCommand::RightButtonPressEvent, this, SLOT(onVtkRightButtonPress(vtkObject*)));
    _vtk_connect->Connect(inter, vtkCommand::RightButtonReleaseEvent, this, SLOT(onVtkRightButtonRelease(vtkObject*)));
    _vtk_connect->Connect(inter, vtkCommand::MouseMoveEvent, this, SLOT(onVtkMouseMove(vtkObject*)));

    auto axes_actor = vtkSmartPointer<vtkAxesActor>::New();
    axes_actor->SetTotalLength(10, 10, 10);
    axes_actor->SetPickable(false);
    axes_actor->SetAxisLabels(false);

    _renderer->AddActor(axes_actor);

    onStatusRenderer(true);
}

TestVtk::~TestVtk()
{
    onStatusRenderer(false);

    delete ui;
}

void TestVtk::on_add_cube_clicked()
{
    auto source = vtkSmartPointer<vtkSphereSource>::New();

    double x, y, z, radius;
    // random position and radius
    x = _random_sequence->GetRangeValue(-5.0, 5.0);
    _random_sequence->Next();
    y = _random_sequence->GetRangeValue(-5.0, 5.0);
    _random_sequence->Next();
    z = _random_sequence->GetRangeValue(-5.0, 5.0);
    _random_sequence->Next();
    radius = _random_sequence->GetRangeValue(0.5, 1.0);
    _random_sequence->Next();

    source->SetRadius(radius);
    source->SetCenter(x, y, z);
    source->SetPhiResolution(11);
    source->SetThetaResolution(21);

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(source->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    _renderer->AddActor(actor);

    _renderer->ResetCamera();
}

void TestVtk::on_delete_cube_clicked()
{
    _my_interactor_style.RemoveSelected();
}

void TestVtk::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == _render_timer)
    {
        ui->vtkWidget->GetRenderWindow()->Render();
    }
}

void TestVtk::onStatusRenderer(bool renderer)
{
    if (renderer)
    {
        if (_render_timer == -1)
            _render_timer = startTimer(40ms, Qt::PreciseTimer);
    }
    else
    {
        if (_render_timer != -1)
        {
            killTimer(_render_timer);
            _render_timer = -1;
        }
    }
}

void TestVtk::onVtkLeftButtonPress(vtkObject* obj)
{
    vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);

    if (iren->GetAltKey())
        onStatusRenderer(false);

    _my_interactor_style.OnLeftButtonDown();
}

void TestVtk::onVtkLeftButtonRelease(vtkObject*)
{
    onStatusRenderer(true);

    _my_interactor_style.OnLeftButtonUp();
}

void TestVtk::onVtkRightButtonPress(vtkObject* obj)
{
    vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);

    auto event_pos = iren->GetEventPosition();

    vtkNew<vtkPointPicker> picker;
    picker->Pick(event_pos[0], event_pos[1], event_pos[2], _renderer);

    QMenu menu;

    auto cur_actor = picker->GetActor();
    if (cur_actor)
    {
        if (_my_interactor_style.IsSelectedActor(cur_actor))
        {
            menu.addAction(tr("Clear Selected"), [this, cur_actor]() { this->ClearCubeAt(cur_actor); });
        }

        menu.addAction(tr("Remove"), [this, cur_actor]() { this->RemoveCubeAt(cur_actor); });
    }
    else
    {
        double pos[3];
        picker->GetPickPosition(pos);
        menu.addAction(tr("Add"), [this, x = pos[0], y = pos[1], z = pos[2]]() { this->AddCubeAt(x, y, z); });
    }

    // 在鼠标位置显示
    menu.exec(QCursor::pos());
}

void TestVtk::onVtkRightButtonRelease(vtkObject*) {}

void TestVtk::onVtkMouseMove(vtkObject* obj)
{
    _my_interactor_style.OnMouseMove();
}

void TestVtk::AddCubeAt(double x, double y, double z)
{
    auto source = vtkSmartPointer<vtkSphereSource>::New();

    double radius = _random_sequence->GetRangeValue(0.5, 1.0);
    _random_sequence->Next();

    source->SetRadius(radius);
    source->SetCenter(x, y, z);
    source->SetPhiResolution(11);
    source->SetThetaResolution(21);

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(source->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    _renderer->AddActor(actor);
}

void TestVtk::RemoveCubeAt(vtkActor* actor)
{
    if (!_my_interactor_style.IsSelectedActor(actor))
    {
        _renderer->RemoveActor(actor);
    }
    _my_interactor_style.RemoveSelected();
}

void TestVtk::ClearCubeAt(vtkActor* actor)
{
    _my_interactor_style.RemoveSelected(actor);
}
