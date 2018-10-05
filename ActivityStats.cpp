#include <cstring>
#include <iostream>
#include <map>

#include "ActivityStats.h"

using namespace std;

ActivityStats::ActivityStats(){
	exitTime = 0;
	parked = false;
}

void ActivityStats::printStats() {
	cout << type << ':' << numMean << ':' << numStandardDev << ':';
	cout << speedMean << ':' << speedStandardDev;
}

