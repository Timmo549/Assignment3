#include <cstring>
#include <iostream>
#include <map>

#include "ActivityStats.h"

using namespace std;

ActivityStats::ActivityStats(string type, int currTime, double speed){
	active = true;
	this->type = type;	
	this->speed = speed;
	
	speedMean = 0;
	distanceTravelled = 0;
	
	arrivalTime = currTime;
	movedTime = currTime;
	exitTime = 0;
	parked = false;
}



