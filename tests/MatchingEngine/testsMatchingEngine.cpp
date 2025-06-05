// FICHIER DE TESTS SUR LA LOGIQUE ET LES EXCEPTIONS DU MATCHING ENGINE
// On s'attache à suivre la structure classique "GIVEN - WHEN - THEN"

#include "core/MatchingEngine.h"
#include <iostream>
#include <vector>
#include <cassert>

// Macros de test : une de comparaison, une de vérité
#define EXPECT_EQ(actual, expected) \
    if ((actual) != (expected)) { \
        std::cerr << "FAIL : expected '" << expected << "' but got '" << actual << "'\n"; \
        std::exit(1); \
    }

#define EXPECT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "FAIL : expected condition to be true\n"; \
        std::exit(1); \
    }

// ###########################################################################################################
// Test qui vérifie que la modification de la quantité ne rend jamais cette dernière négative
// Ex : quantité initiale = 50, et 20 a déjà été exécutée. Si l'ordre est modifié pour une quantité de 10,
// cela veut dire que le client souhaite réduire sa quantité de 40. Or l'ordre n'a plus qu'une quantité de 30. 
// Donc on doit tomber à 0. 
// ###########################################################################################################

void testModifyQtyNotNeg() {
    std::cout << "Test d'impossibilité de quantité négative avec un Modify" << std::endl;

    MatchingEngine engine;

    // GIVEN (ordre partiellement exécuté et sa modification substantielle)
    std::vector<Order> orders = {
        {1000, 1, "AAPL", "BUY", "LIMIT", 50, 150.0, "NEW"},
        {2000, 2, "AAPL", "SELL", "LIMIT", 20, 150.0, "NEW"},
        {3000, 1, "AAPL", "BUY", "LIMIT", 10, 151.0, "MODIFY"}
    };

     // WHEN (processus de matching)
    auto results = engine.processAllOrders(orders);

    // THEN (le MODIFY doit transformer l'ordre en un ordre exécuté)
    bool modify_found = false;
    for (const auto& result : results) {
        if (result.original_order.order_id == 1 && result.original_order.action == "MODIFY") {
            EXPECT_EQ(result.status, "EXECUTED");
            EXPECT_EQ(result.original_order.quantity, 0);
            modify_found = true;
        }
    }
    EXPECT_TRUE(modify_found);
    std::cout << "PASS : Un MODIFY ne donne pas de quantité négative\n";
}

// ###########################################################################################################
// Test qui vérifie qu'aucun duplicate d'ID n'est possible. Concrètement, si un ordre avec ID = 1 existe déjà,
// alors un ordre avec action NEW et ID = 1 sera automatiquement rejeté. 
// ###########################################################################################################

void testIdNoDuplicatePossible() {
    std::cout << "Test d'impossibilité d'ID dupliqué pour NEW" << std::endl;

    MatchingEngine engine;
    // GIVEN : deux ordres avec le même ID et action NEW
    std::vector<Order> orders = {
        {1000, 123, "AAPL", "BUY", "LIMIT", 100, 150.0, "NEW"},
        {2000, 123, "AAPL", "SELL", "LIMIT", 50, 151.0, "NEW"}
    };

    // WHEN : quand ils entrent dans le matching engine
    auto results = engine.processAllOrders(orders);

    // THEN : le second est rejeté
    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].status, "PENDING");
    EXPECT_EQ(results[1].status, "REJECTED");

    std::cout << "PASS : ID non dupliqué pour NEW\n";
}

// ###########################################################################################################
// Test qui vérifie qu'un ordre de modification est rejeté si l'ID n'existe pas encore.
// ###########################################################################################################

void testRejectModifyWithInexistantID() {
    std::cout << "Test de rejet d'un MODIFY sur ID inexistant" << std::endl;

    MatchingEngine engine;

    // GIVEN : un nouvel ordre et un ordre de modification avec un ID inexistant
    std::vector<Order> orders = {
        {1000, 1, "AAPL", "BUY", "LIMIT", 100, 150.0, "NEW"},
        {2000, 999, "AAPL", "BUY", "LIMIT", 50, 151.0, "MODIFY"}
    };

    // WHEN : entrée dans le matching engine
    auto results = engine.processAllOrders(orders);

    // THEN : le statut de l'ordre doit être "REJECTED"
    bool modify_rejected = false;
    for (const auto& result : results) {
        if (result.original_order.order_id == 999 && result.original_order.action == "MODIFY") {
            EXPECT_EQ(result.status, "REJECTED");
            modify_rejected = true;
        }
    }
    EXPECT_TRUE(modify_rejected);
    std::cout << "PASS : MODIFY sur ID inexistant rejeté\n";
}

// ###########################################################################################################
// Test qui vérifie qu'un ordre de suppression est rejeté si l'ID n'existe pas encore. Fonctionne 
// rigoureusement comme le test précédent.
// ###########################################################################################################

void testRejectCancelWithInexistantID() {
    std::cout << "Test de rejet d'un CANCEL sur ID inexistant" << std::endl;

    MatchingEngine engine;

    std::vector<Order> orders = {
        {1000, 1, "AAPL", "BUY", "LIMIT", 100, 150.0, "NEW"},
        {2000, 999, "AAPL", "BUY", "LIMIT", 0, 0, "CANCEL"}
    };

    auto results = engine.processAllOrders(orders);

    bool cancel_rejected = false;
    for (const auto& result : results) {
        if (result.original_order.order_id == 999 && result.original_order.action == "CANCEL") {
            EXPECT_EQ(result.status, "REJECTED");
            cancel_rejected = true;
        }
    }
    EXPECT_TRUE(cancel_rejected);
    std::cout << "PASS : CANCEL sur ID inexistant rejeté\n";
}

// ###########################################################################################################
// Test qui vérifie qu'un ordre au marché est rejeté si le carnet d'ordre opposé est vide.
// ###########################################################################################################

void testRejectMarketIfNoCounterparty() {
    std::cout << "Test de rejet d'un ordre au marché sans contrepartie" << std::endl;

    MatchingEngine engine;

    // GIVEN : un ordre au marché seul
    std::vector<Order> orders = {
        {1000, 1, "AAPL", "BUY", "MARKET", 100, 0, "NEW"}
    };

    // WHEN : entrée dans le matching engine (ne peut pas matcher par définition)
    auto results = engine.processAllOrders(orders);

    // THEN : le statut de l'ordre = rejeté
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].status, "REJECTED");

    std::cout << "PASS : MARKET order sans contrepartie rejeté\n";
}

// ###########################################################################################################
// Test qui vérifie qu'un ordre au marché est automatiquement matché si le carnet opposé n'est pas vide.
// ###########################################################################################################
void testExecuteMarketIfCounterparty() {
    std::cout << "Test d'exécution immédiate d'un ordre au marché avec contrepartie" << std::endl;

    MatchingEngine engine;

    // GIVEN : un ordre de vente limite qui reste au carnet et un ordre d'achat au marché de quantité plus faible (qui doit donc matcher)
    std::vector<Order> orders = {
        {1000, 1, "AAPL", "SELL", "LIMIT", 50, 150.0, "NEW"},
        {2000, 2, "AAPL", "BUY", "MARKET", 30, 0, "NEW"}
    };

    // WHEN : entrée dans le matching engine
    auto results = engine.processAllOrders(orders);

    // THEN : il doit être exécuté 
    bool market_executed = false;
    for (const auto& result : results) {
        if (result.original_order.order_id == 2 && result.original_order.action == "NEW") {
            EXPECT_EQ(result.status, "EXECUTED");
            EXPECT_EQ(result.executed_quantity, 30);
            EXPECT_EQ(result.execution_price, 150.0);
            market_executed = true;
        }
    }
    EXPECT_TRUE(market_executed);
    std::cout << "PASS : MARKET order avec contrepartie exécuté immédiatement\n";
}

// ###########################################################################################################
// Test qui vérifie qu'un ordre est automatiquement rejeté s'il ne passe pas les tests de cohérence du 
// CSVReader et est donc labellisé "BAD_INPUT".
// ###########################################################################################################
void testRejectIfBadInput() {
    std::cout << "Test de rejet d'un type BAD_INPUT" << std::endl;

    MatchingEngine engine;

    // GIVEN (ordre avec bad_input)
    std::vector<Order> orders = {
        {1000, 1, "AAPL", "BUY", "BAD_INPUT", 100, 150.0, "NEW"}
    };

    // WHEN : entrée dans le matching engine
    auto results = engine.processAllOrders(orders);

    // THEN : rejet
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].status, "REJECTED");

    std::cout << "PASS : Type BAD_INPUT automatiquement rejeté\n";
}

// ###########################################################################################################
// Test "tout en un" qui vérifie les multiples cas
// ###########################################################################################################
void testMultipleCases() {
    std::cout << "Test: Scénario complexe" << std::endl;

    MatchingEngine engine;

    std::vector<Order> orders = {
        {1000, 1, "AAPL", "BUY", "LIMIT", 100, 150.0, "NEW"},
        {2000, 2, "AAPL", "SELL", "LIMIT", 60, 150.0, "NEW"},
        {3000, 1, "AAPL", "BUY", "LIMIT", 50, 151.0, "NEW"},
        {4000, 1, "AAPL", "BUY", "LIMIT", 10, 152.0, "MODIFY"},
        {5000, 3, "AAPL", "BUY", "MARKET", 50, 0, "NEW"},
        {6000, 4, "AAPL", "SELL", "BAD_INPUT", 25, 145.0, "NEW"},
        {7000, 999, "AAPL", "BUY", "LIMIT", 0, 0, "CANCEL"}
    };

    auto results = engine.processAllOrders(orders);

    int rejected_count = 0;
    for (const auto& result : results) {
        if (result.status == "REJECTED") rejected_count++;
    }

    EXPECT_EQ(rejected_count, 4);
    std::cout << "PASS : Scénario complexe qui rejette tout\n";
}

// ###########################################################################################################
// MAIN
// ###########################################################################################################

int main() {
    std::cout << "\n=== TESTS UNITAIRES - CAS LIMITES TRAITES PAR LE MATCHING ENGINE ===\n" << std::endl;

    testModifyQtyNotNeg();
    testIdNoDuplicatePossible();
    testRejectModifyWithInexistantID();
    testRejectCancelWithInexistantID();
    testRejectMarketIfNoCounterparty();
    testExecuteMarketIfCounterparty();
    testRejectIfBadInput();
    testMultipleCases();

    std::cout << "TOUS LES TESTS ONT ETE PASSES AVEC SUCCES !" << std::endl;
    return 0;
}
