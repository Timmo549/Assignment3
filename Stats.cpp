#include <iostream>

#include "Stats.h"

using namespace std;

Stats::Stats(string type, double numMean, double numStandardDev
		, double speedMean, double speedStandardDev) {
	this->type = type;
	this->numMean = numMean;
	this->numStandardDev = numStandardDev;
	this->speedMean = speedMean;
	this->speedStandardDev = speedStandardDev;			
}

void Stats::print() {
	cout << Stats::numVehicleTypes << ':' << Stats::roadLength << ':';
	cout << Stats::speedLimit << ':' << Stats::numParkingSpaces;	
}

void Stats::printStats() {
	cout << type << ':' << numMean << ':' << numStandardDev << ':';
	cout << speedMean << ':' << speedStandardDev;
}
