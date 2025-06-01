#include <iostream>
#include "includes/data/CSVReader.h"
#include "includes/core/MatchingEngine.h"

int main() {
    // Chargement des ordres
    CsvReader csvReader("input_test.csv");
    csvReader.init();
    csvReader.Display();
    
    // Matching Engine
    MatchingEngine engine;
    engine.processAllOrders(csvReader.getOrders());

    // Affichage des résultats
    engine.displayResults();

    return 0;
}