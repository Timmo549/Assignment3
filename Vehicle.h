#ifndef VEHICLE_H
#define VEHICLE_H

#include <string>

using namespace std;

struct Vehicle {
	string name;
	bool parkFlag;
	char regFormat[10];
	int volWeight;
	int speedWeight;
	
	Vehicle(string name, bool parkFlag, char regFormat[]
		  , int volWeight, int speedWeight);
	void print();
};
			 
#endif
