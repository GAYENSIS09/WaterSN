#ifndef COMPTEURWIDGET_H
#define COMPTEURWIDGET_H

#include <QWidget>
#include "controller/controller.h"

class CompteurWidget : public QWidget {
    Q_OBJECT
public:
    explicit CompteurWidget(Controller* controller, QWidget* parent = nullptr);
};

#endif // COMPTEURWIDGET_H
