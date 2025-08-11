#pragma once
#include <QWidget>
#include <QString>
#include <QTabWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>

// Déclarations anticipées pour QtCharts
class QChartView;
class QBarSet;
class QBarSeries;
class QBarCategoryAxis;

class ClientDetailWidget : public QWidget {
    Q_OBJECT
public:
    bool eventFilter(QObject* obj, QEvent* event) override;
    explicit ClientDetailWidget(QWidget* parent = nullptr);
    void setClientInfo(const QString& id, const QString& nom, const QString& prenom, 
                     const QString& adresse, const QString& telephone);
signals:
    void retourClicked();
    void facturePayeeSignal(const QString& factureId);
    void factureExporteeSignal(const QString& factureId);
    void factureSupprimeeSignal(const QString& factureId);
    void genererFactureSignal(const QString& clientId);
    void ajouterAbonnementSignal(const QString& clientId);
    void abonnementAjouteSignal();
    void factureAjouteSignal();
    void consommationAjouteSignal();

public slots:
    void loadAbonnementsFromDB();
    void loadFacturesFromDB();
    void onTabChanged(int index);
    void onPayerFactureClicked();
    void onExportFactureClicked();
    void onGenererFactureClicked();
    void onAjouterFactureClicked();
    void onAjouterAbonnementClicked();
    void filtrerFactures(int index);
    void onExportFactureDelegate(const QString& factureId);
    void onSupprimerFactureDelegate(const QString& factureId);

protected:
    bool abonnementsTableEventFilter(QObject* obj, QEvent* event);
private:
    // Méthodes d'initialisation UI
    void setupUI();
    void setupInfoTab();
    void setupAbonnementsTab();
    void setupFacturesTab();
    void setupHistoriqueTab();
    void createConsommationChart();
    void loadConsommationFromDB();

    // Méthodes de chargement des données
    // Sauvegarde d'un champ modifié dans la base de données
    void saveClientFieldToDB(const QString& field, const QString& value);

    // Récupère la liste des compteurs disponibles pour l'abonnement
    QStringList getCompteursDisponibles();

    // Informations client
    QString clientId;
    QString nomClient;
    QString prenomClient;
    QString adresseClient;
    QString telephoneClient;
    
    // Widgets principaux
    QPushButton* retourBtn;
    QLabel* titreClientLabel;
    QTabWidget* tabWidget;
    
    // Onglets et zones de défilement
    QWidget* infoTab;
    QScrollArea* infoScrollArea;
    QWidget* abonnementsTab;
    QScrollArea* abonnementsScrollArea;
    QWidget* facturesTab;
    QScrollArea* facturesScrollArea;
    QWidget* historiqueTab;
    QScrollArea* historiqueScrollArea;
    
    // Widgets onglet Informations
    QLabel* nomLabel;
    QLabel* prenomLabel;
    QLabel* adresseLabel;
    QLabel* telephoneLabel;
    QLabel* nbAbonnementsLabel;
    QLabel* totalFacturesLabel;
    QLabel* consoMoyenneLabel;
    QLabel* statutPaiementLabel;
    
    // Widgets onglet Abonnements
    QTableView* abonnementsTable;
    QStandardItemModel* abonnementsModel;
    QPushButton* ajouterAbonnementBtn;
    // Pour édition inline
    int abonnementEditRow = -1;
    int abonnementEditCol = -1;
    
    // Widgets onglet Factures
    QComboBox* periodeCombo;
    QComboBox* statutCombo;
    QTableView* facturesTable;
    QStandardItemModel* facturesModel;
    QPushButton* genererFactureBtn;
    QPushButton* ajouterFactureBtn;
    
    // Widgets onglet Historique
    QChartView* consommationChartView;
    QTableView* paiementsTable;
    QStandardItemModel* paiementsModel;
};
