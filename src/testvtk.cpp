#include "testvtk.h"
#include "qobjectdefs.h"
#include "ui_testvtk.h"

#include "common.h"
#include "version_internal.h"
#include "vtkInteractorStyleTrackballCameraEx.h"

#include <chrono>

#include <QFileDialog>
#include <QStandardPaths>
#include <QMenu>
#include <QtConcurrentRun>

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
#include <vtkPointPicker.h>
#include <vtkFloatArray.h>
#include <vtkAreaPicker.h>
#include <vtkProp3DCollection.h>

using namespace std::chrono_literals;
using namespace lingxi::vtk;

TestVtk::TestVtk(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TestVtk{})
    , _render_timer(-1)
{
    ui->setupUi(this);
    setWindowTitle(tr("Test VTK") + QStringLiteral(" " TEST_VTK_VERSION_FULL));

    /*!
     * 随机数生成器，生成小球位置时需要
     */
    _random_sequence = vtkSmartPointer<vtkMinimalStandardRandomSequence>::New();
    _random_sequence->SetSeed(time(NULL));

    _renderer = vtkSmartPointer<vtkRenderer>::New();
    _renderer->SetBackground(0.1, 0.2, 0.4);

    _render_window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    _render_window->AddRenderer(_renderer);

    ui->vtkWidget->SetRenderWindow(_render_window);

    auto style = vtkSmartPointer<vtkInteractorStyleTrackballCameraEx>::New();
    style->SetDefaultRenderer(_renderer);

    style->AddObserver(LxEventIdsEx::kSingleActorClickedEvent, this, &Self::onSingleActorClicked);
    style->AddObserver(LxEventIdsEx::kActorMoveDeltaEvent, this, &Self::onActorMoveDelta);
    style->AddObserver(LxEventIdsEx::kSelectedAreaStartEvent, this, &Self::onSelectedAreaStart);
    style->AddObserver(LxEventIdsEx::kSelectedAreaEndEvent, this, &Self::onSelectedAreaEnd);
    style->AddObserver(LxEventIdsEx::kRightButtonUpEvent, this, &Self::onRightButtonUp);

    auto inter = ui->vtkWidget->GetInteractor();
    inter->SetInteractorStyle(style);

    auto axes_actor = vtkSmartPointer<vtkAxesActor>::New();
    axes_actor->SetTotalLength(10, 10, 10);
    axes_actor->SetPickable(false);
    axes_actor->SetAxisLabels(false);

    _renderer->AddActor(axes_actor);

    SetRefreshAuto(true);
}

TestVtk::~TestVtk()
{
    SetRefreshAuto(false);

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
}

void TestVtk::on_delete_cube_clicked()
{
    _selected_actor_mgr.RemoveFrom(_renderer);
}

void TestVtk::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == _render_timer)
    {
        ui->vtkWidget->GetRenderWindow()->Render();
    }
}

void TestVtk::SetRefreshAuto(bool renderer)
{
    if (renderer)
    {
        if (_render_timer == -1) _render_timer = startTimer(40ms);
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

void TestVtk::onSingleActorClicked(vtkObject* obj, unsigned long, void* clientData)
{
    auto style = vtkInteractorStyleTrackballCameraEx::SafeDownCast(obj);

    auto actor = (vtkActor*)clientData;

    if (!_selected_actor_mgr.Contain(actor))
    {
        if (!style->GetInteractor()->GetControlKey())
        {
            _selected_actor_mgr.Reset();
            _selected_actor_mgr.Clear();
        }

        _selected_actor_mgr.AddActor(actor);
    }
    else
    {
        if (style->GetInteractor()->GetControlKey())
        {
            _selected_actor_mgr.RemoveActor(actor);
        }
    }
}

void TestVtk::onActorMoveDelta(vtkObject*, unsigned long, void* clientData)
{
    auto delta = (double*)clientData;

    _selected_actor_mgr.AddPosition(delta[0], delta[1], delta[2]);
}

void TestVtk::onSelectedAreaStart(vtkObject*, unsigned long, void*)
{
    SetRefreshAuto(false);
}

void TestVtk::onSelectedAreaEnd(vtkObject*, unsigned long, void* clientData)
{
    auto area = (int*)clientData;

    vtkNew<vtkAreaPicker> picker;
    picker->AreaPick(area[0], area[1], area[2], area[3], _renderer);

    vtkProp3DCollection* props = picker->GetProp3Ds();
    props->InitTraversal();

    _selected_actor_mgr.Reset();
    _selected_actor_mgr.Clear();
    for (vtkIdType i = 0; i < props->GetNumberOfItems(); i++)
    {
        vtkActor* actor = vtkActor::SafeDownCast(props->GetNextProp3D());
        if (!actor) continue;

        _selected_actor_mgr.AddActor(actor);
    }

    SetRefreshAuto(true);
}

void TestVtk::onRightButtonUp(vtkObject*, unsigned long, void* clientData)
{
    auto iren = (vtkRenderWindowInteractor*)clientData;

    auto event_pos = iren->GetEventPosition();

    vtkNew<vtkPointPicker> picker;
    picker->Pick(event_pos[0], event_pos[1], event_pos[2], _renderer);

    QMenu menu;

    auto cur_actor = picker->GetActor();
    if (cur_actor)
    {
        if (_selected_actor_mgr.Contain(cur_actor))
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
    if (!_selected_actor_mgr.Contain(actor))
    {
        _renderer->RemoveActor(actor);
    }
    else
    {
        _selected_actor_mgr.RemoveFrom(_renderer);
    }
}

void TestVtk::ClearCubeAt(vtkActor* actor)
{
    _selected_actor_mgr.RemoveActor(actor);
}

#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/io/pcd_io.h>

static bool IsInvalidPoint(double x, double y, double z)
{
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
    {
        return true;
    }
    return false;
}

void TestVtk::on_load_pcl_clicked()
{
    QString file_path =
        QFileDialog::getOpenFileName(this, tr("open pcl files"), QString(), QStringLiteral("PCL files(*.pcd)"));
    if (file_path.isEmpty()) return;

    // 异步执行，避免卡界面
    QtConcurrent::run(this, &Self::handlePclFile, file_path);
}

void TestVtk::handlePclFile(QString file_path)
{
    auto pcl_data = pcl::PointCloud<pcl::PointXYZ>::Ptr(new pcl::PointCloud<pcl::PointXYZ>);

    if (pcl::io::loadPCDFile<pcl::PointXYZ>(file_path.toLocal8Bit().toStdString(), *pcl_data) == -1)
    {
        return;
    }

    vtkSmartPointer<vtkPolyData> vtk_data = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> vtk_points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtk_vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkFloatArray> vtk_scalars = vtkSmartPointer<vtkFloatArray>::New();

    for (const auto& point : pcl_data->points)
    {
        if (IsInvalidPoint(point.x, point.y, point.z)) continue;

        vtkIdType vtk_id = vtk_points->InsertNextPoint(point.x, point.y, point.z);
        vtk_vertices->InsertNextCell(1);
        vtk_vertices->InsertCellPoint(vtk_id);
        vtk_scalars->InsertNextTuple1(point.z);
    }

    vtk_data->SetPoints(vtk_points);
    vtk_data->SetVerts(vtk_vertices);
    vtk_data->GetPointData()->SetScalars(vtk_scalars);

    double minmax[2];
    vtk_scalars->GetRange(minmax);

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetScalarRange(minmax);
    mapper->SetInputData(vtk_data);

    // 不能使用智能指针，线程退出后会被销毁
    auto actor = vtkActor::New();
    actor->SetMapper(mapper);
    actor->SetPickable(false);

    QMetaObject::invokeMethod(this, "onPclLoadFinished", Qt::QueuedConnection, Q_ARG(void*, actor));
}

void TestVtk::onPclLoadFinished(void* p)
{
    // 捕获指针，并重新赋值给智能指针
    auto actor = vtkSmartPointer<vtkActor>((vtkActor*)p);

    _renderer->AddActor(actor);
    _renderer->Render();
}
