#ifndef STATS_H
#define STATS_H

#include <string>

using namespace std;

struct Stats {
	static int numVehicleTypes;
	static int roadLength;
	static int speedLimit;
	static int numParkingSpaces;
	
	string type;
	double numMean;
	double numStandardDev;
	double speedMean;
	double speedStandardDev;
	
	
	Stats(string type, double numMean, double numStandardDev
		, double speedMean, double speedStandardDev);
	static void print();
	void printStats();
};

#endif
