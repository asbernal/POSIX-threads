#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <unistd.h> //sleep() method

using namespace std;
#define MAXTHREADS 100
#define NOCARS 0

int nCars = 0;
int nShips = 0;
int raiseBridgeTime;
int lowerBridgeTime;
int bridgeStatus = 1;     //when value is 1, then bridge is open to cars


pthread_mutex_t bridge;    //declare mutex, must be static global variable to be accessible from all threads
pthread_cond_t carsCanGo = PTHREAD_COND_INITIALIZER;    //create condition variable for cars

struct cData{
    string carPlateOrShipName;
    int duration;

    cData(string n, int d){
        carPlateOrShipName = n;
        duration = d;
    }
};

void *car(void *arg){
    struct cData *argptr;
    argptr = (struct cData*) arg;

    //save input arguments to local variables, so that it won't be overwritten
    string car = argptr->carPlateOrShipName;
    int timeToCross = argptr->duration;

    cout << "Car " << car << " arrives at the drawbridge." << endl;

    //request mutex
    pthread_mutex_lock(&bridge);

    nCars++;

    //insert the conditional loop if cars can NOT access the bridge
    while(bridgeStatus == NOCARS){
        pthread_cond_wait(&carsCanGo, &bridge); //wait for signal from ship, then proceed
    }
    cout << "Car " << car << " goes on the drawbridge." << endl;
    sleep(timeToCross);
    cout << "Car " << car << " leaves the drawbridge." << endl;

    //release mutex
    pthread_mutex_unlock(&bridge);
    pthread_exit(NULL);
}

void *ship(void *arg){
    struct cData *argptr;
    argptr = (struct cData*) arg;

    //save input arguments to local variables, so that it won't be overwritten
    string ship = argptr->carPlateOrShipName;
    int timeToCross = argptr->duration;

    cout << "Ship " << ship << " arrives at the bridge." << endl; 
    bridgeStatus = 0;   //cars can NOT go accross the bridge
    cout << "Bridge is closed to car traffic." << endl;
    
    //request mutex
    pthread_mutex_lock(&bridge);

    nShips++;
    cout << "Bridge can be safely raised." << endl;
    sleep(raiseBridgeTime);
    cout << "Ship " << ship << " goes under the raised channel." << endl;
    sleep(timeToCross);
    cout << "Ship " << ship << " is leaving." << endl;
    sleep(lowerBridgeTime);
    cout << "Bridge can now accommodate car traffic." << endl;
    bridgeStatus = 1;   //cars can now go across the bridge

    //singal so that carsCanGo and release mutex
    pthread_cond_signal(&carsCanGo);
    pthread_mutex_unlock(&bridge);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int numThreads = 0;
    int arrivalDelay;
    string type;
    string name;
    int time;

    cin >> type >> raiseBridgeTime >> lowerBridgeTime;

    pthread_mutex_init(&bridge, NULL);
    pthread_t tid[MAXTHREADS];
    while((cin >> type >> name >> arrivalDelay >> time))
    {
        sleep(arrivalDelay);
        cData argStruct(name, time);
        if(type == "Car")
        {
            pthread_create(&tid[numThreads], NULL, car, (void*) &argStruct);
            numThreads++;
        }
        else if(type == "Ship")
        {
            pthread_create(&tid[numThreads], NULL, ship, (void *) &argStruct);
            numThreads++;
        }
        else
        {
            printf("Wrong input\n");
        }
    } //while

    for(int i = 0; i < numThreads; i++)
    {
        pthread_join(tid[i], NULL);
    }

    printf("Amount of cars that crossed the bridge: %d\n", nCars);
    printf("Amount of ships that crossed the channel: %d\n", nShips);

    //make sure to destroy the pthreads mutex and cond
    pthread_mutex_destroy(&bridge);
    pthread_cond_destroy(&carsCanGo);
    return 0;
}