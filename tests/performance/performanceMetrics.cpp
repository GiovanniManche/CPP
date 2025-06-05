// FICHIER D'EVALUATION DES PERFORMANCES
// On évalue, pour chaque fichier, le temps d'exécution de tous les ordres, 
// le nombre d'ordres par seconde, la latence et la mémoire utilisée. 
#include "core/MatchingEngine.h"
#include "data/CSVReader.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <sys/resource.h>

// Structure contenant les résultats (les performances)
struct BenchmarkResult {
    std::string filename;
    size_t num_orders;
    double execution_time_ms;
    double orders_per_second;
    double avg_latency_us;
    size_t peak_memory_kb;
    
    BenchmarkResult() : num_orders(0), execution_time_ms(0.0), 
                       orders_per_second(0.0), avg_latency_us(0.0), peak_memory_kb(0) {}
};

// Fonction qui mesure la mémoire utilisée
size_t measurePeakMemory() {
#ifdef __linux__
    // Ouverture du fichier spécial Linux contenant les infos du processus actuel
    std::ifstream status_file("/proc/self/status");
    std::string line;
    
    // Récupération de la RAM effectivement utilisée (ligne VmRSS)
    while (std::getline(status_file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            // Extraire la valeur en kB
            size_t pos = line.find_first_of("0123456789");
            if (pos != std::string::npos) {
                return std::stoull(line.substr(pos));
            }
        }
    }
    return 0;
    
#elif defined(__APPLE__)
    // macOS : utilisation de getrusage()
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // ru_maxrss est en bytes sur macOS
        return usage.ru_maxrss / 1024;
    }
    return 0;
    
#else
    // Fallback pour autres systèmes
    return 0;
#endif
}

// Fonction pour compter les ordres dans un fichier CSV
size_t countOrders(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Erreur: impossible d'ouvrir " << filepath << std::endl;
        return 0;
    }
    
    size_t count = 0;
    std::string line;
    
    // Skip header
    if (std::getline(file, line)) {
        while (std::getline(file, line)) {
            if (!line.empty()) {
                count++;
            }
        }
    }
    return count;
}

// Fonction principale qui calcule toutes les métriques pour un fichier d'ordres
BenchmarkResult benchmarkFile(const std::string& input_file) {
    BenchmarkResult result;
    result.filename = input_file;
    result.num_orders = countOrders(input_file);
    
    if (result.num_orders == 0) {
        std::cout << "Erreur: fichier vide ou inaccessible: " << input_file << std::endl;
        return result;
    }
    
    std::cout << "Performances pour le fichier avec " << result.num_orders << " données : " << input_file << std::endl;
    
    // 1. Mesure du temps d'exécution (on démarre le timer)
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // Lecture des ordres
        CsvReader reader(input_file);
        reader.init();
        auto orders = reader.getOrders();
        
        // Traitement par le matching engine
        MatchingEngine engine;
        auto results = engine.processAllOrders(orders);
        
    } catch (const std::exception& e) {
        std::cout << "Erreur lors du traitement: " << e.what() << std::endl;
        return result;
    }
    // Fin du timer et calcul de la durée d'exécution (c'est en millisecondes)
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // 2. Calcul des autres métriques
    result.execution_time_ms = duration.count() / 1000.0;  // transformation en secondes du temps d'exec
    result.orders_per_second = (result.num_orders * 1000.0) / result.execution_time_ms;  
    result.avg_latency_us = static_cast<double>(duration.count()) / result.num_orders;
    result.peak_memory_kb = measurePeakMemory();
    
    // Affichage des résultats
    std::cout << "Temps d'exécution: " << std::fixed << std::setprecision(2) 
              << result.execution_time_ms << " ms" << std::endl;
    std::cout << "Débit: " << std::fixed << std::setprecision(0) 
              << result.orders_per_second << " ordres/seconde" << std::endl;
    std::cout << "Latence moyenne: " << std::fixed << std::setprecision(2) 
              << result.avg_latency_us << " µs/ordre" << std::endl;
    std::cout << "Mémoire utilisée: " << result.peak_memory_kb << " KB" << std::endl;
    std::cout << "Traitement terminé avec succès !!\n" << std::endl;
    
    return result;
}

// Fonction pour afficher un résumé comparatif
void displaySummary(const std::vector<BenchmarkResult>& results) {
    if (results.empty()) return;
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "RÉSUMÉ COMPARATIF DES PERFORMANCES" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    std::cout << std::left 
              << std::setw(25) << "Fichier"
              << std::setw(15) << "Nb Ordres"
              << std::setw(15) << "Temps (s)"
              << std::setw(15) << "Ordres/sec"
              << std::setw(15) << "Latence (µs)"
              << std::setw(15) << "Mémoire (KB)"
              << std::endl;
    
    std::cout << std::string(110, '-') << std::endl;
    
    for (const auto& result : results) {
        if (result.num_orders > 0) {
            std::cout << std::left << std::fixed
                      << std::setw(25) << result.filename.substr(result.filename.find_last_of("/\\") + 1)
                      << std::setw(15) << result.num_orders
                      << std::setw(15) << std::setprecision(3) << (result.execution_time_ms / 1000.0)
                      << std::setw(15) << std::setprecision(0) << result.orders_per_second
                      << std::setw(15) << std::setprecision(2) << result.avg_latency_us
                      << std::setw(15) << result.peak_memory_kb
                      << std::endl;
        }
    }
            std::cout << std::string(110, '-') << std::endl;
}

// MAIN : on fait les tests de performance sur le nombre de fichiers que l'on souhaite. 
// Il suffit d'ajouter une ligne 
int main() {
    std::cout << "MATCHING ENGINE - BENCHMARK DE PERFORMANCE\n" << std::endl;
    
    std::vector<BenchmarkResult> results;
    
    // Test avec 3 fichiers différents
    results.push_back(benchmarkFile("tests/performance/inputs/10_orders.csv"));
    results.push_back(benchmarkFile("tests/performance/inputs/100_orders.csv"));
    results.push_back(benchmarkFile("tests/performance/inputs/1000_orders.csv"));
    results.push_back(benchmarkFile("tests/performance/inputs/10000_orders.csv"));
    results.push_back(benchmarkFile("tests/performance/inputs/100000_orders.csv"));
    
    // Affichage du résumé comparatif
    displaySummary(results);
    
    return 0;
}