#include "Vehicle.h"
#include "Stats.h"
#include "ActivityStats.h"


using namespace std;

//Initialization
void initialize(); //initialise program
bool initVehicles(); //initialise vehicles
bool initStats(); //initiliase stats

//ActivityEngine/ProbabilityEngine
void activityEngine(int dayCount, int spacesFree); //simulation engine
string probabilityEngine(string category, int spacesFree); //determine probability for an event to occur
//events
void createArrival(string type,int arrival); //event 1, add vehicle into system
void departSideRoad(int currTime); //event 2, vehicle departs from side road
void departEndRoad(ActivityStats &stats, int currTime, double distanceTravelled); //event 3, vehicle departs from end of road
void parked(string category, int &spacesFree); //event 4, vehicle parks or stops parking
void moves(int currTime); //event 5, vehicle moves and may change speed

//AnalysisEngine
void analysisEngine(vector<map<string, Stats>> dayStats, vector<vector<ActivityStats>> dayActivityStats, int days); //analysis activity engine data

//RandomGenerators
Vehicle shuffleVehicleType(); //get random type of vehicle
int randomInt(auto uniform); // generates random number based on range/distribution input

//Calculation Functions
double setSpeed(string type); //set the speed for a particular type of vehicle only when they arrive
double stdDeviation(vector<double> data, int days); //calculate standard deviation
double calAvgSpeed(double time, double distance); //calculate average speed

//Count/Retrieval Functions
int getVehiclesActive(); //get number of vehicles active
bool noVehiclesParkFlag(); //if there are no vehicles available to be parked, then parking isn't an option
bool isTrue(char c); //check if char is true or false






