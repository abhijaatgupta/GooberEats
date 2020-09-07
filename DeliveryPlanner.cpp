#include "provided.h"
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan( //function to generate delivery plan
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_sm;
    string direction(double angle) const{ //function returns what direction to travel based on angle
        if (22.5 < angle && angle <= 67.5) {
            return "northeast";
        }
        else if (67.5 < angle && angle <= 112.5) {
            return "north";
        }
        else if (112.5 < angle && angle <= 157.5) {
            return "northwest";
        }
        else if (157.5 < angle && angle <= 202.5) {
            return "west";
        }
        else if (202.5 < angle && angle <= 247.5) {
            return "southwest";
        }
        else if (247.5 < angle && angle <= 292.5) {
            return "south";
        }
        else if (292.5 < angle && angle <= 337.5) {
            return "southeast";
        }
        else {
            return "east";
        }
    }

};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
    m_sm = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    //call delivery optimizer to optimize order of deliveries vector for efficiency
    DeliveryOptimizer dO(m_sm);
    vector<DeliveryRequest> newDeliveries(deliveries);
    double l = 0;
    double k = 0;
    dO.optimizeDeliveryOrder(depot, newDeliveries, l, k);
    GeoCoord start = depot;
    list<StreetSegment>::iterator p; // using iterator 
    for (int i = 0; i <= newDeliveries.size(); i++){ //for all deliveries that need to be made +1 because we need to head back to the depot at the end
        PointToPointRouter router(m_sm);
        list<StreetSegment> route;
        double dist = 0;
        DeliveryResult del;
        if (i == 0) //case for routing from depot to first delivery
            del = router.generatePointToPointRoute(start, newDeliveries[0].location, route, dist);
        else if(i == newDeliveries.size()) //routing from one delivery spot to next
            del = router.generatePointToPointRoute(newDeliveries[i-1].location, start, route, dist);
        else del = router.generatePointToPointRoute(newDeliveries[i-1].location, newDeliveries[i].location, route, dist); //routing back to depot
        //if del is badCoord or NoRoute then must stop
        if (del == BAD_COORD || del == NO_ROUTE){
            return del;
        }
        totalDistanceTravelled += dist; //adding to total distance traveled the distance traveled for this delivery
        p = route.begin();
        string streetName = route.front().name;
        double directionSegment = angleOfLine(route.front());
        dist = 0;
        int j;
        for (j = 0; j < route.size(); j++){
            if (p->name != streetName){ //case for a turn occuring
                if (dist != 0){
                    DeliveryCommand deliv;
                    list<StreetSegment>::iterator prevp = --p;
                    p++;
                    double dir = angleBetween2Lines(*p, *prevp); //checking what direction to turn in
                    deliv.initAsProceedCommand(direction(directionSegment), prevp->name, dist); //proceed command for the road right before the turn
                    commands.push_back(deliv);
                    directionSegment = angleOfLine(*p);
                    //determining command for the turn
                    if (dir > 359 && dir < 1){ //nearly straight no turning
                    }
                    else if (dir < 180){ //turning right
                        deliv.initAsTurnCommand("right", p->name);
                        commands.push_back(deliv);
                    }
                    else { //turning left
                        deliv.initAsTurnCommand("left", p->name);
                        commands.push_back(deliv);
                    }
                    
                }
                dist = 0; //reset distance road's segment will take you
                streetName = p->name; //street name is now different because of turn
            }
            dist += distanceEarthMiles(p->start, p->end); //add distance between each geoCoord (length of segment)
            p++;
        }
        DeliveryCommand delv;
        delv.initAsProceedCommand(direction(directionSegment), (--p)->name, dist); //proceed Command for when the delivery route has completed
        commands.push_back(delv);
        if (i != newDeliveries.size()){ //case for when headed to a delivery point
            delv.initAsDeliverCommand(newDeliveries[i].item); // delivery command to deliver the item
            commands.push_back(delv);
            continue;
        }
    }
    return DELIVERY_SUCCESS;
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
//int main(){
//    StreetMap sm;
//    sm.load("/Users/abhijaat/Desktop/CS32/Goober Eats/Goober Eats/mapdata.txt");
//    DeliveryPlanner dp(&sm);
//    vector<DeliveryRequest> deliveries;
//    double dist = 0;
//    vector<DeliveryCommand> commands;
//    GeoCoord depot("34.0625329", "-118.4470263");
//    GeoCoord firstDel("34.0712323", "-118.4505969");
//    GeoCoord secondDel("34.0687443", "-118.4449195");
//    DeliveryRequest deliv1("Chicken Tenders", firstDel);
//    DeliveryRequest deliv2("Salmon", secondDel);
//    deliveries.push_back(deliv1);
//    deliveries.push_back(deliv2);
//    dp.generateDeliveryPlan(depot, deliveries, commands, dist);
//    for (int i = 0; i < commands.size(); i++){
//        cout << commands[i].description() << endl;
//    }
//}
