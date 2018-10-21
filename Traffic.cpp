#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include <cmath>

#include "traffic.h"
#include "vehicle.h"
#include "stats.h"
#include "ActivityStats.h"

using namespace std;

char *statsFile;
char *vehiclesFile;

map<string, Stats> stats; // Vehcile Type Stats
vector<ActivityStats> activityStats; // Individual Vehicle Stats for simulation
vector<Vehicle> vehicles; // Vehicle Types
vector<map<string, Stats>> dayStats; //stats every day

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
	
		int dayCount = 1; //current day 
		
		ofstream fout("logFile.txt");
		//Output to log file the initial stats
		fout << "ActivityEngine..." << endl;
		cout << "Activity Engine Started..." << endl << endl;
		
		//start simulation
		while(dayCount <= days){ //while simulation isn't complete
			map<string, Stats> calcStats;
			calcStats = stats;
			
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
			while (current != calcStats.end()){
				(*current).second.numMean = ((*current).second.numVehicles); /// dayCount); // rolling average number of vehicles
				(*current).second.speedMean = ((*current).second.totalSpeed); /// dayCount); // rolling average speed of vehicles	
				
				fout << (*current).second.type << ":" << (*current).second.numMean << ":" 
					 << (*current).second.numStandardDev << ":" << (*current).second.speedMean << ":" 
					 << (*current).second.speedStandardDev << endl;
				fout.flush();
							
				current++;				
			}
			dayStats.push_back(map<string, Stats> (calcStats));
	
			fout << endl;
			
			activityStats.clear();
		
			cout << "DAY " << dayCount << " FINISHED" << endl;
			dayCount++; //increment to the next day
		}	
	    cout << "Analysis Engine Started..." << endl << endl;		
	    analysisEngine(days);
	    cout << "Analysis Engine Finished..." << endl << endl;
	    fout << endl << endl << endl;
	    fout.close();
					
	}
	
	return 0;
}

void activityEngine(int dayCount, int spacesFree){
	int currTime = 0; //time in minutes 0-1440
	
	cout << "DAY " << dayCount << " STARTED" << endl;	
	cout << "..." << endl;

	while(currTime < 1440){ //for the entire day, do this
		if (currTime%60 == 0){ //get the current time
			cout << endl << "THE TIME IS: " << currTime << endl << endl;			
		}
		if(getVehiclesActive() >= 1){ //if there are any vehicles active
			vector<ActivityStats>::iterator itr = activityStats.begin();
			while (itr != activityStats.end()){	// check whether any vehicle has finished the road
				if ((*itr).active && (*itr).parked == false && ((*itr).distanceTravelled) >= (Stats::roadLength * 1000)){ //if past end of road
					departEndRoad((*itr), currTime);	//depart end road	
					cout << "- " <<(*itr).type << " DEPARTED" << endl << endl << endl;
					(*itr).active = false;
				}
				itr++; //increment counter
			}

			if(currTime >= 1380){ //if it is after 11pm (2300)
				string action = probabilityEngine("_after11", spacesFree);
				
				if(action == "_D"){		
					departSideRoad(currTime); 
				}
				else if(action == "_P" || action == "_NP"){
					parked(action, spacesFree);
				}
				else if(action == "_M"){	
					moves(currTime);
				}			
			} 
			else{
				string action = probabilityEngine("_before11", spacesFree);
				
				if(action.substr(0,2) == "_C"){
					createArrival(action.substr(2,action.length()),currTime);
				}
				else if(action == "_D"){		
					departSideRoad(currTime); 
				}
				else if(action == "_P" || action == "_NP"){
					parked(action, spacesFree);
				}
				else if(action == "_M"){	
					moves(currTime);
				}
			}
		}
		else{
			if (currTime <= 1380) {
				string type = probabilityEngine("_onlyArrive", spacesFree);
				
				if (type != "_noEvent"){
					createArrival(type,currTime);
				}
			}
		}
		currTime++;
	}
}

string probabilityEngine(string category, int spacesFree){
	string rCreate = "", rPark = "", rSide = "", rMove = "";
	
	//Arrival, determine probability of which type of vehicle
	double vehicleTypeOdds[Stats::numVehicleTypes]; //odds of each different type of vehicle entering system
	int counter = 0; //iterate through array
	double prev = 0; //add up all previous percentages
	double closest = 100; //determines which event that will be chosen
	
	static uniform_int_distribution<signed> uniform (0,100); //number between 0 and 100 
	double randomVehicle = double (randomInt(uniform))/100; //determine random percentage given number 0-100
	
	string current = ""; //keep track of the 
	double totalMean = 0; //add up how many vehicle there should be per day
	map<string, Stats>::iterator itr = stats.begin();
	while(itr != stats.end()){ //add up how many vehicle there should be per day
		totalMean += (*itr).second.numMean;
		itr++;
	}
	
	map<string, Stats>::iterator iterator = stats.begin();
	while(iterator != stats.end()){
		vehicleTypeOdds[counter] = ((*iterator).second.numMean/totalMean) + prev;
		prev += (*iterator).second.numMean/totalMean;
		if((closest > (vehicleTypeOdds[counter] - randomVehicle)) && (vehicleTypeOdds[counter] - randomVehicle) >= 0){ //determine whether this type of vehicle is closest to random number generated
			closest = vehicleTypeOdds[counter] - randomVehicle;
			current = (*iterator).first;
		}
		counter++;
		iterator++;
	}		
	
	
	//Creation of arrival event
	if(category == "_onlyArrive"){
		double randomArrival = double (randomInt(uniform))/100; //determine random percentage
		if(randomArrival <= double(totalMean)/1440){
			return current;
		}
		else{
			return "_noEvent";
		}
	}
	
	//parks/stops parking
	vector<Vehicle>::iterator it = vehicles.begin(); // Vehicle Types
	int parkable = 0;
	while(it != vehicles.end()){ //determine how many types of vehicles are able to park
	    if((*it).parkFlag){
	    	parkable++;
		}
		it++;
	}
	double park = double(parkable)/Stats::numVehicleTypes;
    park = double(park*0.20); //20% of the time, a vehicle that is allowed to park, parks.
    
    //Determine whether to park or stop parking a vehicle
    if(spacesFree == 0){ //if there are no parking spaces free
    	rPark = "_NP";
	}
	else if(Stats::numParkingSpaces == spacesFree){ //if there are all parking spaces free
		rPark = "_P";
	}
	else if (randomVehicle >= 0.5){ //50% chance to park or not 
		rPark = "_NP";
	}
	else{
		rPark = "_P";
	}
    
    double createEvent = double(totalMean)/1440 + park; //chance to create an event

    double departSide = 0.02 + createEvent; //2% of the time a vehicle wants to depart via side road

	double random = double (randomInt(uniform))/100; //determine random percentage
	
	double min = 101; //used to determine what event should be chosen(is a percentage)
	
	if(category == "_after11"){ //if it is after 11pm, there should be no chance of a vehicle entering
    	createEvent = -1;
    }
    
    if(noVehiclesParkFlag() && rPark == "_P"){ //if there are no vehicles available to be parked, then parking isn't an option
    	park = -1;
	}
	
	string state = ""; //event that will be returned
 
    //check to see what event should happen accoding to the percentages
    if((min > createEvent-random) && createEvent >= random){
		min = createEvent-random;
		state = "_C" + current; //set state = create event and also concantenate the type of vehicle
	}
    if((min > park-random) && park >= random ){ 
        min = park-random;
        state = rPark;
	}
	if((min > departSide-random) && departSide >= random){
		min = departSide-random;
		state = "_D";
	}
	if(min == 101){
		state = "_M";
	}
	
	return state;
}

bool noVehiclesParkFlag(){ //if there are no vehicles available to be parked, then parking isn't an option
	vector<Vehicle>::iterator it = vehicles.begin(); // Vehicle Types
	
	while(it != vehicles.end()){
	    vector<ActivityStats>::iterator itr = activityStats.begin();
	    map<string, Stats>::iterator current;
						
     	while (itr != activityStats.end()){
			if((*itr).active && (*it).name == (*itr).type && (*it).parkFlag == true &&  !(*itr).parked){
				return false;
			}					
			itr++;
		}   
		it++;
	}
	return true;
}

void createArrival(string type, int arrivalTime){ //event 1, add vehicle into system
	ActivityStats v(type, arrivalTime, setSpeed(type)); //create new activity
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
}

void departEndRoad(ActivityStats &stats, int currTime){ //event 3, vehicle departs from end of road
	stats.exitTime = currTime;
	stats.speedMean = calAvgSpeed((stats.exitTime-stats.arrivalTime), Stats::roadLength); // add speedMean to rolling average for Vehicle Type Stats
}

void parked(string category, int &spacesFree){ //event 4, vehicle parks or stops parking
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
			//cout << "Type 1: " << (*itr).type << "type 2: " << randomType.name << endl;
			if((*itr).active && (*itr).type == randomType.name && !(*itr).parked && category == "_P"){ //if vehicle is of same type and is not parked
				(*itr).parked = true;
				spacesFree--;
				found = true;
			 	break;
			}
			else if((*itr).active && (*itr).type == randomType.name && (*itr).parked && category == "_NP"){ //if vehicle is of same type and is parked
				(*itr).parked = false;
			 	spacesFree++;
			 	found = true;
			 	break; 
			}
		itr++;
		}
	}
}

void moves(int currTime){ //event 5, vehicle moves and may change speed
	string randomType = shuffleVehicleType().name; //vehicle chosen at random that is able to park
	
	//search through activities to find a vehicle of same type AND check to see if they are parked or not
	vector<ActivityStats>::iterator itr = activityStats.begin();	
	while (itr != activityStats.end()) { 
		if( (*itr).active && (*itr).type == randomType && (*itr).parked == false){ //if vehicle is of same type and is parked					   
			(*itr).distanceTravelled += (((*itr).speed/3.6)* double (currTime-(*itr).movedTime)); 
			(*itr).movedTime = currTime; // set time moved to current time
			//cout << "Metres2: "<< (*itr).distanceTravelled << endl;						 
			//cout << "Time moved:" << (*itr).movedTime << endl;
		
			static uniform_int_distribution<signed> uniform (-10,10);	//speed changed +-10%
			double random = double (randomInt(uniform))/100; //random number between -10 and +10
			(*itr).speed *= (1.0 + random);	
			return;
		}
		itr++;
	}
}

double setSpeed(string type){//set the speed for a particular type of vehicle
    double speedMean;

    map<string, Stats>::iterator it = stats.begin(); // Vehicle Types
    while(it != stats.end()){
    	if((*it).first == type){
    		speedMean= (*it).second.speedMean;
    		break;
		}
    	it++;
	}
	
	static uniform_int_distribution<signed> uniform (-10,10);	//speed changed +-10%
	double random = double (randomInt(uniform))/100; //random number between -10 and +10
	speedMean *= (1.0 + random);
	return speedMean;
}

int getVehiclesActive(){ //get the number of vehciles active in the system
	int count = 0;
	for (vector<ActivityStats>::iterator itr = activityStats.begin(); itr != activityStats.end(); itr++) { //iteratre through
		if((*itr).arrivalTime > -1 && (*itr).exitTime == 0){
		    if ((*itr).active) {
		 	count++;
		   }  
	    }
	}
	return count;
}

double calAvgSpeed(double time, double distance){ //average speed
	return (((distance)/ double (time/60)));
}

Vehicle shuffleVehicleType(){ //get random type of vehicle
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

void analysisEngine(int days){

ofstream statsOut("BaseStats.txt");

//	vector<ActivityStats> breachedVehicles ();
	
vector<map<string, Stats>>::iterator itr = dayStats.begin();	
//vector<vector<ActivityStats>>::iterator itr2 = 


//fout << "AnalysisEngine..." << endl;

map<string, Stats> calcStats ((*itr));

int dayCount = 0;


map<string, vector<double>> numStats;
map<string, vector<double>> speedStats;

while (itr != dayStats.end()){
	dayCount++;
	map<string, Stats> current = (*itr);
	map<string, Stats>::iterator iterator = current.begin();
	while (iterator != current.end()){
//		(*iterator).second.printStats();

		map<string, Stats>::iterator s = calcStats.find((*iterator).first);
		if (dayCount == 1){
			numStats.insert(pair<string, vector<double>>((*s).first, vector<double>()));
			speedStats.insert(pair<string, vector<double>>((*s).first, vector<double>()));	
		}
		// rolling average number of vehicles
		(*s).second.numMean = (((*s).second.numMean * (dayCount - 1)) 
							+ (*iterator).second.numVehicles) / dayCount;
//			cout << (*iterator).first << " NM: " << (*s).second.numMean << endl;		
		// rolling average speed of vehicles
		(*s).second.speedMean = (((*s).second.speedMean * (dayCount - 1)) 
							  + (*iterator).second.totalSpeed) / dayCount;
//			cout << "SM: " << (*s).second.speedMean << endl;			
		map<string, vector<double>>::iterator v = numStats.find((*s).first); 
		(*v).second.push_back((*iterator).second.numVehicles);
		v = speedStats.find((*s).first);
		(*v).second.push_back((*iterator).second.totalSpeed);
		iterator++;	
	}
//	cout << endl;
	itr++;
}

//cout << endl;

//map<string, vector<double>>::iterator iterator2 = numStats.begin();
//while(iterator2 != numStats.end()){
//	for (int i = 0; i < days; i++){
//		cout << (*iterator2).first << " " << (*iterator2).second[i] << endl;		
//	}
//	iterator2++;
//}

map<string, Stats>::iterator iterator = calcStats.begin();
statsOut << Stats::numVehicleTypes << ' ' << Stats::roadLength << ' ' 
		 << Stats::speedLimit << ' ' << Stats::numParkingSpaces << endl;

while (iterator != calcStats.end()){
	//	(*iterator).second.printStats();
	(*iterator).second.numStandardDev = stdDeviation((*numStats.find((*iterator).first)).second, days);
	(*iterator).second.speedStandardDev = stdDeviation((*speedStats.find((*iterator).first)).second, days);	
		
	statsOut << (*iterator).second.type << ":" << (*iterator).second.numMean << ":" 
			 << (*iterator).second.numStandardDev << ":" << (*iterator).second.speedMean << ":" 
			 << (*iterator).second.speedStandardDev << endl;
	statsOut.flush();	
	iterator++;
}					

statsOut.close();
}

double mean(double sum, int numVehicles){
    return sum/numVehicles;
}

double stdDeviation(vector<double> data, int days){
    double Sum = 0;

//	vector<double>::iterator v = data.begin(); 	
    for(int i = 0; i < days; i++){
       Sum += data[i];
    }

    double mean = Sum/double(days); //length = total amount
    double temp = 0;

    for(int i = 0; i < days; i++){
        temp = data[i] - mean;
        data[i] = pow(temp,2);
    }

    double sum2 = 0;

    for(int i = 0; i < days; i++){
        sum2+=data[i];
    }
    //cout << "Sample Standard Deviation: " <<(sqrt(sum2/numVehicles-1)) << endl;
    //cout << "Population Standard Deviation: " <<(sqrt(sum2/(numVehicles))) << endl
    return sqrt(sum2/days); //return sample standard deviation
}

void initialize () { //initialise program
	if (!initStats() || !initVehicles() ) { //initliase the vehicles and stats and determine whether there was an error
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
	
    int numVehicleTypes;
	fin >> numVehicleTypes; //get number of vehicle types from file
	
	if(numVehicleTypes != Stats::numVehicleTypes){
		cout << "Error, number of vehicle types differ in stats.txt and vehicle.txt. " << endl;
		return false;
	}
	
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


