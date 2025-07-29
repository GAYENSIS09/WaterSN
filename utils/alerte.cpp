#include "alerte.h"
#include <QMessageBox>

void Alerte::showAlerte(const QString& message) {
    QMessageBox::warning(nullptr, "Alerte", message);
}
