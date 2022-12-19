#include <iostream>
#include <pthread.h>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <unistd.h>

/**
 * @author Ramazan Burak Saritas
 * 2020400321
*/

// Pthread IDs for the vending machines.
static pthread_t machine_thread_ids[10];
// Mutexes for vending machines. 
static pthread_mutex_t machine_mutex_ids[] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER
};
// Mutexes for customers. 
static pthread_mutex_t customer_mutex_ids[]{
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER
};
pthread_attr_t attr; // Default attributes for pthread.

/**
 * Updates the balance of given company by the given amount.
*/
int updateBalance(int amount, std::string companyName, std::map<std::string, int> &balances){
    std::map<std::string, int>::iterator iterator = balances.find(companyName);
    if (iterator != balances.end()){
        iterator->second += amount;
        return 1;
    } else {printf("Something went wrong.\n"); return -1;};
}

/**
 * Struct to pass the arguments to a machine id.
*/
struct machineArgs{
    int amount;
    std::string companyName;
    std::map<std::string, int> &balances;
};

/**
 * Map to match each company with an index.
 * For the ease of accessing mutexes.
*/
std::map<std::string, int> companiesIndex = {
    {"Kevin", 0},
    {"Bob", 1},
    {"Stuart", 2},
    {"Otto", 3},
    {"Dave", 4}
};

/**
 * Void function for the vending machines to do the prepayment.
*/
void *doPrepayment(void *args){
    machineArgs *currentArgs = (machineArgs*)args;
    int amount = currentArgs->amount;
    std::string company = currentArgs->companyName;
    std::map<std::string, int>::iterator iterator = companiesIndex.find(company);
    int companyIndex;
    if (iterator != companiesIndex.end()){
        companyIndex = iterator->second;
    } else {printf("Something went wrong.\n");}
    pthread_mutex_lock(&machine_mutex_ids[companyIndex]);
    updateBalance(amount, company, currentArgs->balances);
    pthread_mutex_unlock(&machine_mutex_ids[companyIndex]);
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
    std::map<std::string, int> &balances;
};

/**
 * Void function for the customers to pick the vending machine.
*/
void *pickMachine(void *customerArgs){
    customer *currentCustomer = (customer*)customerArgs;
    // Sleeps for given time in milliseconds. (converts milli to micro.)
    float sleepTimeMicroseconds = (currentCustomer -> sleepTime) * 1000;
    usleep(sleepTimeMicroseconds);
    int machineId = (currentCustomer->machineId);
    int machineIndex = machineId-1;
    machineArgs mArgs = {currentCustomer->paymentAmount, currentCustomer->companyName, currentCustomer->balances};
    pthread_mutex_lock(&customer_mutex_ids[machineIndex]);
    pthread_create(&machine_thread_ids[machineIndex], &attr, doPrepayment, &mArgs);
    pthread_join(machine_thread_ids[machineIndex], NULL);
    pthread_mutex_unlock(&customer_mutex_ids[machineIndex]);
}

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
    // Create unique pthread_t for each customer.
    pthread_t customer_thread_ids[numberOfCustomers];
    // Reads rest of the file for the customers data.
    for (int i = 0; i<numberOfCustomers; i++) {
        // Reads the next line. 
        std::string customerLine;
        std::getline(input, customerLine);
        // Parses the line by commas and creates an instance of customer to keep data.
        std::stringstream lineStream(customerLine);
        std::string sleepTime, machineId, companyName, paymentAmount;
        std::getline(lineStream, sleepTime, ',');
        std::getline(lineStream, machineId, ',');
        std::getline(lineStream, companyName, ',');
        std::getline(lineStream, paymentAmount, ',');
        int customerId = i+1;
        // Create customer instance to keep data.
        customer currentCustomer = {customerId, stoi(sleepTime), stoi(machineId), companyName, stoi(paymentAmount), balances};
        int machineId_int = stoi(machineId);
        pthread_attr_init(&attr);
        // Create customer pthread.
        pthread_create(&customer_thread_ids[i], &attr, pickMachine, &currentCustomer);
    }
    // Join pthreads.
    for (int i = 0; i<numberOfCustomers; i++){
        pthread_join(customer_thread_ids[i], NULL);
    }
    
    return 0;
}