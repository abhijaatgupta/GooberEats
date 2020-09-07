#include "provided.h"
#include "ExpandableHashMap.h"
#include <list>
#include <queue>
using namespace std;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute( //function to generate segment to segment route to reach destination
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_sm;
    struct GeoInfo{ //struct that stores info about geocoord, used for priority Queue
        double distFromEnd;
        double distFromStart;
        GeoCoord correspondingCoord;
        GeoCoord pastCoord;
        string Street;
        GeoInfo(GeoCoord gc){
            correspondingCoord = gc;
            distFromEnd = 0;
            distFromStart = 0;
        }
        GeoInfo(){}
        ~GeoInfo(){}
    };
    struct onOpen{ //struct that stores info about geocoord, used for priority inOpen Hashmap
        bool hasBeenPushed;
        double fVal;
        GeoCoord pastCoord;
        string streetName;
        onOpen(GeoInfo g){
            hasBeenPushed = false;
            fVal = g.distFromEnd+g.distFromStart;
            pastCoord = g.pastCoord;
            streetName = g.Street;
        }
        onOpen(){};
    };
    struct geoComp{  //comparison technique used by priority Queue
      bool operator()(const GeoInfo& lhs, const GeoInfo& rhs) const{
        return (lhs.distFromEnd+lhs.distFromStart) > (rhs.distFromEnd+rhs.distFromStart);
      }
    };
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
    m_sm = sm;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    vector<StreetSegment> segs;
    if (m_sm->getSegmentsThatStartWith(start, segs)==false || m_sm->getSegmentsThatStartWith(end, segs)==false)
        return BAD_COORD; //case for coordinates not being present in streetMap
    route.clear(); //clearing route in case it had some segments in it earlier
    if (start == end){ //case for starting at endpoint
        totalDistanceTravelled = 0;
        return DELIVERY_SUCCESS;
    }
    priority_queue<GeoInfo, vector<GeoInfo>, geoComp> openQueue;  //priority queue will order in terms of lowest f value (total distance traveled to reach end)
    ExpandableHashMap<GeoCoord, onOpen> inOpen; //Hashmap to store if a Geocoord has been pushed from open queue
    ExpandableHashMap<GeoCoord, GeoInfo> closed; //closed list stores all past GeoCoords
    //initializing all start info
    GeoInfo startInfo(start);
    startInfo.distFromEnd = distanceEarthMiles(start, end);
    startInfo.distFromStart = 0;
    startInfo.pastCoord = start;
    startInfo.Street = segs.back().name;
    openQueue.push(startInfo); //push first coord onto open
    onOpen strt(startInfo);
    inOpen.associate(start, strt);  //adding first geocoord to inOpen
    //add to inopen
    while (!openQueue.empty()){ //until the priority que has no Coords in it
        GeoInfo q = openQueue.top(); //take geoCoord with lowest f value
        openQueue.pop();
        inOpen.find(q.correspondingCoord)->hasBeenPushed = true;
        vector<StreetSegment> segs; //vector to store all segments from current geocoord
        m_sm->getSegmentsThatStartWith(q.correspondingCoord, segs);
        for (int i = 0; i < segs.size(); i++){ //for each segment
            GeoCoord current = segs[i].end; //all possible geocoords we can visit from the current one
            //setting GeoInfo of the newest coord
            GeoInfo curInfo(current);
            curInfo.Street = segs[i].name;
            curInfo.pastCoord = segs[i].start;
            curInfo.distFromStart = q.distFromStart + distanceEarthMiles(current, q.correspondingCoord);
            if (current == end){ //case for reaching end
                //stop search because we have successfully traversed
                totalDistanceTravelled = curInfo.distFromStart;
                //include route maker by backtracking through previous coords and forming street segments
                GeoCoord* n1 = &current;
                GeoCoord* n2 = &curInfo.pastCoord;
                string strName = curInfo.Street;
                while (*n1 != *n2){
                    StreetSegment s1(*n2, *n1, strName);
                    route.push_front(s1); //push to front because we are going backwards
                    *n1 = *n2;
                    *n2 = inOpen.find(*n1)->pastCoord;
                    strName = inOpen.find(*n1)->streetName;
                }
                return DELIVERY_SUCCESS;
            }
            curInfo.distFromEnd = distanceEarthMiles(current, end);
            onOpen curO(curInfo);
            if (inOpen.find(current) != nullptr){ //case for the Coords being in inOpen
                if (inOpen.find(current)->fVal < curO.fVal) //case for fVal of coord being > than the same coord when searched earlier
                    continue; //skip to next coord since this has been searched with a lower f val before
            }
            openQueue.push(curInfo);
            inOpen.associate(current, curO);
        }
        closed.associate(q.correspondingCoord, q);
    }
    return NO_ROUTE;  //no route was found
}


//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
