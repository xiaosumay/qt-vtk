#include "testvtk.h"
#include "qnamespace.h"
#include "ui_testvtk.h"

#include "version_internal.h"

#include <chrono>

#include <QFileDialog>
#include <QDebug>
#include <QStandardPaths>

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

#include "selected_actor_mgr.h"
#include "my_interactor_style.h"

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

    _my_interactor_style = vtkSmartPointer<MyInteractorStyle>::New();
    connect(_my_interactor_style, &MyInteractorStyle::statusRenderer, this, &TestVtk::onStatusRenderer);

    _my_interactor_style->SetDefaultRenderer(_renderer);
    ui->vtkWidget->GetInteractor()->SetInteractorStyle(_my_interactor_style);

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
    _my_interactor_style->RemoveSelected();
}

void TestVtk::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == _render_timer) { ui->vtkWidget->GetRenderWindow()->Render(); }
}

void TestVtk::onStatusRenderer(bool renderer)
{
    if (renderer)
    {
        if (_render_timer == -1) _render_timer = startTimer(16ms, Qt::PreciseTimer);
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
