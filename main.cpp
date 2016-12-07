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
	cout << " Bufor contains:";
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
	unsigned int sleepTime = ((rand() % 6) + 4) * 100000;
	unsigned int random = (rand()%10+1);
	usleep(sleepTime);
	cout<<"B wants to write"<<endl;
	empty.p();
	mutex.p();
		bufor.push_back(random);
		int suma= accumulate(bufor.begin(),bufor.end(),0);
		if(suma>=20&&(suma-random)<20)canAwrite.p();
		if( bufor.size() > 3 ) canRead.v();
		cout << "B produced "<<random<<"	      ";
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
	cout<<"A wants to write"<<endl;
	canAwrite.p();
	empty.p();
	mutex.p();	
		bufor.push_back(random);
		if( bufor.size() > 3 ) canRead.v();
		int suma= accumulate(bufor.begin(),bufor.end(),0);
		if(suma<20)canAwrite.v();		
		cout << "A produced "<<random<<"	      ";
		wypisz();
		mutex.v();
	}
}

void* consume(void* numerKonsumenta)
{
	srand( time(NULL) );
	
	while(true)
	{
	unsigned int sleepTime = ((rand() % 4) + 2) * 1000000;
	usleep(sleepTime);
	cout << "Consumer " << *(int*)numerKonsumenta<<" wants to devour"<<endl;
	canRead.p();
	mutex.p();
		cout << "Consumer " << *(int*)numerKonsumenta << " devoured " << bufor.front() << ".";
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
