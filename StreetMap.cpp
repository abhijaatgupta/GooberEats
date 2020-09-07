#include "provided.h"
#include <string>
#include <fstream>
#include <vector>
#include <functional>
#include "ExpandableHashMap.h"
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    //hashmap of geocoords
    ExpandableHashMap<GeoCoord, vector<StreetSegment>> Coords;
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
    //deallocate all streetSegment pointers stored as hashtable values
}

bool StreetMapImpl::load(string mapFile)
{
    ifstream infile(mapFile);
    if (!infile){ //only true if file is empty
        return false;
    }
    //to go through the file for all street segments
    while (infile){
        //for each different street
        string street = "";
        //getting street name
        getline(infile, street);
        int segs = 0;
        //getting amount of segments for the particular street
        infile >> segs;
        infile.ignore(10000, '\n');
        //adding each streetsegment on the street to its geocoord and updating hashtable of geocoords
        for (int i = 0; i < segs; i++){
            //first vector is for first geocoord second is for second geocoord
            vector<StreetSegment> streets;
            vector<StreetSegment> streets2;
            string lon = "";
            string lat = "";
            string lon2 = "";
            string lat2 = "";
            //get long and latt of geoCoords then make them
            infile >> lat;
            infile >> lon;
            infile >> lat2;
            infile >> lon2;
            infile.ignore(10000, '\n');
            GeoCoord newCoord(lat, lon);
            GeoCoord newCoord2(lat2, lon2);
            //seeing if there are already street segments asociated with geocoords
            if (Coords.find(newCoord)!= nullptr){
                streets = *Coords.find(newCoord);
            }
            if (Coords.find(newCoord2)!= nullptr){
                streets2 = *Coords.find(newCoord2);
            }
            //creating street segments with the geocoords
            StreetSegment newStreet(newCoord, newCoord2, street);
            StreetSegment newStreetrev(newCoord2, newCoord, street);
            streets.push_back(newStreet);
            streets2.push_back(newStreetrev);
            //updating or adding (whichecer is necessary) the two GeoCoords
            Coords.associate(newCoord, streets);
            Coords.associate(newCoord2, streets2);
        }
    }
    
    return true;  // in what case is this false other than empty file...
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    if (Coords.find(gc)!= nullptr){
        segs.clear(); //in case segs has random values already in it
        vector<StreetSegment> strts = *Coords.find(gc); //getting all corresponding street segments for geocoord using hashmap
        for (int i = 0; i < strts.size(); i++){
            StreetSegment tempstr = strts[i];
            segs.push_back(tempstr); //pushing each streetSegment to segs so it has the necessary values
        }
        return true;
    }
    return false; // case for find returning nullptr because the GeoCoord hasn't been stored in Coords
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}

//unsigned int hasher(const string& g)
//{
//    std::hash<string> hasher;
//    return hasher(g);
//}

