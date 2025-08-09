#include "widgets/clientwidget.h"
#include "widgets/prelevementwidget.h"
#include <QVBoxLayout>

ClientWidget::ClientWidget(Controller* controller, QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    // Ajout du widget de prélèvements
    PrelevementWidget* prelevementWidget = new PrelevementWidget(this);
    layout->addWidget(prelevementWidget);
    setLayout(layout);
}
