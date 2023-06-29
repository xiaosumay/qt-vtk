#ifndef TESTVTK_H
#define TESTVTK_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class TestVtk; }
QT_END_NAMESPACE

class TestVtk : public QWidget
{
    Q_OBJECT

public:
    TestVtk(QWidget *parent = nullptr);
    ~TestVtk();

private:
    Ui::TestVtk *ui;
};
#endif // TESTVTK_H
