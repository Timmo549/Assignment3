#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <string>
#include <map>

using namespace std;

struct ActivityStats{
	string type;
	double speed;
	double speedMean;
	double distanceTravelled;
	
	double arrivalTime;
	double movedTime;
	double exitTime;
	bool parked;
	
	ActivityStats();
	void printStats();
};


#endif
