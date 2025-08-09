
#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QTableView>
#include <QPushButton>
#include <QScrollArea>
#include <QStandardItemModel>
#include "model/model.h"

class ClientsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ClientsWidget(QWidget* parent = nullptr);
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override; // Ajout de la gestion du redimensionnement

private slots:
    void onAddClientClicked();
    void updateClientInModel(const QString& idClient, const QString& nom, const QString& prenom, const QString& adresse, const QString& telephone);

private:
    void loadClientsFromDatabase(QStandardItemModel* model);
    int addClientToDatabase(const Client& client);
    // Affiche les détails d'un client dans une vue dédiée
    // Note: l'ID client est laissé vide car nous n'avons pas besoin de le récupérer pour le moment
    void showClientDetails(const QString& nom, const QString& prenom, const QString& adresse, const QString& telephone);
    void showClientsList();

    QLineEdit* searchLineEdit = nullptr;
    QComboBox* collectiviteCombo = nullptr;
    QComboBox* consommationCombo = nullptr;
    QTableView* clientsTable = nullptr;
    QPushButton* addClientBtn = nullptr;
    QScrollArea* scrollArea = nullptr;
};