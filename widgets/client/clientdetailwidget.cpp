#include "widgets/clientdetailwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

ClientDetailWidget::ClientDetailWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* topLayout = new QHBoxLayout;
    retourBtn = new QPushButton("← Retour", this);
    retourBtn->setFixedSize(120, 38);
    retourBtn->setStyleSheet("background: #ffd23f; color: #3d554b; font-size: 18px; font-weight: bold; border-radius: 8px; padding: 6px 18px;");
    retourBtn->setCursor(Qt::PointingHandCursor);
    topLayout->addWidget(retourBtn, 0, Qt::AlignLeft);
    topLayout->addStretch();
    layout->addLayout(topLayout);
    nomLabel = new QLabel(this);
    prenomLabel = new QLabel(this);
    adresseLabel = new QLabel(this);
    telephoneLabel = new QLabel(this);
    nomLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #42a5f5;");
    prenomLabel->setStyleSheet("font-size: 18px; color: #e6e6e6;");
    adresseLabel->setStyleSheet("font-size: 16px; color: #b7e0c0;");
    telephoneLabel->setStyleSheet("font-size: 16px; color: #b7e0c0;");
    layout->addWidget(nomLabel);
    layout->addWidget(prenomLabel);
    layout->addWidget(adresseLabel);
    layout->addWidget(telephoneLabel);
    layout->addStretch();
    setStyleSheet("background: #22332d; border-radius: 16px; padding: 32px;");
    connect(retourBtn, &QPushButton::clicked, this, &ClientDetailWidget::retourClicked);
}

void ClientDetailWidget::setClientInfo(const QString& nom, const QString& prenom, const QString& adresse, const QString& telephone) {
    nomLabel->setText("Nom : " + nom);
    prenomLabel->setText("Prénom : " + prenom);
    adresseLabel->setText("Adresse : " + adresse);
    telephoneLabel->setText("Téléphone : " + telephone);
}
