#include <iostream>
#include <map>
#include <vector>
#include "includes/data/CSVReader.h"
#include "includes/data/CSVWriter.h"
#include "includes/core/MatchingEngine.h"

int main() {
    // Chargement des ordres
    CsvReader csvReader("Inputs/input_with_market_orders.csv");
    csvReader.init();
    csvReader.Display();
    
    // Récupération du mapping
    std::map<std::string, std::vector<Order>> map_asset_orders = csvReader.getMapOrder();

    // Boucle sur chaque actif
    for(auto i:map_asset_orders){

        // Récupération du nom de l'actif
        std::string asset_name = i.first;

        // Récupération du vecteur d'ordres
        std::vector<Order> asset_order_vector = i.second;

        // Initialisation du matching engine
        MatchingEngine engine;
        engine.processAllOrders(asset_order_vector);

        // Affichage des résultats
        engine.displayResults();

        // Savegarde en csv
        CsvWriter csvWriter_test("Outputs/output_with_market_orders " + asset_name + ".csv");
        std::vector<OrderResult> trade_historic = engine.getTradeHistoric();
        csvWriter_test.WriteToCsv(trade_historic);
    }

    // Matching Engine
    // MatchingEngine engine;
    // engine.processAllOrders(csvReader.getOrders());

    // Affichage des résultats
    // engine.displayResults();

    // Sauvegarde en csv
    // CsvWriter csvWriter("output.csv");
    // csvWriter.WriteToCsv(engine.historic_trades);
    
    return 0;
}