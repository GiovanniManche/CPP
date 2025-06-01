#ifndef CSV_READER_H
#define CSV_READER_H

#include <map>
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

    // Récupération des ordres
    const std::vector<Order>& getOrders() const {
        return orders;
    }

    // Méthode pour tester la validité d'un ordre
    Order testOrder(std::vector<std::string>);

    // Méthode pour tester la récupération du timestamp
    long long testTimestamp(std::string rowValue);

    // Méthode pour tester l'id
    int testId(std::string rowValue);

    // Méthode pour tester le side
    std::string testSide(std::string rowValue);

    // Méthode pour tester le type d'ordre
    std::string testType(std::string rowValue);

    // Méthode pour tester la quantité
    int testQuantity(std::string rowValue);

    // Méthode pour tester le prix
    float testPrice(std::string rowValue, std::string orderType);

    // Méthode pour tester le type d'action
    std::string testAction(std::string rowValue);

    // Getter pour récupérer la liste des ordres / la map des ordres par actif
    std::vector<Order> getOrder(){return orders;}
    std::map<std::string, std::vector<Order>> getMapOrder(){return map_orders_asset;}

private:
    std::fstream file_;
    std::vector<Order> orders;
    std::map<std::string, std::vector<Order>> map_orders_asset;
    // std::map<int, std::vector<Order>> map_orders_asset;
};

#endif