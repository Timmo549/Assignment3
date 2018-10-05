using namespace std;

void initialize();
bool initVehicles();
bool initStats();

void activityEngine(int dayCount, int spacesFree); //activity engine
int getVehiclesActive(); //get number of vehicles active

//events
void createArrival(string type, int arrival);
void departSideRoad();
void departEndRoad();
void parked();
void moves();
 
bool isTrue(char c); //check if char is true or false
