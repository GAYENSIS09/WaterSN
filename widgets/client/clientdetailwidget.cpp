
#include "widgets/factureactionsdelegate.h"
#include "widgets/clientdetailwidget.h"
#include "widgets/abonnementformdialog.h"
#include "widgets/factureformdialog.h"
#include "model/model.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QComboBox>
#include <QFrame>
#include <QFont>
#include <QPainter>
#include <QWidget>
#include <QScrollArea>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

// Inclusion des classes QtCharts
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>

ClientDetailWidget::ClientDetailWidget(QWidget* parent) : QWidget(parent) {
    // Configuration initiale de l'interface utilisateur
    setupUI();
    
    // Connexion des signaux et slots
    connect(retourBtn, &QPushButton::clicked, this, &ClientDetailWidget::retourClicked);
    connect(tabWidget, &QTabWidget::currentChanged, this, &ClientDetailWidget::onTabChanged);

    // Rafraîchir les données à chaque sélection d'onglet
    connect(tabWidget, &QTabWidget::currentChanged, this, [=](int index){
        switch(index) {
            case 0: /* Informations */
                // Si besoin, recharger les infos client
                setClientInfo(clientId, nomClient, prenomClient, adresseClient, telephoneClient);
                break;
            case 1: /* Abonnements */
                loadAbonnementsFromDB();
                break;
            case 2: /* Factures */
                loadFacturesFromDB();
                break;
            case 3: /* Historique */
                loadConsommationFromDB();
                break;
        }
    });
    
    // Appliquer le style global
    setStyleSheet("background: #22332d; border-radius: 16px; padding: 32px;");
}


// ...existing code...
// Variables pour édition inline des tableaux
int factureEditRow = -1;
int factureEditCol = -1;
int paiementEditRow = -1;
int paiementEditCol = -1;
    // ...existing code...

void ClientDetailWidget::onExportFactureDelegate(const QString& factureId) {
    emit factureExporteeSignal(factureId);
}

void ClientDetailWidget::onSupprimerFactureDelegate(const QString& factureId) {
    QMessageBox::StandardButton confirmation = QMessageBox::question(this,
        "Supprimer la facture",
        "Voulez-vous vraiment supprimer cette facture ?",
        QMessageBox::Yes | QMessageBox::No);
    if (confirmation == QMessageBox::Yes) {
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isOpen()) {
            QMessageBox::critical(this, "Erreur DB", "La base de données n'est pas ouverte.");
            return;
        }
        QSqlQuery query(db);
        query.prepare("DELETE FROM Facture WHERE idFacture = :idFacture");
        query.bindValue(":idFacture", factureId.toInt());
        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur DB", "Échec de la suppression de la facture : " + query.lastError().text());
            return;
        }
        emit factureSupprimeeSignal(factureId);
        loadFacturesFromDB();
    }
}

void ClientDetailWidget::setClientInfo(const QString& id, const QString& nom, const QString& prenom, const QString& adresse, const QString& telephone) {
    // Stocker l'ID et les autres informations
    clientId = id;
    nomClient = nom;
    prenomClient = prenom;
    adresseClient = adresse;
    telephoneClient = telephone;
    
    // Mettre à jour le titre
    titreClientLabel->setText(nom + " " + prenom);
    
    // Mettre à jour les labels d'information (juste la valeur, sans préfixe)
    nomLabel->setText(nom);
    prenomLabel->setText(prenom);
    adresseLabel->setText(adresse);
    telephoneLabel->setText(telephone);
    
    // Charger les données depuis la base de données
    loadAbonnementsFromDB();
    loadFacturesFromDB();
    loadConsommationFromDB();
}

void ClientDetailWidget::onTabChanged(int index) {
    // Implémenter le comportement lors du changement d'onglet
    switch(index) {
        case 0: // Onglet Informations
            // Rafraîchir les informations client si nécessaire
            break;
        case 1: // Onglet Abonnements
            // Charger ou rafraîchir les abonnements
            loadAbonnementsFromDB();
            break;
        case 2: // Onglet Factures
            // Charger ou rafraîchir les factures
            loadFacturesFromDB();
            break;
        case 3: // Onglet Historique
            // Charger ou rafraîchir uniquement la consommation (plus de paiements)
            loadConsommationFromDB();
            break;
    }
}

void ClientDetailWidget::onPayerFactureClicked() {
    // Implémenter le paiement d'une facture
    // Récupérer l'ID de la facture sélectionnée
    QModelIndex index = facturesTable->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une facture à payer.");
        return;
    }
    
    // Récupérer l'ID de la facture depuis le modèle
    QString factureId = facturesModel->data(facturesModel->index(index.row(), 0)).toString();
    
    // Demander confirmation
    QMessageBox::StandardButton confirmation = QMessageBox::question(this, 
        "Confirmer le paiement", 
        "Voulez-vous vraiment marquer cette facture comme payée ?",
        QMessageBox::Yes | QMessageBox::No);
        
    if (confirmation == QMessageBox::Yes) {
        // Émettre le signal pour que le contrôleur gère le paiement
        emit facturePayeeSignal(factureId);
        
        // Rafraîchir l'affichage des factures
        loadFacturesFromDB();
        emit factureAjouteSignal();
    }
}

void ClientDetailWidget::onExportFactureClicked() {
    // Implémenter l'export d'une facture
    // Récupérer l'ID de la facture sélectionnée
    QModelIndex index = facturesTable->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une facture à exporter.");
        return;
    }
    
    // Récupérer l'ID de la facture depuis le modèle
    QString factureId = facturesModel->data(facturesModel->index(index.row(), 0)).toString();
    
    // Émettre le signal pour que le contrôleur gère l'export
    emit factureExporteeSignal(factureId);
}

void ClientDetailWidget::onGenererFactureClicked() {
    // Implémenter la génération d'une nouvelle facture
    if (clientId.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Aucun client sélectionné.");
        return;
    }
    
    // Demander confirmation
    QMessageBox::StandardButton confirmation = QMessageBox::question(this, 
        "Générer une facture", 
        "Voulez-vous générer une nouvelle facture pour ce client ?",
        QMessageBox::Yes | QMessageBox::No);
        
    if (confirmation == QMessageBox::Yes) {
        // Émettre le signal pour que le contrôleur gère la génération
        emit genererFactureSignal(clientId);
        
        // Rafraîchir l'affichage des factures
        loadFacturesFromDB();
    }
}

// ...existing includes...

void ClientDetailWidget::onAjouterAbonnementClicked() {
    // Fenêtre d'ajout d'abonnement
    if (clientId.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Aucun client sélectionné.");
        return;
    }

    QStringList compteursDispo = getCompteursDisponibles();
    AbonnementFormDialog dialog(compteursDispo, this);
    if (dialog.exec() == QDialog::Accepted) {
        // Récupérer les infos saisies
        QString compteur = dialog.getCompteur();
        QDate dateDebut = dialog.getDateDebut();
        // Récupérer le statut depuis la table Compteur
        QString statut;
        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            QSqlQuery queryComp(db);
            queryComp.prepare("SELECT attributComp FROM Compteur WHERE numCompteur = :numCompteur");
            queryComp.bindValue(":numCompteur", compteur);
            if (queryComp.exec() && queryComp.next()) {
                statut = queryComp.value(0).toString();
            } else {
                statut = "";
            }
        } else {
            statut = "";
        }

        // Insertion en base de données SQLite (seulement les champs du schéma)
        if (!db.isOpen()) {
            QMessageBox::critical(this, "Erreur DB", "La base de données n'est pas ouverte.");
            return;
        }

        QSqlQuery query(db);
        query.prepare("INSERT INTO Abonnement (idClient, numCompteur, date_abonnement) VALUES (:idClient, :numCompteur, :date_abonnement)");
        query.bindValue(":idClient", clientId.toInt());
        query.bindValue(":numCompteur", compteur);
        query.bindValue(":date_abonnement", dateDebut.toString("yyyy-MM-dd"));
        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur DB", "Échec de l'ajout de l'abonnement : " + query.lastError().text());
        } else {
            // Mettre à jour l'attribut du compteur à 'Transféré'
            QSqlQuery updateComp(db);
            updateComp.prepare("UPDATE Compteur SET attributComp = 'Transféré' WHERE numCompteur = :numCompteur");
            updateComp.bindValue(":numCompteur", compteur);
            updateComp.exec();
        }

        // Mettre à jour la liste des compteurs disponibles pour le prochain ajout
        compteursDispo = getCompteursDisponibles();

        // Rafraîchir l'affichage des abonnements
        loadAbonnementsFromDB();
        emit abonnementAjouteSignal();
    }
}

void ClientDetailWidget::filtrerFactures(int index) {
    // Implémenter le filtrage des factures
    // Cette méthode sera appelée lorsque l'utilisateur change la sélection dans les combobox
    // de période ou de statut
    
    // Rafraîchir l'affichage des factures avec les filtres appliqués
    loadFacturesFromDB();
}

// Implémentations des méthodes d'initialisation UI
void ClientDetailWidget::setupUI() {
    // Création du layout principal
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    
    // Layout pour le titre et le bouton retour
    QHBoxLayout* topLayout = new QHBoxLayout;
    
    // Bouton retour
    retourBtn = new QPushButton("← Retour", this);
    retourBtn->setFixedSize(120, 38);
    retourBtn->setStyleSheet("background: #ffd23f; color: #3d554b; font-size: 18px; font-weight: bold; border-radius: 8px; padding: 6px 18px;");
    retourBtn->setCursor(Qt::PointingHandCursor);
    
    // Titre
    titreClientLabel = new QLabel("Détails du client", this);
    titreClientLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #ffd23f; background: #22332d; border-radius: 8px; padding: 6px 18px; min-width:260px; max-width:260px; text-align: center; height: 38px;");
    titreClientLabel->setFixedSize(260, 38); // largeur augmentée pour afficher tout le nom et prénom
    
    // Ajout des widgets au layout du haut
    topLayout->addWidget(retourBtn, 0, Qt::AlignLeft);
    topLayout->addWidget(titreClientLabel, 0, Qt::AlignCenter);
    topLayout->addStretch();
    
    // Création du widget à onglets
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: none; background: #22332d; margin: 0px; }"
        "QTabBar::tab { background: #22332d; color: #e6e6e6; font-size: 15px; font-weight: normal; padding: 8px 24px; border-top-left-radius: 0px; border-top-right-radius: 0px; margin-right: 2px; min-width: 120px; }"
        "QTabBar::tab:selected { background: #31463f; color: #ffd23f; border-bottom: 2px solid #42a5f5; }"
        "QTabBar::tab:hover { background: #2c3e38; color: #ffd23f; }"
        "QTabBar::tab:!selected { margin-top: 0px; }"
    );
    
    // Création des onglets
    infoTab = new QWidget;
    infoScrollArea = new QScrollArea;
    abonnementsTab = new QWidget;
    facturesTab = new QWidget;
    facturesScrollArea = new QScrollArea;
    historiqueTab = new QWidget;
    historiqueScrollArea = new QScrollArea;
    
    // Configuration des zones de défilement
    infoScrollArea->setWidgetResizable(true);
    infoScrollArea->setFrameShape(QFrame::NoFrame);
    infoScrollArea->setWidget(infoTab);
    
    // Suppression de la QScrollArea pour l'onglet abonnements
    
    facturesScrollArea->setWidgetResizable(true);
    facturesScrollArea->setFrameShape(QFrame::NoFrame);
    facturesScrollArea->setWidget(facturesTab);
    
    historiqueScrollArea->setWidgetResizable(true);
    historiqueScrollArea->setFrameShape(QFrame::NoFrame);
    historiqueScrollArea->setWidget(historiqueTab);
    
    // Initialisation des onglets
    setupInfoTab();
    setupAbonnementsTab();
    setupFacturesTab();
    setupHistoriqueTab();
    
    // Ajout des onglets au widget à onglets
    tabWidget->addTab(infoScrollArea, "Informations");
    tabWidget->addTab(abonnementsTab, "Abonnements");
    tabWidget->addTab(facturesScrollArea, "Factures");
    tabWidget->addTab(historiqueScrollArea, "Historique");
    
    // Ajout des layouts au layout principal
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(tabWidget, 1);
}

// Méthode pour récupérer les compteurs disponibles (non actifs)
QStringList ClientDetailWidget::getCompteursDisponibles() {
    QStringList disponibles;
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur DB", "La base de données n'est pas ouverte.");
        return disponibles;
    }
    // Sélectionner uniquement les compteurs disponibles
    QSqlQuery queryAll(db);
    if (!queryAll.exec("SELECT numCompteur FROM Compteur WHERE attributComp = 'Disponible'")) {
        QMessageBox::critical(this, "Erreur DB", "Impossible de charger les compteurs : " + queryAll.lastError().text());
        return disponibles;
    }
    while (queryAll.next()) {
        disponibles << queryAll.value(0).toString();
    }
    return disponibles;
}
 

void ClientDetailWidget::setupInfoTab() 
{
    // Layout principal horizontal pour séparer infos personnelles (gauche) et KPI (droite)
    QHBoxLayout* mainHL = new QHBoxLayout(infoTab);
    mainHL->setContentsMargins(20, 20, 20, 20);
    mainHL->setSpacing(20);

    // --- Colonne gauche : Infos personnelles (style KPI) ---
    QWidget* persoWidget = new QWidget();
    QVBoxLayout* persoLayout = new QVBoxLayout(persoWidget);
    persoLayout->setSpacing(15); // même spacing que KPI
    persoLayout->setAlignment(Qt::AlignTop);

    QString persoStyle = "font-size: 15px; color: #e6e6e6; padding: 8px 18px; background: #31463f; border-radius: 6px; min-width:180px; margin-bottom: 0px; display: flex; align-items: center;";
    QString persoLabelStyle = "font-weight: bold; color: #42a5f5;";

    // Nom (modif inline)
   QWidget* nomRow = new QWidget();
    QHBoxLayout* nomRowLayout = new QHBoxLayout(nomRow);
    nomRowLayout->setContentsMargins(0,0,0,0);
    nomRowLayout->setSpacing(8);
    QLabel* nomTitle = new QLabel("Nom :", this);
    nomTitle->setStyleSheet(persoLabelStyle);
    nomLabel = new QLabel(this);
    // Récupérer la liste des compteurs disponibles
    QStringList compteursDispo = getCompteursDisponibles();
    AbonnementFormDialog dialog(compteursDispo, this);
    QLineEdit* nomEdit = new QLineEdit(this);
    nomEdit->setStyleSheet("color: #e6e6e6; background: #22332d; border: 1px solid #31463f; border-radius: 4px; padding: 2px 6px; min-width:60px;");
    nomEdit->hide();
    QPushButton* editNomBtn = new QPushButton(this);
    editNomBtn->setIcon(QIcon(":/icons/material/edit.svg"));
    editNomBtn->setFixedSize(22, 22);
    editNomBtn->setStyleSheet("background: transparent; border: none; margin-left: 6px;");
    nomRowLayout->addWidget(nomTitle);
    nomRowLayout->addWidget(nomLabel);
    nomRowLayout->addWidget(nomEdit);
    nomRowLayout->addWidget(editNomBtn);
    nomRowLayout->addStretch();
    nomRow->setStyleSheet(persoStyle);
    persoLayout->addWidget(nomRow);

    // Prénom (modif inline)
    QWidget* prenomRow = new QWidget();
    QHBoxLayout* prenomRowLayout = new QHBoxLayout(prenomRow);
    prenomRowLayout->setContentsMargins(0,0,0,0);
    prenomRowLayout->setSpacing(8);
    QLabel* prenomTitle = new QLabel("Prénom :", this);
    prenomTitle->setStyleSheet(persoLabelStyle);
    prenomLabel = new QLabel(this);
    prenomLabel->setStyleSheet("color: #e6e6e6;");
    QLineEdit* prenomEdit = new QLineEdit(this);
    prenomEdit->setStyleSheet("color: #e6e6e6; background: #22332d; border: 1px solid #31463f; border-radius: 4px; padding: 2px 6px; min-width:60px;");
    prenomEdit->hide();
    QPushButton* editPrenomBtn = new QPushButton(this);
    editPrenomBtn->setIcon(QIcon(":/icons/material/edit.svg"));
    editPrenomBtn->setFixedSize(22, 22);
    editPrenomBtn->setStyleSheet("background: transparent; border: none; margin-left: 6px;");
    prenomRowLayout->addWidget(prenomTitle);
    prenomRowLayout->addWidget(prenomLabel);
    prenomRowLayout->addWidget(prenomEdit);
    prenomRowLayout->addWidget(editPrenomBtn);
    prenomRowLayout->addStretch();
    prenomRow->setStyleSheet(persoStyle);
    persoLayout->addWidget(prenomRow);

    // Adresse (modif inline)
    QWidget* adresseRow = new QWidget();
    QHBoxLayout* adresseRowLayout = new QHBoxLayout(adresseRow);
    adresseRowLayout->setContentsMargins(0,0,0,0);
    adresseRowLayout->setSpacing(8);
    QLabel* adresseTitle = new QLabel("Adresse :", this);
    adresseTitle->setStyleSheet(persoLabelStyle);
    adresseLabel = new QLabel(this);
    adresseLabel->setStyleSheet("color: #e6e6e6;");
    QLineEdit* adresseEdit = new QLineEdit(this);
    adresseEdit->setStyleSheet("color: #e6e6e6; background: #22332d; border: 1px solid #31463f; border-radius: 4px; padding: 2px 6px; min-width:60px;");
    adresseEdit->hide();
    QPushButton* editAdresseBtn = new QPushButton(this);
    editAdresseBtn->setIcon(QIcon(":/icons/material/edit.svg"));
    editAdresseBtn->setFixedSize(22, 22);
    editAdresseBtn->setStyleSheet("background: transparent; border: none; margin-left: 6px;");
    adresseRowLayout->addWidget(adresseTitle);
    adresseRowLayout->addWidget(adresseLabel);
    adresseRowLayout->addWidget(adresseEdit);
    adresseRowLayout->addWidget(editAdresseBtn);
    adresseRowLayout->addStretch();
    adresseRow->setStyleSheet(persoStyle);
    persoLayout->addWidget(adresseRow);

    // Téléphone (modif inline)
    QWidget* telephoneRow = new QWidget();
    QHBoxLayout* telephoneRowLayout = new QHBoxLayout(telephoneRow);
    telephoneRowLayout->setContentsMargins(0,0,0,0);
    telephoneRowLayout->setSpacing(8);
    QLabel* telephoneTitle = new QLabel("Téléphone :", this);
    telephoneTitle->setStyleSheet(persoLabelStyle);
    telephoneLabel = new QLabel(this);
    telephoneLabel->setStyleSheet("color: #e6e6e6;");
    QLineEdit* telephoneEdit = new QLineEdit(this);
    telephoneEdit->setStyleSheet("color: #e6e6e6; background: #22332d; border: 1px solid #31463f; border-radius: 4px; padding: 2px 6px; min-width:60px;");
    telephoneEdit->hide();
    QPushButton* editTelephoneBtn = new QPushButton(this);
    editTelephoneBtn->setIcon(QIcon(":/icons/material/edit.svg"));
    editTelephoneBtn->setFixedSize(22, 22);
    editTelephoneBtn->setStyleSheet("background: transparent; border: none; margin-left: 6px;");
    telephoneRowLayout->addWidget(telephoneTitle);
    telephoneRowLayout->addWidget(telephoneLabel);
    telephoneRowLayout->addWidget(telephoneEdit);
    telephoneRowLayout->addWidget(editTelephoneBtn);
    telephoneRowLayout->addStretch();
    telephoneRow->setStyleSheet(persoStyle);
    persoLayout->addWidget(telephoneRow);

    // Connexions pour activer l'édition inline sur bouton
    auto activateEdit = [](QLabel* label, QLineEdit* edit) {
        edit->setText(label->text());
        label->hide();
        edit->show();
        edit->setFocus();
        edit->selectAll();
    };
    editNomBtn->connect(editNomBtn, &QPushButton::clicked, [=]() { activateEdit(nomLabel, nomEdit); });
    editPrenomBtn->connect(editPrenomBtn, &QPushButton::clicked, [=]() { activateEdit(prenomLabel, prenomEdit); });
    editAdresseBtn->connect(editAdresseBtn, &QPushButton::clicked, [=]() { activateEdit(adresseLabel, adresseEdit); });
    editTelephoneBtn->connect(editTelephoneBtn, &QPushButton::clicked, [=]() { activateEdit(telephoneLabel, telephoneEdit); });

    // Installer un eventFilter sur chaque QLabel pour détecter le double-clic
    nomLabel->installEventFilter(this);
    prenomLabel->installEventFilter(this);
    adresseLabel->installEventFilter(this);
    telephoneLabel->installEventFilter(this);
    // Stocker les QLineEdit pour l'eventFilter
    this->setProperty("nomEditPtr", QVariant::fromValue(static_cast<void*>(nomEdit)));
    this->setProperty("prenomEditPtr", QVariant::fromValue(static_cast<void*>(prenomEdit)));
    this->setProperty("adresseEditPtr", QVariant::fromValue(static_cast<void*>(adresseEdit)));
    this->setProperty("telephoneEditPtr", QVariant::fromValue(static_cast<void*>(telephoneEdit)));
// Event filter pour double-clic sur QLabel (ajouté à la fin du fichier)

    // Sauvegarde dans la base dès que l'édition est terminée
    connect(nomEdit, &QLineEdit::editingFinished, [=]() {
        nomLabel->setText(nomEdit->text());
        nomEdit->hide();
        nomLabel->show();
        this->saveClientFieldToDB("nom", nomEdit->text());
    });
    connect(prenomEdit, &QLineEdit::editingFinished, [=]() {
        prenomLabel->setText(prenomEdit->text());
        prenomEdit->hide();
        prenomLabel->show();
        this->saveClientFieldToDB("prenom", prenomEdit->text());
    });
    connect(adresseEdit, &QLineEdit::editingFinished, [=]() {
        adresseLabel->setText(adresseEdit->text());
        adresseEdit->hide();
        adresseLabel->show();
        this->saveClientFieldToDB("adresse", adresseEdit->text());
    });
    connect(telephoneEdit, &QLineEdit::editingFinished, [=]() {
        QString tel = telephoneEdit->text();
        QRegularExpression regexSenegal("^(7[05678])\\s\\d{3}\\s\\d{2}\\s\\d{2}$");
        if (!regexSenegal.match(tel).hasMatch()) {
            QMessageBox::warning(this, "Numéro invalide", "Le numéro doit être au format : 77 465 23 54 (espaces obligatoires)");
            telephoneEdit->setFocus();
            telephoneEdit->selectAll();
            return;
        }
        telephoneLabel->setText(tel);
        telephoneEdit->hide();
        telephoneLabel->show();
        this->saveClientFieldToDB("telephone", tel);
    });
    
    // --- Colonne droite : KPI verticaux ---
    QWidget* kpiWidget = new QWidget();
    QVBoxLayout* kpiLayout = new QVBoxLayout(kpiWidget);
    kpiLayout->setSpacing(15);
    kpiLayout->setAlignment(Qt::AlignTop);

    nbAbonnementsLabel = new QLabel("Nombre d'abonnements: 0", this);
    totalFacturesLabel = new QLabel("Total des factures: 0 FCFA", this);
    consoMoyenneLabel = new QLabel("Consommation moyenne: 0 m³", this);
    statutPaiementLabel = new QLabel("Statut des paiements: À jour", this);

    QString kpiStyle = "font-size: 15px; color: #e6e6e6; padding: 8px 18px; background: #31463f; border-radius: 6px; min-width:180px; margin-bottom: 0px;";
    nbAbonnementsLabel->setStyleSheet(kpiStyle);
    totalFacturesLabel->setStyleSheet(kpiStyle);
    consoMoyenneLabel->setStyleSheet(kpiStyle);
    statutPaiementLabel->setStyleSheet("font-size: 15px; color: #ffd23f; padding: 8px 18px; background: #31463f; border-radius: 6px; min-width:180px; margin-bottom: 8px;");

    kpiLayout->addWidget(nbAbonnementsLabel);
    kpiLayout->addWidget(totalFacturesLabel);
    kpiLayout->addWidget(consoMoyenneLabel);
    kpiLayout->addWidget(statutPaiementLabel);
    // Ne pas ajouter de stretch pour garder l'alignement parfait

    // Ajout des deux colonnes au layout principal
    mainHL->addWidget(persoWidget, 2); // infos perso plus large
    mainHL->addWidget(kpiWidget, 1); // kpi plus étroit
}



bool ClientDetailWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        // Édition infos perso
        if (obj == nomLabel) {
            auto* edit = reinterpret_cast<QLineEdit*>(this->property("nomEditPtr").value<void*>());
            if (edit) {
                edit->setText(nomLabel->text());
                nomLabel->hide();
                edit->show();
                edit->setFocus();
                edit->selectAll();
                return true;
            }
        } else if (obj == prenomLabel) {
            auto* edit = reinterpret_cast<QLineEdit*>(this->property("prenomEditPtr").value<void*>());
            if (edit) {
                edit->setText(prenomLabel->text());
                prenomLabel->hide();
                edit->show();
                edit->setFocus();
                edit->selectAll();
                return true;
            }
        } else if (obj == adresseLabel) {
            auto* edit = reinterpret_cast<QLineEdit*>(this->property("adresseEditPtr").value<void*>());
            if (edit) {
                edit->setText(adresseLabel->text());
                adresseLabel->hide();
                edit->show();
                edit->setFocus();
                edit->selectAll();
                return true;
            }
        } else if (obj == telephoneLabel) {
            auto* edit = reinterpret_cast<QLineEdit*>(this->property("telephoneEditPtr").value<void*>());
            if (edit) {
                edit->setText(telephoneLabel->text());
                telephoneLabel->hide();
                edit->show();
                edit->setFocus();
                edit->selectAll();
                return true;
            }
        }
        // Édition tableau abonnements
        if (obj == abonnementsTable) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QModelIndex index = abonnementsTable->indexAt(mouseEvent->pos());
            if (index.isValid() && (index.column() == 2 || index.column() == 3)) {
                abonnementEditRow = index.row();
                abonnementEditCol = index.column();
                abonnementsTable->edit(index);
                return true;
            }
        }
        // Édition tableau factures
        if (obj == facturesTable) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QModelIndex index = facturesTable->indexAt(mouseEvent->pos());
            if (index.isValid() && (index.column() == 1 || index.column() == 2 || index.column() == 3 || index.column() == 4)) {
                factureEditRow = index.row();
                factureEditCol = index.column();
                facturesTable->edit(index);
                return true;
            }
        }
        // Édition tableau paiements
        if (obj == paiementsTable) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QModelIndex index = paiementsTable->indexAt(mouseEvent->pos());
            if (index.isValid() && (index.column() == 0 || index.column() == 2)) {
                paiementEditRow = index.row();
                paiementEditCol = index.column();
                paiementsTable->edit(index);
                return true;
            }
        }
    }
    // Contrôle de la modification
    if (event->type() == QEvent::FocusOut && obj == abonnementsTable) {
        if (abonnementEditRow >= 0 && abonnementEditCol >= 0) {
            QModelIndex index = abonnementsModel->index(abonnementEditRow, abonnementEditCol);
            QString value = abonnementsModel->data(index).toString();
            if (abonnementEditCol == 2) { // Date
                QDate date = QDate::fromString(value, "dd/MM/yyyy");
                if (!date.isValid() || date > QDate::currentDate()) {
                    QMessageBox::warning(this, "Date invalide", "La date doit être valide et ne pas être dans le futur.");
                    abonnementsModel->setData(index, "01/01/2024");
                }
            } else if (abonnementEditCol == 3) { // Statut
                QString statut = value.trimmed().toLower();
                if (statut != "actif" && statut != "inactif") {
                    QMessageBox::warning(this, "Statut invalide", "Le statut doit être 'Actif' ou 'Inactif'.");
                    abonnementsModel->setData(index, "Actif");
                } else {
                    // Mettre la casse correcte
                    abonnementsModel->setData(index, statut == "actif" ? "Actif" : "Inactif");
                }
            }
            abonnementEditRow = -1;
            abonnementEditCol = -1;
        }
    }
    // Contrôle de la modification tableau factures
    if (event->type() == QEvent::FocusOut && obj == facturesTable) {
        if (factureEditRow >= 0 && factureEditCol >= 0) {
            QModelIndex index = facturesModel->index(factureEditRow, factureEditCol);
            QString value = facturesModel->data(index).toString();
            if (factureEditCol == 1) { // Date
                QDate date = QDate::fromString(value, "dd/MM/yyyy");
                if (!date.isValid() || date > QDate::currentDate()) {
                    QMessageBox::warning(this, "Date invalide", "La date doit être valide et ne pas être dans le futur.");
                    facturesModel->setData(index, "15/07/2024");
                }
            } else if (factureEditCol == 2) { // Montant
                QString montant = value;
                montant.remove(" FCFA").replace(",", ".");
                bool ok = false;
                double val = montant.toDouble(&ok);
                if (!ok || val < 0) {
                    QMessageBox::warning(this, "Montant invalide", "Le montant doit être un nombre positif.");
                    facturesModel->setData(index, "0,00 FCFA");
                }
            } else if (factureEditCol == 3) { // Statut
                QString statut = value.trimmed();
                if (statut != "Payée" && statut != "En attente" && statut != "En retard") {
                    QMessageBox::warning(this, "Statut invalide", "Le statut doit être 'Payée', 'En attente' ou 'En retard'.");
                    facturesModel->setData(index, "En attente");
                }
            } else if (factureEditCol == 4) { // Échéance
                QDate date = QDate::fromString(value, "dd/MM/yyyy");
                if (!date.isValid() || date < QDate::currentDate()) {
                    QMessageBox::warning(this, "Date d'échéance invalide", "La date doit être valide et dans le futur.");
                    facturesModel->setData(index, "15/08/2024");
                }
            }
            factureEditRow = -1;
            factureEditCol = -1;
        }
    }
    // Contrôle de la modification tableau paiements
    if (event->type() == QEvent::FocusOut && obj == paiementsTable) {
        if (paiementEditRow >= 0 && paiementEditCol >= 0) {
            QModelIndex index = paiementsModel->index(paiementEditRow, paiementEditCol);
            QString value = paiementsModel->data(index).toString();
            if (paiementEditCol == 0) { // Date
                QDate date = QDate::fromString(value, "dd/MM/yyyy");
                if (!date.isValid() || date > QDate::currentDate()) {
                    QMessageBox::warning(this, "Date invalide", "La date doit être valide et ne pas être dans le futur.");
                    paiementsModel->setData(index, "20/07/2024");
                }
            } else if (paiementEditCol == 2) { // Montant
                QString montant = value;
                montant.remove(" FCFA").replace(",", ".");
                bool ok = false;
                double val = montant.toDouble(&ok);
                if (!ok || val < 0) {
                    QMessageBox::warning(this, "Montant invalide", "Le montant doit être un nombre positif.");
                    paiementsModel->setData(index, "0,00 FCFA");
                }
            }
            paiementEditRow = -1;
            paiementEditCol = -1;
        }
    }
    return QWidget::eventFilter(obj, event);
}



// Méthode pour sauvegarder un champ modifié dans la base de données
void ClientDetailWidget::saveClientFieldToDB(const QString& field, const QString& value) {
    // Connexion à la base de données SQLite (supposée déjà ouverte ailleurs)
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur DB", "La base de données n'est pas ouverte.");
        return;
    }

    // Construction de la requête SQL dynamique
    QString queryStr = QString("UPDATE Client SET %1 = :val WHERE idClient = :id").arg(field);
    QSqlQuery query(db);
    query.prepare(queryStr);
    query.bindValue(":val", value);
    query.bindValue(":id", clientId.toInt());
    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur DB", "Échec de la mise à jour : " + query.lastError().text());
    } else {
        qDebug() << "Champ client mis à jour :" << field << "=" << value << "pour client" << clientId;
        QMessageBox::information(this, "Modification enregistrée", QString("Le champ '%1' a bien été mis à jour.").arg(field));
        // Recharger les infos du client depuis la base
        QSqlQuery reloadQuery(QSqlDatabase::database());
        reloadQuery.prepare("SELECT nom, prenom, adresse, telephone FROM Client WHERE idClient = :idClient");
        reloadQuery.bindValue(":idClient", clientId.toInt());
        if (reloadQuery.exec() && reloadQuery.next()) {
            setClientInfo(clientId,
                reloadQuery.value(0).toString(),
                reloadQuery.value(1).toString(),
                reloadQuery.value(2).toString(),
                reloadQuery.value(3).toString());
            // Recharger la liste des clients si le widget parent possède la méthode
            QWidget* parentWidget = this->parentWidget();
            if (parentWidget) {
                auto reloadClients = parentWidget->metaObject()->indexOfMethod("loadClientsFromDB()");
                if (reloadClients != -1) {
                    QMetaObject::invokeMethod(parentWidget, "loadClientsFromDB");
                }
            }
        }
    }
}


void ClientDetailWidget::setupAbonnementsTab() {
    // Layout pour l'onglet Abonnements (style compteur)
    QVBoxLayout* layout = new QVBoxLayout(abonnementsTab);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(8);
    // Suppression du titre pour plus d'espace au tableau
    abonnementsTable = new QTableView(this);
    abonnementsModel = new QStandardItemModel(this);
    QStringList headers = {"ID", "Compteur", "Date début", "Statut"};
    abonnementsModel->setHorizontalHeaderLabels(headers);
    abonnementsTable->setModel(abonnementsModel);
    abonnementsTable->setStyleSheet(
        "QTableView { background-color: #22332d; color: #e6e6e6; border: none; font-size: 15px; }"
        "QHeaderView::section { background-color: #22332d; color: #ffd23f; padding: 6px; border: none; font-size: 15px; }"
        "QTableView::item:selected { background-color: #ffd23f; color: #22332d; }"
    );
    abonnementsTable->setAlternatingRowColors(true);
    abonnementsTable->setStyleSheet(
        "QTableView { background-color: #22332d; alternate-background-color: #31463f; color: #e6e6e6; border: none; font-size: 15px; }"
        "QHeaderView::section { background-color: #22332d; color: #ffd23f; padding: 6px; border: none; font-size: 15px; }"
        "QTableView::item:selected { background-color: #ffd23f; color: #22332d; }"
    );
    abonnementsTable->verticalHeader()->setVisible(false);
    abonnementsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    abonnementsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    abonnementsTable->horizontalHeader()->setStretchLastSection(true);
    abonnementsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    abonnementsTable->setEditTriggers(QAbstractItemView::DoubleClicked);
    abonnementsTable->installEventFilter(this);
    ajouterAbonnementBtn = new QPushButton("+ Ajouter un abonnement", this);
    ajouterAbonnementBtn->setStyleSheet(
        "QPushButton { background: #ffd23f; color: #22332d; font-weight: bold; padding: 8px 18px; border-radius: 4px; margin-top: 18px; }"
        "QPushButton:hover { background: #ffdb6f; }"
    );
    ajouterAbonnementBtn->setCursor(Qt::PointingHandCursor);
    connect(ajouterAbonnementBtn, &QPushButton::clicked, this, &ClientDetailWidget::onAjouterAbonnementClicked);
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(ajouterAbonnementBtn);
    // layout->addWidget(abonnementsTitle); // supprimé pour plus d'espace

    // Barre de recherche et filtre statut
    QHBoxLayout* filterLayout = new QHBoxLayout;
    QLabel* filterLabel = new QLabel("Filtrer par statut :", this);
    QComboBox* statutCombo = new QComboBox(this);
    statutCombo->addItems({"Tous", "Actif", "Inactif"});
    QLineEdit* searchAbonnementEdit = new QLineEdit(this);
    searchAbonnementEdit->setPlaceholderText("Rechercher abonnement...");
    searchAbonnementEdit->setMinimumWidth(160);
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(statutCombo);
    filterLayout->addWidget(searchAbonnementEdit);
    layout->addLayout(filterLayout);

    layout->addWidget(abonnementsTable, 1);
    layout->addLayout(btnLayout);

    // Filtrage dynamique
    auto filterAbonnements = [=]() {
        QString text = searchAbonnementEdit->text();
        QString statut = statutCombo->currentText();
        for (int row = 0; row < abonnementsModel->rowCount(); ++row) {
            bool matchNum = abonnementsModel->item(row, 1)->text().contains(text, Qt::CaseInsensitive);
            QString rowStatut = abonnementsModel->item(row, 3)->text();
            bool matchStatut = (statut == "Tous") || (rowStatut == statut);
            abonnementsTable->setRowHidden(row, !(matchNum && matchStatut));
        }
    };
    connect(searchAbonnementEdit, &QLineEdit::textChanged, this, filterAbonnements);
    connect(statutCombo, &QComboBox::currentTextChanged, this, filterAbonnements);
}

void ClientDetailWidget::onAjouterFactureClicked() {
    if (clientId.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Aucun client sélectionné.");
        return;
    }
    // Récupérer la liste des compteurs actifs pour ce client, uniquement ceux dont le compteur est 'Transféré'
    QStringList compteursActifs;
    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("SELECT a.numCompteur FROM Abonnement a JOIN Compteur c ON a.numCompteur = c.numCompteur WHERE a.idClient = :idClient AND a.date_abonnement = (SELECT MAX(date_abonnement) FROM Abonnement WHERE numCompteur = a.numCompteur) AND c.attributComp = 'Transféré'");
        query.bindValue(":idClient", clientId.toInt());
        if (query.exec()) {
            while (query.next()) {
                compteursActifs << query.value(0).toString();
            }
        }
    }
    FactureFormDialog dialog(compteursActifs, this);
    dialog.setWindowTitle("Ajouter une facture");
    dialog.setFacture(Facture(0, compteursActifs.isEmpty() ? "" : compteursActifs.first(), clientId.toInt(), 0, 0));
    if (dialog.exec() == QDialog::Accepted) {
        Facture facture = dialog.getFacture(clientId.toInt());
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isOpen()) {
            QMessageBox::critical(this, "Erreur DB", "La base de données n'est pas ouverte.");
            return;
        }
        QSqlQuery query(db);
        query.prepare("INSERT INTO Facture (numCompteur, idClient, soldeanterieur, consommation) VALUES (:numCompteur, :idClient, :soldeanterieur, :consommation)");
        query.bindValue(":numCompteur", facture.numCompteur);
        query.bindValue(":idClient", facture.idClient);
        query.bindValue(":soldeanterieur", facture.soldeanterieur);
        query.bindValue(":consommation", facture.consommation);
        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur DB", "Échec de l'ajout de la facture : " + query.lastError().text());
            return;
        }
        // Mettre à jour la liste des compteurs actifs pour le prochain ajout (avec critère 'Transféré')
        compteursActifs.clear();
        QSqlQuery refreshQuery(db);
        refreshQuery.prepare("SELECT a.numCompteur FROM Abonnement a JOIN Compteur c ON a.numCompteur = c.numCompteur WHERE a.idClient = :idClient AND a.date_abonnement = (SELECT MAX(date_abonnement) FROM Abonnement WHERE numCompteur = a.numCompteur) AND c.attributComp = 'Transféré'");
        refreshQuery.bindValue(":idClient", clientId.toInt());
        if (refreshQuery.exec()) {
            while (refreshQuery.next()) {
                compteursActifs << refreshQuery.value(0).toString();
            }
        }
        loadFacturesFromDB();
        emit factureAjouteSignal();
    }
}
// ...existing includes...


void ClientDetailWidget::setupFacturesTab() {
    // Layout principal de l'onglet Factures (copie du style Compteur)
    QVBoxLayout* layout = new QVBoxLayout(facturesTab);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    facturesTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Filtres horizontaux
    QHBoxLayout* filterLayout = new QHBoxLayout;
    QLabel* filterLabel = new QLabel("Filtrer par statut :", this);
    statutCombo = new QComboBox(this);
    statutCombo->addItems({"Tous", "Payée", "En attente", "En retard"});
    QLineEdit* searchFactureEdit = new QLineEdit(this);
    searchFactureEdit->setPlaceholderText("Rechercher facture...");
    searchFactureEdit->setMinimumWidth(160);
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(statutCombo);
    filterLayout->addWidget(searchFactureEdit);
    layout->addLayout(filterLayout);

    // Tableau des factures
    facturesTable = new QTableView(this);
    facturesModel = new QStandardItemModel(this);
    QStringList headers = {"ID", "Date", "Statut", "Échéance", "Actions"};
    facturesModel->setHorizontalHeaderLabels(headers);
    facturesTable->setModel(facturesModel);
    facturesTable->setStyleSheet(
        "QTableView { background-color: #22332d; color: #e6e6e6; border: none; font-size: 15px; }"
        "QHeaderView::section { background-color: #22332d; color: #ffd23f; padding: 6px; border: none; font-size: 15px; }"
        "QTableView::item:selected { background-color: #ffd23f; color: #22332d; }"
    );
    facturesTable->setAlternatingRowColors(true);
    facturesTable->setStyleSheet(
        "QTableView { background-color: #22332d; alternate-background-color: #31463f; color: #e6e6e6; border: none; font-size: 15px; }"
        "QHeaderView::section { background-color: #22332d; color: #ffd23f; padding: 6px; border: none; font-size: 15px; }"
        "QTableView::item:selected { background-color: #ffd23f; color: #22332d; }"
    );
    facturesTable->verticalHeader()->setVisible(false);
    facturesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    facturesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    facturesTable->horizontalHeader()->setStretchLastSection(true);
    facturesTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    facturesTable->setEditTriggers(QAbstractItemView::DoubleClicked);
    facturesTable->installEventFilter(this);
    layout->addWidget(facturesTable, 1);

    // Boutons en bas à droite
    QHBoxLayout* btnLayout = new QHBoxLayout;
    ajouterFactureBtn = new QPushButton("+ Ajouter une facture", this);
    ajouterFactureBtn->setStyleSheet("QPushButton { background: #ffd23f; color: #22332d; font-weight: bold; padding: 8px 18px; border-radius: 4px; margin-right: 12px; } QPushButton:hover { background: #ffdb6f; }");
    ajouterFactureBtn->setCursor(Qt::PointingHandCursor);
    connect(ajouterFactureBtn, &QPushButton::clicked, this, &ClientDetailWidget::onAjouterFactureClicked);
    btnLayout->addStretch();
    btnLayout->addWidget(ajouterFactureBtn);
    layout->addLayout(btnLayout);
// ...existing code...

    // Filtrage dynamique
    auto filterFactures = [=]() {
        QString text = searchFactureEdit->text();
        QString statut = statutCombo->currentText();
        int statutCol = 3; // colonne Statut
        for (int row = 0; row < facturesModel->rowCount(); ++row) {
            QStandardItem *numItem = facturesModel->item(row, 0);
            QStandardItem *statutItem = facturesModel->item(row, statutCol);
            if (!numItem || !statutItem) {
                facturesTable->setRowHidden(row, true);
                continue;
            }
            bool matchNum = numItem->text().contains(text, Qt::CaseInsensitive);
            QString rowStatut = statutItem->text();
            bool matchStatut = (statut == "Tous") || (rowStatut == statut);
            facturesTable->setRowHidden(row, !(matchNum && matchStatut));
        }
    };
    connect(searchFactureEdit, &QLineEdit::textChanged, this, filterFactures);
    connect(statutCombo, &QComboBox::currentTextChanged, this, filterFactures);
}

void ClientDetailWidget::setupHistoriqueTab() {
    // Layout principal vertical pour l'onglet Historique (graphique seul, sans historique des paiements)
    QVBoxLayout* mainVL = new QVBoxLayout(historiqueTab);
    mainVL->setContentsMargins(20, 20, 20, 20);
    mainVL->setSpacing(0);

    // --- Bloc graphique consommation amélioré et fixe ---
    QLabel* graphTitle = new QLabel("Consommation annuelle (m³)", this);
    graphTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #ffd23f; background: transparent; padding: 8px 0px; margin-bottom: 8px; text-align: left;");
    mainVL->addWidget(graphTitle, 0, Qt::AlignHCenter);

    createConsommationChart();
    // Fixer la taille du graphique pour qu'il soit toujours visible sans scroll
    consommationChartView->setMinimumHeight(340);
    consommationChartView->setMaximumHeight(340);
    consommationChartView->setMinimumWidth(900);
    consommationChartView->setMaximumWidth(900);
    consommationChartView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    consommationChartView->setStyleSheet("background: #31463f; border-radius: 16px; box-shadow: 0px 4px 18px #22332d;");
    mainVL->addWidget(consommationChartView, 0, Qt::AlignHCenter);
}

void ClientDetailWidget::createConsommationChart() {
    // Création d'un ensemble de barres
    QBarSet *barSet = new QBarSet("");
    barSet->setColor(QColor("#00cfff")); // Couleur bleu vive
    barSet->setLabelColor(QColor("#ffd23f"));
    barSet->setLabelFont(QFont("Arial", 13, QFont::Bold));
    // Ajout des données d'exemple pour chaque mois (à remplacer par les données réelles)
    *barSet << 10 << 8 << 12 << 15 << 13 << 18 << 16 << 14 << 10 << 9 << 11 << 13;

    // Création de la série de barres
    QBarSeries *series = new QBarSeries();
    series->append(barSet);
    series->setBarWidth(0.7); // Barres plus fines pour meilleure lisibilité
    series->setLabelsPosition(QAbstractBarSeries::LabelsInsideEnd);
    series->setLabelsVisible(true);

    // Création du graphique
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setBackgroundBrush(QBrush(QColor("#31463f"))); // Fond plus clair
    chart->setAnimationOptions(QChart::NoAnimation); // Pas d'animation pour affichage instantané
    chart->legend()->setVisible(false); // Masquer la légende pour plus de place
    chart->setMargins(QMargins(30, 30, 30, 30)); // Marges pour ne pas couper le graphique

    // Création de l'axe des catégories (mois)
    QStringList categories = {"Jan", "Fév", "Mar", "Avr", "Mai", "Jun", "Jul", "Aoû", "Sep", "Oct", "Nov", "Déc"};
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(QColor("#ffd23f"));
    axisX->setLabelsFont(QFont("Arial", 14, QFont::Bold));
    axisX->setGridLineVisible(false); // Pas de grille verticale
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Création de l'axe des valeurs (consommation)
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Consommation (m³)");
    axisY->setRange(0, 20);
    axisY->setTickCount(5);
    axisY->setLabelFormat("%d");
    axisY->setTitleBrush(QBrush(QColor("#ffd23f")));
    axisY->setLabelsColor(QColor("#ffd23f"));
    axisY->setLabelsFont(QFont("Arial", 14, QFont::Bold));
    axisY->setGridLineColor(QColor("#ffd23f")); // Grille jaune
    axisY->setGridLineVisible(true);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Création de la vue du graphique
    consommationChartView = new QChartView(chart);
    consommationChartView->setRenderHint(QPainter::Antialiasing);
}

// Implémentations des méthodes de chargement des données
void ClientDetailWidget::loadAbonnementsFromDB() {
    // Effacer le modèle existant
    abonnementsModel->removeRows(0, abonnementsModel->rowCount());

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur DB", "La base de données n'est pas ouverte.");
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id, numCompteur, date_abonnement FROM Abonnement WHERE idClient = :idClient");
    query.bindValue(":idClient", clientId.toInt());
    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur DB", "Impossible de charger les abonnements : " + query.lastError().text());
        return;
    }

    int count = 0;
    QStringList logAbonnements;
    while (query.next()) {
        QString id = query.value(0).toString();
        QString numCompteur = query.value(1).toString();
        QDate dateDebut = query.value(2).toDate();
        QString dateDebutStr = dateDebut.toString("dd/MM/yyyy");

        // Vérifier si le compteur est bien 'Transféré'
        QString attributComp;
        QSqlQuery queryComp(db);
        queryComp.prepare("SELECT attributComp FROM Compteur WHERE numCompteur = :numCompteur");
        queryComp.bindValue(":numCompteur", numCompteur);
        if (queryComp.exec() && queryComp.next()) {
            attributComp = queryComp.value(0).toString();
        } else {
            attributComp = "";
        }

        // Vérifier si cet abonnement est le plus récent pour ce compteur ET que le compteur est transféré
        QSqlQuery queryMax(db);
        queryMax.prepare("SELECT MAX(date_abonnement) FROM Abonnement WHERE numCompteur = :numCompteur");
        queryMax.bindValue(":numCompteur", numCompteur);
        QString statut = "Inactif";
        if (queryMax.exec() && queryMax.next()) {
            QDate maxDate = queryMax.value(0).toDate();
            if (dateDebut == maxDate && attributComp == "Transféré") {
                statut = "Actif";
            }
        }

        QList<QStandardItem*> items;
    QStandardItem* itemId = new QStandardItem(id);
    itemId->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    items << itemId;
    QStandardItem* itemCompteur = new QStandardItem(numCompteur);
    itemCompteur->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    items << itemCompteur;
    QStandardItem* itemDate = new QStandardItem(dateDebutStr);
    itemDate->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    items << itemDate;
    QStandardItem* itemStatut = new QStandardItem(statut);
    itemStatut->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    items << itemStatut;
        abonnementsModel->appendRow(items);
        logAbonnements << QString("id=%1, compteur=%2, date=%3, statut=%4").arg(id, numCompteur, dateDebutStr, statut);
        count++;
    }
    qDebug() << "Abonnements chargés:" << logAbonnements;
    nbAbonnementsLabel->setText("Nombre d'abonnements: " + QString::number(count));
}

void ClientDetailWidget::loadFacturesFromDB() {
    // Effacer le modèle existant
    facturesModel->removeRows(0, facturesModel->rowCount());
    // Réappliquer le style et les propriétés après reload
    facturesTable->setStyleSheet(
        "QTableView { background-color: #22332d; color: #e6e6e6; border: none; font-size: 15px; }"
        "QHeaderView::section { background-color: #22332d; color: #ffd23f; padding: 6px; border: none; font-size: 15px; }"
        "QTableView::item:selected { background-color: #ffd23f; color: #22332d; }"
    );
    facturesTable->setAlternatingRowColors(false);
    facturesTable->verticalHeader()->setVisible(false);
    facturesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    facturesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    facturesTable->horizontalHeader()->setStretchLastSection(true);
    facturesTable->setEditTriggers(QAbstractItemView::DoubleClicked);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur DB", "La base de données n'est pas ouverte.");
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT idFacture, numCompteur, soldeanterieur, consommation FROM Facture WHERE idClient = :idClient");
    query.bindValue(":idClient", clientId.toInt());
    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur DB", "Impossible de charger les factures : " + query.lastError().text());
        return;
    }

    double total = 0.0;
    int count = 0;
    while (query.next()) {
        QString id = query.value(0).toString();
        QString numCompteur = query.value(1).toString();
        double solde = query.value(2).toDouble();
        double conso = query.value(3).toDouble();
        QList<QStandardItem*> items;
    QStandardItem* itemId = new QStandardItem(id);
    itemId->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    items << itemId;
    QStandardItem* itemCompteur = new QStandardItem(numCompteur);
    itemCompteur->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    items << itemCompteur;
    QStandardItem* itemSolde = new QStandardItem(QString::number(solde, 'f', 2).replace(".", ",") + " FCFA");
    itemSolde->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    items << itemSolde;
    QStandardItem* itemConso = new QStandardItem(QString::number(conso, 'f', 2).replace(".", ",") + " m³");
    itemConso->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    items << itemConso;

        // Créer un widget d'actions avec deux boutons
        QWidget* actionsWidget = new QWidget();
        QHBoxLayout* actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->setContentsMargins(0, 0, 0, 0);
        actionsLayout->setSpacing(6);

        QPushButton* exportBtn = new QPushButton();
    exportBtn->setIcon(QIcon(":/icons/material/visibility.svg"));
    exportBtn->setToolTip("Exporter la facture");
    exportBtn->setIconSize(QSize(20, 20));
    exportBtn->setFixedSize(28, 28);
    exportBtn->setStyleSheet("background: #ffd23f; border-radius: 6px; padding: 2px; display: flex; align-items: center; justify-content: center;");
    exportBtn->setCursor(Qt::PointingHandCursor);

    QPushButton* deleteBtn = new QPushButton();
    deleteBtn->setIcon(QIcon(":/icons/material/delete.svg"));
    deleteBtn->setToolTip("Supprimer la facture");
    deleteBtn->setIconSize(QSize(20, 20));
    deleteBtn->setFixedSize(28, 28);
    deleteBtn->setStyleSheet("background: #ffd23f; border-radius: 6px; padding: 2px; display: flex; align-items: center; justify-content: center;");
    deleteBtn->setCursor(Qt::PointingHandCursor);

        actionsLayout->addWidget(exportBtn);
        actionsLayout->addWidget(deleteBtn);
        actionsLayout->addStretch();

        // Créer un QStandardItem pour l'Actions (vide, juste pour la structure)
        QStandardItem* itemActions = new QStandardItem();
        items << itemActions;
        facturesModel->appendRow(items);

        // Placer le widget dans la vue
        QModelIndex idx = facturesModel->index(facturesModel->rowCount() - 1, 4);
        facturesTable->setIndexWidget(idx, actionsWidget);

        // Connecter les boutons aux slots
        connect(exportBtn, &QPushButton::clicked, this, [=]() {
            onExportFactureDelegate(id);
        });
        connect(deleteBtn, &QPushButton::clicked, this, [=]() {
            onSupprimerFactureDelegate(id);
        });
        count++;
    }
    totalFacturesLabel->setText(QString("Total des factures: %1 FCFA").arg(total, 0, 'f', 2).replace(".", ","));
    statutPaiementLabel->setText("Statut des paiements: N/A");
    statutPaiementLabel->setStyleSheet("font-size: 16px; color: #FFD23F; padding: 8px; background: #31463f; border-radius: 4px;");
}


void ClientDetailWidget::loadConsommationFromDB() {
    // Pour l'instant, on utilise les données d'exemple du graphique
    // À l'avenir, ces données proviendront de la base de données
    // Calculer la consommation moyenne
    double consoMoyenne = 11.6; // Exemple: moyenne des valeurs dans le graphique
    consoMoyenneLabel->setText(QString("Consommation moyenne: %1 m³").arg(consoMoyenne, 0, 'f', 1).replace(".", ","));
    emit consommationAjouteSignal();
}
