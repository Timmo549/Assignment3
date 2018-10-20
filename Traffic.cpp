#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include <math>

#include "traffic.h"
#include "vehicle.h"
#include "stats.h"
#include "ActivityStats.h"

//#define asciiZero '0';

using namespace std;

char *statsFile;
char *vehiclesFile;

ofstream fout ("logFile.txt");

map<string, Stats> stats; // Vehcile Type Stats
vector<ActivityStats> activityStats; // Individual Vehicle Stats 
vector<Vehicle> vehicles; // Vehicle Types

vector<map<string, Stats>> dayStats;

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
/*		for (map<string, Stats>::iterator itr = stats.begin(); itr != stats.end(); itr++) { //get total mean vehicles for simulation
			totalVehicles += (*itr).second.numMean;
		} */
		
/*		for (map<string, Stats>::iterator itr = stats.begin(); itr != stats.end(); itr++) { //set the average vehices each day for each vehicle type
			(*itr).second.avgVehiclePerDay = (*itr).second.numMean/totalVehicles;
		}*/		
		int dayCount = 1; //current day 
		
		//Output to log file the initial stats
		fout << "ActivityEngine..." << endl;
		fout << Stats::numVehicleTypes << ':' << Stats::roadLength << ':' << Stats::speedLimit << ':' << Stats::numParkingSpaces << endl << endl;


		
		cout << "Activity Engine Started..." << endl << endl;
		
		//start simulation
		while(dayCount <= days){ //while simulation isn't complete

			static map<string, Stats> calcStats (stats);
			
			for (map<string, Stats>::iterator itr = calcStats.begin(); itr != calcStats.end(); itr++){
				(*itr).second.numMean = 0;
				(*itr).second.numStandardDev = 0;			
				(*itr).second.speedMean = 0;
				(*itr).second.speedStandardDev = 0;		
				(*itr).second.numVehicles = 0;
				(*itr).second.totalSpeed = 0;				
				itr++;	
			}

			activityEngine(dayCount, Stats::numParkingSpaces); //simulate current day
			
			fout << "Day " << dayCount << ": " << endl;
			
			vector<ActivityStats>::iterator itr = activityStats.begin();
			map<string, Stats>::iterator current;
						
			while (itr != activityStats.end() && !(*itr).active){
					current = calcStats.find((*itr).type);				
					(*current).second.numVehicles++;
//					cout << "AVG speed: " << (*itr).speedMean << endl;
					(*current).second.totalSpeed += (*itr).speedMean;
//					cout << "Mean speed: " << (*current).second.totalSpeed << endl;											

					itr++;
			}
			
			current = calcStats.begin();

//			static map<string, Stats> AEStats (calcStats);			
			
			while (current != calcStats.end()){
				//cout << (*current).first << endl;
				// rolling average number of vehicles
				(*current).second.numMean = ((*current).second.numVehicles / dayCount);
				
				// rolling average speed of vehicles
				(*current).second.speedMean = ((*current).second.totalSpeed / dayCount);
				
				//cout << (*current).second.speedMean << "SM TS" << (*current).second.totalSpeed << endl; 
				
				//(*current).second.printStats();	
				//cout << endl;		
				
				fout << (*current).second.type << ":" << (*current).second.numMean << ":" 
					 << (*current).second.numStandardDev << ":" << (*current).second.speedMean << ":" 
					 << (*current).second.speedStandardDev << endl;
				fout.flush();
				
				

				
				current++;				
			}

			dayStats.push_back(map<string, Stats> (calcStats));
	
			fout  << endl;
			
			activityStats.clear();
			
			cout << "DAY " << dayCount << " FINISHED" << endl;
			
			dayCount++; //increment to the next day
		}	
		

		cout << "Analysis Engine Started..." << endl << endl;		
		
		analysisEngine();

		cout << "Analysis Engine Finished..." << endl << endl;
					
	}
	
	fout << endl << endl << endl;
		
	fout.close();
	
	return 0;
}

void activityEngine(int dayCount, int spacesFree){
	int currTime = 0; //time in minutes 0-1440

	cout << "DAY " << dayCount << " STARTED" << endl;	
	cout << "..." << endl;

	while(currTime < 1440){ //for the entire day, do this
	
		if (currTime%60 == 0){
			cout << endl << "THE TIME IS: " << currTime << endl << endl;			
		}
	
		if(getVehiclesActive() == 1){ //if there are any vehicles active
			// check whether any vehicle has finished the road
			vector<ActivityStats>::iterator itr = activityStats.begin();
			while (itr != activityStats.end()){ //while more vehicles to process
				if ((*itr).active && (*itr).parked == false && ((*itr).distanceTravelled) >= (Stats::roadLength * 1000)){ //if past end of road
					departEndRoad((*itr), currTime);	//depart end road	
					cout << "- " <<(*itr).type << " DEPARTED" << endl << endl << endl;
					(*itr).active = false;
				}
				itr++; //increment counter
			}
			//moves(currTime);
			
			if(currTime >= 1380){ //if it is after 11pm (2300)
				static uniform_int_distribution<unsigned> uniform (1,3);

				//determine probabilities and "roll the dice for which event"
				//only 2,3,4,5
				int action = randomInt(uniform); // random number generated
//				cout << "A11: " << action << endl;
				switch(action){ //depending on random number
					case(1):
						departSideRoad(currTime);						
//						cout << "departed via side road" << endl;
						break;
					case(2):
						if (spacesFree <= Stats::numParkingSpaces){
							parked(spacesFree);						
						}
						//void parked() returns false if there are no vehicles to park or stop parking
						break;
					case(3):
						moves(currTime);
//						cout << "moves" << endl;
						//void moves() incomplete
						break;
				}				
			} 
			else{
				static uniform_int_distribution<unsigned> uniform (1,4);
	
				//determine probabilities and "roll the dice for which event"
				//the probabilities should go up and down depending on what should happen
				//e.g. an arrival should be at a low chance if there the activity system is almost full
				// 1,2,3,4 
				int action = randomInt(uniform); // random number generated
//					cout << "B11: " << action << endl;
				switch(action){ //depending on random number
					case(1):
						createArrival(currTime);
						break;
					case(2):
						departSideRoad(currTime); 
//							cout << "departed via side road" << endl;
						break;
					case(3):
						if (spacesFree <= Stats::numParkingSpaces){
							parked(spacesFree);
						}
						//void parked() returns false if there are no vehicles to park or stop parking
						break;
					case(4):
						moves(currTime);
//							cout << "moves" << endl;
						//void moves() incomplete
						break;												
				}
			}
		} 
		else{
			if (currTime <= 1380) {
				static uniform_int_distribution<unsigned> uniform (1,2);
				int random = randomInt(uniform); // random number generated
				
				//determine probability(event happens or not. 1 or nothing 50/50 chance)
				if (random == 1){
					createArrival(currTime);
				}
			}
		}
		currTime++;
	}
	
	
}

void createArrival(int arrivalTime){ //event 1, add vehicle into system
	string random = shuffleVehicleType().name;

	ActivityStats v(random, arrivalTime, setSpeed()); //create new activity
	cout << "+ " << v.type << " Created -- Arrival Speed: "<< v.speed << endl;
	activityStats.push_back(v); //insert into all activities
}

void departSideRoad(int currTime){ //event 2, vehicle departs from side road
	vector<ActivityStats>::iterator itr = activityStats.begin(); //shuffleVehicleType();
	bool found = false;
	
	while (!found && itr != activityStats.end()){
		if ((*itr).active && (*itr).distanceTravelled > 0 && (*itr).parked == false) {
			(*itr).exitTime = currTime;
	//cout << (*itr).distanceTravelled/1000 << " " << ((*itr).exitTime-(*itr).arrivalTime)/60 << endl;
	//cout << "AVERAGE::: " << double ((*itr).distanceTravelled/1000) / int (((*itr).exitTime-(*itr).arrivalTime)/60)	<< endl;		
			(*itr).speedMean = calAvgSpeed( int ((*itr).exitTime-(*itr).arrivalTime), double ((*itr).distanceTravelled/1000));
			(*itr).active = false;					
			cout << "- " << (*itr).type << " DEPARTED VIA SIDE ROAD" << endl << endl << endl;
			found = true;
		}
		itr++;
	}


	//incomplete possibly idk
}

void departEndRoad(ActivityStats &stats, int currTime){ //event 3, vehicle departs from end of road
	//incomplete possibly idk
	
	stats.exitTime = currTime;
//	cout << "departed end road" << endl;
	stats.speedMean = calAvgSpeed((stats.exitTime-stats.arrivalTime), Stats::roadLength);
	// add speedMean to rolling average for Vehicle Type Stats
}

void parked(int &spacesFree){ //event 4, vehicle parks or stops parking
	bool found = false; //used to determine if there is an activity that can be parked
	
	while(!found){ //while there is an activity that needs to be parked
		Vehicle randomType = shuffleVehicleType(); //vehicle chosen at random that is able to park
		
		while (1){
			if ((randomType.parkFlag)){
				break;
			}
			randomType = shuffleVehicleType();
		}
	
		//search through activities to find a vehicle of same type AND check to see if they are parked or not
		vector<ActivityStats>::iterator itr = activityStats.begin();
		
		while (itr != activityStats.end()) { 
			if( (*itr).active && (*itr).type == randomType.name && (*itr).parked == true){ //if vehicle is of same type and is parked
			 	(*itr).parked = false;
			 	spacesFree++;
			 	found = true;
			 	break;
			}  
			else if( (*itr).active && (*itr).type == randomType.name && (*itr).parked == false){ //if vehicle is of same type and is not parked
				(*itr).parked = true;
				spacesFree--;
				found = true;
			 	break;
			}
			itr++;
		}
		
		found = true;
	}
}

void moves(int currTime){ //event 5, vehicle moves and may change speed
	//incomplete
	string randomType = shuffleVehicleType().name; //vehicle chosen at random that is able to park
	
	//search through activities to find a vehicle of same type AND check to see if they are parked or not
	vector<ActivityStats>::iterator itr = activityStats.begin();	
	while (itr != activityStats.end()) { 
		if( (*itr).active && (*itr).type == randomType && (*itr).parked == false){ //if vehicle is of same type and is parked					   
			(*itr).distanceTravelled += (((*itr).speed/3.6)
									   * double (currTime-(*itr).movedTime)); 
	
//				cout << "Metres2: "<< (*itr).distanceTravelled << endl;						   
									   
			//recalculate distance travelled
			(*itr).movedTime = currTime; // set time moved to current time
//			 	cout << "Time moved:" << (*itr).movedTime << endl;
			
			
			
			//speed changed +-10%
			static uniform_int_distribution<signed> uniform (-10,10);	
			double random = double (randomInt(uniform))/100; //random number between -10 and +10
			
//			cout << "RANDOM IS: " << random << endl;
			
			(*itr).speed *= (1.0 + random);	
			
			return;
		}
		itr++;
	}
}

double setSpeed(){
	static uniform_int_distribution<unsigned> uniform (Stats::speedLimit/2, Stats::speedLimit);
	//speed changed +-15%
	int random = randomInt(uniform); //random number between -15 and +15
	double avgSpeed = random;
	return avgSpeed;
}

int getVehiclesActive(){ //get the number of vehciles active in the system
//	int count = 0;
	for (vector<ActivityStats>::iterator itr = activityStats.begin(); itr != activityStats.end(); itr++) { //iteratre through
		 //if((*itr).arrivalTime > -1 && (*itr).exitTime == 0){
		 if ((*itr).active) {
		 	//count++;
		 	return 1;
		 }  
	}
	return 0;
//	return count;
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
	cout << endl << " Vehicle Stats:" << endl;
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
		fin.ignore(256, '\n');		
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

	fin.ignore(256, '\n');
	
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
		fin.ignore(256, '\n');		
//		cout << "T-"<< vehicleType << " Added" << endl;
		
		stats.insert(pair<string, Stats>(vehicleType, Stats (vehicleType, numMean
							 , numStandardDev, speedMean, speedStandardDev)));
	
		counter++;
	}		

	fin.close();	
	return true;	
}

double calAvgSpeed(double time, double distance){ //average speed
//	cout << (distance) << " " << (time/60) << endl;
	return (((distance)/ double (time/60)));
}

Vehicle shuffleVehicleType(){
	static uniform_int_distribution<unsigned> uniform (0, Stats::numVehicleTypes-1);
	return vehicles[randomInt(uniform)];
}

int randomInt(auto uniform){ // generates random number based on range/distribution input
	static default_random_engine randEng(chrono::system_clock::now().time_since_epoch().count());
	return uniform(randEng);
}

bool isTrue(char c) { //test whether char is true or false
	if (c == '1') { //if 1 then true
		return true;
	} else {
		return false;
	}	
}

void analysisEngine(){

ofstream statsOut("BaseStats.txt");

//	vector<ActivityStats> breachedVehicles ();
	
vector<map<string, Stats>>::iterator itr = dayStats.begin();	
//vector<vector<ActivityStats>>::iterator itr2 = 

double numMean = 0;
double speedMean = 0;

while (itr != dayStats.end()){
	map<string, Stats> current = (*itr);
	map<string, Stats>::iterator iterator = current.begin();
	while (iterator != current.end()){
//		(*iterator).second.printStats();


		// rolling average number of vehicles
		numMean = ((numMean * (dayCount - 1)) + numVehicles) / dayCount;
					
		// rolling average speed of vehicles
		speedMean = ((speedMean * (dayCount - 1)) + (*current).second.totalSpeed) / dayCount;
	
		iterator++;	
	}
	itr++;
}

		fout << (*iterator).second.type << ":" << (*iterator).second.numMean << ":" 
			 << (*iterator).second.numStandardDev << ":" << (*iterator).second.speedMean << ":" 
			 << (*iterator).second.speedStandardDev << endl;
		fout.flush();
						
/*						
			vector<ActivityStats>::iterator itr = activityStats.begin();
			map<string, Stats>::iterator current;
						
			while (itr != activityStats.end() && !(*itr).active){
					current = calcStats.find((*itr).type);				
					(*current).second.numVehicles++;
					(*current).second.totalSpeed += (*itr).speedMean;										

					itr++;
			}
			
			current = calcStats.begin();			
			
			while (current != calcStats.end()){

				(*current).second.numMean = ((*current).second.numVehicles / dayCount);
				
				(*current).second.speedMean = ((*current).second.totalSpeed / dayCount);
				
				fout << (*current).second.type << ":" << (*current).second.numMean << ":" 
					 << (*current).second.numStandardDev << ":" << (*current).second.speedMean << ":" 
					 << (*current).second.speedStandardDev << endl;
				fout.flush();

				current++;				
			}						
*/						

statsOut.close();
}

double mean(double sum, int numVehicles){
    return sum/numVehicles;
}

double stdDeviation(double data[], int numVehicles){
    double Sum = 0;

    for(int i = 0; i < length; i++){
        Sum += data[i];
    }

    double mean = Sum/(numVehicles); //length = total amount

    double temp = 0;

    for(int i = 0; i < numVehicles; i++){
        temp = data[i] - mean;
        data[i] = pow(temp,2);
    }

    double sum2 = 0;

    for(int i = 0; i < numVehicles; i++){
        sum2+=data[i];
    }
    //cout << "Sample Standard Deviation: " <<(sqrt(sum2/length)) << endl;
    //cout << "Population Standard Deviation: " <<(sqrt(sum2/(length-1))) << endl
    return sqrt(sum2/numVehicles);
}
