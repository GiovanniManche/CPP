#include <iostream>
#include <sstream>
#include <vector>
#include "data/CSVReader.h"

CsvReader::CsvReader(std::string filename):file_(filename){
}

CsvReader::CsvReader(){
    std::cout << "Initialisation du CSV Reader" << std::endl;
}

CsvReader::~CsvReader() {
    std::cout << "Destruction du CSV reader" << std::endl;
}

void CsvReader::init(){
    std::string line, word;
    std::vector<std::string> row;
    std::stringstream s;

    while (std::getline(file_, line)) {
        row.clear();

        std::cout << line << '\n';
    
        // récupération de la ligne
        // Stream pour parcourir une string avec getline
        std::stringstream s(line);
        
        // Boucle sur la ligne pour récupérer chaque mot
        while(getline(s, word, ',')){
            std::cout << word << std::endl;
            row.push_back(word);
        }
        
        // Initialisation et récupération de chaque ordre
        // Mettre des tests pour toutes ces composantes
        Order order;
        std::cout << row[0] << std::endl;
        order.timestamp = std::stoll(row[0]); // Conversion du timestamp et récupération
        order.order_id = std::stoi(row[1]); // Conversion de l'ID de l'ordre et récupération
        order.instrument = row[2]; // Récupération du nom de l'instrument financier
        order.side = row[3]; // Récupération du sens de l'ordre (Achat / Vente)
        order.type = row[4]; // Récupération du type d'ordre
        order.quantity = std::stoi(row[5]); // Récupération de la quantité
        order.price = std::stof(row[6]); // Récupération du prix
        order.action = row[7]; // Récupération de l'action à réaliser

        // Récupération dans le vecteur orders
        orders.push_back(order);
    }
}

// Méthode permettant d'afficher le contenu d'un vecteur d'ordre
void CsvReader::Display(){

    // Boucle sur les éléments du vecteur
    for(u_long i = 0; i < orders.size(); i++){
        // Récupération de l'ordre courant
        Order order = orders[i];

        // Affichage de ses caractéristiques
        std::cout << "Timestamp - ID - Instrument - Side - Type - Quantité - Prix - Action" << std::endl;
        std::cout << order.timestamp << " " << order.order_id << " " << order.instrument << " " <<
        order.side << " " << order.quantity << " " << order.price << " " << order.action << std::endl;
    };
}