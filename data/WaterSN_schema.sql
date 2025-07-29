-- Création d'un utilisateur MySQL dédié pour l'application WaterSN
-- À adapter selon votre politique de sécurité
-- Exemple : utilisateur = watersn_admin, mot de passe = WaterSN2025!2026
CREATE USER IF NOT EXISTS 'watersn_admin'@'localhost' IDENTIFIED BY 'WaterSN2025!2026';
GRANT ALL PRIVILEGES ON WaterSN.* TO 'watersn_admin'@'localhost';
FLUSH PRIVILEGES;

-- Schéma de la base de données WaterSN (MySQL)

CREATE DATABASE IF NOT EXISTS WaterSN;
USE WaterSN;

-- Utilisateurs (authentification)
CREATE TABLE User (
    id INT AUTO_INCREMENT PRIMARY KEY,
    nom VARCHAR(50),
    prenom VARCHAR(50),
    email VARCHAR(100) UNIQUE,
    civilite VARCHAR(20),
    password VARCHAR(255)
);

-- Rôles (admin, agent, utilisateur)
CREATE TABLE Role (
    id INT AUTO_INCREMENT PRIMARY KEY,
    nom VARCHAR(30) UNIQUE
);

-- Association User <-> Role (plusieurs rôles possibles)
CREATE TABLE UserRole (
    user_id INT,
    role_id INT,
    PRIMARY KEY (user_id, role_id),
    FOREIGN KEY (user_id) REFERENCES User(id),
    FOREIGN KEY (role_id) REFERENCES Role(id)
);

-- Clients (abonnés)
CREATE TABLE Client (
    idClient INT AUTO_INCREMENT PRIMARY KEY,
    nom VARCHAR(50),
    prenom VARCHAR(50),
    adresse VARCHAR(255),
    telephone VARCHAR(20)
);

-- Compteurs
CREATE TABLE Compteur (
    numCompteur VARCHAR(30) PRIMARY KEY,
    attributComp VARCHAR(50),
    ancienindex DOUBLE,
    serie VARCHAR(30)
);

-- Abonnements
CREATE TABLE Abonnement (
    id INT AUTO_INCREMENT PRIMARY KEY,
    idClient INT,
    numCompteur VARCHAR(30),
    date_abonnement DATE,
    FOREIGN KEY (idClient) REFERENCES Client(idClient),
    FOREIGN KEY (numCompteur) REFERENCES Compteur(numCompteur)
);

-- Factures
CREATE TABLE Facture (
    idFacture INT AUTO_INCREMENT PRIMARY KEY,
    numCompteur VARCHAR(30),
    idClient INT,
    soldeanterieur DOUBLE,
    consommation DOUBLE,
    FOREIGN KEY (numCompteur) REFERENCES Compteur(numCompteur),
    FOREIGN KEY (idClient) REFERENCES Client(idClient)
);

-- Facturation (paiement d'une facture)
CREATE TABLE Facturation (
    id INT AUTO_INCREMENT PRIMARY KEY,
    date_paiement DATE,
    numCompteur VARCHAR(30),
    idFacture INT,
    idClient INT,
    mensualite DOUBLE,
    FOREIGN KEY (numCompteur) REFERENCES Compteur(numCompteur),
    FOREIGN KEY (idFacture) REFERENCES Facture(idFacture),
    FOREIGN KEY (idClient) REFERENCES Client(idClient)
);

-- Prélèvements
CREATE TABLE Prelevement (
    idpreleve INT AUTO_INCREMENT PRIMARY KEY,
    dateprelevement DATE,
    numCompteur VARCHAR(30),
    newIndex DOUBLE,
    ancienIndex DOUBLE,
    FOREIGN KEY (numCompteur) REFERENCES Compteur(numCompteur)
);

-- Alertes (consommation anormale, impayés, etc.)
CREATE TABLE Alerte (
    id INT AUTO_INCREMENT PRIMARY KEY,
    type VARCHAR(50),
    message TEXT,
    date_alerte DATETIME DEFAULT CURRENT_TIMESTAMP,
    idClient INT,
    FOREIGN KEY (idClient) REFERENCES Client(idClient)
);

-- Logs (actions, notifications, exports, etc.)
CREATE TABLE Log (
    id INT AUTO_INCREMENT PRIMARY KEY,
    action VARCHAR(100),
    user_id INT,
    date_action DATETIME DEFAULT CURRENT_TIMESTAMP,
    details TEXT,
    FOREIGN KEY (user_id) REFERENCES User(id)
);
