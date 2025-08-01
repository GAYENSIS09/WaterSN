#pragma once
#include <QWidget>
#include <QString>

class QLabel;
class QPushButton;

class ClientDetailWidget : public QWidget {
    Q_OBJECT
public:
    explicit ClientDetailWidget(QWidget* parent = nullptr);
    void setClientInfo(const QString& nom, const QString& prenom, const QString& adresse, const QString& telephone);
signals:
    void retourClicked();
private:
    QLabel* nomLabel;
    QLabel* prenomLabel;
    QLabel* adresseLabel;
    QLabel* telephoneLabel;
    QPushButton* retourBtn;
};
