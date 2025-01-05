//Author: Peter Chang
//CSS 503
//5/10/2022
//The Sleeping Barbers Problem

#ifndef _SHOP_H_
#define _SHOP_H_

#include <queue>
#include <string>
#include <pthread.h>

using namespace std;

#define DEFAULT_CHAIRS 3 //the default number of chairs waiting = 3
#define DEFAULT_BARBERS 1 //default nubmer of barbers = 1

class Shop {
public:

     //constructor that initializes shop object with nBarbers and nChairs as params
    Shop(int nBarbers, int nChairs);
    
    //default constructor
    Shop( );
    
    //Destructor
    ~Shop();

    //visitShop method will either return a barberid (if matched with a barber successfully) or -1 (if not served)
    //takes in customerid as param
     int visitShop(int customerId);

    //leaveShop method takes care of transactions at the shop. Ensures barber is paid before leaving the shop
    //takes in customerid and barberid as params
     void leaveShop(int customerId, int barberId);
    
    //helloCustomer will help determine if barber should sleep or stay awake. If there's no work to do then barber sleeps
    //takes in int id as param
     void helloCustomer(int id);
    
    //byeCustomer facilitates the barber moving on and signaling next customer
    //takes id as param
     void byeCustomer(int id);
    
    //the number of customers dropped off
     int nDropsOff;

private:
    
    //initialize method - used as a helper for constructor
     void initialize( );

    //int2string takes in an int and converst to string
     string int2string(int i);

    //print method takes in person and msg params and prints output
     void print(int person, string msg);

    //freeBarbers method helps determine if any barbers are free
     bool freeBarbers();

    //requestBarber method helps to pick the next free barber
    //returns barberid
     int requestBarber();
    
    //mutexes and condition variables to coordinate threads
    //mutex is used in conjuction with all conditional variables
    pthread_mutex_t mutex;
    pthread_cond_t cond_customers_waiting_;
    pthread_cond_t *cond_customer_served_;
    pthread_cond_t *cond_barber_paid_;
    pthread_cond_t *cond_barber_sleeping_;
    
    //the max number of threads that can wait
    int max;
    
    //indicate the current customer thread id
    int* serviceChair;
    
    //indicate current status of barber
    bool* inService;
    
    //indicate whether barber has been paid
    bool* moneyPaid;
    
    //num of total barbers
    int totalBarbers;
    
    //num of available chairs
    int chairsAvail;
};


#endif
