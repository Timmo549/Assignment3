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
void createArrival(string type,int arrival);
void departSideRoad(int currTime);
void departEndRoad(ActivityStats &stats, int currTime);
void parked(string category, int &spacesFree);
void moves(int currTime);

double setSpeed(string type); 
int randomInt(auto uniform);
bool isTrue(char c); //check if char is true or false
double stdDeviation(double data[], int numVehicles);
double mean(double sum, int numVehicles);
bool noVehiclesParkFlag();

void analysisEngine();
string probabilityEngine(string category, int spacesFree);
