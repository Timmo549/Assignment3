#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include <cmath>
#include "Traffic.h"
#include <iomanip>

using namespace std;

char *statsFile;
char *vehiclesFile;
ofstream fout("logFile.txt");

map<string, Stats> stats; // Vehcile Type Stats
vector<ActivityStats> activityStats; // Individual Vehicle Stats for simulation
vector<Vehicle> vehicles; // Vehicle Types

//int numVehicleTypes;

int Stats::numVehicleTypes; //total number of types of vehicles
int Stats::roadLength; //length of road
int Stats::speedLimit; //vehicle speed
int Stats::numParkingSpaces; //spaces available

/*-------------------------Main-------------------------*/

main(int argc, char* argv[]) { 
	if (argc != 4){ //too many arguments
		cerr << "Invalid number of command-line arguments\nUsage: " 
			 << argv[0] << " <VEHICLE_FILE.TXT> <STATS_FILE.TXT> <DAYS>" << endl;		
		return 1;
	} 
	else if (atoi(argv[3]) < 1){ //days must be 1 or greater
		cerr << "Invalid number of days, must be greater than 0" << endl;
		return 2;			
	} 
	else{ //if arguments are feasible
		vehiclesFile = argv[1]; //get the name of the vehicle file
		statsFile = argv[2]; //get the name of the stats file
		const int days = atoi(argv[3]); //number of days in simulation
		
		initialize(); //initialise program and check for problems reading files
		
		double totalVehicles; //total mean number of vehicles through entire simulation
	
		int dayCount = 1; //current day 
	
		//Output to log file the initial stats
		fout << "ActivityEngine..." << endl;
		cout << "Activity Engine Started..." << endl << endl;
		
		vector<map<string, Stats>> dayStats; //stats every day
		vector<vector<ActivityStats>> dayActivityStats; //stats every day				
		
		//start simulation
		while(dayCount <= days){ //while simulation isn't complete
			map<string, Stats> calcStats;
			calcStats = stats;
			
			for (map<string, Stats>::iterator itr = calcStats.begin(); itr != calcStats.end(); itr++){ //reset the calcStats every day
				(*itr).second.numMean = 0;
				(*itr).second.numStandardDev = 0;		
				(*itr).second.speedMean = 0;
				(*itr).second.speedStandardDev = 0;		
				(*itr).second.numVehicles = 0;
				(*itr).second.totalSpeed = 0;				
				itr++;	
			}
			
			//simulate current day
			activityEngine(dayCount, Stats::numParkingSpaces); 
			
			fout << "Day " << dayCount << ": " << endl;
			
			vector<ActivityStats>::iterator itr = activityStats.begin(); //iterator for each vehicle in system
			map<string, Stats>::iterator current; //keep track of the current vehicle
						
			while (itr != activityStats.end() && !(*itr).active){ //iterate through every vehicle that has entered and exited the simulation
				current = calcStats.find((*itr).type);	//current vehicle type			
				(*current).second.numVehicles++; //incremenet total number of vehicles
				(*current).second.totalSpeed += (*itr).speedMean; //increment total speed for that type of vehicle							
				itr++;
			}
		
			current = calcStats.begin(); //keep track of current vehicle
			while (current != calcStats.end()){
				(*current).second.numMean = ((*current).second.numVehicles); // rolling average number of vehicles
				(*current).second.speedMean = ((*current).second.totalSpeed)/(*current).second.numVehicles; // rolling average speed of vehicles	
				
				if((*current).second.numVehicles != 0){
					//write info to file
					fout << fixed << setprecision(0);
					fout << (*current).second.type << ": No. of Vehicles = " << (*current).second.numMean 
						 << ", Average Speed = " << fixed << setprecision(2) << (*current).second.speedMean << "km/h;" << endl;
					fout.flush();
				}
				else{
					(*current).second.numMean = 0;
					(*current).second.speedMean = 0;
					//write info to file
					fout << fixed << setprecision(0);
					fout << (*current).second.type << ": No. of Vehicles = " << (*current).second.numMean 
						 << ", Average Speed = " << fixed << setprecision(2) << (*current).second.speedMean << "km/h;" << endl;
					fout.flush();
				}
							
				current++;				
			}
			dayStats.push_back(map<string, Stats> (calcStats)); //insert stats for current day
			dayActivityStats.push_back(vector<ActivityStats>(activityStats)); //insert the current days stats into a map holding every days stats
			activityStats.clear(); //clear this days stats
			
			fout << endl;
			cout << "DAY " << dayCount << " FINISHED" << endl;
			dayCount++; //increment to the next day
		}	
	    cout << "Analysis Engine Started..." << endl << endl;		
	    analysisEngine(dayStats, dayActivityStats, days); //start analysis engine
	    cout << "Analysis Engine Finished..." << endl << endl;
	    fout << endl << endl << endl;
	    fout.close();	
	}
	
	return 0;
}


/*-------------------------Initialization-------------------------*/

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

bool initStats() { //initiliase stats
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
		
		stats.insert(pair<string, Stats>(vehicleType, Stats (vehicleType, numMean
							 , numStandardDev, speedMean, speedStandardDev)));
	
		counter++;
	}		

	fin.close();	
	return true;	
}


/*-------------------ActivityEngine/ProbabilityEngine-------------------*/

void activityEngine(int dayCount, int spacesFree){ //simulation engine
	
	cout << "DAY " << dayCount << " STARTED" << endl;	
	cout << "..." << endl;

	while(currTime < 1440){ //for the entire day, do this
		if (currTime%60 == 0){ //get the current time
			cout << endl << "THE TIME IS: " << currTime << endl << endl;			
		}
		if(getVehiclesActive() >= 1){ //if there are any vehicles active
			vector<ActivityStats>::iterator itr = activityStats.begin();
			while (itr != activityStats.end()){	// check whether any vehicle has finished the road
				if ((*itr).active && (*itr).parked == false && ((*itr).distanceTravelled) >= (Stats::roadLength)){ //if past end of road
					departEndRoad((*itr), currTime, (*itr).distanceTravelled);	//depart end road	
					cout << "- " <<(*itr).type << " DEPARTED" << endl << endl << endl;
					(*itr).active = false;
				}
				itr++; //increment counter
			}

			if(currTime >= 1380){ //if it is after 11pm (2300)
				string action = probabilityEngine("_after11", spacesFree); //get probability
				
				//determine event based on probability
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
			else{ //if it is before 11pm in the day
				string action = probabilityEngine("_before11", spacesFree); //get probability
				
				//determine event based on probability
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
		else{ //if there are no active vehicles within the simulation 
			if (currTime <= 1380) {
				string type = probabilityEngine("_onlyArrive", spacesFree); //get probability
				
				//determine event based on probability
				if (type != "_noEvent"){
					createArrival(type,currTime);
				}
			}
		}
		currTime++;
	}
}

string probabilityEngine(string category, int spacesFree){ //determine probability for an event to occur
	string rCreate = "", rPark = ""; //stores information in a string of their respective events
	//Arrival, determine probability of which type of vehicle
	vector<double> vehicleTypeOdds(Stats::numVehicleTypes); //odds of each different type of vehicle entering system
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
	while(iterator != stats.end()){ //iterate through all vehicles and determine statistics for each type of vehicle.
		vehicleTypeOdds[counter] = (((*iterator).second.numMean/totalMean) + prev);
		prev += (*iterator).second.numMean/totalMean;
		if((closest > (vehicleTypeOdds[counter] - randomVehicle)) && (vehicleTypeOdds[counter] - randomVehicle) >= 0){ //determine whether this type of vehicle is closest to random number generated
			closest = vehicleTypeOdds[counter] - randomVehicle; //if it is, reset the closest variable
			current = (*iterator).first; //set the vehicle that is chosen
		}
		counter++;
		iterator++;
	}		
	
	//Creation of arrival event
	if(category == "_onlyArrive"){
		double randomArrival = double (randomInt(uniform))/100; //determine random percentage
		if(randomArrival <= double(totalMean)/1440){ //if generated percentage is less than odds to create new vehicle, create vehicle
			return current;
		}
		else{ //no vehicle is created
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
	double park = double(parkable)/Stats::numVehicleTypes; //determine percentage of vehicle types that are allowed to park
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
    
    //Determine final percentages 
    double createEvent = double(totalMean)/1440 + park; //chance to create an event

    double departSide = 0.02 + createEvent; //2% of the time a vehicle wants to depart via side road
	
	if(category == "_after11"){ //if it is after 11pm, there should be no chance of a vehicle entering
    	createEvent = -1;
    }
    
    if(noVehiclesParkFlag() && rPark == "_P"){ //if there are no vehicles available to be parked, then parking isn't an option
    	park = -1;
	}
	
	double random = double (randomInt(uniform))/100; //determine random percentage
	double min = 101; //used to determine what event should be chosen(is a percentage)
	string state = ""; //event that will be returned
 
    //check to see what event should happen accoding to the percentages
    if((min > createEvent-random) && createEvent >= random){ //create vehicle
		min = createEvent-random;
		state = "_C" + current; //set state = create event and also concantenate the type of vehicle
	}
    if((min > park-random) && park >= random ){ //parking/not-parking
        min = park-random;
        state = rPark; //a vehicle can park or not park
	}
	if((min > departSide-random) && departSide >= random){ //depart from side road
		min = departSide-random;
		state = "_D";
	}
	if(min == 101){ //if none of the prior statments are triggered, move a vehicle
		state = "_M";
	}
	
	return state;
}

void createArrival(string type, int arrivalTime){ //event 1, add vehicle into system
	ActivityStats v(type, arrivalTime, setSpeed(type)); //create new activity
	cout << "+ " << v.type << " Created -- Arrival Speed: "<< v.speed << endl;
	activityStats.push_back(v); //insert into all activities
}

void departSideRoad(int currTime){ //event 2, vehicle departs from side road
	vector<ActivityStats>::iterator itr = activityStats.begin();  //start from the first vehicle
	bool found = false; //true when a vehicle gets departed from side road
	
	while (!found && itr != activityStats.end()){ //iterate through every vehicle until one can depart from side road
		if ((*itr).active && (*itr).distanceTravelled > 0 && (*itr).parked == false) {
			(*itr).exitTime = currTime; //set the exit time, so that this vehicle has finished simulation
 			(*itr).speedMean = calAvgSpeed( double((*itr).exitTime-(*itr).arrivalTime)/60, (*itr).distanceTravelled); //get the average speed
			(*itr).active = false; //vehicle has finish simulation					
			found = true; //exit
		}
		itr++;
	}
}

void departEndRoad(ActivityStats &stats, int currTime, double distanceTravelled){ //event 3, vehicle departs from end of road
	stats.exitTime = currTime; //set the exit time, so that this vehicle has finished simulation
	stats.speedMean = calAvgSpeed(double(stats.exitTime-stats.arrivalTime)/60, distanceTravelled); // add speedMean to rolling average for Vehicle Type Stats
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
			if((*itr).active && (*itr).type == randomType.name && !(*itr).parked && category == "_P"){ //if vehicle is of same type and is not parked
				(*itr).parked = true;
				spacesFree--; //decrement known parking spaces
				found = true;
			 	break;
			}
			else if((*itr).active && (*itr).type == randomType.name && (*itr).parked && category == "_NP"){ //if vehicle is of same type and is parked
				(*itr).parked = false;
			 	spacesFree++; //increment known parking spaces
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
			(*itr).distanceTravelled += (((*itr).speed)* double (currTime-(*itr).movedTime)/60); 
			(*itr).movedTime = currTime; // set time moved to current time
		
			static uniform_int_distribution<signed> uniform (-15,15);	//speed changed +-15%
			double random = double (randomInt(uniform))/100; //random number between -15 and +15
			(*itr).speed *= (1.0 + random);	//update the speed with previous speed
			break;
		}
		itr++;
	}
}


/*-------------------------AnalysisEngine-------------------------*/

void analysisEngine(vector<map<string, Stats>> dayStats, vector<vector<ActivityStats>> dayActivityStats, int days){ //analysis activity engine data

	ofstream statsOut("BaseStats.txt"); //open the basestats file
		
	fout << "AnalysisEngine..." << endl;
		
	vector<map<string, Stats>>::iterator itr = dayStats.begin(); //keep track of each days stats
	
	map<string, Stats> calcStats ((*itr)); //create a copy of calcstats for daysStats(each days stats)
	
	map<string, vector<double>> numStats; //store the volume of vehicles for stats
	map<string, vector<double>> speedStats; //store the speeds of vehicles for stats
	
	int dayCount = 0; //keep track of what day we are up to
	while (itr != dayStats.end()){ //iterate through every day
		dayCount++; //increment to the next day
		map<string, Stats> current = (*itr);
		map<string, Stats>::iterator iterator = current.begin();
		while (iterator != current.end()){
	
			map<string, Stats>::iterator s = calcStats.find((*iterator).first);
			if (dayCount == 1){
				numStats.insert(pair<string, vector<double>>((*s).first, vector<double>()));
				speedStats.insert(pair<string, vector<double>>((*s).first, vector<double>()));	
			}
			// rolling average number of vehicles
			(*s).second.numMean = (((*s).second.numMean * (dayCount - 1)) 
								+ (*iterator).second.numVehicles) / dayCount;	
			
			// rolling average speed of vehicles
			
			(*s).second.speedMean = (((*s).second.speedMean * (dayCount - 1)) 
								+ (*iterator).second.speedMean) / dayCount;		
			
			map<string, vector<double>>::iterator v = numStats.find((*s).first); 
			(*v).second.push_back((*iterator).second.numVehicles);
			
			v = speedStats.find((*s).first);
			(*v).second.push_back((*iterator).second.speedMean);
			iterator++;	
		}
		itr++;
	}
	
	map<string, Stats>::iterator iterator = calcStats.begin();
	statsOut << Stats::numVehicleTypes << ' ' << Stats::roadLength << ' ' 
			 << Stats::speedLimit << ' ' << Stats::numParkingSpaces << endl;
	
	while (iterator != calcStats.end()){
		(*iterator).second.numStandardDev = stdDeviation((*numStats.find((*iterator).first)).second, days);
		(*iterator).second.speedStandardDev = stdDeviation((*speedStats.find((*iterator).first)).second, days);	
			
		statsOut << fixed << setprecision(2);
		statsOut << (*iterator).second.type << ":" << (*iterator).second.numMean << ":" 
				 << (*iterator).second.numStandardDev << ":" << (*iterator).second.speedMean << ":" 
				 << (*iterator).second.speedStandardDev << endl;
		statsOut.flush();	
		iterator++;
	}		
	
	fout << "\tVehicles That Violated Speed Limit" << endl;
	vector<vector<ActivityStats>>::iterator it = dayActivityStats.begin();
	int count = 0;
	while (it != dayActivityStats.end()){
		fout << "Day " << (count+1) << ":" << endl;	
		vector<ActivityStats>::iterator current = (*it).begin();
		while (current != (*it).end()){
			if ( (*current).distanceTravelled >= (Stats::roadLength)  && !(*current).active) {
				double speed = calAvgSpeed(double((*current).exitTime-(*current).arrivalTime)/60, (*current).distanceTravelled);
				if (double(speed) >= Stats::speedLimit ){
					fout << (*current).type << " was caught going " << speed << "km/h in a " << Stats::speedLimit << "km/h zone" << endl;					
				}
			}
			current++;
		}
		it++;
		count++;
	}
	fout << endl;	
	
	statsOut.close();
}


/*-------------------------RandomGenerators-------------------------*/

Vehicle shuffleVehicleType(){ //get random type of vehicle
	static uniform_int_distribution<signed> uniform (0, Stats::numVehicleTypes-1); //get a random number in the range of how many type of vehicles
	return vehicles[randomInt(uniform)]; //return random number
}

int randomInt(uniform_int_distribution<signed int> uniform){ //generates random number based on range/distribution input
	static default_random_engine randEng(chrono::system_clock::now().time_since_epoch().count()); //generate seed with number
	return uniform(randEng); //return random number
}


/*----------------------Calculation Functions----------------------*/

double setSpeed(string type){//set the speed for a particular type of vehicle only when they arrive
    double speed = 0;

	// Iterate through all vehicle types to determine their mean speed
    map<string, Stats>::iterator it = stats.begin(); 
    while(it != stats.end()){
    	if((*it).first == type){
    		speed= (*it).second.speedMean; //get mean speed for their type of vehicle
    		break; //stop looking
		}
    	it++;
	}
	
	static uniform_int_distribution<signed> uniform (-15,15);	//speed changed +-15%
	double random = double (randomInt(uniform))/100; //random number between -0.15 and +0.15
	speed *= (double(random)+1); //update the current speed
	return speed; //return new speed
}

double stdDeviation(vector<double> data, int days){ //calculate standard deviation
    double sum = 0;

    for(int i = 0; i < days; i++){ //sum all of the data inside vector
       sum += data[i];
    }

    double mean = sum/double(days); //length = total amount
    double temp = 0;

    for(int i = 0; i < days; i++){ //calculate difference and square
        temp = data[i] - mean;
        data[i] = pow(temp,2);
    }

    double sum2 = 0;

    for(int i = 0; i < days; i++){ //get final sum of squares 
        sum2+=data[i];
    }
    return sqrt(sum2/days); //return sample standard deviation(square root of sum/days)
}

double calAvgSpeed(double time, double distance){ //calculate average speed
	return (double (distance)/ double (time)); 
}


/*----------------------Count/Retrieval Functions----------------------*/

int getVehiclesActive(){ //get the number of vehciles active in the system
	int count = 0;
	for (vector<ActivityStats>::iterator itr = activityStats.begin(); itr != activityStats.end(); itr++) { //iteratre through every vehcile in system
		if((*itr).arrivalTime > -1 && (*itr).exitTime == 0){
		    if ((*itr).active) {
		 		count++; //increment the amount active
		   }  
	    }
	}
	return count; //return how many vehicles are active
}

bool noVehiclesParkFlag(){ //if there are no vehicles available to be parked, then parking isn't an option
	vector<Vehicle>::iterator it = vehicles.begin(); // Vehicle Types
	
	while(it != vehicles.end()){ //iterate through every type of vehicle
	    vector<ActivityStats>::iterator itr = activityStats.begin();
	    map<string, Stats>::iterator current;
						
     	while (itr != activityStats.end()){ //check to see if they are able to be parked
			if((*itr).active && (*it).name == (*itr).type && (*it).parkFlag == true &&  !(*itr).parked){
				return false;
			}					
			itr++;
		}   
		it++;
	}
	return true;
}

bool isTrue(char c) { //test whether char is true or false
	if (c == '1') //if parkable, then true
		return true;
	 else 
		return false;	
}






