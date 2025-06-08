#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>  
#include <iomanip>   
#include "data/CSVWriter.h"

// Constructeur et destructeur par défaut
CsvWriter::CsvWriter(){
    std::cout << "Initialisation du CSV writer" << std::endl;
}

CsvWriter::~CsvWriter(){
    std::cout << "Destruction du CSV writer" << std::endl;
}

// Initialisation avec le nom du fichier à créer
CsvWriter::CsvWriter(std::string filename){
    CsvWriter::filename = filename;
}

std::string CsvWriter::formatPrice(float price) {
    // Cas spécial pour 0
    if (price == 0.0f) {
        return "0";
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << price;
    std::string str = oss.str();
    
    // On supprime les zéros inutiles et le point si pas de décimale
    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
    str.erase(str.find_last_not_of('.') + 1, std::string::npos);
    
    return str;
}

// Méthode permettant de transformer un ordre en chaîne de caractère
std::string CsvWriter::OrderToString(OrderResult order_result){

    // Récupération des caractéristiques de l'ordre initial
    Order order = order_result.original_order;

    // Concaténation de toutes les valeurs de OrderResult
    std::string output = std::to_string(order.timestamp) + "," + 
                        std::to_string(order.order_id) + "," + 
                        order.instrument + "," +
                        order.side + "," + 
                        order.type + "," + 
                        std::to_string(order.quantity) + "," +
                        formatPrice(order.price) + "," + // Pour arrondir à deux décimales
                        order.action + "," + 
                        order_result.status + "," +
                        std::to_string(order_result.executed_quantity) + "," + 
                        formatPrice(order_result.execution_price) + "," + // Pour arrondir à 2 décimales 
                        std::to_string(order_result.counterparty_id); 

    // Récupération de l'output
    return(output);
}

void CsvWriter::WriteToCsv(std::vector<OrderResult> resOrders){

    // Création du fichier
    std::ofstream output_file;

    // Modification du nom du fichier
    output_file.open(filename);

    output_file << "timestamp,order_id,instrument,side,type,quantity,price,action,status,executed_quantity,execution_price,counterparty_id" << std::endl;

    // Boucle sur les les trades de l'historique
    for(u_long i = 0; i < resOrders.size(); i++){
        
        // Récupération de l'ordre courant
        OrderResult current_order = resOrders[i];

        // Récupération de la chaîne de caractère à passer dans le CSV
        std::string string_order = OrderToString(current_order);

        // Ecriture dans le fichier csv
        output_file << string_order << std::endl;
    }

    // Fermeture du fichier
    output_file.close();
}
