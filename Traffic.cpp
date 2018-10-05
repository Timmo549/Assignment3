#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>

#include "traffic.h"
#include "vehicle.h"
#include "stats.h"
#include "ActivityStats.h"

//#define asciiZero '0';

using namespace std;

char *statsFile;
char *vehiclesFile;

map<string, Stats> stats;
multimap<string, ActivityStats> activityStats;
vector<Vehicle> vehicles;
int numVehicleTypes;

int Stats::numVehicleTypes; //total number of types of vehicles
int Stats::roadLength; //length of road
int Stats::speedLimit; //vehicle speed
int Stats::numParkingSpaces; //spaces available

main(int argc, char* argv[]) { 
	if (argc != 4) { //too many arguments
		cerr << "Invalid number of command-line arguments\nUsage: " 
			 << argv[0] << " <VEHICLE_FILE.TXT> <STATS_FILE.TXT> <DAYS>" << endl;		
		return 1;
	} else if (atoi(argv[3]) < 1) { //days must be 1 or greater
		cerr << "Invalid number of days, must be greater than 0" << endl;
		return 2;			
	} else { //if arguments are feasible
		vehiclesFile = argv[1]; //get the name of the vehicle file
		statsFile = argv[2]; //get the name of the stats file
		const int days = atoi(argv[3]); //number of days in simulation
		
		initialize(); //initialise program and check for problems reading files
	
		double totalVehicles; //total mean number of vehicles through entire simulation
		for (map<string, Stats>::iterator itr = stats.begin(); itr != stats.end(); itr++) { //get total mean vehicles for simulation
			totalVehicles += (*itr).second.numMean;
		}
		
		for (map<string, Stats>::iterator itr = stats.begin(); itr != stats.end(); itr++) { //set the average vehices each day for each vehicle type
			(*itr).second.avgVehiclePerDay = (*itr).second.numMean/totalVehicles;
		}		
		
		int dayCount = 1; //current day 
		int spacesFree; //spaces free for parking
		
		//start simulation
		while(dayCount <= days){ //while simulation isn't complete
			spacesFree = Stats::numParkingSpaces; //reset the spaces free at the start of every day(*****Please confirm you need to reset spaces every day*****)
			activityEngine(dayCount, spacesFree); //simulate current day
			dayCount++; //increment to the next day
		}
	}
	return 0;
}

void activityEngine(int dayCount, int spacesFree){
	int currTime = 0; //time in minutes 0-1440
	while(currTime < 1440){ //for the entire day, do this
		
		if(getVehiclesActive() >= 1){ //if there are any vehicles active

			if(currTime >= 1380){ //if it is after 11pm (2300)
				//determine probabilities and "roll the dice for which event"
				
				//only 2,3,4,5
				//void departSideRoad() incomplete
				//void departEndRoad() incomplete
				//void parked() returns false if there are no vehicles to park or stop parking
				//void moves() incomplete
			}
			else{
				//determine probabilities and "roll the dice for which event"
				//the probabilities should go up and down depending on what should happen
				//e.g. an arrival should be at a low chance if there the activity system is almost full
				
				// 1,2,3,4,5 
				//void createArrival(vehicle type, currTime)
				//void departSideRoad() incomplete
				//void departEndRoad() incomplete
				//void parked()  returns false if there are no vehicles to park or stop parking
				//void moves() incomplete
			}
			
		}
		else{
			//determine probability(event happens or not. 1 or nothing 50/50 chance)
			
			//createArrival(vehicle type, currTime);
			//dont create arrival
		}
		currTime++;
	}
}

void createArrival(string type, int arrival){ //event 1, add vehicle into system
	ActivityStats v; //create new activity
	v.type = type; //set the type of vehicle
	v.arrivalTime = arrival; //set the arrival time(start) time of the activity
	activityStats.insert(pair<string, ActivityStats>(v.type, v)); //insert into all activities
}

void departSideRoad(){ //event 2, vehicle departs from side road
	//incomplete
}

void departEndRoad(){ //event 3, vehicle departs from end of road
	//incomplete
}

void parked(){ //event 4, vehicle parks or stops parking
	string random; //vehicle chosen at random that is able to pork
	bool found = false; //used to determine if there is an activity that can be parked
	
	random_shuffle(vehicles.begin(),vehicles.end()); //shuffles the order of all vehicle types
	vector<Vehicle>::iterator itr = vehicles.begin();
	auto current = itr; //variable to store the current position in the vector
	
	while(!found){ //while there is an activity that needs to be parked
		for (vector<Vehicle>::iterator itr = current; itr != vehicles.end(); itr++) { //find the type of vehicle to be parked
			if((*itr).parkFlag){ //if the type of vehicle can be parked
			 	random = (*itr).name; //get the name of vehicle
			 	current = itr; //reset the start position of iterator
			 	break;
			}  
		}
		
		//search through activities to find a vehicle of same type AND check to see if they are parked or not
		for (multimap<string, ActivityStats>::iterator itr = activityStats.begin(); itr != activityStats.end(); itr++) { 
			if((*itr).first == random && (*itr).second.parked == true){ //if vehicle is of same type and is parked
			 	(*itr).second.parked = false;
			 	found = true;
			 	break;
			}  
			else if((*itr).first == random && (*itr).second.parked == false){ //if vehicle is of same type and is not parked
				(*itr).second.parked = true;
				found = true;
			 	break;
			}
		}
	}
}

void moves(){ //event 5, vehicle moves and may change speed
	//incomplete
}

int getVehiclesActive(){ //get the number of vehciles active in the system
	int count = 0;
	for (multimap<string, ActivityStats>::iterator itr = activityStats.begin(); itr != activityStats.end(); itr++) { //iteratre through
		 if((*itr).second.arrivalTime > 0 && (*itr).second.exitTime == 0){
		 	count++;
		 }  
	}
	return count;
}

void initialize () { //initialise program
	if (!initVehicles() || !initStats()) { //initliase the vehicles and stats and determine whether there was an error
		cerr << "Error whilst initializing" << endl;
		exit (3);
	}
	
	cout << "Vehicles File..." << endl; //print info for vehicles file
	for (vector<Vehicle>::iterator itr = vehicles.begin(); itr != vehicles.end(); itr++) {
		(*itr).print();
	}
	
	//print info for stats
	cout << endl << endl << "Stats File..." << endl;
	
	Stats::print();
	for (map<string, Stats>::iterator itr = stats.begin(); itr != stats.end(); itr++) { 
		(*itr).second.printStats();
	}	
	cout << endl;
}

bool initVehicles() { //initialise vehicles
	ifstream fin (vehiclesFile);
	if (!fin.good()) {
		cerr << "Could not open file: " << vehiclesFile << endl;
		return false;
	}

	int counter = 0;
	
	string name; //name of vehicle
	char in; //used to determine if parkflag is bool or not
	bool parkFlag; //whether it is able to park
	char regFormat[10]; //registration(number plate)
	int volWeight; //volume weight
	int speedWeight; //speed weight

	fin >> numVehicleTypes; //get number of vehicle types from file
	cout << "Number Of Vehicles Read In: " << numVehicleTypes << endl << endl;
	fin.ignore(256, '\n'); //ignore \n to use getline
	
	while (!fin.eof() && counter != numVehicleTypes) { //read vehicle info
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

bool isTrue(char c) { //test whether char is true or false
	if (c == '1') { //if 1 then true
		return true;
	} else {
		return false;
	}	
}
