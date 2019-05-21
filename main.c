#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define EMPTY       0x00
#define HAMBURGER   0x01
#define FRIES       0x02
#define SODA        0x04
#define MEAL        0x07
#define ITERATIONS  100

typedef struct{
    int item1;
    int item2;
} counter;

int table[3];
int eatCount[3];
int produced, consumed, scheduled;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bell = PTHREAD_COND_INITIALIZER;
pthread_cond_t finished = PTHREAD_COND_INITIALIZER;
pthread_cond_t served = PTHREAD_COND_INITIALIZER;
pthread_cond_t scheduleClear = PTHREAD_COND_INITIALIZER;
int bellF, finishedF, servedF;

counter restaurantCounter;
void *customer(void *arg);
void *chef(void*);
void *functionalSmell(void*);
void printStats(void);

int main() {
    bellF = 0;
    finishedF = 0;
    produced = 0;
    consumed = 0;
    int customerA = HAMBURGER;
    int customerB = FRIES;
    int customerC = SODA;

    pthread_t cA;
    pthread_t cB;
    pthread_t cC;
    pthread_t ch;
    pthread_t nose;

    pthread_create(&cA, NULL, customer, &customerA);
    pthread_create(&cB, NULL, customer, &customerB);
    pthread_create(&cC, NULL, customer, &customerC);
    pthread_create(&ch, NULL, chef, NULL);
    pthread_create(&nose, NULL, functionalSmell, NULL);

    pthread_mutex_lock(&lock);
    bellF = 1;
    finishedF = 0;
    pthread_cond_signal(&bell);
    while(finishedF == 0){
        pthread_cond_wait(&finished, &lock);
    }
    printStats();
    pthread_mutex_unlock(&lock);
    return 0;
}

void *functionalSmell(void* arg){ //force the customers back into their control block if there's food or a full table
    while(1){ //horrible I know
        if(restaurantCounter.item1 != EMPTY || restaurantCounter.item2 != EMPTY){
            servedF = 1;
            pthread_cond_signal(&served);
        }
        if(table[0] == 7 || table[1] == 7 || table[2] == 7){
            servedF = 1;
            pthread_cond_signal(&served);
        }
    }
}

void printStats(void){
    printf("Customer:\tMeals:\n");
    printf("A:\t\t\t%d\n", eatCount[0]);
    printf("B:\t\t\t%d\n", eatCount[1]);
    printf("C:\t\t\t%d\n", eatCount[2]);
    printf("Total Meals Eaten:\t\t%d\n", eatCount[0] + eatCount[1] + eatCount[2]);
    printf("Total Dishes Prepared:\t%d\n", produced);
}

void *chef(void *arg){
    int choice;
    srandom( (unsigned int) time(NULL) );
    while(produced < ITERATIONS * 2){
        pthread_mutex_lock(&lock);
        while(bellF == 0){
            pthread_cond_wait(&bell, &lock);
        }
        choice = (int) random() % 3;
        switch(choice){
            case 0:
                restaurantCounter.item1 = HAMBURGER;
                restaurantCounter.item2 = FRIES;
                break;
            case 1:
                restaurantCounter.item1 = HAMBURGER;
                restaurantCounter.item2 = SODA;
                break;
            case 2:
                restaurantCounter.item1 = SODA;
                restaurantCounter.item2 = FRIES;
                break;
            default:
                break;
        }
        bellF = 0;
        servedF = 1;
        produced += 2;
        scheduled = 0;
        pthread_cond_signal(&scheduleClear);
        pthread_cond_signal(&served);
        pthread_mutex_unlock(&lock);
    }
    sleep(1);
    finishedF = 1;
    pthread_cond_signal(&finished);
    return NULL;
}
