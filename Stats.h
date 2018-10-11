#ifndef STATS_H
#define STATS_H

#include <string>

using namespace std;

struct Stats {
	static int numVehicleTypes; //total number of vehicle types
	static int roadLength; //road distance
	static int speedLimit; //speed limit
	static int numParkingSpaces; // parking spaces availiable
	
	string type;
	double numMean; //mean total vehicles of a type for simulation
	double numStandardDev; //sd for vehicles of a type in simulation
	double speedMean; //mean speed for vehciles " " " in simulation
	double speedStandardDev; //sd speed for vehciles " " " in simulation
	
	double avgVehiclePerDay; //average vehicles per day of this type
	
	Stats(string type, double numMean, double numStandardDev
		, double speedMean, double speedStandardDev);
	static void print();
	void printStats();
};

#endif
