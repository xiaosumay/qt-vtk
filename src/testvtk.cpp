#include "testvtk.h"
#include "qwindowdefs.h"
#include "ui_testvtk.h"

#include <chrono>

#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkFloatArray.h>
#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation.h>
#include <vtkJPEGReader.h>
#include <vtkImageActor.h>
#include <vtkInteractorStyleImage.h>

#include <QFileDialog>
#include <QDebug>
#include <QStandardPaths>

#include "version_internal.h"
#include "vtkObject.h"
#include "vtkSmartPointer.h"

using namespace std::chrono_literals;

class MyStyle final : public vtkInteractorStyleImage
{
    vtkTypeMacro(MyStyle, vtkInteractorStyleImage);

public:
    static MyStyle *New() { return new MyStyle; }

    void OnLeftButtonDown() override
    {
        std::cout << "Pressed left mouse button." << std::endl;
        // Forward events
        vtkInteractorStyleImage::OnLeftButtonDown();
    }

    void OnLeftButtonUp() override
    {
        std::cout << "Release left mouse button." << std::endl;

        vtkInteractorStyleImage::OnLeftButtonUp();
    }

    void OnRightButtonDown() override
    {
        std::cout << "Pressed right mouse button." << std::endl;

        vtkInteractorStyleImage::OnRightButtonDown();
    }

    void OnRightButtonUp() override
    {
        std::cout << "Release right mouse button." << std::endl;

        vtkInteractorStyleImage::OnRightButtonUp();
    }

    void OnMouseMove() override
    {
        //
        vtkInteractorStyleImage::OnMouseMove();
    }
};

TestVtk::TestVtk(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestVtk{})
    , _render_timer(-1)
{
    ui->setupUi(this);
    setWindowTitle(tr("Test VTK") + QStringLiteral(" " TEST_VTK_VERSION_FULL));

    _jpeg_reader = vtkSmartPointer<vtkJPEGReader>::New();

    _image_actor = vtkSmartPointer<vtkImageActor>::New();
    _image_actor->SetInputData(_jpeg_reader->GetOutput());

    _renderer = vtkSmartPointer<vtkRenderer>::New();
    _renderer->SetBackground(0.1, 0.2, 0.4);

    _renderer->AddActor(_image_actor);

    _render_window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    _render_window->AddRenderer(_renderer);

    ui->vtkWidget->SetRenderWindow(_render_window);

    auto style = vtkSmartPointer<MyStyle>::New();
    ui->vtkWidget->GetInteractor()->SetInteractorStyle(style);

    _render_timer = startTimer(100ms, Qt::PreciseTimer);
}

TestVtk::~TestVtk()
{
    if (_render_timer != -1) killTimer(_render_timer);

    delete ui;
}

void TestVtk::on_open_clicked()
{
    auto path = QFileDialog::getOpenFileName(this,
        tr("Image"),
        QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0),
        "images(*.jpg *.jpeg)");

    if (path.isEmpty()) return;

    _jpeg_reader->SetFileName(path.toLocal8Bit().data());
    _jpeg_reader->Update();

    _image_actor->SetInputData(_jpeg_reader->GetOutput());

    _renderer->ResetCamera();
}

void TestVtk::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == _render_timer)
    {
        ui->vtkWidget->GetRenderWindow()->Render();
    }
}
