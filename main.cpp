#include <iostream>
#include <pthread.h>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

/**
 * @author Ramazan Burak Saritas
 * 2020400321
*/

/**
 * Updates the balance of given company by the given amount.
*/
int updateBalance(int amount, std::string companyName, std::map<std::string, int> &balances){
    std::map<std::string, int>::iterator iterator = balances.find(companyName);
    if (iterator != balances.end()){
        iterator->second += amount;
        return 1;
    } else return -1;
}
/**
 * Struct to keep track of the customers data.
*/
struct customer{
    int id;
    int sleepTime; 
    int machineId;
    std::string companyName; 
    int paymentAmount;
};

/**
 * Main driver program.
*/
int main(int argc, char *argv[]){
    // Reads input file name and create output file name.
    std::string input_filename = argv[1];
    std::string output_filename = input_filename.substr(0, input_filename.size() - 4) + "_log.txt";
    // Initializes input and output file streams.
    std::ifstream input(input_filename);
    std::ofstream output(output_filename);
    // Reads first line to get number of customers.
    std::string firstLine;
    std::getline(input, firstLine);
    int numberOfCustomers = std::stoi(firstLine);
    // Map for bank account balances of companies.
    std::map<std::string, int> balances;
    balances.insert(std::pair<std::string, int>("Kevin", 0));
    balances.insert(std::pair<std::string, int>("Bob", 0));
    balances.insert(std::pair<std::string, int>("Stuart", 0));
    balances.insert(std::pair<std::string, int>("Otto", 0));
    balances.insert(std::pair<std::string, int>("Dave", 0));
    // Reads rest of the file for the customers data.
    std::string customerLine;
    for (int id=1; id<numberOfCustomers+1; id++) {
        std::getline(input, customerLine);
        std::stringstream lineStream(customerLine);
        std::string sleepTime, machineId, companyName, paymentAmount;
        std::getline(lineStream, sleepTime, ',');
        std::getline(lineStream, machineId, ',');
        std::getline(lineStream, companyName, ',');
        std::getline(lineStream, paymentAmount, ',');
        customer currentCustomer = {id, stoi(sleepTime), stoi(machineId), companyName, stoi(paymentAmount)};
    }


    return 0;
}