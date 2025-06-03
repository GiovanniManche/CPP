#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include <vector>
#include <iostream>
#include "core/MatchingEngine.h"  // Pour accéder à la structure OrderResults

// Création d'une classe pour construire un fichier au format csv
class CsvWriter{
public :

    // Constructeur et destructeur par défaut
    CsvWriter();
    ~CsvWriter();

    // Constructeur qui prend le nom d'un fichier en entrée et 
    CsvWriter(std::string filename);

    // Ecriture dans le fichier à partir des résultats du matching engine
    void WriteToCsv(std::vector<OrderResult>);

    // Méthode permettant de transformer les attributs d'un OrderResult en chaine de caractère
    std::string OrderToString(OrderResult order);

private:
    std::string filename; 
    std::string formatPrice(float price);
};
#endif