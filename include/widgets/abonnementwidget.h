#ifndef ABONNEMENTWIDGET_H
#define ABONNEMENTWIDGET_H

#include <QWidget>
#include "controller/controller.h"

class AbonnementWidget : public QWidget {
    Q_OBJECT
public:
    explicit AbonnementWidget(Controller* controller, QWidget* parent = nullptr);
};

#endif // ABONNEMENTWIDGET_H
