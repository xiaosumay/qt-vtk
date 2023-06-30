#include "testvtk.h"
#include "ui_testvtk.h"

#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkImageViewer2.h"
#include "QVTKOpenGLNativeWidget.h"
#include "vtkJPEGReader.h"
#include "vtkImageActor.h"

#include <QFileDialog>
#include <QDebug>
#include <QStandardPaths>

TestVtk::TestVtk(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestVtk{})
{
    ui->setupUi(this);

    m_renderer = vtkSmartPointer<vtkRenderer>::New();

    m_imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
    m_imageViewer->SetRenderer(m_renderer);
    m_imageViewer->SetRenderWindow(ui->vtkWidget->renderWindow());
    m_imageViewer->SetupInteractor(ui->vtkWidget->renderWindow()->GetInteractor());

    m_renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_renderWindow->AddRenderer(m_renderer);

    ui->vtkWidget->setRenderWindow(m_renderWindow);
}

TestVtk::~TestVtk()
{
    delete ui;
}

void TestVtk::on_open_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
        tr("open image"),
        QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0),
        tr("JPEG image file(*.jpg *.jpeg)"));

    if (filename.isEmpty()) return;

    const char *image_path = filename.toLocal8Bit().data();

    vtkSmartPointer<vtkJPEGReader> reader = vtkSmartPointer<vtkJPEGReader>::New();
    reader->SetFileName(image_path);
    reader->Update();

    m_imageViewer->SetInputData(reader->GetOutput());
    m_imageViewer->UpdateDisplayExtent();

    m_imageViewer->SetSliceOrientationToXY();
    m_imageViewer->GetImageActor()->InterpolateOff();

    m_renderer->ResetCamera();
    m_renderer->DrawOn();

    ui->vtkWidget->renderWindow()->Render();
}
