#ifndef MODEL_H
#define MODEL_H

#include <QString>
#include <QDate>
#include <vector>

// Classe représentant un utilisateur du système
class User {
public:
    int id;
    QString nom;
    QString prenom;
    QString email;
    QString civilite;
    QString password;

    User(int id, const QString& nom, const QString& prenom, const QString& email, const QString& civilite, const QString& password)
        : id(id), nom(nom), prenom(prenom), email(email), civilite(civilite), password(password) {}
};

// Classe représentant un client (abonné)
class Client {
public:
    int idClient;
    QString nom;
    QString prenom;
    QString adresse;
    QString telephone;

    Client(int idClient, const QString& nom, const QString& prenom, const QString& adresse, const QString& telephone)
        : idClient(idClient), nom(nom), prenom(prenom), adresse(adresse), telephone(telephone) {}
};

// Classe représentant un compteur
class Compteur {
public:
    QString numCompteur;
    QString attributComp;
    double ancienIndex;
    QString serie;

    Compteur(const QString& numCompteur, const QString& attributComp, double ancienIndex, const QString& serie)
        : numCompteur(numCompteur), attributComp(attributComp), ancienIndex(ancienIndex), serie(serie) {}
};

// Classe représentant un abonnement
class Abonnement {
public:
    int idClient;
    QString numCompteur;
    QDate date_abonnement;

    Abonnement(int idClient, const QString& numCompteur, const QDate& date_abonnement)
        : idClient(idClient), numCompteur(numCompteur), date_abonnement(date_abonnement) {}
};

// Classe représentant une facturation
class Facturation {
public:
    QDate date_paiement;
    QString numCompteur;
    int idFacture;
    int idClient;
    double mensualite;

    Facturation(const QDate& date_paiement, const QString& numCompteur, int idFacture, int idClient, double mensualite)
        : date_paiement(date_paiement), numCompteur(numCompteur), idFacture(idFacture), idClient(idClient), mensualite(mensualite) {}
};

// Classe représentant un prélèvement
class Prelevement {
public:
    int idpreleve;
    QDate dateprelevement;
    QString numCompteur;
    double newIndex;
    double ancienIndex;

    Prelevement(int idpreleve, const QDate& dateprelevement, const QString& numCompteur, double newIndex, double ancienIndex)
        : idpreleve(idpreleve), dateprelevement(dateprelevement), numCompteur(numCompteur), newIndex(newIndex), ancienIndex(ancienIndex) {}
};

// Classe représentant une facture
class Facture {
public:
    int idFacture;
    QString numCompteur;
    int idClient;
    double soldeanterieur;
    double consommation;

    Facture(int idFacture, const QString& numCompteur, int idClient, double soldeanterieur, double consommation)
        : idFacture(idFacture), numCompteur(numCompteur), idClient(idClient), soldeanterieur(soldeanterieur), consommation(consommation) {}
};

#endif // MODEL_H
