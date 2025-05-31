#include <iostream>
#include "includes/data/CSVReader.h"
#include "includes/data/CSVWriter.h"
#include "includes/core/MatchingEngine.h"

int main() {
    // Chargement des ordres
    CsvReader csvReader("input_orders_final.csv");
    csvReader.init();
    csvReader.Display();
    
    // Matching Engine
    MatchingEngine engine;
    engine.processAllOrders(csvReader.getOrders());

    // Affichage des résultats
    engine.displayResults();

    // Sauvegarde en csv
    CsvWriter csvWriter("output.csv");
    csvWriter.WriteToCsv(engine.historic_trades);
    
    return 0;
}