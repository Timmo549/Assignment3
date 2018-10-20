#include "Vehicle.h"
#include "ActivityStats.h"

using namespace std;

void initialize();
bool initVehicles();
bool initStats();

void activityEngine(int dayCount, int spacesFree); //activity engine
int getVehiclesActive(); //get number of vehicles active
double calAvgSpeed(double time, double distance);
Vehicle shuffleVehicleType();

//events
void createArrival(int arrival);
void departSideRoad(int currTime);
void departEndRoad(ActivityStats &stats, int currTime);
void parked(int &spacesFree);
void moves(int currTime);

double setSpeed(); 
int randomInt(auto uniform);
bool isTrue(char c); //check if char is true or false

void analysisEngine();
