#include <cstring>
#include <iostream>

#include "Vehicle.h"

using namespace std;


Vehicle::Vehicle(string name, bool parkFlag, char regFormat[],
				 int volWeight, int speedWeight) {
	this->name = name;
	this->parkFlag = parkFlag;
	strcpy(this->regFormat, regFormat);
	this->volWeight = volWeight;
	this->speedWeight = speedWeight;	
}

void Vehicle::print() {
	cout << name << ":" << parkFlag << ":";
	for (int i = 0; i < strlen(regFormat); i++) {
		cout << regFormat[i];
	}
	cout << ":" << volWeight << ":" << speedWeight;// << endl;
}
