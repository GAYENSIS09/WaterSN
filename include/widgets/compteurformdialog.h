#ifndef COMPTEURFORMDIALOG_H
#define COMPTEURFORMDIALOG_H

#include <QDialog>
#include "model/model.h"

class CompteurFormDialog : public QDialog {
    Q_OBJECT
public:
    explicit CompteurFormDialog(QWidget* parent = nullptr);
    void setCompteur(const Compteur& compteur);
    Compteur getCompteur() const;
};

#endif // COMPTEURFORMDIALOG_H
