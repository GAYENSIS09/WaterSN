#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "model/model.h"
#include <vector>
#include <utility>
#include <QString>
#include <QDate>

class Controller {
public:
    // Stockage en mémoire (pour simplifier, on utilise std::vector)
    std::vector<User> users;
    std::vector<Client> clients;
    std::vector<Compteur> compteurs;
    std::vector<Abonnement> abonnements;
    std::vector<Facturation> facturations;
    std::vector<Prelevement> prelevements;
    std::vector<Facture> factures;

    // Méthodes de gestion de base
    void ajouterUser(const User& user);
    void ajouterClient(const Client& client);
    void ajouterCompteur(const Compteur& compteur);
    void ajouterAbonnement(const Abonnement& abonnement);
    void ajouterFacturation(const Facturation& facturation);
    void ajouterPrelevement(const Prelevement& prelevement);
    void ajouterFacture(const Facture& facture);

    // Recherche
    Client* trouverClientParId(int idClient);
    Compteur* trouverCompteurParNum(const QString& numCompteur);

    // Gestion des abonnés
    std::vector<Client> listerClients() const;
    bool modifierClient(int idClient, const Client& nouveauClient);
    std::vector<Client> trierClientsParCollectivite() const;
    std::vector<std::pair<Client, std::vector<Compteur>>> listerFichesAbonneCompteur() const;

    // Gestion des compteurs
    std::vector<Compteur> listerCompteursDisponibles() const;
    bool archiverCompteur(const QString& numCompteur);
    bool transfererCompteur(const QString& numCompteur, const QString& nouvelleSerie);
    bool assignerCompteurAClient(const QString& numCompteur, int idClient, const QDate& date_abonnement);

    // Gestion de la facturation
    Facture genererFacture(int idClient, const QString& numCompteur, double consommation, double soldeanterieur);
    bool effectuerPaiement(int idFacture, const QDate& date_paiement);
};

#endif // CONTROLLER_H
