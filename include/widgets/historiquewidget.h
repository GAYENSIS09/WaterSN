#ifndef HISTORIQUEWIDGET_H
#define HISTORIQUEWIDGET_H

#include <QWidget>
#include "controller/controller.h"

class HistoriqueWidget : public QWidget {
    Q_OBJECT
public:
    explicit HistoriqueWidget(Controller* controller, QWidget* parent = nullptr);
};

#endif // HISTORIQUEWIDGET_H
