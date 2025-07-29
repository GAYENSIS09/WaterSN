#include "watersn.h"
#include "./ui_watersn.h"

WaterSN::WaterSN(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WaterSN)
{
    ui->setupUi(this);
}

WaterSN::~WaterSN()
{
    delete ui;
}
