// FICHIER DE TESTS SUR LA LOGIQUE ET LES EXCEPTIONS DU CSV Reader

#include "data/CSVReader.h"
#include <iostream>
#include <vector>
#include <cassert>

// Macros de test : une de comparaison, une de vérité
#define EXPECT_EQ(actual, expected) \
    if ((actual) != (expected)) { \
        std::cerr << "FAIL : expected '" << expected << "' but got '" << actual << "'\n"; \
        std::exit(1); \
    }

//////////////////////////////////////////////////////////////////////
// Test qui vérifie que lorsque tous les inputs contiennent une erreur, 
// tous les ordres sont référencés avec la mention bad input 
//////////////////////////////////////////////////////////////////////

void testOnlyBadInputs(){

    std::cout << "Test sur les imports lorsque tous les ordres contiennent une erreur " << std::endl;

    // Importation des données contenant toutes les erreurs
    CsvReader csvReader("input_test_all_error.csv"); 
    csvReader.init();

    // Récupération de l'ensemble des ordres 
    std::vector<Order> orders_computed = csvReader.getOrders();

    // Output attendu
    std::vector<Order> orders_expected = {
        {0, 0,"AAPL" ,"BUY" ,"BAD_INPUT" , 0, 0, "NEW"},
        {1617278400000000100, 0,"AAPL" ,"SELL" ,"BAD_INPUT" , 0, 0, "NEW"},
        {1617278400000000200, 3 ,"ERROR" ,"SELL" ,"LIMIT" , 60, 150.300000, "NEW"},
        {1617278400000000300, 0,"AAPL" ,"ERROR" ,"BAD_INPUT" , 0, 0, "NEW"},
        {1617278400000000400, 0,"AAPL" ,"BUY" ,"BAD_INPUT", 0, 0, "MODIFY"},
        {1617278400000000500, 0,"AAPL" ,"SELL" ,"BAD_INPUT" , 0, 0, "CANCEL"},
        {1617278400000000600, 0,"AAPL" ,"SELL" ,"BAD_INPUT" , 0, 0, "CANCEL"},
        {1617278400000000700, 0, "AAPL", "SELL", "BAD_INPUT", 0, 0.0, "ERROR"}
    };

    // Comparaison 
    for(u_long i = 0; i < orders_computed.size(); i++){
        Order current_order = orders_computed[i];
        Order current_order_expected = orders_expected[i];
        
        EXPECT_EQ(current_order.action, current_order_expected.action);
        EXPECT_EQ(current_order.instrument, current_order_expected.instrument);
        EXPECT_EQ(current_order.order_id, current_order_expected.order_id);
        EXPECT_EQ(current_order.price, current_order_expected.price);
        EXPECT_EQ(current_order.quantity, current_order_expected.quantity);
        EXPECT_EQ(current_order.side, current_order_expected.side);
        EXPECT_EQ(current_order.timestamp, current_order_expected.timestamp);
        EXPECT_EQ(current_order.type, current_order_expected.type);
    }

    // Résultat si valide
    std::cout << "Test ok - tous les ordres renvoient une erreur " << std::endl;
}


/////////////////////////////////////////////////////////////////////////////
// Test qui vérifie le comportement lorsque certains inputs sont manquants //
/////////////////////////////////////////////////////////////////////////////

void testWithBadInputs(){

    std::cout << "Test sur les imports lorsque certains ordres contiennent une erreur " << std::endl;

    // Importation des données contenant toutes les erreurs
    CsvReader csvReader("input_with_errors.csv"); 
    csvReader.init();

    // Résultat
    std::vector<Order> orders_computed = csvReader.getOrders();

    // Résultat attendu
    std::vector<Order> orders_expected = {
        {1617278400000000000, 0 ,"AAPL" ,"BUY" ,"BAD_INPUT" , 0, 0, "NEW"},
        {1617278400000000100, 2,"AAPL" ,"SELL" ,"LIMIT" , 50, 150.25, "NEW"},
        {1617278400000000200, 3,"AAPL" ,"SELL" ,"LIMIT" , 60, 150.3, "NEW"},
        {1617278400000000300, 4 ,"AAPL" ,"BUY" ,"LIMIT" , 40, 150.2, "NEW"},
        {1617278400000000400, 0 ,"AAPL" ,"BUY" ,"BAD_INPUT", 0, 0, "TEST"},
        {1617278400000000500, 0 ,"AAPL" ,"SELL" ,"BAD_INPUT", 0, 0, "CANCEL"}
    };

        // Comparaison 
    for(u_long i = 0; i < orders_computed.size(); i++){
        Order current_order = orders_computed[i];
        Order current_order_expected = orders_expected[i];

        EXPECT_EQ(current_order.action, current_order_expected.action);
        EXPECT_EQ(current_order.instrument, current_order_expected.instrument);
        EXPECT_EQ(current_order.order_id, current_order_expected.order_id);
        EXPECT_EQ(current_order.price, current_order_expected.price);
        EXPECT_EQ(current_order.quantity, current_order_expected.quantity);
        EXPECT_EQ(current_order.side, current_order_expected.side);
        EXPECT_EQ(current_order.timestamp, current_order_expected.timestamp);
        EXPECT_EQ(current_order.type, current_order_expected.type);
    }

    // Résultat si valide
    std::cout << "Test ok" << std::endl;
}

int main() {
    std::cout << "\n=== TESTS UNITAIRES - CAS LIMITES TRAITES PAR LE MATCHING ENGINE ===\n" << std::endl;

    testOnlyBadInputs();
    testWithBadInputs();

    std::cout << "TOUS LES TESTS ONT ETE PASSES AVEC SUCCES !" << std::endl;
    return 0;
}