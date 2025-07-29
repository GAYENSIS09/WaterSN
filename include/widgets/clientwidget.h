#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include "controller/controller.h"

class ClientWidget : public QWidget {
    Q_OBJECT
public:
    explicit ClientWidget(Controller* controller, QWidget* parent = nullptr);
};

#endif // CLIENTWIDGET_H
