#include "provided.h"
#include <vector>
#include <cmath>
#include <random>
using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder( //function to optimize delivery order
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private:
    const StreetMap* m_sm;
    double acceptProbability(double energy, double newEnergy, double temp) const{ //returns probability used for annealing method
        if (newEnergy < energy)
            return 1.0;
        return exp((energy-newEnergy)/temp);
    }
    double calcCrowDistance(const GeoCoord& depot,
                            vector<DeliveryRequest> deliveries) const{ //calculates crow distance to make all deliveries
        double dist = 0;
        GeoCoord past = depot;
        for (int i = 0; i < deliveries.size(); i++){
            dist += distanceEarthMiles(past, deliveries[i].location);
            past = deliveries[i].location;
        }
        return dist;
    }
    void swapDels(int ind1, int ind2, vector<DeliveryRequest>& deliveries) const{ //swaps to deliveries to help optimize order
        DeliveryRequest temp = deliveries[ind2];
        deliveries[ind2] = deliveries[ind1];
        deliveries[ind1] = temp;
    }
    
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
    m_sm = sm;
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    oldCrowDistance = calcCrowDistance(depot, deliveries);
    double temp = 10000;
    double coolingRate = .003;
    vector<DeliveryRequest> currentSolution = deliveries; //vector of changing possible solutions
    vector<DeliveryRequest> bestSolution = deliveries; //vector containing best solution
    int randInd1 = 0;
    int randInd2 = 0;
    while (temp > 1){ //until temp reaches 1 loop will continue to try to optimize
        randInd1 = rand() % deliveries.size();
        randInd2 = rand() % deliveries.size();
        swapDels(randInd1, randInd2, currentSolution);
        double curDist = calcCrowDistance(depot, currentSolution);
        double bestDist = calcCrowDistance(depot, bestSolution);
        if (curDist < bestDist){ //case for a more optimal solution if we swap
            bestSolution = currentSolution;
        }
        else {
            double prob = (rand()%100)/100;
            if (prob < acceptProbability(bestDist, curDist, temp)){ //case for a high chance of finding a more optimal solution if we swap
                bestSolution = currentSolution;
            }
            else swapDels(randInd1, randInd2, currentSolution); // case where swapping does no good
        }
        temp *= 1-coolingRate; //decreasing temp so it eventually reaches 1
    }
    deliveries = bestSolution;
    newCrowDistance = calcCrowDistance(depot, deliveries);
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
