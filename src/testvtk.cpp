#include "testvtk.h"
#include "ui_testvtk.h"

#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkFloatArray.h>

#include <QFileDialog>
#include <QDebug>

#include "version_internal.h"

TestVtk::TestVtk(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestVtk{})
{
    ui->setupUi(this);
    setWindowTitle(tr("Test VTK") + QStringLiteral(" " TEST_VTK_VERSION_FULL));

    m_cylinderActor = vtkSmartPointer<vtkActor>::New();

    const int num_points = 5;
    float x[num_points][3] = {
        {   0,    0, 0},
        {   1,    0, 0},
        {   1,    1, 0},
        {   0,    1, 0},
        {0.5f, 0.5f, 1}
    };

    vtkIdType pts[4][3] = {
        {0, 1, 4},
        {1, 2, 4},
        {2, 3, 4},
        {3, 0, 4}
    };

    auto points = vtkSmartPointer<vtkPoints>::New();
    for (int i = 0; i < num_points; i++)
        points->InsertPoint(i, x[i]);

    auto polys = vtkSmartPointer<vtkCellArray>::New();
    for (int i = 0; i < 4; i++)
        polys->InsertNextCell(3, pts[i]);
    polys->InsertNextCell(4);
    for (int i = 0; i < 4; i++)
        polys->InsertCellPoint(i);

    auto scalars = vtkSmartPointer<vtkFloatArray>::New();
    for (int i = 0; i < num_points; i++)
        scalars->InsertTuple1(i, i);

    auto cube = vtkSmartPointer<vtkPolyData>::New();
    cube->SetPoints(points);
    cube->SetPolys(polys);
    cube->GetPointData()->SetScalars(scalars);

    m_polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_polyDataMapper->SetInputData(cube);
    m_cylinderActor->SetMapper(m_polyDataMapper);

    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->AddActor(m_cylinderActor);
    m_renderer->SetBackground(0.1, 0.2, 0.4);

    m_renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_renderWindow->AddRenderer(m_renderer);

    ui->vtkWidget->SetRenderWindow(m_renderWindow);
}

TestVtk::~TestVtk()
{
    delete ui;
}

void TestVtk::on_asc_clicked()
{
    ui->vtkWidget->GetRenderWindow()->Render();
}

void TestVtk::on_desc_clicked()
{
    ui->vtkWidget->GetRenderWindow()->Render();
}

void TestVtk::on_close_clicked()
{
    qApp->aboutQt();
    qApp->quit();
}
