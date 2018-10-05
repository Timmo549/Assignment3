#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <string>
#include <map>

using namespace std;

struct ActivityStats{
	string type;
	double numMean;
	double numStandardDev;
	double speedMean;
	double speedStandardDev;
	
	double arrivalTime;
	double exitTime;
	bool parked;
	
	ActivityStats();
	void printStats();
};


#endif
