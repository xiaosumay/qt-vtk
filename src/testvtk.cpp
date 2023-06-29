#include "testvtk.h"
#include "ui_testvtk.h"

TestVtk::TestVtk(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestVtk)
{
    ui->setupUi(this);
}

TestVtk::~TestVtk()
{
    delete ui;
}
