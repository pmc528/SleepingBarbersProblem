//Author: Peter Chang
//CSS 503
//5/10/2022
//The Sleeping Barbers Problem

#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include "Shop.h"

using namespace std;

//THREADPARAM
//Description: Object which is used to pass more than one argument to the thread function.
class ThreadParam {
public:
     ThreadParam (Shop *shop, int id, int serviceTime) :
                shop(shop), id(id), serviceTime(serviceTime) {};

     Shop *shop;     //pointer to the Shop object
     int id;
     int serviceTime;
};

void *barber(void *);
void *customer(void *);

int main(int argc, char *argv[]) {
    
    //validate the arguments
    if (argc != 5) {
      cerr << "usage: nBarbers nChairs nCustomers serviceTime" << endl;
      return -1;
    }

    //receives the line arguments
    int nBarbers = atoi(argv[1]);
    int nChairs = atoi(argv[2]);
    int nCustomers = atoi(argv[3]);
    int serviceTime = atoi(argv[4]);

    cout << "barbers: " << nBarbers << " chairs: " << nChairs << " customer: " << nCustomers << " serviceTime: " << serviceTime << endl;
    pthread_t barber_thread[nBarbers];
    pthread_t customer_threads[nCustomers];

    //instantiates a shop which is an object from Shop class
    Shop shop(nBarbers, nChairs);

    //creates the barber threads
    //passed as a pointer to the shop object
    for (int i = 0; i < nBarbers; i++) {
        ThreadParam * param = new ThreadParam (&shop, i, serviceTime);
        pthread_create(&barber_thread[i], NULL, barber, (void *)param);
    }

    //creates nCustomers at a random time
    //customer threads are passed in a pointer to the shop object
    for (int i = 0; i < nCustomers; i++) {
        usleep(rand( ) % 1000);
        int id = i + 1;
        ThreadParam * param = new ThreadParam (&shop, id, serviceTime);
        pthread_create(&customer_threads[i], NULL, customer, (void *)param);
    }

    //wait until all the customer threads are serviced and terminated
    for (int i = 0; i < nCustomers; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    //cancels/terminates barber threads
    for (int i = 0; i < nBarbers; i++) {
        pthread_cancel(barber_thread[i]);
    }

    //print num of drop offs and program is eover
    cout << "# customers who didn't receive a service = " << shop.nDropsOff << endl;
    cout << "end of program" << endl;

    return 0;
}

//barber method to create new barber thread
void *barber(void *arg) {
    // extract parameters
    ThreadParam &param = *(ThreadParam *) arg;
    Shop &shop = *(param.shop);
    int id = param.id;
    int serviceTime = param.serviceTime;
    delete &param;

    while(true) {
        shop.helloCustomer(id); //new customer
        usleep(serviceTime);
        shop.byeCustomer(id); //say bye to the customer
    }
}

//customer method to create new customer thread
void *customer(void *arg) {
    ThreadParam &param = *(ThreadParam *) arg;
    Shop &shop = *(param.shop);
    int id = param.id;
    delete &param;

    //if assigned to barber i then wait for service to finish
    int barber = shop.visitShop(id);
    if (barber != -1) { //return -1 if no barber
        shop.leaveShop(id, barber); //leave shop if no barber
    }
    pthread_exit(NULL);
}


