// FICHIER DE TESTS D'OUTPUT SIMPLES, POUR VERIFIER LES RESULTATS

#include "core/MatchingEngine.h"
#include "data/CSVReader.h"
#include "data/CSVWriter.h"
#include <iostream>
#include <fstream>
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
// Fonction utilitaire pour comparer deux fichiers CSV ligne par ligne
// ###########################################################################################################

bool compareCSVFiles(const std::string& expected_file, const std::string& generated_file) {
    std::ifstream expected(expected_file);
    std::ifstream generated(generated_file);

    if (!expected.is_open()) {
        std::cerr << "ERREUR : Impossible d'ouvrir " << expected_file << std::endl;
        return false;
    }

    if (!generated.is_open()) {
        std::cerr << "ERREUR : Impossible d'ouvrir " << generated_file << std::endl;
        return false;
    }

    std::string line_expected, line_generated;
    int line_number = 1;
    bool all_match = true;

    // Comparaison ligne à ligne
    while (std::getline(expected, line_expected) && std::getline(generated, line_generated)) {
        if (line_expected != line_generated) {
            std::cerr << "DIFFERENCE ligne " << line_number << " :" << std::endl;
            std::cerr << "  Attendu  : " << line_expected << std::endl;
            std::cerr << "  Généré   : " << line_generated << std::endl;
            all_match = false;
        }
        line_number++;
    }

    // On vérifie qu'il n'y a pas de lignes supplémentaires
    if (std::getline(expected, line_expected)) {
        std::cerr << "ERREUR : Fichier attendu a plus de lignes que le généré" << std::endl;
        all_match = false;
    }

    if (std::getline(generated, line_generated)) {
        std::cerr << "ERREUR : Fichier généré a plus de lignes que l'attendu" << std::endl;
        all_match = false;
    }

    if (all_match) {
        std::cout << "Tous les " << (line_number - 1) << " lignes correspondent" << std::endl;
    }

    return all_match;
}

// ###########################################################################################################
// Test de concordance qui prend en paramètre les fichiers input et output attendu
// Ce test vérifie que le matching engine produit exactement le même output que celui attendu
// ###########################################################################################################

void testConcordanceWithExpectedOutput(const std::string& input_file, const std::string& expected_output_file, const std::string& test_name) {
    std::cout << "Test de concordance : " << test_name << std::endl;

    MatchingEngine engine;

    // GIVEN : fichier d'input et output attendu
    CsvReader reader(input_file);
    reader.init();
    auto orders = reader.getOrders();

    EXPECT_TRUE(!orders.empty()); // On s'assure que les ordres sont bien lus
    std::cout << "Input lu depuis " << input_file << " : " << orders.size() << " ordres" << std::endl;

    // WHEN : exécution du matching engine
    auto results = engine.processAllOrders(orders);

    // MODIFICATION : Générer le fichier dans GeneratedOutputs
    // Extraire juste le nom du fichier sans le chemin
    std::string input_filename = input_file.substr(input_file.find_last_of('/') + 1);
    std::string base_name = input_filename.substr(0, input_filename.find_last_of('.'));
    std::string generated_file = "tests/SimpleOutputs/GeneratedOutputs/" + base_name + "_generated_output.csv";
    
    // Créer le dossier GeneratedOutputs s'il n'existe pas
    system("mkdir -p tests/SimpleOutputs/GeneratedOutputs");
    
    // On écrit l'output généré
    CsvWriter writer(generated_file);
    writer.WriteToCsv(results);
    std::cout << "Output généré dans " << generated_file << " : " << results.size() << " résultats" << std::endl;

    // THEN : comparaison ligne par ligne avec l'output attendu
    bool concordance = compareCSVFiles(expected_output_file, generated_file);

    EXPECT_TRUE(concordance);
    std::cout << "PASS : " << test_name << " - Concordance parfaite\n";
}


// =============================================================================
// MAIN AVEC SUPPORT D'ARGUMENTS
// =============================================================================

int main(int argc, char* argv[]) {
    std::cout << "\n=== TEST DE CONCORDANCE AVEC RESULTATS CONNUS ===\n" << std::endl;
    
    // Mode par défaut : test du prof
    std::cout << "Test 1 : résultats du sujet donné" << std::endl;
    testConcordanceWithExpectedOutput(
            "tests/SimpleOutputs/Inputs/Test1.csv",
            "tests/SimpleOutputs/ExpectedOutputs/Test1_ExpectedOutput.csv",
            "Exemple du sujet"
            );
    
    std::cout << "Test 2 : avec erreurs" << std::endl;
    testConcordanceWithExpectedOutput(
            "tests/SimpleOutputs/Inputs/Test2.csv",
            "tests/SimpleOutputs/ExpectedOutputs/Test2_ExpectedOutput.csv",
            "Input avec erreurs"
            );

    std::cout << "\nTOUS LES TESTS ONT ETE PASSES AVEC SUCCES !" << std::endl;
    return 0;
}