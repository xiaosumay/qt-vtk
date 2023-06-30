#include "testvtk.h"
#include "ui_testvtk.h"

#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkProperty.h>
#include <vtkLight.h>
#include <vtkCamera.h>

#include <QFileDialog>
#include <QDebug>
#include <QStandardPaths>

TestVtk::TestVtk(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestVtk{})
{
    ui->setupUi(this);

    m_cylinderActor = vtkSmartPointer<vtkActor>::New();

    m_cylinderSource = vtkSmartPointer<vtkCylinderSource>::New();
    m_cylinderSource->SetHeight(3.0);
    m_cylinderSource->SetRadius(1.0);
    m_cylinderSource->SetResolution(3);

    m_polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_polyDataMapper->SetInputConnection(m_cylinderSource->GetOutputPort());
    m_cylinderActor->SetMapper(m_polyDataMapper);

    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->AddActor(m_cylinderActor);
    m_renderer->SetBackground(0.1, 0.2, 0.4);

    m_renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_renderWindow->AddRenderer(m_renderer);

    auto iren = ui->vtkWidget->interactor();

    m_style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    iren->SetInteractorStyle(m_style);

    vtkSmartPointer<vtkLight> myLight = vtkSmartPointer<vtkLight>::New();
    myLight->SetColor(1, 0, 0);
    myLight->SetPosition(0, 0, 1);
    myLight->SetFocalPoint(m_renderer->GetActiveCamera()->GetFocalPoint());
    m_renderer->AddLight(myLight);

    ui->vtkWidget->setRenderWindow(m_renderWindow);
}

TestVtk::~TestVtk()
{
    delete ui;
}

void TestVtk::on_asc_clicked()
{
    m_cylinderSource->SetResolution(m_cylinderSource->GetResolution() + 1);

    ui->vtkWidget->renderWindow()->Render();
}

void TestVtk::on_desc_clicked()
{
    m_cylinderSource->SetResolution(m_cylinderSource->GetResolution() - 1);

    ui->vtkWidget->renderWindow()->Render();
}
