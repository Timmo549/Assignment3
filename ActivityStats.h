#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <string>
#include <map>

using namespace std;

struct ActivityStats{
	bool active;
	string type;
	double speed;
	double speedMean;
	double distanceTravelled;
	
	int arrivalTime;
	int movedTime;
	int exitTime;
	bool parked;
	
	ActivityStats(string type, int currTime, double speed);
	void printStats();
};


#endif
