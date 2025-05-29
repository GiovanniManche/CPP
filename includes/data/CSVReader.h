#ifndef CSV_READER_H
#define CSV_READER_H

#include <vector>
#include <fstream>


// Création d'une structure de donnée associée à un ordre 
struct Order{
    long long timestamp;
    int order_id;
    std::string instrument; 
    std::string side;
    std::string type;
    int quantity;
    float price;
    std::string action;
};

// Création d'une classe pour lire un fichier au format CSV
class CsvReader {
public:
    // Construction et destructeur par défaut
    CsvReader();
    ~CsvReader();

    // Constructeur qui prend le nom d'un fichier en entrée
    CsvReader(std::string filename);

    // Récupération des ordres du csv sous forme de vecteur
    void init();

    // Affichage des ordres (debug)
    void Display();

private:
    std::fstream file_;
    std::vector<Order> orders;

};

#endif