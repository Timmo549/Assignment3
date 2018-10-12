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

map<string, Stats> stats; // Vehcile Type Stats
multimap<string, ActivityStats> activityStats; // Individual Vehicle Stats 
vector<Vehicle> vehicles; // Vehicle Types
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
	
		if(getVehiclesActive() == 1){ //if there are any vehicles active
			// check whether any vehicle has finished the road
			multimap<string, ActivityStats>::iterator itr = activityStats.begin();
			while (itr != activityStats.end()){ //while more vehicles to process
				if ((*itr).second.distanceTravelled >= Stats::roadLength){ //if past end of road
					departEndRoad((*itr).second, currTime);	//depart end road			
				}
				itr++; //incrememnt counter
			}
			
			if(currTime >= 1380){ //if it is after 11pm (2300)
				static uniform_int_distribution<unsigned> uniform (1,3);

				//determine probabilities and "roll the dice for which event"
				//only 2,3,4,5
				int action = randomInt(uniform); // random number generated
				switch(action){ //depending on random number
					case(1):
						departSideRoad(currTime);			
						// void departSideRoad(int currTime);			
						break;
					case(2):
						parked();
						//void parked() returns false if there are no vehicles to park or stop parking
						break;
					case(3):
						//moves();
						//void moves() incomplete
						break;
				}				
			} else{
				static uniform_int_distribution<unsigned> uniform (1,4);

				//determine probabilities and "roll the dice for which event"
				//the probabilities should go up and down depending on what should happen
				//e.g. an arrival should be at a low chance if there the activity system is almost full
				// 1,2,3,4 
				int action = randomInt(uniform); // random number generated
				switch(action){ //depending on random number
					case(1):
						//createArrival(); 
						//void createArrival(vehicle type, currTime)
						break;
					case(2):
						departSideRoad(currTime); 
						// void departSideRoad(int currTime);
						break;
					case(3):
						parked();
						//void parked() returns false if there are no vehicles to park or stop parking
						break;
					case(4):
						moves(currTime);
						//void moves() incomplete
						break;												
				}
			}
		} else{
			static uniform_int_distribution<unsigned> uniform (1,2);
			int random = randomInt(uniform); // random number generated
			
			//determine probability(event happens or not. 1 or nothing 50/50 chance)
			if (random == 1){
				//createArrival();
				//createArrival(vehicle type, currTime);
			} else{
				//dont create arrival			
			}
		}
		currTime++;
	}
}




void createArrival(string type, int arrival){ //event 1, add vehicle into system
	ActivityStats v; //create new activity
	v.type = type; //set the type of vehicle
	v.arrivalTime = arrival; //set the arrival time(start) time of the activity
	v.distanceTravelled = 0;
	v.parked = false;
	v.speed = 0;
	v.movedTime = 0;
	//v.speed = insertspeedfunction()
	activityStats.insert(pair<string, ActivityStats>(v.type, v)); //insert into all activities
}

void departSideRoad(int currTime){ //event 2, vehicle departs from side road
	vector<Vehicle>::iterator current = shuffleVehicleType();
	
	multimap<string, ActivityStats>::iterator itr = activityStats.find((*current).name);
	(*itr).second.exitTime = currTime;
	(*itr).second.speedMean = calAvgSpeed((*itr).second.exitTime, Stats::roadLength);

	//incomplete possibly idk
}

void departEndRoad(ActivityStats stats, int currTime){ //event 3, vehicle departs from end of road
	//incomplete possibly idk
	
	stats.exitTime = currTime;
	stats.speedMean = calAvgSpeed(stats.exitTime, Stats::roadLength);
	// add speedMean to rolling average for Vehicle Type Stats
}

void parked(){ //event 4, vehicle parks or stops parking
	string random; //vehicle chosen at random that is able to pork
	bool found = false; //used to determine if there is an activity that can be parked
	
	vector<Vehicle>::iterator current = shuffleVehicleType();
	
	while(!found){ //while there is an activity that needs to be parked
		for (vector<Vehicle>::iterator itr = current; itr != vehicles.end(); itr++) { //find the type of vehicle to be parked
			if((*itr).parkFlag){ //if the type of vehicle can be parked
			 	random = (*itr).name; //get the type of vehicle
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

void moves(int currTime){ //event 5, vehicle moves and may change speed
	//incomplete
	static uniform_int_distribution<signed> uniform (-10,10);
	
	multimap<string, ActivityStats>::iterator itr = activityStats.begin();
	while (itr != activityStats.end()){ //while more vehicles to process
		(*itr).second.distanceTravelled += (((*itr).second.speed/3.6)
									   * (currTime-(*itr).second.movedTime)); 
		//recalculate distance travelled
		(*itr).second.movedTime = currTime; // set time moved to current time
	}
	itr++; //incrememnt counter
	
	//speed changed +-10%
	int random = randomInt(uniform); //random number between -10 and +10
	if (random != 0){
		(*itr).second.speed *= (random / 100);	
	}
}



int getVehiclesActive(){ //get the number of vehciles active in the system
//	int count = 0;
	for (multimap<string, ActivityStats>::iterator itr = activityStats.begin(); itr != activityStats.end(); itr++) { //iteratre through
		 if((*itr).second.arrivalTime > -1 && (*itr).second.exitTime == 0){
		 	//count++;
		 	return 1;
		 }  
	}
	return 0;
//	return count;
}

double calAvgSpeed(int time, int distance){ //average speed
	return distance/time;
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
	cout << endl << " Vehicle Stats:";
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

vector<Vehicle>::iterator shuffleVehicleType(){
	random_shuffle(vehicles.begin(),vehicles.end()); //shuffles the order of all vehicle types
	vector<Vehicle>::iterator current = vehicles.begin();	//variable to store the current position in the vector
	return current;
}

int randomInt(auto uniform){ // generates random number based on range/distribution input
	static default_random_engine randEng;
	return uniform(randEng);
}

bool isTrue(char c) { //test whether char is true or false
	if (c == '1') { //if 1 then true
		return true;
	} else {
		return false;
	}	
}
