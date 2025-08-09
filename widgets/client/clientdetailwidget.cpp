// ...existing includes...
#include "widgets/factureactionsdelegate.h"
#include "widgets/clientdetailwidget.h"
#include "widgets/abonnementformdialog.h"
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
    
    // Appliquer le style global
    setStyleSheet("background: #22332d; border-radius: 16px; padding: 32px;");
}


// ...existing code...
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
    loadPrelevementsFromDB();
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
            // Charger ou rafraîchir l'historique des paiements
            loadPrelevementsFromDB();
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

    AbonnementFormDialog dialog(this);
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
        }

        // Les champs adresse et consommation restent affichés dans le tableau (modèle local)
        // Le statut affiché dans le tableau est celui de attributComp

        // Rafraîchir l'affichage des abonnements
        loadAbonnementsFromDB();
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
        "QTabWidget::pane { border: 1px solid #31463f; background: #31463f; border-radius: 8px; }"
        "QTabBar::tab { background: #22332d; color: #b7e0c0; padding: 10px 20px; border-top-left-radius: 4px; border-top-right-radius: 4px; margin-right: 2px; }"
        "QTabBar::tab:selected { background: #31463f; color: #ffd23f; }"
        "QTabBar::tab:hover { background: #2c3e38; }"
    );
    
    // Création des onglets
    infoTab = new QWidget;
    infoScrollArea = new QScrollArea;
    abonnementsTab = new QWidget;
    abonnementsScrollArea = new QScrollArea;
    facturesTab = new QWidget;
    facturesScrollArea = new QScrollArea;
    historiqueTab = new QWidget;
    historiqueScrollArea = new QScrollArea;
    
    // Configuration des zones de défilement
    infoScrollArea->setWidgetResizable(true);
    infoScrollArea->setFrameShape(QFrame::NoFrame);
    infoScrollArea->setWidget(infoTab);
    
    abonnementsScrollArea->setWidgetResizable(true);
    abonnementsScrollArea->setFrameShape(QFrame::NoFrame);
    abonnementsScrollArea->setWidget(abonnementsTab);
    
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
    tabWidget->addTab(abonnementsScrollArea, "Abonnements");
    tabWidget->addTab(facturesScrollArea, "Factures");
    tabWidget->addTab(historiqueScrollArea, "Historique");
    
    // Ajout des layouts au layout principal
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(tabWidget, 1);
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
    nomLabel->setStyleSheet("color: #e6e6e6;");
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
    }
}


void ClientDetailWidget::setupAbonnementsTab() {
    // Layout pour l'onglet Abonnements
    QVBoxLayout* layout = new QVBoxLayout(abonnementsTab);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Tableau des abonnements qui occupe tout l'espace
    abonnementsTable = new QTableView(this);
    abonnementsModel = new QStandardItemModel(this);
    QStringList headers = {"ID", "Compteur", "Date début", "Statut"};
    abonnementsModel->setHorizontalHeaderLabels(headers);
    abonnementsTable->setModel(abonnementsModel);
    abonnementsTable->setStyleSheet(
        "QTableView { background-color: #31463f; alternate-background-color: #2c3e38; color: #e6e6e6; border: none; }"
        "QTableView::item:selected { background-color: #42a5f5; color: white; }"
    );
    abonnementsTable->setAlternatingRowColors(true);
    abonnementsTable->verticalHeader()->setVisible(false);
    abonnementsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    abonnementsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    abonnementsTable->horizontalHeader()->setStretchLastSection(true);
    abonnementsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // Autoriser l'édition par double-clic uniquement
    abonnementsTable->setEditTriggers(QAbstractItemView::DoubleClicked);
    // Installer l'eventFilter pour contrôler l'édition
    abonnementsTable->installEventFilter(this);

    // Bouton pour ajouter un abonnement
    ajouterAbonnementBtn = new QPushButton("+ Ajouter un abonnement", this);
    ajouterAbonnementBtn->setStyleSheet(
        "QPushButton { background: #ffd23f; color: #22332d; font-weight: bold; padding: 10px; border-radius: 4px; margin-top: 32px; }"
        "QPushButton:hover { background: #ffdb6f; }"
    );
    ajouterAbonnementBtn->setCursor(Qt::PointingHandCursor);
    connect(ajouterAbonnementBtn, &QPushButton::clicked, this, &ClientDetailWidget::onAjouterAbonnementClicked);

    // Layout pour le bouton en bas à droite
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(ajouterAbonnementBtn);

    // Ajout des widgets au layout principal de l'onglet
    layout->addWidget(abonnementsTable, 1);
    layout->addLayout(btnLayout);
}

void ClientDetailWidget::setupFacturesTab() {
    // Layout principal de l'onglet Factures
    QVBoxLayout* layout = new QVBoxLayout(facturesTab);
    layout->setContentsMargins(0, 0, 0, 0); // aucune marge
    layout->setSpacing(2); // espacement minimal
    facturesTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Layout d'en-tête (titre et filtres)
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* title = new QLabel("Factures", this);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #ffd23f;");
    QLabel* periodeLabel = new QLabel("Période:", this);
    periodeLabel->setStyleSheet("color: #e6e6e6;");
    periodeCombo = new QComboBox(this);
    periodeCombo->addItems({"Toutes", "Ce mois", "Ce trimestre", "Cette année"});
    periodeCombo->setStyleSheet(
        "QComboBox { background: #31463f; color: #e6e6e6; padding: 5px; border-radius: 4px; }"
        "QComboBox::drop-down { border: 0px; }"
        "QComboBox::down-arrow { image: url(:/icons/material/arrow_drop_down.svg); width: 16px; height: 16px; }"
    );
    QLabel* statutLabel = new QLabel("Statut:", this);
    statutLabel->setStyleSheet("color: #e6e6e6;");
    statutCombo = new QComboBox(this);
    statutCombo->addItems({"Tous", "Payée", "En attente", "En retard"});
    statutCombo->setStyleSheet(
        "QComboBox { background: #31463f; color: #e6e6e6; padding: 5px; border-radius: 4px; }"
        "QComboBox::drop-down { border: 0px; }"
        "QComboBox::down-arrow { image: url(:/icons/material/arrow_drop_down.svg); width: 16px; height: 16px; }"
    );
    connect(periodeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ClientDetailWidget::filtrerFactures);
    connect(statutCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ClientDetailWidget::filtrerFactures);
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(periodeLabel);
    headerLayout->addWidget(periodeCombo);
    headerLayout->addWidget(statutLabel);
    headerLayout->addWidget(statutCombo);

    // Tableau des factures directement dans le layout
    facturesTable = new QTableView(this);
    facturesModel = new QStandardItemModel(this);
    QStringList headers = {"ID", "Date", "Montant", "Statut", "Échéance", "Actions"};
    facturesModel->setHorizontalHeaderLabels(headers);
    facturesTable->setModel(facturesModel);
    facturesTable->setStyleSheet(
        "QTableView { background-color: #31463f; alternate-background-color: #2c3e38; color: #e6e6e6; border: none; }"
        "QHeaderView::section { background-color: #22332d; color: #ffd23f; padding: 6px; border: 1px solid #31463f; }"
        "QTableView::item:selected { background-color: #42a5f5; color: white; }"
    );
    facturesTable->setAlternatingRowColors(true);
    facturesTable->verticalHeader()->setVisible(false);
    facturesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    facturesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    facturesTable->horizontalHeader()->setStretchLastSection(true);
    facturesTable->setEditTriggers(QAbstractItemView::DoubleClicked);
    facturesTable->installEventFilter(this);
    facturesTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Seul le bouton Générer une facture reste en bas
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    genererFactureBtn = new QPushButton("Générer une facture", this);
    QString btnStyle = "QPushButton { padding: 10px 22px; border-radius: 6px; font-size: 16px; font-weight: 500; box-shadow: 0px 2px 8px #22332d; }";
    genererFactureBtn->setStyleSheet(btnStyle + "QPushButton { background: #ffd23f; color: #22332d; font-weight: bold; } QPushButton:hover { background: #ffdb6f; }");
    genererFactureBtn->setCursor(Qt::PointingHandCursor);
    connect(genererFactureBtn, &QPushButton::clicked, this, &ClientDetailWidget::onGenererFactureClicked);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(genererFactureBtn);

    // Ajout au layout principal : header, tableau, boutons fixes
    layout->addLayout(headerLayout);
    layout->addSpacing(2); // espacement minimal entre filtres et tableau
    layout->addWidget(facturesTable, 1); // stretch pour occuper tout l'espace
    layout->addSpacing(2); // espacement minimal entre tableau et boutons
    layout->addLayout(buttonsLayout);
}

void ClientDetailWidget::setupHistoriqueTab() {
    // Layout principal horizontal pour l'onglet Historique
    QHBoxLayout* mainHL = new QHBoxLayout(historiqueTab);
    mainHL->setContentsMargins(20, 20, 20, 20);
    mainHL->setSpacing(30);

    // --- Bloc graphique consommation ---
    QWidget* graphWidget = new QWidget();
    QVBoxLayout* graphLayout = new QVBoxLayout(graphWidget);
    graphLayout->setContentsMargins(0, 0, 0, 0);
    graphLayout->setSpacing(10);
    QLabel* graphTitle = new QLabel("Historique de consommation", this);
    graphTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: #ffd23f; margin-bottom: 12px; background: #22332d; border-radius: 8px; padding: 8px 18px;");
    createConsommationChart();
    consommationChartView->setMinimumHeight(420);
    consommationChartView->setMinimumWidth(520);
    consommationChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graphLayout->addWidget(graphTitle);
    graphLayout->addWidget(consommationChartView, 1);
    graphWidget->setStyleSheet("background: #31463f; border-radius: 12px; box-shadow: 0px 2px 12px #22332d;");

    // --- Bloc tableau paiements ---
    QWidget* tableWidget = new QWidget();
    QVBoxLayout* tableLayout = new QVBoxLayout(tableWidget);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->setSpacing(10);
    QLabel* paiementsTitle = new QLabel("Historique des paiements", this);
    paiementsTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: #ffd23f; margin-bottom: 12px; background: #22332d; border-radius: 8px; padding: 8px 18px;");
    paiementsTable = new QTableView(this);
    paiementsModel = new QStandardItemModel(this);
    QStringList paiementsHeaders = {"Date", "Facture", "Montant"};
    paiementsModel->setHorizontalHeaderLabels(paiementsHeaders);
    paiementsTable->setModel(paiementsModel);
    paiementsTable->setStyleSheet(
        "QTableView { background-color: #31463f; alternate-background-color: #2c3e38; color: #e6e6e6; border: none; font-size: 16px; }"
        "QHeaderView::section { background-color: #22332d; color: #ffd23f; padding: 8px; border: 1px solid #31463f; font-size: 16px; }"
        "QTableView::item:selected { background-color: #42a5f5; color: white; }"
    );
    paiementsTable->setAlternatingRowColors(true);
    paiementsTable->verticalHeader()->setVisible(false);
    paiementsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    paiementsTable->horizontalHeader()->setStretchLastSection(true);
    paiementsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    paiementsTable->setEditTriggers(QAbstractItemView::DoubleClicked);
    paiementsTable->installEventFilter(this);
    tableLayout->addWidget(paiementsTitle);
    tableLayout->addWidget(paiementsTable, 1);
    tableWidget->setStyleSheet("background: #31463f; border-radius: 12px; box-shadow: 0px 2px 12px #22332d;");

    // Ajout des deux blocs côte à côte
    mainHL->addWidget(graphWidget, 3); // Augmente le stretch du graphique
    mainHL->addWidget(tableWidget, 2);
}

void ClientDetailWidget::createConsommationChart() {
    // Création d'un ensemble de barres
    QBarSet *barSet = new QBarSet("Consommation d'eau");
    barSet->setColor(QColor("#42a5f5")); // Couleur bleu eau
    
    // Ajout des données d'exemple pour chaque mois (à remplacer par les données réelles)
    *barSet << 10 << 8 << 12 << 15 << 13 << 18 << 16 << 14 << 10 << 9 << 11 << 13;
    
    // Création de la série de barres
    QBarSeries *series = new QBarSeries();
    series->append(barSet);
    
    // Création du graphique
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Consommation Annuelle (m³)");
    chart->setTitleFont(QFont("Arial", 12, QFont::Bold));
    chart->setTitleBrush(QBrush(QColor("#ffd23f")));
    chart->setBackgroundBrush(QBrush(QColor("#31463f")));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    // Création de l'axe des catégories (mois)
    QStringList categories = {"Jan", "Fév", "Mar", "Avr", "Mai", "Jun", "Jul", "Aoû", "Sep", "Oct", "Nov", "Déc"};
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(QColor("#e6e6e6"));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    // Création de l'axe des valeurs (consommation)
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Consommation (m³)");
    axisY->setRange(0, 20);
    axisY->setTickCount(5);
    axisY->setLabelFormat("%d");
    axisY->setTitleBrush(QBrush(QColor("#e6e6e6")));
    axisY->setLabelsColor(QColor("#e6e6e6"));
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
    
    // Pour l'instant, on ajoute des données fictives (à remplacer par la vraie requête DB)
    QList<QStringList> donnees = {
        {"AB001", "C00123", "01/01/2024", "Actif"},
        {"AB002", "C00456", "15/03/2024", "Actif"},
    };
    
    // Remplir le modèle avec les données
    for (const QStringList& ligne : donnees) {
        QList<QStandardItem*> items;
        for (int i = 0; i < ligne.size(); ++i) {
            QString valeur = ligne[i];
            if (i == 3) { // colonne statut
                bool statutBool = (valeur.trimmed().toLower() == "actif");
                valeur = statutBool ? "Actif" : "Inactif";
            }
            QStandardItem* item = new QStandardItem(valeur);
            // Rendre les colonnes 0 et 1 non éditables
            if (i == 0 || i == 1) item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            items << item;
        }
        abonnementsModel->appendRow(items);
    }
    
    // Mettre à jour le compteur d'abonnements dans l'onglet Info
    nbAbonnementsLabel->setText("Nombre d'abonnements: " + QString::number(donnees.size()));
}

void ClientDetailWidget::loadFacturesFromDB() {
    // Effacer le modèle existant
    facturesModel->removeRows(0, facturesModel->rowCount());
    
    // Pour l'instant, on ajoute des données fictives (à remplacer par la vraie requête DB)
    QList<QStringList> donnees = {
        {"F00123", "15/07/2024", "78,50 FCFA", "Payée", "15/08/2024", ""},
        {"F00124", "15/08/2024", "92,20 FCFA", "En attente", "15/09/2024", ""},
    };
    
    // Remplir le modèle avec les données, style pro
    for (const QStringList& ligne : donnees) {
        QList<QStandardItem*> items;
        for (int i = 0; i < ligne.size(); ++i) {
            QString valeur = ligne[i];
            QStandardItem* item = new QStandardItem(valeur);
            if (i == 2) { // Montant
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                QFont f = item->font(); f.setBold(true); item->setFont(f);
            }
            if (i == 3) { // Statut
                // Badge coloré
                if (valeur == "Payée") item->setBackground(QColor("#4CAF50"));
                else if (valeur == "En attente") item->setBackground(QColor("#FFC107"));
                else if (valeur == "En retard") item->setBackground(QColor("#F44336"));
                item->setForeground(QBrush(QColor("#22332d")));
                QFont f = item->font(); f.setBold(true); item->setFont(f);
            }
            if (i == 5) { // Actions
                // Ajout d'icônes d'action (fictif, à remplacer par vrais boutons délégués si besoin)
                item->setText("💳  📄  🗑️");
                item->setTextAlignment(Qt::AlignCenter);
            }
            items << item;
        }
        facturesModel->appendRow(items);
    }
    
    // Calculer le total des factures
    double total = 0.0;
    for (const QStringList& ligne : donnees) {
        QString montant = ligne[2];
        montant.remove(" FCFA").replace(",", ".");
        total += montant.toDouble();
    }
    // Mettre à jour le total des factures dans l'onglet Info
    totalFacturesLabel->setText(QString("Total des factures: %1 FCFA").arg(total, 0, 'f', 2).replace(".", ","));
    
    // Mettre à jour le statut de paiement
    bool toutPaye = true;
    for (const QStringList& ligne : donnees) {
        if (ligne[3] != "Payée") {
            toutPaye = false;
            break;
        }
    }
    
    statutPaiementLabel->setText("Statut des paiements: " + QString(toutPaye ? "À jour" : "En attente"));
    statutPaiementLabel->setStyleSheet("font-size: 16px; color: " + QString(toutPaye ? "#4CAF50" : "#FFC107") + 
                                      "; padding: 8px; background: #31463f; border-radius: 4px;");
}

void ClientDetailWidget::loadPrelevementsFromDB() {
    // Charger les données des paiements
    paiementsModel->removeRows(0, paiementsModel->rowCount());
    
    QList<QStringList> paiements = {
        {"20/07/2024", "F00123", "78,50 FCFA"},
        {"18/06/2024", "F00122", "85,30 FCFA"},
        {"15/05/2024", "F00121", "92,40 FCFA"},
        {"20/04/2024", "F00120", "79,80 FCFA"},
        {"18/03/2024", "F00119", "84,10 FCFA"},
        {"15/02/2024", "F00118", "76,70 FCFA"},
    };
    for (const QStringList& ligne : paiements) {
        QList<QStandardItem*> items;
        for (const QString& valeur : ligne) {
            items << new QStandardItem(valeur);
        }
        paiementsModel->appendRow(items);
    }
}

void ClientDetailWidget::loadConsommationFromDB() {
    // Pour l'instant, on utilise les données d'exemple du graphique
    // À l'avenir, ces données proviendront de la base de données
    
    // Calculer la consommation moyenne
    double consoMoyenne = 11.6; // Exemple: moyenne des valeurs dans le graphique
    consoMoyenneLabel->setText(QString("Consommation moyenne: %1 m³").arg(consoMoyenne, 0, 'f', 1).replace(".", ","));
}
