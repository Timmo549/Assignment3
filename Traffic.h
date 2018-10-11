#include "Vehicle.h"
#include "ActivityStats.h"

using namespace std;

void initialize();
bool initVehicles();
bool initStats();

void activityEngine(int dayCount, int spacesFree); //activity engine
int getVehiclesActive(); //get number of vehicles active
double calAvgSpeed(int time, int distance);
vector<Vehicle>::iterator shuffleVehicleType();

//events
void createArrival(string type, int arrival);
void departSideRoad(int currTime);
void departEndRoad(ActivityStats stats, int currTime);
void parked();
void moves();
 
bool isTrue(char c); //check if char is true or false
