#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>

#include "traffic.h"
#include "vehicle.h"
#include "stats.h"

//#define asciiZero '0';

using namespace std;


char *statsFile;
char *vehiclesFile;

map<string, Stats> stats;
int numVehicleTypes;
vector<Vehicle> vehicles;

int Stats::numVehicleTypes;
int Stats::roadLength;
int Stats::speedLimit;
int Stats::numParkingSpaces;


main(int argc, char* argv[]) {
	if (argc != 4) {
		cerr << "Invalid number of command-line arguments\nUsage: " 
			 << argv[0] << " <VEHICLE_FILE.TXT> <STATS_FILE.TXT> <DAYS>" << endl;		
		return 1;
	} else if (atoi(argv[3]) < 1) {
		cerr << "Invalid number of days, must be greater than 0" << endl;
		return 2;			
	} else {
		vehiclesFile = argv[1];
		statsFile = argv[2];
		const int days = atoi(argv[3]);
		
		initialize();
	}
	return 0;
}

void initialize () {
	if (!initVehicles() || !initStats()) {
		cerr << "Error whilst initializing" << endl;
		exit (3);
	}
	
	cout << "Vehicles File..." << endl;
	for (vector<Vehicle>::iterator itr = vehicles.begin(); itr != vehicles.end(); itr++) {
		(*itr).print();
	}
	
	cout << endl << endl << "Stats File..." << endl;
	
	Stats::print();
	
	cout << endl;
	
	for (map<string, Stats>::iterator itr = stats.begin(); itr != stats.end(); itr++) {
		(*itr).second.printStats();
	}	
	cout << endl;
}

bool initVehicles() {
	ifstream fin (vehiclesFile);
	if (!fin.good()) {
		cerr << "Could not open file: " << vehiclesFile << endl;
		return false;
	}

	int counter = 0;
	
	string name;
	char in;
	bool parkFlag;
	char regFormat[10];
	int volWeight;
	int speedWeight;

	fin >> numVehicleTypes;
	cout << "Number Of Vehicles Read In: " << numVehicleTypes << endl << endl;
	fin.ignore(256, '\n');
	
	while (!fin.eof() && counter != numVehicleTypes) {
		getline(fin, name, ':');
		fin.get(in);	
		parkFlag = isTrue(in);
		fin.ignore(256, ':');
		fin.getline(regFormat, 256, ':');
		fin >> volWeight;
		fin.ignore(256, ':');
		fin >> speedWeight;
		fin.ignore(256, ':');
		
		vehicles.push_back(Vehicle (name, parkFlag, regFormat, volWeight, speedWeight));
		
		counter++;
	}
	
	fin.close();	
	return true;
}

bool initStats() {
	ifstream fin (statsFile);
	if (!fin.good()) {
		cerr << "Could not open file: " << statsFile << endl;
		return false;
	}

	fin >> Stats::numVehicleTypes;
	fin >> Stats::roadLength;
	fin >> Stats::speedLimit;
	fin >> Stats::numParkingSpaces;
	
	int counter = 0;
	
	string vehicleType;
	double numMean;
	double numStandardDev;
	double speedMean;
	double speedStandardDev;
	
	while (!fin.eof() && counter != Stats::numVehicleTypes) {
		getline(fin, vehicleType, ':');
		fin >> numMean;
		fin.ignore(256, ':');
		fin >> numStandardDev;
		fin.ignore( 256, ':');
		fin >> speedMean;
		fin.ignore(256, ':');
		fin >> speedStandardDev;
		fin.ignore(256, ':');
		
		stats.insert(pair<string, Stats>(vehicleType, Stats (vehicleType, numMean
							 , numStandardDev, speedMean, speedStandardDev)));
		
		counter++;
	}		

	fin.close();	
	return true;	
}

bool isTrue(char c) {
	if (c == '1') {
		return true;
	} else {
		return false;
	}	
}
