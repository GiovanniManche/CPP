#include <iostream>
#include <sstream>
#include <vector>
#include "data/CSVReader.h"
// Constructeur avec nom du fichier dans filename (correspond ensuite à l'attribut file_)
CsvReader::CsvReader(std::string filename):file_(filename){
}

// Constructeur sans nom de fichier
CsvReader::CsvReader(){
    std::cout << "Initialisation du CSV Reader" << std::endl;
}

// Destructeur
CsvReader::~CsvReader() {
    std::cout << "Destruction du CSV reader" << std::endl;
}

void CsvReader::init(){
    // line contiendra chaque ligne du fichier, word chaque mot extrait de la ligne
    std::string line, word;
    // row = vecteur qui contiendra les mots de la ligne (passage CSV -> C++)
    std::vector<std::string> row;
    
    // On ignore la ligne de titre (le curseur au départ est nécessairement sur la première ligne)
    if (std::getline(file_, line)) {
        std::cout << "Header ignoré: " << line << std::endl;
    }
    
    // Boucle sur les lignes, tant qu'il y a une nouvelle ligne
    while (std::getline(file_, line)) {
        row.clear();
        
        // Transformation de la ligne en un "flux" qu'on pourra séparer par les virgules
        std::stringstream s(line);
        
        // Boucle sur les éléments de la ligne
        while(getline(s, word, ',')){
            // Ajout du mot au vecteur row
            row.push_back(word);
        }
        
        // Vérification
        if (row.size() < 8) {
            std::cout << "Ligne incomplète ignorée" << std::endl;
            continue;
        }
        
        // Création de l'ordre (instance de "Order" dans notre code)
        Order order;
        order.timestamp = std::stoll(row[0]);       // On convertit le string en long long 
        order.order_id = std::stoi(row[1]);         // Idem mais en int
        order.instrument = row[2];
        order.side = row[3];
        order.type = row[4];
        order.quantity = std::stoi(row[5]);
        order.price = std::stof(row[6]);            // Idem mais en float
        order.action = row[7];
        
        // Ajout de l'ordre au vecteur des ordres
        orders.push_back(order);
    }
    std::cout << "Chargement de " << orders.size() << " ordres avec succès!" << std::endl;
}
// Méthode permettant d'afficher le contenu d'un vecteur d'ordre
void CsvReader::Display(){

    std::cout << "Timestamp - ID - Instrument - Side - Type - Quantité - Prix - Action" << std::endl;

    // Boucle sur les éléments du vecteur contenant tous les ordres chargés
    for(u_long i = 0; i < orders.size(); i++){
        // On récupère l'ordre et on affiche ses caractéristiques
        Order order = orders[i];
        std::cout << order.timestamp << " " << order.order_id << " " << order.instrument << " " << order.side << " " << order.type << " " << order.quantity << " " << order.price << " " << order.action << std::endl;
    };
}