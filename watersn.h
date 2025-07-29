#ifndef WATERSN_H
#define WATERSN_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class WaterSN;
}
QT_END_NAMESPACE

class WaterSN : public QMainWindow
{
    Q_OBJECT

public:
    WaterSN(QWidget *parent = nullptr);
    ~WaterSN();

private:
    Ui::WaterSN *ui;
};
#endif // WATERSN_H
