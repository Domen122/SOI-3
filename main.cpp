#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include "monitor.h"
#include <iostream>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>

using namespace std;

#define MAX_BUFOR 9
#define MIN_BUFOR 3
sem_t semm;
vector<int> bufor;

Semaphore empty(MAX_BUFOR), canAwrite(1), canRead(0), mutex(1);

void Wypisz()
{
    cout<<"Bufor: ";
    for(auto it= bufor.begin();it!=bufor.end();++it)
    {
        cout<<" "<<*it<<" ";
    }
    cout<<endl;
}

void* produceB(void *ptr)
{
    srand(time(NULL));

    while(true)
    {
        int random= rand()%10+1;
        int sleepTime= (rand()%4+2)*100000;
        usleep(sleepTime);

        empty.p();
        mutex.p();
            bufor.push_back(random);
            for(int i=0;i<random;++i) canAwrite.p();
            if(bufor.size()>3) canRead.v();
            cout<<"B wyprodukował";
            Wypisz();
        mutex.v();
        int sum_of_elements= 0;
        sem_getvalue(canAwrite.getValue() ,&sum_of_elements);
        if(sum_of_elements>20)canAwrite.v();

    }
}

void* produceA(void* ptr)
{
    srand(time(NULL));
    while(true)
    {
        int random= rand()%10+1;
        int sleepTime= (rand()%4+2)*100000;
        usleep(sleepTime);

        {
        empty.p();
        mutex.p();
            bufor.push_back(random);
            for(int i=0;i<random;++i) canAwrite.p();
            if(bufor.size()>3) canRead.v();
            cout<<"A wyprodukował";
            Wypisz();
        mutex.v();
        int sum_of_elements= 0;
        sem_getvalue(canAwrite.getValue() ,&sum_of_elements);
        if(sum_of_elements>20)canAwrite.v();
        }
    }
}

void* consume(void *Consumernumber)
{
    srand(time(NULL));

    while(true)
    {
        int sleepTime= (rand()%4+2)*100000;
        usleep(sleepTime);
        canRead.p();
        mutex.p();
            cout<<"Konsument "<< *(int*)Consumernumber<<"usuwa "<<bufor.front()<<endl;
            for(int i=0;i<*bufor.begin();++i) canAwrite.v();
            bufor.erase(bufor.begin());
            Wypisz();
            empty.v();
        mutex.v();
    }
}
int main()
{
    pthread_t kons1, kons2, prodA, prodB;

	unsigned int *iDKonsumenta1, *iDKonsumenta2;

	iDKonsumenta1 = new unsigned int;
	iDKonsumenta2 = new unsigned int;

	*iDKonsumenta1 = 1;
	*iDKonsumenta2 = 2;

	void* ptr = NULL;

	pthread_create(&kons1, NULL, consume, (void*) iDKonsumenta1);
   	pthread_create(&kons2, NULL, consume, (void*) iDKonsumenta2);
	pthread_create(&prodB, NULL, produceB, (void*) ptr);
	pthread_create(&prodA, NULL, produceA, (void*) ptr);

	pthread_join(kons1, NULL);
    pthread_join(prodB, NULL);
    pthread_join(kons2, NULL);
    pthread_join(prodA, NULL);

	delete iDKonsumenta1;
	delete iDKonsumenta2;

	cout << "	W buforze pozostalo " << bufor.size() << " elementow." << endl;
	return 0;
}
