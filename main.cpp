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
			canAwrite(1), 
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
			int suma= accumulate(bufor.begin(),bufor.end(),0);
			if(suma>=20&&(suma-random)<20)canAwrite.p();
			if( bufor.size() > 3 ) canRead.v();
			cout << "B wyprodukowało "<<random;
			wypisz();
		mutex.v();
		 
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
			int suma= accumulate(bufor.begin(),bufor.end(),0);
			if(suma<20)canAwrite.v();		
			cout << "A wyprodukowało "<<random;
			wypisz();
		mutex.v();
/*		if((suma-random)<20&&suma>=20)
		{
			for(int i=1;i<(20-suma+random);++i)
			{
				canAwrite.p();
				stansem--;
				cout<<"Sem1 "<<stansem;
			}
		}
		else if((suma-random)<20){for(int i=1;i<random;++i){canAwrite.p();stansem--;cout<<"Sem2 "<<stansem;}}
		//cout<<"Sem "<<stansem; */
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
			int suma= accumulate(bufor.begin(),bufor.end(),0);
			if((suma-bufor.front())<20&&suma>=20)canAwrite.v();
			bufor.erase(bufor.begin());
			wypisz();				
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
