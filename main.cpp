#include <fstream>
#include <string>
#include <iostream>
#include <memory> // Shared ptr pointeur intelligents ==> dès qu'ils ne sont plus utilisés, sont delete automatiquement
#include "includes/data/CSVReader.h"

int main() {
    // Initialisation du CSV reader
    CsvReader test("input.csv");

    // Récupération des ordres
    test.init();

    // Visualisation
    test.Display();
    return 0;
}
