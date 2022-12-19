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
pthread_t m1, m2, m3, m4, m5, m6, m7, m8, m9, m10;
// Mutex Lock IDs for vending machines and customers.
pthread_mutex_t ml1, ml2, ml3, ml4, ml5 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cl1, cl2, cl3, cl4, cl5, cl6, cl7, cl8, cl9, cl10 = PTHREAD_MUTEX_INITIALIZER;
pthread_attr_t attr; // Default attributes for pthread.

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
 * Struct to pass the arguments to a machine id.
*/
struct machineArgs{
    int amount;
    std::string companyName;
    std::map<std::string, int> &balances;
};

/**
 * Void function for the vending machines to do the prepayment.
*/
void *doPrepayment(void *args){
    machineArgs *currentArgs = (machineArgs*)args;
    std::string company = currentArgs->companyName;
    pthread_mutex_t mutex;
    if (company == "Kevin"){mutex = ml1;}
    else if (company == "Bob"){mutex = ml2;}
    else if (company == "Stuart"){mutex = ml3;}
    else if (company == "Otto"){mutex = ml4;}
    else if (company == "Dave"){mutex = ml5;}
    pthread_mutex_lock(&mutex);
    updateBalance(currentArgs->amount, currentArgs->companyName, currentArgs->balances);
    pthread_mutex_unlock(&mutex);
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
 * Void function for the customers to pick the vending machine.
*/
void *pickMachine(void *customerArgs, std::map<std::string, int> &balances){
    customer *currentCustomer = (customer*)customerArgs;
    // Sleeps for given time in milliseconds. (converts milli to micro.)
    float sleepTimeMicroseconds = (currentCustomer -> sleepTime) * 1000;
    usleep(sleepTimeMicroseconds);
    int machine = currentCustomer->machineId;
    machineArgs mArgs = {currentCustomer->paymentAmount, currentCustomer->companyName, balances};
    if (machine == 1){
        pthread_mutex_lock(&cl1);
        pthread_create(&m1, &attr, doPrepayment, &mArgs);
        pthread_mutex_unlock(&cl1);
    } else if (machine == 2){
        pthread_mutex_lock(&cl2);
        pthread_create(&m2, &attr, doPrepayment, &mArgs);
        pthread_mutex_unlock(&cl2);
    } else if (machine == 3){
        pthread_mutex_lock(&cl3);
        pthread_create(&m3, &attr, doPrepayment, &mArgs);
        pthread_mutex_unlock(&cl3);
    } else if (machine == 4){
        pthread_mutex_lock(&cl4);
        pthread_create(&m4, &attr, doPrepayment, &mArgs);
        pthread_mutex_unlock(&cl4);
    } else if (machine == 5){
        pthread_mutex_lock(&cl5);
        pthread_create(&m5, &attr, doPrepayment, &mArgs);
        pthread_mutex_unlock(&cl5);
    } else if (machine == 6){
        pthread_mutex_lock(&cl6);
        pthread_create(&m6, &attr, doPrepayment, &mArgs);
        pthread_mutex_unlock(&cl6);
    } else if (machine == 7){
        pthread_mutex_lock(&cl7);
        pthread_create(&m7, &attr, doPrepayment, &mArgs);
        pthread_mutex_unlock(&cl7);
    } else if (machine == 3){
        pthread_mutex_lock(&cl8);
        pthread_create(&m8, &attr, doPrepayment, &mArgs);
        pthread_mutex_unlock(&cl8);
    } else if (machine == 9){
        pthread_mutex_lock(&cl9);
        pthread_create(&m9, &attr, doPrepayment, &mArgs);
        pthread_mutex_unlock(&cl9);
    } else if (machine == 10){
        pthread_mutex_lock(&cl10);
        pthread_create(&m10, &attr, doPrepayment, &mArgs);
        pthread_mutex_unlock(&cl10);
    } 
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
    // Reads rest of the file for the customers data.
    for (int id=1; id<numberOfCustomers+1; id++) {
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
        customer currentCustomer = {id, stoi(sleepTime), stoi(machineId), companyName, stoi(paymentAmount)};
        pthread_t customerThread;
        pthread_attr_init(&attr);
    }


    return 0;
}