#ifndef COMPTEURWIDGET_H
#define COMPTEURWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QTableView>
#include "controller/controller.h"


class CompteurWidget : public QWidget {
    Q_OBJECT
public:
    explicit CompteurWidget(Controller* controller, QWidget* parent = nullptr);
    void refreshAllTabs();
    // ...autres méthodes...
private:
    QComboBox *filterCombo = nullptr;
    QLineEdit *searchCompteurEdit = nullptr;
    QLineEdit *searchBar = nullptr; // Barre de recherche générale
    QTableView *compteursTable = nullptr;

    // Ajout pour les autres onglets
    QLineEdit *prelevementSearchBar = nullptr; // Barre de recherche prélèvements
    QLineEdit *facturationSearchBar = nullptr; // Barre de recherche facturation
    // ...autres membres privés...
};

#endif // COMPTEURWIDGET_H
