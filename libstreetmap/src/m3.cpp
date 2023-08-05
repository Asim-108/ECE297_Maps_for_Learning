//CPP file created by team 033 for functions used in milestone 3

#include "m1.h"
#include "m2.h"
#include "m3.h"
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


double computePathTravelTime(const std::vector<StreetSegmentIdx>& path, const double turn_penalty){
    StreetIdx prevStreet;
    double travelTime;

    //checking to see if empty vector is given
    if(path.size() > 0){
        prevStreet = getStreetSegmentInfo(path[0]).streetID;
        travelTime = findStreetSegmentTravelTime(path[0]);
    }
    else{
        return 0;
    }

    //loop through all street segments in vector given, add travel time for them to total travel time
    for(int streetNum = 1; streetNum < path.size(); streetNum++){
        //if we change streets, add turn penalty to travel time
        if(getStreetSegmentInfo(path[streetNum]).streetID != prevStreet){
            travelTime += turn_penalty;
            prevStreet = getStreetSegmentInfo(path[streetNum]).streetID;
        }
        travelTime += findStreetSegmentTravelTime(path[streetNum]);
    }
    return travelTime;
}

std::vector<StreetSegmentIdx> findPathBetweenIntersections(const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids, const double turn_penalty){
    //priority queue used for A*
    std::priority_queue<WaveElem> wavefront;

    //initializing starting vector of node class size of total intersections in city, all values initialized to infinity
    //declaring and initializing all values to infinity this way is very quick, not same as looping through all values and manually setting infinity
    std::vector <Node> nodes(getNumIntersections(), Node(std::numeric_limits<double>::infinity()));

    //insert first intersection into priority queue
    wavefront.push(WaveElem(intersect_ids.first, NO_EDGE, 0));
    bool pathFound = false;

    while(wavefront.size() > 0 && !pathFound){
        //remove shortest travel time element in priority queue
        WaveElem wave = wavefront.top();
        wavefront.pop();

        int currentID = wave.nodeID;

        //if best time to intersection, update best time and reaching edge
        if(wave.travelTime < nodes[currentID].bestTime){
            nodes[currentID].reachingEdge = wave.edgeID;
            nodes[currentID].bestTime = wave.travelTime;
            
            //if at end intersection, we are done
            if(currentID == intersect_ids.second){
                pathFound = true;
                continue;
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

    std::vector<StreetSegmentIdx> path;

    //reconstructing path
    if(pathFound){
        int currentID = intersect_ids.second;
        int prevEdge = nodes[currentID].reachingEdge;

        while (prevEdge != NO_EDGE){
            path.insert(path.begin(), prevEdge);
            StreetSegmentInfo curStreet = getStreetSegmentInfo(prevEdge);
            if(curStreet.to == currentID){
                currentID = curStreet.from;
            }
            else{
                currentID = curStreet.to;
            }
            prevEdge = nodes[currentID].reachingEdge;
        }
    }
    return path;
}


/*
#include <chrono>
#define Time_Limit 50

auto startTime = std::chrono::high_resolution_clock::now();
bool timeOut = false;
while(!timeOut){
    myOptimizer();
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto wallClock = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - startTime);
    if(wallClock.count()> 0.9*Time_Limit){
        timeOut = true;
    }
}

*/