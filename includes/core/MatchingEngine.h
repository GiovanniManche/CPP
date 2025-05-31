#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include <vector>
#include <queue>
#include <map>
#include <iostream>
#include "data/CSVReader.h"  // Pour accéder à la structure Order

// Structure pour représenter une transaction exécutée (on a besoin du timestamp correspondant au moment du trade,
// des ID des ordres d'achat et de vente qui se rencontrent, du nom de l'action (AAPL,...), de la quantité échangée et du prix)
struct Trade {
    long long timestamp;
    int buy_order_id;
    int sell_order_id;
    std::string instrument;
    int quantity;
    float price;
    
    // Constructeur
    Trade(long long ts, int buy_id, int sell_id, const std::string& inst, int qty, float p)
        : timestamp(ts), buy_order_id(buy_id), sell_order_id(sell_id), 
          instrument(inst), quantity(qty), price(p) {}
};

// Structure pour les ordres avec état (pour l'output final, on veut présenter en plus des caractéristiques de l'ordre
// la quantité exécutée, l'ID de la contrepartie si besoin, le prix d'exécution et naturellement le statut.)
struct OrderResult {
    Order original_order;
    std::string status;           // EXECUTED, PARTIALLY_EXECUTED, PENDING, CANCELED, REJECTED
    int executed_quantity;
    float execution_price;
    int counterparty_id;
    
    // Constructeur
    OrderResult(const Order& order) 
        : original_order(order), status("PENDING"), executed_quantity(0), 
          execution_price(0.0f), counterparty_id(0) {}
};

// Comparateur pour les ordres d'achat (prix décroissant, puis FIFO)
// Logique : Meilleurs acheteurs (prix plus élevés) en tête de queue
// En cas d'égalité de prix : ordre chronologique (FIFO)
// Nécessaire pour les objets priority_queue
struct BuyComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) {
            return a.price < b.price;  // Prix décroissant (plus haut prix en tête)
        }
        return a.timestamp > b.timestamp;  // Si même prix, plus ancien en tête (timestamp plus petit = plus ancien)
    }
};

// Comparateur pour les ordres de vente (prix croissant, puis FIFO)
// Logique : Meilleurs vendeurs (prix plus faibles) en tête de queue
// En cas d'égalité de prix : ordre chronologique (FIFO)
// Nécessaire pour les objets priority_queue
struct SellComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) {
            return a.price > b.price;  // Prix croissant (plus bas prix en tête)
        }
        return a.timestamp > b.timestamp;  // Si même prix, plus ancien en tête (timestamp plus petit = plus ancien)
    }
};

class MatchingEngine {
private:
    // Carnets d'ordres (priority queues)
    // Les objets priority_queue permettent d'ordonner automatiquement les données contenues
    // selon une règle spécifique (la comparaison ici, pour avoir le prix le plus haut dans le book d'achat en premier par exemple)
    std::priority_queue<Order, std::vector<Order>, BuyComparator> buy_book;
    std::priority_queue<Order, std::vector<Order>, SellComparator> sell_book;
    
    // Ordres impactés temporaires (pour l'ordre d'affichage)
    std::vector<OrderResult> pending_impacted_orders;
    
    // Map pour retrouver rapidement les ordres par ID (pour MODIFY/CANCEL, car on ne peut pas retirer une ligne directement d'un priority_queue)
    std::map<int, Order> order_map;
    
    // Timestamp actuel pour les modifications
    long long current_timestamp;

public:

    // Historique des trades (output final)
    std::vector<OrderResult> historic_trades;

    // Constructeur
    MatchingEngine();
    
    // Destructeur
    ~MatchingEngine();
    
    // VOIR MatchingEngine.cpp POUR PLUS D'EXPLICATIONS SUR LES METHODES !! 

    // Méthode principale pour boucler sur tous les ordres
    std::vector<OrderResult> processAllOrders(const std::vector<Order>& orders);
    
    // Méthode pour traiter un ordre individuel
    void processOrder(const Order& order);
    
    // Gestion des actions
    void handleNew(const Order& order);
    void handleModify(const Order& order);
    void handleCancel(const Order& order);
    
    // Algorithme de matching
    std::vector<Trade> tryMatch(Order& incoming_order);
    
    // Ajout d'un ordre au carnet approprié
    void addToBook(const Order& order);
    
    // Recherche et suppression d'un ordre du carnet
    bool removeFromBook(int order_id, const std::string& side);
    
    // Affichage des carnets (debug)
    void displayBooks() const;
    
    // Récupération des résultats
    const std::vector<OrderResult>& getResults() const;
    
    // Affichage des résultats
    void displayResults() const;
    
private:
    // Méthodes utilitaires
    long long getCurrentTimestamp();
    OrderResult createResult(const Order& order, const std::string& status, 
                           int exec_qty = 0, float exec_price = 0.0f, int counterparty = 0);
};

#endif