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

// Méthode permettant de tester la récupération d'un timestamp
long long CsvReader::testTimestamp(std::string rowValue){

    long long timestamp;
    // Vérification que la conversion en long long est possible
    try{
        timestamp = std::stoll(rowValue);
    }catch (long long error){
        std::cout << "Problème dans la conversion du timestamp" << std::endl;
        std::cout << error << std::endl;
    }

    // Si le test est passé, vérification que le timestamp ne soit pas négatif
    if(timestamp < 0){
        throw std::runtime_error("Un timestamp ne peut pas être négatif");
    }

    // Récupération du timestamp
    return(timestamp);
}

// Méthode permettant de tester la récupération du side
std::string CsvReader::testSide(std::string rowValue){
    std::string side = rowValue;
    if(side != "BUY" || side != "SELL"){
        std::cout << side << std::endl;
        throw std::runtime_error("L'ordre doit être un ordre d'achat (BUY) ou de vente (SELL)");
    }

    // Récupération
    return(side);
}

// Méthode permettant de tester le type d'ordre
std::string CsvReader::testType(std::string rowValue){
    std::string type;

    // Vérification du type d'ordre : seul limite et marché sont implémentés
    if(rowValue == "LIMIT"){
        type = rowValue;
    }else if(rowValue == "MARKET"){
        type = rowValue;
    }else{
        throw std::runtime_error("Seuls les ordres à cours limité / au marché sont implémentés");
    }

    // Récupération du type d'ordre
    return(type);
}

// Méthode permettant de tester la quantité
int CsvReader::testQuantity(std::string rowValue){

    int quantity;

    // Vérification que l'on peut convertir la quantité
    try{
        quantity = std::stoi(rowValue);
    }catch(int error){
        std::cout << rowValue << std::endl;
        std::runtime_error("Problème dans la conversion de la quantité");
    }

    // Si la conversion a bien eu lieu, vérification qu'elle est positive
    if(quantity <= 0){
        std::cout << quantity << std::endl;
        std::runtime_error("La quantité ne peut pas être négative ou nulle");
    }

    // Si les tests sont passés, on récupère la quantité
    return(quantity);
}

// Méthode permettant de tester le type d'action
std::string CsvReader::testAction(std::string rowValue){

    std::string action;
    if(rowValue == "NEW" || rowValue == "MODIFY" || rowValue == "CANCEL"){
        action = rowValue;
    }else{
        std::cout << rowValue << std::endl;
        throw std::runtime_error("Les seules actions implémentées sont : NEW, MODIFY et CANCEL");
    }
    return(action);
}