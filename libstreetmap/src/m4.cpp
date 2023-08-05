#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "m4.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <math.h>
#include <iostream>
#include <ctype.h>
#include <string>
#include <sstream>
#include <vector> 
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <functional>
#include <queue>
#include <string_view>
#include <limits>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <unordered_set>
#include <utility>
#include <algorithm>


#define NO_EDGE -1
#define MAX_SPEED 27.77

//structure used to update estimates of times to reach intersections, next intersection to explore is based off "travelTime" metric
//used in min priority queue
struct WaveElem{
    IntersectionIdx nodeID;     //current intersection ID
    StreetSegmentIdx edgeID;    //edge that got to this node
    double travelTime;          //total travel time to this node

    WaveElem(int n, int e, float time){ //constructor for struct
        nodeID = n;
        edgeID = e;
        travelTime = time;
    }

    //operator overloading the < operator in order to make a minlist priority queue for A*
    bool operator<(const WaveElem& data) const{
        return travelTime > data.travelTime;
    }

};

//class used to store best times to intersections, used to reconstruct path
class Node{
    public:
    StreetSegmentIdx reachingEdge;  //edge used to reach this node
    double bestTime;    //best time to this node
    Node(double time) : bestTime(time) {}   //constructor to use inside code
};
    

std::vector<CourierSubPath> bestPath(std::vector<IntersectionIdx> intersectionPath, const std::vector<DeliveryInf>& deliveries, const std::vector<IntersectionIdx>& depots, float totalTime);
bool checkLegal(std::vector<CourierSubPath> intersectionPath, const std::vector<DeliveryInf>& deliveries);
std::vector<IntersectionIdx> swapIntersections(std::vector<IntersectionIdx> intersectionPath, int id1, int id2);
double addTime(std::vector<CourierSubPath> path);
std::vector<CourierSubPath> depotToPickUp(const std::vector<IntersectionIdx> depots);
bool pathlegal(IntersectionIdx start, IntersectionIdx end);
bool pickup_before_dropoff(IntersectionIdx pointGoingTo, DeliveryInf deliveryNo);
void setup_inter_and_deliver(std::vector<DeliveryInf> delivery_info);
void keepTrackDeliveries (CourierSubPath pathway);
bool check_if_productive(std::vector<std::pair<DeliveryInf, std::string>> related_delivery_points, std::vector<IntersectionIdx> depots);
CourierSubPath endPointToDepot(const std::vector<IntersectionIdx> depots, IntersectionIdx endPoint);


struct PathInfo{
    std::vector<StreetSegmentIdx> path;
    double time;

    PathInfo(std::vector<StreetSegmentIdx> p, double t){//constructor
        path = p;
        time = t;
    }

    bool operator<(const PathInfo& data) const{
        return time > data.time;
    }

};

// struct ComparePair {
//     bool operator()(const std::pair<IntersectionIdx, PathInfo>& p1, const std::pair<IntersectionIdx, PathInfo>& p2) {
//         return p1.second.time > p2.second.time; // Compare based on the value of MyData
//     }
// };

// Define a hash function for the DeliveryInf struct
struct DeliveryInfHash {
    size_t operator()(const DeliveryInf& di) const {
        size_t h1 = std::hash<IntersectionIdx>()(di.pickUp);
        size_t h2 = std::hash<IntersectionIdx>()(di.dropOff);
        return h1 ^ (h2 << 1);
    }
};

// Define a hash function for the pair
struct pairHash {
    template<typename T1, typename T2>
    size_t operator()(const std::pair<T1, T2>& p) const {
        size_t h1 = DeliveryInfHash()(p.first);
        size_t h2 = std::hash<T2>()(p.second);
        return h1 ^ (h2 << 1);
    }
}; 

bool operator==(const DeliveryInf& a, const DeliveryInf& b);

bool operator==(const DeliveryInf& a, const DeliveryInf& b)
{
    return a.pickUp == b.pickUp && a.dropOff == b.dropOff;
}

bool operator==(const PathInfo& a, const PathInfo& b);

bool operator==(const PathInfo& a, const PathInfo& b)
{
    return a.path == b.path && a.time == b.time;
}

//compare operator for std::sort() function used in depotToPickup
bool compareTime(const std::pair<CourierSubPath, double>& p1, const std::pair<CourierSubPath, double>& p2);
bool compareTime(const std::pair<CourierSubPath, double>& p1, const std::pair<CourierSubPath, double>& p2){
    return p1.second < p2.second;
}

bool orderMatrix(const std::pair<int, PathInfo>& p1, const std::pair<int, PathInfo>& p2){
    return p1.second.time < p2.second.time;
}

/*
struct pairHash{
    std::size_t operator()(const std::pair<DeliveryInf, std::string>& p)const{
        return std::hash<IntersectionIdx>()(p.first.pickUp) ^ std::hash<IntersectionIdx>()(p.first.dropOff) ^ std::hash<std::string>()(p.second);
    }
};*/

std::unordered_set<std::pair<DeliveryInf, std::string>, pairHash> location_visited;
std::unordered_map<IntersectionIdx, std::vector<std::pair<DeliveryInf, std::string>>> corralation_btw_interID_and_Delivery_info;
void multiDijkstra(IntersectionIdx start, std::unordered_map<IntersectionIdx, bool> points, double turn_penalty);

//setup vector of all information regarding deliveries
std::vector <std::pair<DeliveryInf, IntersectionIdx>> deliveries_reached;
void goToDeliveryIntersection(CourierSubPath smallPath);
std::unordered_set<IntersectionIdx> setupImportantPointsSet(const std::vector<DeliveryInf>& deliveries, const std::vector<IntersectionIdx>& depots);
std::unordered_set<IntersectionIdx> setupImportantPointsSet(const std::vector<DeliveryInf>& deliveries, const std::vector<IntersectionIdx>& depots){
    std::unordered_set<IntersectionIdx> importantPoints;
    
    //adding all the pickup and dropoff points to the unordered map to check in multiDijkstra
    for(int deliveryNum = 0; deliveryNum < deliveries.size(); deliveryNum++){
        importantPoints.emplace(deliveries[deliveryNum].pickUp);
        importantPoints.emplace(deliveries[deliveryNum].dropOff);
    }

    //adding all depots to unordered map in check multiDijkstra
    for(int depotNum = 0; depotNum < depots.size(); depotNum++){
        importantPoints.emplace(depots[depotNum]);
    }

    return importantPoints;

}

//old matrix
//std::unordered_map<std::pair<IntersectionIdx, IntersectionIdx>, PathInfo, boost::hash<std::pair<IntersectionIdx, IntersectionIdx>>> matrix; //boost the pair key, switch for std::hash?

std::unordered_map<IntersectionIdx, std::vector<std::pair<IntersectionIdx, PathInfo>>> matrix;


std::unordered_map <std::pair <IntersectionIdx, IntersectionIdx>, PathInfo, boost::hash<std::pair<IntersectionIdx, IntersectionIdx>>> pathsMatrix;



void multiDijkstra(IntersectionIdx start, std::unordered_set<IntersectionIdx> points, double turn_penalty);
void multiDijkstra(IntersectionIdx start, std::unordered_set<IntersectionIdx> points, double turn_penalty){
    //priority queue used for A*
    std::priority_queue<WaveElem> wavefront;

    //initializing starting vector of node class size of total intersections in city, all values initialized to infinity
    //declaring and initializing all values to infinity this way is very quick, not same as looping through all values and manually setting infinity
    std::vector <Node> nodes(getNumIntersections(), Node(std::numeric_limits<double>::infinity()));

    //insert first intersection into priority queue
    wavefront.push(WaveElem(start, NO_EDGE, 0));

    while(wavefront.size() > 0 && points.size() > 0){
        //remove shortest travel time element in priority queue
        WaveElem wave = wavefront.top();
        wavefront.pop();

        int currentID = wave.nodeID;

        //if best time to intersection, update best time and reaching edge
        if(wave.travelTime < nodes[currentID].bestTime){
            nodes[currentID].reachingEdge = wave.edgeID;
            nodes[currentID].bestTime = wave.travelTime;
            
            //only add to matrix if we aren't at the starting intersection to save computational time
            if(wave.edgeID != NO_EDGE){
                //if at a special point, add this path to unordered map

                //auto it = points.find(currentID);
                //if(it != points.end());

                //is a special point
                if(points.count(currentID) > 0){

                    //create vector to travel to this point
                    std::vector<StreetSegmentIdx> path;
                    int traceBackID = currentID;
                    int prevEdge = nodes[currentID].reachingEdge;
                    while (prevEdge != NO_EDGE){
                        path.insert(path.begin(), prevEdge);
                        StreetSegmentInfo curStreet = getStreetSegmentInfo(prevEdge);
                        if(curStreet.to == traceBackID){
                            traceBackID = curStreet.from;
                        }
                        else{
                            traceBackID = curStreet.to;
                        }
                        prevEdge = nodes[traceBackID].reachingEdge;
                    }

                    //compute travel time to this special point
                    int travelTime = computePathTravelTime(path, turn_penalty);

                    //add starting intersection, special point, path taken, and travel time into matrix
                    
                    //old matrix add statement
                    //matrix.emplace(std::make_pair(start, *it), PathInfo(path, travelTime));

                    PathInfo tempPath = {path, travelTime};

                    #pragma omp critical
                    matrix[start].push_back(std::make_pair(currentID, tempPath));

                    #pragma omp critical
                    pathsMatrix.emplace(std::make_pair(start, currentID), PathInfo(path, travelTime));

                    //remove the special point from the unordered map
                    points.erase(currentID);
                }
            }

            //updating our estimates for time to reach next node
            for(const auto& outEdge : findStreetSegmentsOfIntersection(currentID)){
                
                StreetSegmentInfo curStreet = getStreetSegmentInfo(outEdge);
                
                //if street is not one way street that we can't travel down
                StreetSegmentIdx to_street = curStreet.to;
                if(!(curStreet.oneWay && to_street == currentID)){
                    int toNode;
                    if(to_street == currentID){
                        toNode = curStreet.from;
                    }
                    else{
                        toNode = to_street;
                    }

                    double heuristic = 0;

                    //add turn penalty to heuristic if we are changing streets
                    if(nodes[currentID].reachingEdge != -1 && getStreetSegmentInfo(nodes[currentID].reachingEdge).streetID != curStreet.streetID){
                        heuristic += turn_penalty;           
                    }

                    //add all time elements, push new element into priority queue
                    wavefront.push(WaveElem(toNode, outEdge, nodes[currentID].bestTime + findStreetSegmentTravelTime(outEdge) + heuristic));
                }
            }
        }
    }
}

/*****************************************************************Greedy Algorithm**********************************************************************/
//group all the important deliveries of one intersection
void setup_inter_and_deliver(std::vector<DeliveryInf> delivery_info){
    //loop through the DeliveryInf vector for pick up
    for (int sizeOfVector = 0; sizeOfVector < delivery_info.size(); sizeOfVector++){
        //create iterator to find delivery info in the unordered map
        auto it = corralation_btw_interID_and_Delivery_info.find(delivery_info.at(sizeOfVector).pickUp);
        //if it isn't in the unordered map...
        if ( it == corralation_btw_interID_and_Delivery_info.end()){
            //puts the information of the delivery_info into the unordered map
            corralation_btw_interID_and_Delivery_info.insert(std::make_pair(delivery_info[sizeOfVector].pickUp, std::vector<std::pair<DeliveryInf, std::string>>{{delivery_info[sizeOfVector], "p"}}));       
             //if it is in the unordered map...
        } 
        else {
            it->second.push_back(std::make_pair(delivery_info.at(sizeOfVector), "p"));
        }

        auto iter = corralation_btw_interID_and_Delivery_info.find(delivery_info.at(sizeOfVector).dropOff);
        //if it isn't in the unordered map...
        if ( iter == corralation_btw_interID_and_Delivery_info.end()){
            //puts the information of the delivery_info into the unordered map
            corralation_btw_interID_and_Delivery_info.insert(std::make_pair(delivery_info[sizeOfVector].dropOff, std::vector<std::pair<DeliveryInf, std::string>>{{delivery_info[sizeOfVector], "d"}}));
        //if it is in the unordered map...
        } else {
            iter->second.push_back(std::make_pair(delivery_info.at(sizeOfVector), "d"));
        }
    }
}

//"p" -> pickup
//"d" -> drop off
//Finds the closest intersections between a depot and a pick up point
std::vector<CourierSubPath> depotToPickUp(const std::vector<IntersectionIdx> depots){
    //float tempTime = INFINITY;
    //IntersectionIdx tempPickUp = 0;
    //IntersectionIdx tempDepot = 0;
    std::vector<std::pair<IntersectionIdx, PathInfo>> temp_prio;
    std::vector<std::pair<DeliveryInf, std::string>> related_delivery_points;
    std::vector<std::pair<IntersectionIdx, PathInfo>> hold_pop;
    std::vector<StreetSegmentIdx> hold_path;
    //loop through depo points
    std::vector<std::pair<CourierSubPath, double>> depotPaths;
    
    for (int depotNum = 0; depotNum < depots.size(); depotNum++){
        //gets the priority q of this depo
        
        temp_prio = matrix[(depots.at(depotNum))];
        related_delivery_points = corralation_btw_interID_and_Delivery_info[(temp_prio.front().first)];
        //gets all delivery points at the top intersection
        while(!temp_prio.empty()){
            //loop through those delivery points
            related_delivery_points = corralation_btw_interID_and_Delivery_info[(temp_prio.front().first)];
            for(int lengthOfRelatedPoints = 0; lengthOfRelatedPoints < related_delivery_points.size(); lengthOfRelatedPoints++){
                //checks if its a pickup
                if(related_delivery_points.at(lengthOfRelatedPoints).second == "p"){
                    //sees if its faster
                    // if(tempTime > temp_prio.top().second.time){
                        
                    //     //update fastest time
                    //     tempTime = temp_prio.top().second.time;
                    //     tempPickUp = temp_prio.top().first;
                    //     tempDepot = depots.at(depotNum);
                    //     found = true;
                    //     hold_path = temp_prio.top().second.path;
                    //     break;
                    // }
                    struct CourierSubPath depToPickup;
                    depToPickup.start_intersection = depots.at(depotNum);
                    depToPickup.end_intersection = temp_prio.front().first;
                    depToPickup.subpath = temp_prio.front().second.path;
                    depotPaths.push_back(std::make_pair(depToPickup, temp_prio.front().second.time));
                }
                //if we prev doesnt have a pickup, renove top and proceed. (will input back into matrix after)
                //hold_pop.push_back(temp_prio.top());

            }
            temp_prio.erase(temp_prio.begin()); 
            //loop through to put back the prio q to original state
            //for(int size = 0; size < hold_pop.size(); size++){
            //    temp_prio.push(hold_pop.at(size));
            //}
            //hold_pop = {};
            //found = true;
        }
    }

    //sort the vector
    std::sort(depotPaths.begin(), depotPaths.end(), compareTime);

    std::vector<CourierSubPath> depotVecToReturn;

    for(int depotNum = 0; depotNum < 700 && depotNum < depotPaths.size(); depotNum++){
        depotVecToReturn.push_back(depotPaths[depotNum].first);
    }

    return depotVecToReturn;

    // struct CourierSubPath temp_path;
    // temp_path.start_intersection = tempDepot;
    // temp_path.end_intersection = tempPickUp;
    // temp_path.subpath = hold_path;
    // return temp_path;
}

//Function to keep track of all deliveries
void keepTrackDeliveries (CourierSubPath pathway){
    //next intersection
    IntersectionIdx nextPoint = pathway.end_intersection;
    //vector to hold all deliveries at this next intersection
    std::vector<std::pair<DeliveryInf, std::string>> allDeleveriesAtPoint = corralation_btw_interID_and_Delivery_info[nextPoint];

    //loop through all deliveries at this next intersection
    for (int deliveriesAtIntersection = 0; deliveriesAtIntersection < allDeleveriesAtPoint.size(); deliveriesAtIntersection++){
        //gets iterator to see if we've already picked up or dropped off a delivery
        auto found = location_visited.find((allDeleveriesAtPoint.at(deliveriesAtIntersection)));
        //if there hasn't been a delivery...
        if (found == location_visited.end()){
            //if the delivery is a pick up...
            if(allDeleveriesAtPoint.at(deliveriesAtIntersection).second == "p"){
                //immediately picks up
                location_visited.insert(allDeleveriesAtPoint.at(deliveriesAtIntersection));
            } 
            //if the delivery is a drop off...
            else if (allDeleveriesAtPoint.at(deliveriesAtIntersection).second == "d"){
                //checks for the correlating pick up...
                auto foundD = location_visited.find(std::make_pair(allDeleveriesAtPoint.at(deliveriesAtIntersection).first, "p"));
                //if we have the correlating pick up...
                if (foundD != location_visited.end()){
                    //drop it off
                    location_visited.insert(allDeleveriesAtPoint.at(deliveriesAtIntersection));
                }
            }
        }
    }
}

CourierSubPath endPointToDepot(const std::vector<IntersectionIdx> depots, IntersectionIdx endPoint){
    std::vector<std::pair<IntersectionIdx, PathInfo>> temp_prio;
    std::vector<std::pair<DeliveryInf, std::string>> related_delivery_points;
    std::vector<std::pair<IntersectionIdx, PathInfo>> hold_pop;
    std::vector<StreetSegmentIdx> hold_path;
    CourierSubPath endpath;  
    temp_prio = matrix[endPoint];
    int init_temp_prio = temp_prio.size();
    for(int i = 0; i < init_temp_prio;i++){
        for (int j = 0; j < depots.size(); j++){
            if (temp_prio.front().first == depots[j]){
                endpath.start_intersection = endPoint;
                endpath.end_intersection = depots[j];
                endpath.subpath = temp_prio.front().second.path;
                break; 
            }
            
        }
        temp_prio.erase(temp_prio.begin());  
    }
    return endpath;
}

//Function to return the total path travelled by the salesman
std::vector<CourierSubPath> travelingCourier(const std::vector<DeliveryInf>& deliveries,const std::vector<IntersectionIdx>& depots, const float turn_penalty){
    //setting up the important points unordered map to be used in multiDijkstra function
    std::vector<DeliveryInf> remove_dup_deliveries = deliveries;
    std::vector<IntersectionIdx> intersectionPath = {};
    // Remove duplicates
    std::sort(remove_dup_deliveries.begin(), remove_dup_deliveries.end(), [](const DeliveryInf& a, const DeliveryInf& b){
        return std::tie(a.pickUp, a.dropOff) < std::tie(b.pickUp, b.dropOff);
    });

    remove_dup_deliveries.erase(std::unique(remove_dup_deliveries.begin(), remove_dup_deliveries.end()), remove_dup_deliveries.end());


    std::unordered_set<IntersectionIdx> importantPoints;
    std::vector <CourierSubPath> totalPath;
    std::vector <CourierSubPath> finalPath;
    importantPoints = setupImportantPointsSet(deliveries, depots);
    
    IntersectionIdx next_point;
    std::vector<std::pair<IntersectionIdx, PathInfo>> temp_prio;
    std::vector<std::pair<DeliveryInf, std::string>> related_delivery_points;

    std::vector<std::pair<IntersectionIdx, PathInfo>> hold_pop;

    CourierSubPath minipath;

    bool found = false; 
    //calling multiDijkstra function to setup matrix
    //step 1: setup dijsters 
    #pragma omp parallel for
    for(int deliveryNum = 0; deliveryNum < deliveries.size(); deliveryNum++){
        multiDijkstra(deliveries[deliveryNum].pickUp, importantPoints, turn_penalty);
        multiDijkstra(deliveries[deliveryNum].dropOff, importantPoints, turn_penalty);
    }
    #pragma omp parallel for
    for(int depotNum = 0; depotNum < depots.size(); depotNum++){
        multiDijkstra(depots[depotNum], importantPoints, turn_penalty);
    }

    for(const auto& it : importantPoints){
        std::sort(matrix[it].begin(), matrix[it].end(), orderMatrix);
    }


    //step 2: setup unordermap for relationship btw deliveryinfo and intersectionidx 
    setup_inter_and_deliver(deliveries);
    //step 3: Adds that first pick up to the list of deliveries that has been picked up or dropped off.
    // totalPath.push_back(depotToPickUp(depots));

    std::vector<CourierSubPath> depotsToStart = depotToPickUp(depots);
    std::vector<std::vector<CourierSubPath>> mulit_start_path;
    mulit_start_path.resize(depotsToStart.size());
    
    std::vector<int> time; 
    time.resize(depotsToStart.size());

    for(int i = 0; i < depotsToStart.size();i++){
        location_visited.clear();
        totalPath = {};
        totalPath.push_back(depotsToStart[i]);
        //totalPath.push_back(depotsToStart[0]);
        //intersectionPath.push_back(totalPath[0].start_intersection);
        //intersectionPath.push_back(totalPath[0].end_intersection);
        keepTrackDeliveries(totalPath.front());
        time[i] = time[i] + (pathsMatrix.find(std::make_pair(totalPath.front().start_intersection, totalPath.front().end_intersection)))->second.time; 
        //step 4: Loop through the other delivery points and see if it is a valid delivery, based on the closest distance based off of time
        next_point = totalPath.front().end_intersection;
        //while we haven't visited all of the delivery pmakoints
        while(location_visited.size() != remove_dup_deliveries.size()*2){
            //looks through the priority queue
            temp_prio = matrix[next_point];
            //loops through the priority queue until we find a point that makes progress
            while(found == false){
                //gets a vector of related deliveries of based off an intersection
                related_delivery_points = corralation_btw_interID_and_Delivery_info[(temp_prio.front().first)];
                //if going to that intersection is productive...
                
                auto hello = std::find(depots.begin(), depots.end(),(temp_prio.front().first) );
                if(check_if_productive(related_delivery_points, depots) && hello == depots.end()){
                    minipath.start_intersection = next_point;
                    next_point = temp_prio.front().first;
                    minipath.end_intersection = next_point;
                    minipath.subpath = temp_prio.front().second.path;
                    time[i] = time[i] + temp_prio.front().second.time; 
                    found = true;
                    //update package deliveries
                    keepTrackDeliveries(minipath);
                    //intersectionPath.push_back(minipath.end_intersection);
                }
                else{
                    //removes the top of the priority queue to keep looking for viable delivery points
                    hold_pop.push_back(temp_prio.front());
                    temp_prio.erase(temp_prio.begin());
                }
            }
            //loops through vector and resets the priority queue for later use
            hold_pop = {};
            found = false;
            //adds path to final path
            totalPath.push_back(minipath);
            time[i] = time[i] + (pathsMatrix.find(std::make_pair(minipath.start_intersection, minipath.end_intersection)))->second.time; 
            minipath = {};
        }
        totalPath.push_back( endPointToDepot(depots, next_point));
        mulit_start_path[i] = totalPath;
    }
    
    float small_time = INFINITY;
    int index = 0; 

    for(int speed = 0; speed < time.size(); speed++){
        if(small_time > time[speed]){
            small_time = time[speed];
            index = speed; 
        }
    }
    
    std::vector<CourierSubPath> bestone = mulit_start_path.at(index);
    intersectionPath.push_back(bestone.front().start_intersection);
    for (int i = 0; i < bestone.size(); i++){
        intersectionPath.push_back(bestone[i].end_intersection);
    }

    //finalPath = bestPath(intersectionPath, deliveries, depots, small_time);

    //reset matrix before returning
    matrix.clear();
    corralation_btw_interID_and_Delivery_info.clear();
    location_visited.clear();
    pathsMatrix.clear();

    return bestone;

    // if(finalPath.empty()){
    //     return bestone;
    // }
    // else{
    //     return finalPath;
    // }
}

//swapping intersections
std::vector<CourierSubPath> bestPath(std::vector<IntersectionIdx> intersectionPath, const std::vector<DeliveryInf>& deliveries, const std::vector<IntersectionIdx>& depots, float totalTime){
    int tempTime;
    //totalTime = addTime(intersectionPath);
    std::vector<CourierSubPath> tempPath(intersectionPath.size() - 1);
    std::vector<CourierSubPath> newPath;
    std::vector<StreetSegmentIdx> tempIds = {};
    PathInfo tempInfo(tempIds, totalTime);
    
    //loop through delivery points
    for (int id1 = 1; id1 < intersectionPath.size()-2; id1++){
        //loops through all other delivery points (doesn't include end depot);
        for (int id2 = id1 + 1; id2 < intersectionPath.size() - 1; id2++){
            //swaps the intersection
            auto newPotentialPath = swapIntersections(intersectionPath, id1, id2);
            tempPath.front().start_intersection = intersectionPath.front();
            tempPath.front().end_intersection = newPotentialPath.at(1);
            tempPath.front().subpath = pathsMatrix.find(std::make_pair(tempPath.front().start_intersection, tempPath.front().end_intersection))->second.path;
            //tempIds.clear();
            //builds new courier subpath with the swapped intersections
            int shift = 0;
            for (int i = 1; i + shift< newPotentialPath.size() - 1; i++){
                //gets the start and end intersection, and the street segment id vector from the matrix
                if(newPotentialPath[i] == newPotentialPath[i+1]){
                    while(newPotentialPath[i + shift] == newPotentialPath[i+1+shift]){
                        shift += 1;
                    }
                    tempPath.resize(intersectionPath.size() - 1 - shift);
                }
                tempPath[i].start_intersection = newPotentialPath[i+shift];
                tempPath[i].end_intersection = newPotentialPath[i+1+shift];
                tempPath[i].subpath = pathsMatrix.find(std::make_pair(newPotentialPath[i+shift], newPotentialPath[i+1+shift]))->second.path;
            }
            tempPath[tempPath.size() - 1] = endPointToDepot(depots, tempPath[tempPath.size() - 2].end_intersection);
            //if the path is legal...
            if (checkLegal(tempPath, deliveries)){
                //gets the total time of the path
                //tempTime = addTime(newPotentialPath);
                tempTime = addTime(tempPath);
                //if the time is smaller than replace final path
                if (tempTime < totalTime){
                    totalTime = tempTime;
                    newPath.resize(tempPath.size());
                    newPath = tempPath;
                }
            }
            tempPath.clear();
            tempPath.resize(intersectionPath.size() - 1);
        }
    }

    return newPath;
}

//funtion to swap intersections
std::vector<IntersectionIdx> swapIntersections(std::vector<IntersectionIdx> intersectionPath, int id1, int id2){
    std::vector<IntersectionIdx> potentialPath = intersectionPath;

    auto itr = &potentialPath[id1];
    auto itr2 = &potentialPath[id2];
    std::iter_swap(itr, itr2);

    return potentialPath;
}

//function to check if the path is legal
bool checkLegal(std::vector<CourierSubPath> intersectionPath, const std::vector<DeliveryInf>& deliveries){
    location_visited.clear();
    //loop through path to check if it is legal, and inserts into locations-visited
    for (int i = 0; i < intersectionPath.size(); i++){
        keepTrackDeliveries(intersectionPath[i]);
    }

    //if location_visited size is equal to deliveries size*2
    if(location_visited.size() == deliveries.size()*2){
        return true;
    } else {
        return false;
    }
}

//function to add total time of path
double addTime(std::vector<CourierSubPath> path){
    double totalTime = 0;

    //Loop through time and adds time from the matrix
    for (auto& it : path){
        //totalTime = totalTime + pathsMatrix.find(std::make_pair(intersectionPath[addTime], intersectionPath[addTime+1]))->second.time;
        totalTime += pathsMatrix.find(std::make_pair(it.start_intersection, it.end_intersection))->second.time;
    }
    return totalTime;
}

bool check_if_productive(std::vector<std::pair<DeliveryInf, std::string>> related_delivery_points, std::vector<IntersectionIdx> depots){
    //loop through related delivery points
    for(int length = 0; length < related_delivery_points.size(); length++ ){
        //if the delivery point is a pick up point and it is not in the list of finished deliveries...
        if(related_delivery_points[length].second == "p"){
            if (location_visited.find(related_delivery_points[length]) == location_visited.end()){
                return true;
            }
        }
        //if the delivery point is a drop off point...
        else if(related_delivery_points[length].second == "d"){
            //if it's not in the list of finished deliveries...
            if(location_visited.find(related_delivery_points[length]) == location_visited.end()){
                //if there is a correlating pick up to the drop off...
                if(location_visited.find(std::make_pair(related_delivery_points[length].first, "p")) != location_visited.end()){
                    return true;
                }
            }       
        }  
    }
    return false;

    /*
    int count = 0;
    for(int length = 0; length < related_delivery_points.size(); length++ ){
        
        if(related_delivery_points[length].second == "p"){
            if (location_visited.find(related_delivery_points[length]) == location_visited.end()){
                count++;
            }
        }
        else if(related_delivery_points[length].second == "d"){
            //if it's not in the list of finished deliveries...
            if(location_visited.find(related_delivery_points[length]) == location_visited.end()){
                //if there is a correlating pick up to the drop off...
                if(location_visited.find(std::make_pair(related_delivery_points[length].first, "p")) != location_visited.end()){
                    count++;
                }
            }       
        }          
    }
    if(count == related_delivery_points.size()){
            return true;
    }
    return false;*/

}







/*
//adds the points we have been to to an unordered_set
void goToDeliveryIntersection( DeliveryInf delivery_info, std::string delivery_or_pickup){
    location_visited.emplace(std::make_pair(delivery_info, delivery_or_pickup));
}

//loops through unordered set and sees if we have dealt with there order
bool Check_if_delivered_or_pick(DeliveryInf delivery_info, std::string delivery_or_pickup){
    //loops through unordered set
    auto it = location_visited.find(std::make_pair(delivery_info, delivery_or_pickup));
    //If element is in the set
    if(it != location_visited.end()){
        return true;
    }
    //otherwise its not in set
    return false;
}
*/

/*
std::vector<std::pair(DeliveryInf delivery_info, std::string delivery_or_pickup)> find_next_closest_available_point(IntersectionIdx current_position){
    IntersectionIdx potential_intersection;
    std::vector<std::pair<IntersectionIdx, PathInfo>> popped_data;
    float time;
    // loop through priority queue of end intersection paired with PathInfo
    for(int size_of_prio = 0; size_of_prio < matrix.find(current_position).size() ; size_of_prio++){
        potential_intersection = matrix.find(current_position).top().first;
        time = matrix.find(current_position).top().second.time;
        std::vector<std::pair<DeliveryInf, std::string>> DeliveryInf_at_intersection;

        //found all delivery infos correlating to the intersection based on time.
        DeliveryInf_at_intersection = corralation_btw_interID_and_Delivery_info.find(potential_intersection);
        for(int legal =  0; legal < DeliveryInf_at_intersection.size(); legal++){
            if (DeliveryInf_at_intersection.at(legal).second == "p"){
                potential_intersection = DeliveryInf_at_intersection.at(legal).first;
                break;
            } else if (DeliveryInf_at_intersection.at(legal).second == "d") {
                Check_if_delivered_or_pick()depotToPickUp


        auto it = location_visited.find(std::make_pair(delivery_info));
        
    }
    //step 1: check priority queue
    // -> looking at front, if its part of the unordered set, move to the next.
    //step 2: check if dropoff 
    // -> look through unordered set to see if we have the package
    //   -> if we have the package, variable to hold this intersection value
    //      -> find all other drop off deliveries at that intersection
    //          -> loop through vector to find the same times
    //   -> if we don't have the package, move on to pick up if
    //step 3: if pick up
    // -> same check for pickup from drop off
    //step 4: if variable is not empty, then we return vector of delivery info that contains pickups and drop offs that we would have
    //else return empty vector
    // if empty vector -> no more deliveries
    // go to a function that finds closest depot for ending
}*/


/*
    for (int deliver = 0; deliver < deliveries.size(); deliver++){
        deliveryIntersections.insert(std::make_pair(deliveries[deliver].pickUp, std::make_pair (deliver, 1)));
        deliveryIntersections.insert(std::make_pair(deliveries[deliver].dropOff, std::make_pair (deliver, 2)));
    }
    for (int depotNo = 0; depotNo < depots.size(); depotNo ++){
        deliveryIntersections.insert(std::make_pair(depots[depotNo], std::make_pair (depotNo, 3)));
    }

    smallPath = depotToPickUp(depots, deliveries);
    goToDeliveryIntersection(smallPath);*/
    
    
    
    
    
    
    /*
    for(int deliver = 0; deliver < deliveries.size(); deliver++ ){
        deliveries_reached.push_back((std::make_pair(deliveries[deliver],deliveries[deliver].pickUp)));
        deliveries_reached.push_back((std::make_pair(deliveries[deliver],deliveries[deliver].dropOff)));
    }
    
    //the entire path of all deliveries
    std::vector<CourierSubPath> totalpath;
    
    //find minimum time
    float tempTime = INFINITY;
    IntersectionIdx tempPickUp, tempDepot;
    IntersectionIdx tempDelivery;
    int* position;
    
    //checks if no deliveries
    if (deliveries.size() == 0){
        return;
    }
    
    //calling multiDijkstra function to setup matrix
    for(int deliveryNum = 0; deliveryNum < deliveries.size(); deliveryNum++){
        multiDijkstra(deliveries[deliveryNum].pickUp, importantPoints, turn_penalty);
        multiDijkstra(deliveries[deliveryNum].dropOff, importantPoints, turn_penalty);
    }
    //gives the depot and pickup intersection that are closest together
    for(int depotNum = 0; depotNum < depots.size(); depotNum++){
        for (int pickUpPoints = 0; pickUpPoints < deliveries.size(); pickUpPoints++){
            if (tempTime < matrix[depots[std::make_pair(depots[depotNum], deliveries.at(pickUpPoints).pickUp)]].time){
                tempTime = matrix[depots[std::make_pair(depots[depotNum], deliveries.at(pickUpPoints).pickUp)]].time;
                tempPickUp = deliveries[pickUpPoints].pickUp;
                tempDepot = depots[depotNum];
            }
        }
    }
    //sets subpath for depot -> first pickup
    struct CourierSubPath miniPath;
    miniPath.start_intersection = tempDepot;
    miniPath.end_intersection = tempPickUp;
    miniPath.subpath = matrix[std::make_pair(tempDepot, tempPickUp)].path;
    totalpath.push_back(miniPath);
    
    //continues until we traveled through all intersections
    while(deliveries_reached.size()!=0){
        tempTime = INFINITY;
        //loops through available deliveries left
        for(int deliveries_left = 0; deliveries_left < deliveries_reached.size(); deliveries_left++){
            //checks if path is available
            if(pathlegal(tempPickUp, deliveries_reached[deliveries_left])){
                //sees if we have package: .second returns a DeliveryInf, .first returns the IntersectionId
                if(pickup_before_dropoff(deliveries_reached[deliveries_left].second, deliveries_reached[deliveries_left].first)){
                    //if temporary time is less than time it takes to reach that intersection...
                    if (tempTime < matrix[std::make_pair(tempPickUp, deliveries_reached[deliveries_left])].time){
                        tempTime = matrix[std::make_pair(tempPickUp, deliveries_reached[deliveries_left])].time;
                        tempDelivery = deliveries_reached[deliveries_left];
                        position = deliveries_reached[deliveries_left];
                    } else {
                        continue;
                    }
                } else {
                    continue;
                }
            } else {
                continue;
            }
        }
        miniPath.start_intersection = tempPickUp;
        miniPath.end_intersection = tempDelivery;
        miniPath.subpath = matrix[std::make_pair(tempPickUp, tempDelivery)].path;
        totalpath.push_back(miniPath);
        tempPickUp = tempDelivery;
        tempDelivery = -1;
        
        deliveries_reached.erase(position);
    }
    
    
    //matrix is created by this point
    
}*/