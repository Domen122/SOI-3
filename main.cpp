#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include "monitor.h"
#include <numeric>

#define MAX_BUFOR 9 // Maksymalna pojemnosc bufora

using namespace std;

vector<int> bufor;

Semaphore 		empty(MAX_BUFOR), 
			canRead(0), 
			canAwrite(20), 
			mutex(1); 

void wypisz()
{
	cout << " 			Bufor zawiera:";
	for(auto it = bufor.begin(); it != bufor.end(); ++it)
	{
		cout << " " << *it << " ";
	}
	cout << endl;
}	

void* produceB(void* ptr)
{
	srand( time(NULL) );

	while(true)
	{
		unsigned int sleepTime = ((rand() % 5) + 3) * 100000;
		unsigned int random = (rand()%10+1);
		usleep(sleepTime);
	
		empty.p();
		mutex.p();
			bufor.push_back(random);
			if( bufor.size() > 3 ) canRead.v();
			cout << "B wyprodukowało "<<random;//<<"suma to "<<suma; 
			wypisz();
		mutex.v();
		int suma = accumulate(bufor.begin(),bufor.end(),0);
		if(suma<20)for(int i=0;i<random;++i)canAwrite.p();

	}
} 

void* produceA(void* ptr)
{
	srand( time(NULL) );

	while(true)
	{	
		unsigned int sleepTime = ((rand() % 5) + 3) * 100000;
		unsigned int random = (rand()%10+1);
		usleep(sleepTime);
		
		canAwrite.p();
		
		empty.p();
		mutex.p();
			bufor.push_back(random);
			if( bufor.size() > 3 ) canRead.v();
			canAwrite.v();
			cout << "A wyprodukowało "<<random;//<<"suma to "<<suma; 
			wypisz();
		mutex.v();
		
	}
}

void* consume(void* numerKonsumenta)
{
	srand( time(NULL) );
	
	while(true)
	{
		unsigned int sleepTime = ((rand() % 4) + 2) * 100000;
		usleep(sleepTime);
			
		canRead.p();
		mutex.p();
			cout << "Konsument " << *(int*)numerKonsumenta << " zdejmuje " << bufor.front() << ".";
			bufor.erase(bufor.begin());
			wypisz();				
			empty.v();
		mutex.v();
		int suma = accumulate(bufor.begin(),bufor.end(),0);
		if(suma<20)for(int i=0;i<20-suma;++i)canAwrite.v();			
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
