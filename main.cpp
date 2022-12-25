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

#define NUMBER_OF_MACHINES 10
#define NUMBER_OF_COMPANIES 5
int numberOfCustomers;

/**
 * Mutexes for vending machines to update the balanes of companies.
 * Seperate mutex for each company balance to avoid miscalculations.
*/
static pthread_mutex_t machine_mutex_ids[NUMBER_OF_COMPANIES] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER
};

/**
 * Mutexes for customer threads to write the data to the heads of queues.
 * Seperate mutex for each queue head.
*/ 
static pthread_mutex_t queue_write_mutexes[NUMBER_OF_MACHINES] = {
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

/**
 * Mutexes for vending machine threads to read the data from the heads of queues.
 * Seperate mutex for each queue head.
*/ 
static pthread_mutex_t queue_read_mutexes[NUMBER_OF_MACHINES] = {
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

/**
 * Struct to keep track of the customers data.
 * Used to pass the data to customer threads from main thread.
*/
struct customer{
    int id;
    int sleepTime; 
    int machineId;
    std::string companyName; 
    int paymentAmount;
};

/**
 * Struct to pass the id to machine threads.
*/
struct machineArgs{
    int id;
};

static std::map<std::string, int> balances; // Map to store bank account balances of companies.
static std::map<std::string, int> companiesIndex;  // Map to match each company with an index.

std::vector<customer> headsOfQueues(10);    // Vector to keep track of the heads of queues of each machine.

// Global file stream for outputs. 
std::ofstream output;

// Global counter to keep track of number of prepayments.
int prepayment_counter = 0;

// Global mutex for the vending machines to increment the counter.
pthread_mutex_t counterLock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Updates the balance of given company by the given amount.
*/
int doPrepayment(int amount, std::string companyName){
    std::map<std::string, int>::iterator iterator = balances.find(companyName);
    if (iterator != balances.end()){
        iterator->second += amount;
        return 1;
    } else {printf("Something went wrong.\n"); return -1;};
}

/**
 * Thread for vending machines.
*/
void *machineThread(void *args){
    machineArgs *currentArgs = (machineArgs*)args;
    
    // Gets machine index to access mutex from array.
    int machineIndex = currentArgs->id - 1;
    
    int previousCustomerID;
    while (true){
        // Catches if the head of queue is changed.
        int currentCustomerID = headsOfQueues[machineIndex].id;
        if(currentCustomerID != previousCustomerID){
            
            // Gets the current customers prepayment information from the head of queue.
            pthread_mutex_lock(&queue_read_mutexes[machineIndex]);
            customer currentCustomer = headsOfQueues[machineIndex];
            std::string companyName = currentCustomer.companyName; 
            std::map<std::string, int>::iterator iterator = balances.find(companyName);
            int companyIndex;
            if (iterator != companiesIndex.end()){
                companyIndex = iterator->second;
            } else {printf("Something went wrong.\n");}
            int amount = currentCustomer.paymentAmount;
            pthread_mutex_unlock(&queue_write_mutexes[machineIndex]);
            
            // Does the prepayment in behalf of customer.
            pthread_mutex_lock(&machine_mutex_ids[companyIndex]);
            doPrepayment(amount, companyName);
            pthread_mutex_unlock(&machine_mutex_ids[companyIndex]);
            
            // Increments transaction counter and logs the operation to the output file. 
            pthread_mutex_lock(&counterLock);
            prepayment_counter++;
            output << "Customer" << currentCustomer.id << "," << amount << "TL," << companyName << std::endl;
            pthread_mutex_unlock(&counterLock);

            // Sets the previous customer id to check with next one.
            previousCustomerID = currentCustomerID;
        }

        // Unlock the mutex when the prepayments are complete so all threads can exit.
        if(prepayment_counter>=numberOfCustomers){
            pthread_mutex_unlock(&queue_read_mutexes[machineIndex]);
            break;
        }
    }
    // Vending machine exits when all the prepayments are completed.
    pthread_exit(0);
}

/**
 * Thread for customers.
*/
void *customerThread(void *args){
    customer *cArgs = (customer*)args;
    
    // Sleeps for given time in milliseconds. (converts milli to micro.)
    float sleepTimeMicroseconds = (cArgs -> sleepTime) * 1000;
    usleep(sleepTimeMicroseconds);

    // Gets the necessary index data. 
    int machineIndex = (cArgs -> machineId) - 1;
    int customerIndex = (cArgs -> id) - 1;
    
    // Puts the customer data to the head of queue of selected machine.
    pthread_mutex_lock(&queue_write_mutexes[machineIndex]);
    customer currentCustomer = {cArgs->id, cArgs->sleepTime, cArgs->machineId, cArgs->companyName, cArgs->paymentAmount};
    headsOfQueues[machineIndex] = currentCustomer;
    pthread_mutex_unlock(&queue_read_mutexes[machineIndex]);
    
    // Customer leaves the environment after providing the necessary data.
    pthread_exit(0);
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
    output.open(output_filename);
    // Reads first line to get number of customers.
    std::string firstLine;
    std::getline(input, firstLine);
    numberOfCustomers = std::stoi(firstLine);
    
    // Fills the map for bank account balances of companies.
    balances.insert(std::pair<std::string, int>("Kevin", 0));
    balances.insert(std::pair<std::string, int>("Bob", 0));
    balances.insert(std::pair<std::string, int>("Stuart", 0));
    balances.insert(std::pair<std::string, int>("Otto", 0));
    balances.insert(std::pair<std::string, int>("Dave", 0));

    // Fills the map for indexing the companies to find correct mutex.
    companiesIndex.insert(std::pair<std::string, int>("Kevin", 0));
    companiesIndex.insert(std::pair<std::string, int>("Bob", 1));
    companiesIndex.insert(std::pair<std::string, int>("Stuart", 2));
    companiesIndex.insert(std::pair<std::string, int>("Otto", 3));
    companiesIndex.insert(std::pair<std::string, int>("Dave", 4));

    // Create unique pthread_t for each vending machine and customer.
    pthread_t machine_thread_ids[NUMBER_OF_MACHINES];
    pthread_t customer_thread_ids[numberOfCustomers];

    pthread_attr_t attr; // Default attributes for pthread.
    pthread_attr_init(&attr); // Initialize default attributes.
    
    // Creates machine threads.
    for (int i = 0; i<NUMBER_OF_MACHINES; i++) {
        // Locks vending machines from reading initially.
        pthread_mutex_lock(&queue_read_mutexes[i]);
        
        // Creates machine thread with id = i+1.
        machineArgs mArgs = {i+1};
        pthread_create(&machine_thread_ids[i], &attr, machineThread, &mArgs);
        usleep(10);
    }
    
    // Vector to store all customers.
    std::vector<customer> allCustomers(numberOfCustomers);
    
    // Reads rest of the file for the customers data.
    for (int i = 0; i<numberOfCustomers; i++) {
        // Reads the next line. 
        std::string customerLine;
        std::getline(input, customerLine);
        
        // Parses the line by commas.
        std::stringstream lineStream(customerLine);
        std::string sleepTime, machineId, companyName, paymentAmount;
        std::getline(lineStream, sleepTime, ',');
        std::getline(lineStream, machineId, ',');
        std::getline(lineStream, companyName, ',');
        std::getline(lineStream, paymentAmount, ',');
        int customerId = i+1;
        
        // Create customer instance to keep data.
        customer currentCustomer = {customerId, stoi(sleepTime), stoi(machineId), companyName, stoi(paymentAmount)};
        // Store the customer in vector of customers.
        allCustomers[i] = currentCustomer;
    }

    // Creates customer threads.
    for (int i = 0; i<numberOfCustomers; i++) {
        customer currentCustomer = allCustomers[i];  
        pthread_create(&customer_thread_ids[i], &attr, customerThread, &allCustomers[i]);
    }

    // Joins customer threads after they provided the necessary data.
    for (int i = 0; i<numberOfCustomers; i++){
        pthread_join(customer_thread_ids[i], NULL);
    }

    // Joins machine threads after all prepayments are completed.
    for (int i = 0; i<NUMBER_OF_MACHINES; i++){
        pthread_join(machine_thread_ids[i], NULL);
    }

    output << "All payments are completed" << std::endl;
    
    // Map to iterate over company names.
    std::vector<std::string> companyNames;
    companyNames.push_back("Kevin");
    companyNames.push_back("Bob");
    companyNames.push_back("Stuart");
    companyNames.push_back("Otto");
    companyNames.push_back("Dave");

    std::map<std::string, int>::iterator balanceIterator;
    // Logs balances of each company to the output file.
    for (int i = 0; i<NUMBER_OF_COMPANIES; i++){
        std::string companyName = companyNames[i];
        balanceIterator = balances.find(companyName);
        int balance = balanceIterator -> second;
        output << companyName << ": " << balance << "TL" << std::endl;
    }

    return 0;
}