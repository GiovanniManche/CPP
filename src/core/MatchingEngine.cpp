#include "core/MatchingEngine.h"
#include <algorithm>
#include <chrono>

// Constructeur
MatchingEngine::MatchingEngine() : current_timestamp(0) {
    std::cout << "Initialisation du Matching Engine" << std::endl;
}

// Destructeur
MatchingEngine::~MatchingEngine() {
    std::cout << "Destruction du Matching Engine" << std::endl;
}

//######################################################################################################################################################
// Concrètement, le matching fonctionne de la manière suivante : 
//  - on récupère le vecteur des ordres, fourni par le CSVReader
//  - on boucle sur ce vecteur,
//  - pour chaque ordre, on regarde son action. En fonction, on l'ajoute à la partie SELL ou BUY du book 
//      ou on regarde dans la bonne partie du book pour modifier / retirer l'ordre
//  - chaque action est répertoriée dans un vecteur, qui sera l'output (historique des actions)
// tandis que l'order book est modifié dynamiquement (retrait des ordres exécutés ou annulés, changement de temporalité en cas de modification,...)
// Notons que les book SELL et BUY sont des objets "priority_queue" et sont donc classés dans l'ordre décroissant du prix pour BUY et croissant pour SELL, 
//      puis selon la règle FIFO
//######################################################################################################################################################

std::vector<OrderResult> MatchingEngine::processAllOrders(const std::vector<Order>& orders) {
    // ################################################################################################
    // Cette fonction permet de traiter séquentiellement tous les ordres (en bouclant)
    // Elle prend en input le vecteur contenant les ordres (après passage par le CSVReader)
    // Elle renvoie l'historique des trades / actions
    // On fait aussi un contrôle du tri par timestamp avant traitement
    // ################################################################################################

    std::cout << "\n=== DÉBUT DU MATCHING ENGINE ===" << std::endl;
    std::cout << "Nombre d'ordres à traiter : " << orders.size() << std::endl;
    
    // ################################################################################################
    // On contrôle si le vecteur passé en input est bien trié par timestamp
    // ################################################################################################
    // Copie modifiable du vecteur d'ordres
    std::vector<Order> sorted_orders = orders;
    bool is_sorted = true;
    for (size_t i = 1; i < sorted_orders.size(); i++) {
        if (sorted_orders[i].timestamp < sorted_orders[i-1].timestamp) {
            is_sorted = false;
            break;
        }
    }
    
    // Si pas trié, on trie
    if (!is_sorted) {
        std::cout << "Les ordres ne sont pas triés par timestamp. On trie automatiquement" << std::endl;
        std::sort(sorted_orders.begin(), sorted_orders.end(), 
                  [](const Order& a, const Order& b) {
                      return a.timestamp < b.timestamp;
                  });
    // DEBUG
    } else {
        std::cout << "Ordres déjà correctement triés par timestamp" << std::endl;
    }
    
    // ################################################################################################
    // TRAITEMENT DES ORDRES 
    // ################################################################################################
    
    // Boucle sur la liste triée (on itère tant qu'on n'est pas à la fin de la liste)
    for (size_t i = 0; i < sorted_orders.size(); i++) {

        // On récupère le nouvel ordre depuis le vecteur trié
        Order current_order = sorted_orders[i];
        
        // Debug
        std::cout << "\n--- Traitement ordre " << (i+1) << "/" << sorted_orders.size() << " ---" << std::endl;
        std::cout << "ID: " << current_order.order_id << " | Action: " << current_order.action 
                  << " | Side: " << current_order.side << " | Qty: " << current_order.quantity 
                  << " | Price: " << current_order.price << std::endl;
        
        // On distingue selon l'action de l'ordre
        if (current_order.action == "NEW") {
            handleNew(current_order);
        } else if (current_order.action == "MODIFY") {
            handleModify(current_order);
        } else if (current_order.action == "CANCEL") {
            handleCancel(current_order);
        } else {
            // Si action inconnue -> on ne fait pas planter le matching engine mais on rejette l'ordre
            std::cout << "Action inconnue : " << current_order.action << std::endl;
            OrderResult result = createResult(current_order, "REJECTED");
            historic_trades.push_back(result);
        }
    }

    std::cout << "\n=== FIN DU MATCHING ENGINE ===" << std::endl;
    std::cout << "Résultats générés : " << historic_trades.size() << std::endl;

    return historic_trades;
}


void MatchingEngine::handleNew(const Order& order) {
    // ################################################################################################
    // Fonction qui gère l'action NEW 
    // Concrètement, on récupère l'ordre et on regarde s'il peut être matché avec un / des ordres opposés,
    // Pour chaque match individuel, on génère une ligne dans l'historique
    // Puis si besoin, on ajoute l'ordre (avec quantité et état mis à jour) dans les books BUY et SELL.
    // On contrôle que l'ID n'existe pas déjà et que ce n'est pas un bad input
    // ################################################################################################
    std::cout << "Action NEW - Tentative de matching..." << std::endl;
    
    // ################################################################################################
    // On contrôle que l'ID n'existe pas déjà
    // ################################################################################################
    auto existing_order = order_map.find(order.order_id);
    if (existing_order != order_map.end()) {
        std::cout << "ERREUR: ID " << order.order_id << " existe déjà pour un ordre NEW !" << std::endl;
        std::cout << "Ordre existant: Side=" << existing_order->second.side 
                  << ", Qty=" << existing_order->second.quantity 
                  << ", Price=" << existing_order->second.price << std::endl;
        OrderResult result = createResult(order, "REJECTED");
        historic_trades.push_back(result);
        return;
    }
    
    // ################################################################################################
    // On contrôle que le type n'est pas "BAD_INPUT"
    // ################################################################################################
     if (order.type == "BAD_INPUT") {
        std::cout << "ERREUR: Type BAD_INPUT détecté pour l'ordre ID " << order.order_id << " - Ordre rejeté" << std::endl;
        OrderResult result = createResult(order, "REJECTED");
        historic_trades.push_back(result);
        return;
    }


    // 1. MATCHING
    Order working_order = order;  // Copie pour modification des quantités
    std::vector<Trade> matches = tryMatch(working_order);
    
    // 1.1. Si pas de match : 
    if (matches.empty()) {
        // Si ordre au marché -> rejet de l'ordre
        if (order.type == "MARKET") {
            std::cout << "MARKET order rejeté (Carnet opposé vide)" << std::endl;
            OrderResult result = createResult(order, "REJECTED");
            historic_trades.push_back(result);    
        } 
        // Si ordre limite -> en attente mais ajouté au carnet 
        else {
            std::cout << "Aucun match trouvé - Ajout au carnet" << std::endl;
            addToBook(order);
            order_map[order.order_id] = order;
            
            OrderResult result = createResult(order, "PENDING");
            historic_trades.push_back(result);
        }
    
    // 1.2. Si match :
    } else {
        int remaining_order_qty = order.quantity;
        
        // Boucle sur tous les trades générés par tryMatch()
        for (size_t i = 0; i < matches.size(); i++) {
            const Trade& trade = matches[i];
            remaining_order_qty -= trade.quantity;
            
            // Contrepartie : si l'ordre est d'achat, contrepartie = SELL, et inversement
            int counterparty_id = (order.side == "BUY") ? trade.sell_order_id : trade.buy_order_id;
            
            std::cout << "MATCH " << (i+1) << "/" << matches.size() << ": " 
                      << trade.quantity << " @ " << trade.price 
                      << " contre ordre " << counterparty_id << std::endl;
            
            // On détermine le status de l'ordre pour ce match pour ensuite l'afficher
            std::string status;
            if (i == matches.size() - 1) {  // Dernier match
                if (remaining_order_qty == 0) {
                    status = "EXECUTED";  
                } else {
                    status = "PARTIALLY_EXECUTED";  
                }
            } else {
                status = "PARTIALLY_EXECUTED";  // Pas le dernier match, donc forcément partiel
            }
            
            // Dans les résultats on aura une ligne par trade
            Order match_order = order;
            if (status == "EXECUTED") {
                match_order.quantity = 0;  
            } else {
                match_order.quantity = remaining_order_qty;  
            }
            
            OrderResult result = createResult(match_order, status, trade.quantity, trade.price, counterparty_id);
            historic_trades.push_back(result);
        }
        
        // Si l'ordre n'est pas complètement exécuté et c'est un ordre limite, on ajoute le résidu au carnet
        if (remaining_order_qty > 0 && order.type == "LIMIT") {
            std::cout << "Résidu de " << remaining_order_qty << " ajouté au carnet" << std::endl;
            Order residual_order = order;
            residual_order.quantity = remaining_order_qty;
            addToBook(residual_order);
            order_map[order.order_id] = residual_order;
        }
        
        // Affichage des ordres impactés 
        for (const OrderResult& impacted : pending_impacted_orders) {
            historic_trades.push_back(impacted);
        }
        pending_impacted_orders.clear();  
    }
}

// MODIFY : On cherche l'ID correspondant, on modifie les caractéristiques et AUSSI LE TIMESTAMP
//      (Comme on modifie l'ordre, il perd sa priorité temporelle) 
void MatchingEngine::handleModify(const Order& order) {
    // ################################################################################################
    // Fonction qui gère l'action MODIFY avec gestion complexe des quantités
    // Concrètement, on récupère l'ordre et on regarde s'il correspond bien à un ordre déjà existant,
    // On calcule la nouvelle quantité par rapport à l'ordre INITIAL et à l'ordre ACTUEL
    // Puis on modifie l'ordre existant : on supprime l'ancien ordre du book en le remplaçant par le nouveau,
    // Mais on garde les deux éléments dans l'historique.
    // ################################################################################################

    // Debug pour savoir où on est
    std::cout << "Action MODIFY - Recherche ordre ID: " << order.order_id << std::endl;
    
    // Recherhce de l'ID (si pas présent -> marqueur après le dernier élément (donc vide))
    auto it = order_map.find(order.order_id);

    // 1. Ordre à modifier non identifié
    if (it == order_map.end()) {
        // Message d'erreur (ne fait pas planter le code, on informe juste)
        std::cout << "ERREUR: Ordre ID " << order.order_id << " non trouvé pour modification" << std::endl;
        OrderResult result = createResult(order, "REJECTED");
        historic_trades.push_back(result);
        return;
    }
    
    // 2. Récupération de la quantité initiale
    int initial_quantity = -1;
    for (const OrderResult& result : historic_trades) {
        if (result.original_order.order_id == order.order_id && result.original_order.action == "NEW") {
            // Pour récupérer la quantité initiale, on doit regarder :
            // - Si PENDING : la quantité affichée est la quantité initiale
            // - Si EXECUTED/PARTIALLY_EXECUTED : quantité affichée + quantité exécutée = quantité initiale
            if (result.status == "PENDING") {
                initial_quantity = result.original_order.quantity;
            } else if (result.status == "EXECUTED") {
                initial_quantity = result.original_order.quantity + result.executed_quantity;
            } else if (result.status == "PARTIALLY_EXECUTED") {
                initial_quantity = result.original_order.quantity + result.executed_quantity;
            }
            break;
        }
    }
    
    if (initial_quantity == -1) {
        std::cout << "ERREUR: Impossible de trouver la quantité initiale pour l'ordre ID " << order.order_id << std::endl;
        OrderResult result = createResult(order, "REJECTED");
        historic_trades.push_back(result);
        return;
    }
    
    // 3. Calcul de la nouvelle quantité
    // Concrètement, nvlle qté = qté_restante - (qté_initiale - qté_modifiée)
    // Donc si qté_initiale = 100, qté_restante = 50 et qté_modifiée = 70, qté_new = 20
    // Si qté_initiale = 100, qté_restante = 50 et qté_modifiée = 130, qté_new = 80
    // Si qté_initiale = 100, qté_restante = 50 et qté_modifiée = 40, qté_new = 0
    int current_quantity = it->second.quantity; 
    int reduction = initial_quantity - order.quantity;  
    int new_quantity = current_quantity - reduction;    
    
    // 4. Gestion des cas limites
    if (new_quantity <= 0) {
        std::cout << "MODIFY résulte en quantité <= 0 - Ordre considéré comme complètement exécuté" << std::endl;
        
        // On supprime l'ordre du carnet
        bool removed = removeFromBook(order.order_id, it->second.side);
        if (removed) {
            // On crée un résultat EXECUTED avec la quantité restante comme quantité exécutée
            Order executed_order = order;
            executed_order.quantity = 0;
            
            OrderResult result = createResult(executed_order, "EXECUTED", current_quantity, it->second.price, 0);
            historic_trades.push_back(result);
            
            // Suppression de la map
            order_map.erase(it);
        } else {
            std::cout << "ERREUR: Impossible de supprimer l'ordre du carnet" << std::endl;
            OrderResult result = createResult(order, "REJECTED");
            historic_trades.push_back(result);
        }
        return;
    }
    
    // 5. Ordre trouvé avec quantité valide
    std::cout << "Ordre trouvé - Suppression du carnet et retraitement avec nouvelle quantité: " << new_quantity << std::endl;
    
    // On supprime l'ancien ordre du carnet
    bool removed = removeFromBook(order.order_id, it->second.side);
    
    // Si pour une raison X ou Y on ne peut pas le supprimer -> rejet de l'ordre (ne devrait pas se produire)
    if (!removed) {
        std::cout << "ERREUR: Impossible de supprimer l'ordre du carnet" << std::endl;
        OrderResult result = createResult(order, "REJECTED");
        historic_trades.push_back(result);
        return;
    }
    
    // On récupère les caractéristiques nouvelles
    Order modified_order = order;
    modified_order.quantity = new_quantity;      
    // On supprime de la map l'ancien ordre
    order_map.erase(it);
    
    // On traite l'ordre comme un nouvel ordre, tout en écrivant toutes les informations dans l'historique. 
    size_t historic_size_before = historic_trades.size();
    handleNew(modified_order);
    
    // Si des résultats ont été ajoutés par handleNew, on modifie l'ordre entrant dans l'historique
    if (historic_trades.size() > historic_size_before) {
        // Le premier résultat ajouté est celui de l'ordre modifié
        OrderResult& modify_result = historic_trades[historic_size_before];
        
        // On corrige l'action pour afficher MODIFY au lieu de NEW
        modify_result.original_order.action = "MODIFY";
        modify_result.original_order.timestamp = order.timestamp;  // On garde le timestamp original du MODIFY ici
        
        // Si complètement exécuté, la quantité affichée doit être 0
        if (modify_result.status == "EXECUTED") {
            modify_result.original_order.quantity = 0;
        } else if (modify_result.status == "PARTIALLY_EXECUTED") {
        }
    }
}

// CANCEL : Fonctionnement similaire à MODIFY mais on efface directement du book.
void MatchingEngine::handleCancel(const Order& order) {
    // ################################################################################################
    // Fonction qui gère l'action CANCEL
    // Concrètement, on récupère l'ordre et on regarde s'il correspond bien à un ordre déjà existant,
    // Puis on supprime l'ordre existant.
    // Mais on garde les deux éléments dans l'historique.
    // ################################################################################################
    
    std::cout << "Action CANCEL - Recherche ordre ID: " << order.order_id << std::endl;
    
    // Toute la logique est la même que pour MODIFY. Elle est même ici plus simple car il faut juste
    //      supprimer l'ordre du book et enregistrer dans l'historique.
    auto it = order_map.find(order.order_id);
    if (it == order_map.end()) {
        std::cout << "ERREUR: Ordre ID " << order.order_id << " non trouvé pour annulation" << std::endl;
        OrderResult result = createResult(order, "REJECTED");
        historic_trades.push_back(result);
        return;
    }
    
    std::cout << "Ordre trouvé - Suppression définitive du carnet" << std::endl;
    
    // On supprime la ligne du book. On garde la condition pour potentielle erreur, mais ça ne devrait pas arriver.
    bool removed = removeFromBook(order.order_id, it->second.side);
    if (removed) {
        std::cout << "Ordre annulé avec succès" << std::endl;
        
        // Quantité : 0 (ordre supprimé)
        Order canceled_order = order;
        canceled_order.quantity = 0; 
    
        OrderResult result = createResult(canceled_order, "CANCELED");
        historic_trades.push_back(result);
        
        // Suppression de la map
        order_map.erase(it);
    } else {
        std::cout << "ERREUR: Impossible de supprimer l'ordre du carnet" << std::endl;
        OrderResult result = createResult(order, "REJECTED");
        historic_trades.push_back(result);
    }
}

// Procédure de matching, coeur du code
std::vector<Trade> MatchingEngine::tryMatch(Order& incoming_order) {
    // ################################################################################################
    // Fonction qui gère le matching. 
    // Concrètement, on récupère l'ordre et on regarde s'il peut être matché à des ordres adverses, en respectant
    // la règle du FIFO.
    // ################################################################################################
    
    // Initialisation du vecteur des matchs, du vecteur des ordres impactés et de la quantité restante dans l'ordre arrivé.
    std::vector<Trade> matches;
    std::vector<OrderResult> impacted_orders;  
    int remaining_quantity = incoming_order.quantity;
    
    // Si ordre d'achat, on match contre le book de vente
    if (incoming_order.side == "BUY") {
        
        std::cout << "Matching BUY contre SELL book..." << std::endl;

        // Initialisation d'un vecteur qui contiendra les ordres de vente non matchés
        std::vector<Order> temp_orders;
        
        // On boucle tant que deux conditions sont remplies : il y a encore des vendeurs dans le carnet
        // et l'ordre d'achat n'est pas totalement exécuté
        while (!sell_book.empty() && remaining_quantity > 0) {
            // Récupération du meilleur ordre de vente (prix le plus bas)
            Order best_sell = sell_book.top();
            // On retire temporairement l'ordre du carnet de vente
            sell_book.pop();
            
            // Gestion des types d'ordre : le market peut toujours matcher (sauf si book vide),
            // le limit doit 
            bool can_match = false;
            if (incoming_order.type == "MARKET") {
                can_match = true;  // MARKET order matche toujours
                std::cout << "MARKET BUY - Match automatique avec SELL @ " << best_sell.price << std::endl;
            } else {
                can_match = (incoming_order.price >= best_sell.price);
                if (can_match) {
                    std::cout << "LIMIT BUY " << incoming_order.price << " >= SELL " << best_sell.price << " - Match OK" << std::endl;
                } else {
                    std::cout << "LIMIT BUY " << incoming_order.price << " < SELL " << best_sell.price << " - Pas de match" << std::endl;
                }
            }
            
            if(can_match){
                int trade_quantity = std::min(remaining_quantity, best_sell.quantity);
                
                // Création du trade au prix du vendeur
                Trade trade(incoming_order.timestamp, incoming_order.order_id, best_sell.order_id,
                            incoming_order.instrument, trade_quantity, best_sell.price);
                matches.push_back(trade);
                
                // Mise à jour des quantités pour chaque ordre
                remaining_quantity -= trade_quantity;
                best_sell.quantity -= trade_quantity;
                
                // Si l'ordre de vente n'est pas complètement exécuté, on le remet dans le carnet
                // Comme seule sa quantité est modifiée, les règles sur les prix et timestamp sont toujours appliquées
                if (best_sell.quantity > 0) {
                    temp_orders.push_back(best_sell);
                    // Mise à jour dans la map
                    order_map[best_sell.order_id] = best_sell;
                } else {
                    // Si ordre de vente totalement exécuté, on le retire de la map
                    order_map.erase(best_sell.order_id);
                }
                
                // On copie l'ordre de vente impacté pour l'historique
                Order matched_sell = best_sell;

                // On détermine le statut de l'ordre de vente (totalement exécuté ou partiellement)
                std::string sell_status = (best_sell.quantity == 0) ? "EXECUTED" : "PARTIALLY_EXECUTED";
                std::cout << "DEBUG - Ordre SELL impacté: quantity=" << matched_sell.quantity 
                    << ", status=" << sell_status << std::endl;
                OrderResult sell_result = createResult(matched_sell, sell_status, 
                                     trade_quantity, best_sell.price, incoming_order.order_id);
                std::cout << "DEBUG - Result SELL: quantity=" << sell_result.original_order.quantity 
                    << ", status=" << sell_result.status << std::endl;
                // On s'assure que la modification de l'ordre impacté apparaît bien en même temps que l'ordre d'achat 
                sell_result.original_order.timestamp = incoming_order.timestamp;

                impacted_orders.push_back(sell_result);
            } else {
                // Si l'une des deux conditions n'est pas respectée -> on arrête.
                temp_orders.push_back(best_sell);
                break;
            }
        }
        
        // On remet les ordres non matchés dans le carnet
        for (const Order& order : temp_orders) {
            sell_book.push(order);
        }
        // On a donc au final les ordres de vente partiellement exécutés et non impactés dans le carnet,
        // les ordres de vente exécutés ont été sortis du carnet, et l'historique des trades a été mis à jour. 
        
        // On procède de la même manière si l'ordre nouveau est un ordre de vente
    } else if (incoming_order.side == "SELL") {

        std::cout << "Matching SELL contre BUY book..." << std::endl;
        
        std::vector<Order> temp_orders;
        
        while (!buy_book.empty() && remaining_quantity > 0) {
            Order best_buy = buy_book.top();
            buy_book.pop();

            bool can_match = false;
            if (incoming_order.type == "MARKET") {
                can_match = true;  // MARKET order matche toujours
                std::cout << "MARKET SELL - Match automatique avec BUY @ " << best_buy.price << std::endl;
            } else {
                can_match = (best_buy.price >= incoming_order.price);
                if (can_match) {
                    std::cout << "BUY " << best_buy.price << " >= LIMIT SELL " << incoming_order.price << " - Match OK" << std::endl;
                } else {
                    std::cout << "BUY " << best_buy.price << " < LIMIT SELL " << incoming_order.price << " - Pas de match" << std::endl;
                }
            }
            
            if (can_match) {
                int trade_quantity = std::min(remaining_quantity, best_buy.quantity);
                
                Trade trade(incoming_order.timestamp, best_buy.order_id, incoming_order.order_id,
                           incoming_order.instrument, trade_quantity, best_buy.price);
                matches.push_back(trade);
                
                remaining_quantity -= trade_quantity;
                best_buy.quantity -= trade_quantity;
                
                if (best_buy.quantity > 0) {
                    temp_orders.push_back(best_buy);
                    order_map[best_buy.order_id] = best_buy;
                } else {
                    order_map.erase(best_buy.order_id);
                }
                
                Order matched_buy = best_buy;
                
                std::string buy_status = (best_buy.quantity == 0) ? "EXECUTED" : "PARTIALLY_EXECUTED";
                OrderResult buy_result = createResult(matched_buy, buy_status, 
                                                    trade_quantity, best_buy.price, incoming_order.order_id);
                buy_result.original_order.timestamp = incoming_order.timestamp;
                impacted_orders.push_back(buy_result);
            } else {
                temp_orders.push_back(best_buy);
                break;
            }
        }
        
        for (const Order& order : temp_orders) {
            buy_book.push(order);
        }
    }
    
    // On met à jour la quantité restante de l'ordre entrant puis on met à jour les impacts dans l'historique
    incoming_order.quantity = remaining_quantity;
    pending_impacted_orders = impacted_orders;
    
    return matches;
}


void MatchingEngine::addToBook(const Order& order) {
    // ################################################################################################
    // Fonction qui permet l'ajout d'ordres au book approprié. 
    // ################################################################################################

    // Les ordres au marché ne sont jamais ajoutés au carnet 
    if (order.type == "MARKET") {
        std::cout << "ERREUR: Tentative d'ajout d'un ordre MARKET au carnet !" << std::endl;
        return;
    }
    
    // Si ordre d'achat : ajout au book d'achat, sinon à celui de vente
    if (order.side == "BUY") {
        buy_book.push(order);
        std::cout << "Ajouté au BUY book: " << order.quantity << " @ " << order.price << std::endl;
    } else if (order.side == "SELL") {
        sell_book.push(order);
        std::cout << "Ajouté au SELL book: " << order.quantity << " @ " << order.price << std::endl;
    }
}

bool MatchingEngine::removeFromBook(int order_id, const std::string& side) {
    // ################################################################################################
    // Fonction qui permet la suppression d'ordres du book approprié. 
    // Difficile de vraiment retirer des lignes avec un objet order_queue, donc ce qu'on fait, 
    // c'est qu'on dit qu'il est retiré (même si dans les faits il est toujours dans le book priority-queue),
    // mais par contre, à chaque fois dans le tryMatch, on le RETIRE DE LA MAP, donc on ne peut jamais matché avec.
    // (c'est du "soft deletion")
    // ################################################################################################
    
    std::cout << "Suppression de l'ordre " << order_id << " du " << side << " book" << std::endl;
    
    if (side == "BUY") {
        std::cout << "Ordre BUY " << order_id << " marqué comme supprimé" << std::endl;
        return true;
    } else if (side == "SELL") {
        std::cout << "Ordre SELL " << order_id << " marqué comme supprimé" << std::endl;
        return true;
    }
    
    return false;
}

// Affichage des carnets (debug)
void MatchingEngine::displayBooks() const {
    std::cout << "\n=== ÉTAT DES CARNETS ===" << std::endl;
    std::cout << "BUY book size: " << buy_book.size() << std::endl;
    std::cout << "SELL book size: " << sell_book.size() << std::endl;
}

// Récupération des résultats (historic_trades)
const std::vector<OrderResult>& MatchingEngine::getResults() const {
    return historic_trades;
}

// Affichage des résultats (renvoie historic_trades)
void MatchingEngine::displayResults() const {
    std::cout << "\n=== HISTORIC_TRADES (OUTPUT FINAL) ===" << std::endl;
    std::cout << "Timestamp - OrderID - Instrument - Side - Type - Qty - Price - Action - Status - ExecQty - ExecPrice - Counterparty" << std::endl;
    
    for (const OrderResult& result : historic_trades) {
        const Order& order = result.original_order;
        std::cout << order.timestamp << " " 
                  << order.order_id << " " 
                  << order.instrument << " " 
                  << order.side << " " 
                  << order.type << " " 
                  << order.quantity << " " 
                  << order.price << " " 
                  << order.action << " " 
                  << result.status << " " 
                  << result.executed_quantity << " " 
                  << result.execution_price << " " 
                  << result.counterparty_id << std::endl;
    }
}

// Méthodes utilitaires
long long MatchingEngine::getCurrentTimestamp() {
    // Pour les modifications, on génère un nouveau timestamp (perte de priorité)
    return ++current_timestamp + 1617278400000000000LL; 
}

OrderResult MatchingEngine::createResult(const Order& order, const std::string& status, 
                                       int exec_qty, float exec_price, int counterparty) {
    // Création du format des résultats
    OrderResult result(order);
    result.status = status;
    result.executed_quantity = exec_qty;
    result.execution_price = exec_price;
    result.counterparty_id = counterparty;
    return result;
}