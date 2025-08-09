#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include "controller/controller.h"
#include "widgets/prelevementwidget.h"

class ClientWidget : public QWidget {
    Q_OBJECT
public:
    explicit ClientWidget(Controller* controller, QWidget* parent = nullptr);
private:
    PrelevementWidget* prelevementWidget;
};

#endif // CLIENTWIDGET_H
