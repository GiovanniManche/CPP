#include <iostream>
#include <sstream>
#include <vector>
#include "data/CSVReader.h"
// Constructeur avec nom du fichier dans filename
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
    // line contiendra chaque ligne du fichier, word contiendra chaque mot extrait de la ligne
    std::string line, word;
    // row sera le vecteur qui contiendra les mots de la ligne (passage CSV -> C++)
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
        order = testOrder(row);
        
        // Ajout de l'ordre au vecteur des ordres
        orders.push_back(order);

        // Ajout de l'ordre à la map : une clé par actif différent (==> un vecteur d'ordre par actif)
        // Vérification que si le ticker est déjà sélectionné 
        auto it = map_orders_asset.find(order.instrument);

        // Si l'actif est déjà dans le mapping, on ajoute l'ordre
        if(it != map_orders_asset.end()){
            it->second.push_back(order);
        // Sinon, on crée le couple clé/valeur
        }else{
            std::cout << "Instrument non trouvé" << std::endl;
            // Etape 1 : création d'un vecteur pour stocker les ordres de l'actif
            std::vector<Order> orders_asset;

            // Etape 2 : création du couple clé/valeur
            map_orders_asset[order.instrument] = orders_asset;

            // Etape 3 : Récupération de l'ordre
            map_orders_asset[order.instrument].push_back(order);
        }
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

// Méthode permettant de tester le contenu d'un ordre
Order CsvReader::testOrder(std::vector<std::string> row){
   
    Order order;
    bool hasError = false;
    bool hasErrorTS = false;
 
    try{
        // Récupération des paramètres
        order.timestamp = testTimestamp(row[0]);
 
    }catch(std::runtime_error& error_ts){
        hasErrorTS = true;
        hasError = true;
        order.timestamp = 0;
    }

    try{
        order.order_id = testId(row[1]);        
        order.instrument = row[2];
        order.side = testSide(row[3]);
        order.type = testType(row[4]);
        order.quantity = testQuantity(row[5]);
        order.price = testPrice(row[6], row[4]);            
        order.action = testAction(row[7]);
    }catch(std::runtime_error& error){
        hasError = true;
    }
   
    // En cas d'erreur, on modifie le type de l'ordre en BAD_INPUT pour le rejeter automatiquement par la suite
    if (hasError) {
        if(hasErrorTS == false){
            order.timestamp = testTimestamp(row[0]);
        }
        order.order_id = 0;
        order.instrument = row[2];
        order.side = row[3];
        order.type = "BAD_INPUT";  
        order.quantity = 0;  
        order.price = 0;    
        order.action = row[7];
    }
   
    return(order);
}
// Méthode permettant de tester la récupération d'un timestamp
long long CsvReader::testTimestamp(std::string rowValue){

    long long timestamp;
    // Vérification que la conversion en long long est possible
    try{
        timestamp = std::stoll(rowValue);
    }catch (long long error){
        std::cout << error << std::endl;
        throw std::runtime_error("Problème dans la conversion du timestamp");
    }catch(std::invalid_argument error){
        throw std::runtime_error("Problème dans la conversion du timestamp");
    }

    // Si le test est passé, vérification que le timestamp ne soit pas négatif
    if(timestamp < 0){
        throw std::runtime_error("Un timestamp ne peut pas être négatif");
    }

    // Récupération du timestamp
    return(timestamp);
}

// Méthode permettant de tester la récupération de l'ID
int CsvReader::testId(std::string rowValue){

    // Test pour la conversion en int
    int id; 
    try{
        id = std::stoi(rowValue);
    }catch(int error){
        std::cout << error << std::endl;
        throw std::runtime_error("Problème sur la valeur de l'ID");
    }catch(std::invalid_argument error){
        throw std::runtime_error("Problème dans la conversion de l'ID");
    }

    // Vérification que  l'ID est positif
    if(id <= 0){
        std::cout << id << std::endl;
        std::runtime_error("L'id doit être positif");
    }

    // Si les tests sont passés, récupération de l'ID
    return(id);
}

// Méthode permettant de tester la récupération du side
std::string CsvReader::testSide(std::string rowValue){
    std::string side = rowValue;
    if(side != "BUY" && side != "SELL"){
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
        throw std::runtime_error("Problème dans la conversion de la quantité");
    }catch(std::invalid_argument error){
        throw std::runtime_error("Problème dans la conversion de la quantité");
    }

    // Si la conversion a bien eu lieu, on vérifie qu'elle est positive
    if(quantity <= 0){
        std::cout << quantity << std::endl;
        throw std::runtime_error("La quantité ne peut pas être négative ou nulle");
    }

    // Si les tests sont passés, on récupère la quantité
    return(quantity);
}

// Méthode permettant de tester le prix
float CsvReader::testPrice(std::string rowValue, std::string orderType){

    std::string LIMIT_LABEL = "LIMIT";
    std::string MARKET_LABEL = "MARKET";
    float price;
    // Deux cas à tester : ordre à cours limité et ordre au marché (tous les autres ordres auraient déjà provoqué une erreur)
    if(orderType == LIMIT_LABEL){

        // Vérification que la conversion est possible
        try{
            price = std::stof(rowValue);
        }catch(float error){
            std::cout << error << std::endl;
            throw std::runtime_error("Problème dans la conversion du prix pour un ordre limite");
        }catch(std::invalid_argument error){
            throw std::runtime_error("Problème dans la conversion du prix d'un ordre");
        }

        // Deuxième check : prix  > 0
        if(price < 0){
            std::cout << price << std::endl;
            throw std::runtime_error("Le prix ne peut pas être négatif");
        }
    }else{

        // Pour un ordre au marché, on fixe arbitrairement un prix de 0 pour les traitements ultérieurs
        price = 0;
    }
    
    // Récupération du prix
    return(price);
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
