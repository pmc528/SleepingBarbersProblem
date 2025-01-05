//Author: Peter Chang
//CSS 503
//5/10/2022
//The Sleeping Barbers Problem

#include <iostream> // cout
#include <sstream>  // stringstream
#include <string>   // string
#include "Shop.h"

//constructor
Shop::Shop(int nBarbers, int nChairs){
//    cout << "flag 1!" << endl;
    totalBarbers = nBarbers;
    chairsAvail = nChairs;
    nDropsOff = 0;
    max = nChairs;
    
    initialize();
}

//default constructor
Shop::Shop(){
//    cout << "flag 2" << endl;
    totalBarbers = DEFAULT_BARBERS;
    chairsAvail = DEFAULT_CHAIRS;
    nDropsOff = 0;
    max = DEFAULT_CHAIRS;
    
    initialize();
}
//destructor
Shop::~Shop() {
   pthread_cond_destroy(cond_customer_served_);
   free(cond_customer_served_);
   pthread_cond_destroy(cond_barber_paid_);
   free(cond_barber_paid_);
   pthread_cond_destroy(cond_barber_sleeping_);
   free(cond_barber_sleeping_);
   
    //take care of dangling pointers
    inService = NULL;
    serviceChair = NULL;
    moneyPaid = NULL;
    
    //delete
    delete[] inService;
    delete[] serviceChair;
    delete[] moneyPaid;
}

//initialize method for initializing and setting conditions
void Shop::initialize() {
//    cout << "flag" << endl;
    // initialize lock
    pthread_mutex_init(&mutex, NULL);

    //initialize conditions
    pthread_cond_init(&cond_customers_waiting_, NULL);
    cond_customer_served_ = new pthread_cond_t[totalBarbers];
    cond_barber_paid_ = new pthread_cond_t[totalBarbers];
    cond_barber_sleeping_ = new pthread_cond_t[totalBarbers];

    inService = new bool[totalBarbers]; //whether or not barber is in service
    serviceChair = new int[totalBarbers];
    moneyPaid = new bool[totalBarbers]; //whether or not each barber was paid
    
    for(int i = 0;i < totalBarbers; i++) {
        serviceChair[i] = 0;
        inService[i] = false; //set in service flag to false
        moneyPaid[i] = false;
        pthread_cond_init(&cond_customer_served_[i], NULL);
    }
    for(int i = 0; i < totalBarbers; i++) {
        pthread_cond_init(&cond_barber_paid_[i], NULL);
        pthread_cond_init(&cond_barber_sleeping_[i], NULL);
    }
}

//takes in an int and converts to string
string Shop::int2string(int i) {
    stringstream out;
    out << i;
    return out.str();
}

//takes in an int and a string message and prints output
void Shop::print(int person, string msg) {
    cout << ((person > 0) ? "customer[": "barber [") <<abs(person) << "]: " <<msg <<endl;
}

//freeBarbers method helps determine if any barbers are free
//loops through all the barbers to find if any are free
bool Shop::freeBarbers() {
    int i = 0; //counter
    while(i < totalBarbers) {
        if (serviceChair[i] <= 0) { //checks service chair for available barbers
            return true; //increment counter
            break;
        }
        else {
            i++;
        }
    }
    return false;
    
}

//requestBarber method helps to pick the next free barber
//returns barberid
int Shop::requestBarber() {
    int myBarber = -1;
    int i = 0;
    while(i < totalBarbers) {
        if (serviceChair[i] <= 0) { //checks service chair for available barbers
            myBarber = i; //barber index
            break; //exit loop
        }
        i++;
    }
    return myBarber;
}

//visitShop is called by the customer thread. It takes in customerid as param
//Returns barberid (if successfully matched) or -1 (meaning not served)
int Shop::visitShop(int customerId) {
   //mutex lock before entering the critical section
    pthread_mutex_lock(&mutex);
    int myBarberId; //initialize barberid
    //checks if there are available chairs and available barbers - if not, then leave the shop
    if (chairsAvail == 0 && freeBarbers() == false) {
        print(customerId, "leaves the shop because of no available waiting chairs.");
        ++nDropsOff; //increment dropoffs counter
        pthread_mutex_unlock(&mutex);
        return -1; //leave the shop
    }

    //checks if all the barbers are busy
    if (freeBarbers() == false) {
        //take a waiting chair since barber is busy
        //decrement total available chairs
        chairsAvail--;
        print(customerId, "takes a waiting chair. # waiting seats available = " + int2string(chairsAvail));
        //wait for barbers call
        pthread_cond_wait(&cond_customers_waiting_, &mutex);
        //increment chairsAvail
        chairsAvail++;
    }

    //set barberid
    myBarberId = requestBarber();
    //sit in customer chair
    serviceChair[myBarberId] = customerId;
    print(customerId, "moves to the service chair[" + int2string(myBarberId) + "] # waiting seats available = " + int2string(chairsAvail));
    //use barberid to start haircut
    inService[myBarberId] = true;
    //wake up the barber just in case if he is sleeping
    pthread_cond_broadcast(&cond_barber_sleeping_[myBarberId]);
    //leave critical section
    pthread_mutex_unlock(&mutex); //unlock mutex
    //return barberID
    return myBarberId;
}

//leaveShop method takes care of transactions at the shop. Ensures barber is paid before leaving the shop
//takes in customerid and barberid as params
void Shop::leaveShop(int customerId, int barberId) {
    //lock mutex before entering critical section
    pthread_mutex_lock(&mutex);
    print(customerId, "wait for barber[" + int2string(barberId) + "] to be done with hair-cut");
    //loop while barber is cutting hair
    while(inService[barberId]) {
        //wait
        pthread_cond_wait(&cond_customer_served_[barberId], &mutex);
    }

    //pay barber
    moneyPaid[barberId] = true;
    //signal paid barber
    pthread_cond_broadcast(&cond_barber_paid_[barberId]);
    print( customerId, " says good-bye to barber["+ int2string(barberId) +"]");
    //leave critical section
    pthread_mutex_unlock(&mutex); //unlock and release critical section
}

//helloCustomer will help determine if barber should sleep or stay awake. If there's no work to do then barber sleeps
//takes in int id as param
void Shop::helloCustomer(int barberId) {
    //lock mutex before entering critical section
    pthread_mutex_lock(&mutex);

    //checks if anyone is in the waiting chair and if the barber is done servicing then barber can sleep
    if (chairsAvail > 0 && serviceChair[barberId] == 0) {
        print(-barberId, "sleeps because there are no customers.");
        inService[barberId] = false; //set inService flag to false
        serviceChair[barberId] = 0 - barberId;
        while (!inService[barberId]) {
            //sleep
            pthread_cond_wait(&cond_barber_sleeping_[barberId], &mutex);
        }
    }

    //checks if someone is waiting and if the barber is currently not cutting hair
    if (serviceChair[barberId] == 0) {
        while (!inService[barberId]) {
            pthread_cond_wait(&cond_barber_sleeping_[barberId], &mutex); //wait for signal
        }

        print(-barberId, "starts a hair-cut service for customer[" + int2string(serviceChair[barberId]) +"].");
    }

    //unlock mutex, release critical section to other barbers
    pthread_mutex_unlock(&mutex);
}

//byeCustomer facilitates the barber moving on and signaling next customer
//takes barberid as param
void Shop::byeCustomer(int barberId) {
    //lock before entering critical section
    pthread_mutex_lock(&mutex);
    print(-barberId, "says he's done with a hair-cut service for customer[" + int2string(serviceChair[barberId]) +"]");

    //update arrays flags
    inService[barberId] = false;
    moneyPaid[barberId] = false;

    //wakes up customer
    pthread_cond_broadcast(&cond_customer_served_[barberId]);
    //wait for customer to pay before I take a new one
    while (!moneyPaid[barberId]) {
        pthread_cond_wait(&cond_barber_paid_[barberId], &mutex);
    }
    serviceChair[barberId] = 0;
    print(-barberId, "calls in another customer");
    //signal another waiting customer
    pthread_cond_signal(&cond_customers_waiting_);

    //leave critical section
    pthread_mutex_unlock(&mutex); //unlock
}
