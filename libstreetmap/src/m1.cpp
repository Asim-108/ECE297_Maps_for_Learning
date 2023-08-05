/* 
 * Copyright 2023 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>
#include <thread>
#include <vector>
#include <math.h>
#include <cmath>
#include <utility>
#include <vector>
#include "m1.h"
#include "m4.h"
#include <set>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "helper.h"
#include <unordered_map>
#include <map>
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <limits>

#include <boost/functional/hash.hpp>

/****************************Data Structure Declarations ********************************/
// Global Variable Declaration for Data Structures
double latitudeAverage;
double min_lon;
double max_lon;
double min_lat;
double max_lat;

//function declaration
double x_from_lon(float longitude);
double y_from_lat(float latitude);
double lon_from_x(double x);
double lat_from_y(double y);

// Converts a y-coordinate in meters to longitude 
double x_from_lon(float longitude){
    float x = 0;
    return x = longitude*kDegreeToRadian*kEarthRadiusInMeters*cos(latitudeAverage*kDegreeToRadian);
}
// Converts a y-coordinate in meters to latitude 
double y_from_lat(float latitude){
    float y = 0;
    return y = latitude*kDegreeToRadian*kEarthRadiusInMeters;
}
// Converts longitude to a x-coordinate in meters
double lon_from_x(double x){
    float lon;
    return lon = x/(kDegreeToRadian*kEarthRadiusInMeters*cos(latitudeAverage*kDegreeToRadian));
}

// Converts latitude to a y-coordinate in meters
double lat_from_y(double y){
    float lat;
    return lat = y/kDegreeToRadian/kEarthRadiusInMeters;
}

// Intersection Struct
struct intersection_data {
    LatLon position;
    std::string name;
    ezgl::point2d xandy_coor_intersection;
    bool highlighted = false;
};
std::vector <intersection_data> intersections;

// POI Struct
struct POI_data {
    LatLon POI_position;
    std::string POI_name;
    ezgl::point2d xandy_coor_POI;
    bool highlighted = false;
};
std::vector <POI_data> POI_INFO;

// Street Segment Struct
struct Street_segment_data {
    OSMID ID;
    std::string Street_seg_name;
    int num_curve_points;
    float speed_limit;
    ezgl::point2d from;
    ezgl::point2d to;
    std::vector <LatLon> curvepoints;
    std::vector <ezgl::point2d> Street_seg_points_xy;
    ezgl::point2d draw_text;
    double angle;
    std::vector <std::pair<ezgl::point2d,ezgl::point2d >> to_arrowhead;
    std::vector <std::pair<ezgl::point2d,ezgl::point2d >>  from_arrowhead;
    //bool highlighted = false;
};
std::vector <Street_segment_data> Street_segments_INFO;

// Feature Struct
struct Feature_data {
    TypedOSMID Feature_ID;
    std::string Feature_name;
    int Feature_num_points;
    FeatureType Feature_type;
    std::vector <LatLon> Feature_points;
    std::vector <ezgl::point2d> Feature_points_xy;
    bool area_closed;
};
std::vector <Feature_data> Feature_INFO;

// Used for Subway Information
std::vector <ezgl::point2d> Subway_INFO;

// Used for Library Information
std::vector <ezgl::point2d> Library_INFO;

// Used for FastFood Information
std::vector <ezgl::point2d> FastFood_INFO;

// Used for Restaurant Information
std::vector <ezgl::point2d> Restaurant_INFO;

// Used for previously highlighted intersections
std::vector <int> prevHighlightedIntersections;

//Used in: findStreetLength() 
std::vector<double> streetlength;

//Used in: findStreetSegmentsOfIntersection() & findAdjacentIntersections()
std::vector<std::vector<StreetSegmentIdx>> street_segs;

//Used in: findIntersectionsOfStreet() & findIntersectionsOfTwoStreets()
std::vector<std::vector< IntersectionIdx >> street_inter;

//Used in: getOSMNodeTagValue()
std::unordered_map<OSMID, int> OSMNodeMap;

//Used in: findStreetSegmentLength(), findStreetSegmentTravelTime(), and findFeatureArea()
std::vector<double> street_seg_length;

//Used in: findStreetSegmentTravelTime()
std::vector<double> street_segment_time;

//Used in: findFeatureArea()
std::unordered_map<OSMID, int> OSMWay;

//Used in: findStreetIdsFromPartialStreetName()
std::map<std::string, std::vector<StreetIdx>> partialStreetName;

//Used in: findStreetIdsFromPartialStreetName()
std::unordered_map<char, std::vector<StreetIdx>> singleLetterStreetName;

//used to print features in order on map
std::vector<Feature_data>lakes;
std::vector<Feature_data>islands;
std::vector<Feature_data>parks;
std::vector<Feature_data>greenspaces;
std::vector<Feature_data>golfCourses;
std::vector<Feature_data>glaciers;
std::vector<Feature_data>beaches;
std::vector<Feature_data>rivers;
std::vector<Feature_data>streams;
std::vector<Feature_data>buildings;
std::vector<Feature_data>unknowns;

//Gets all of the street_ids for gtk autocomplete
std::vector <StreetIdx> street_INFO;

//Holds all the street ids when searching
std::vector<StreetIdx> centerStreet;

//Holds all the intersection ids when searching
std::vector<IntersectionIdx> centerIntersection;
std::vector<IntersectionIdx> centerIntersection2;

//Vectors used to hold information in order to find the direction you are traveling in
std::vector <std::string> Directions_to_2_from;
std::vector <std::pair<std::string, std::string>> Directions_NESW;

//global vector of the street segments associated to a path
std::vector <StreetSegmentIdx> street_seg_path;

//straight -> Straight | KR -> Keep Right | R -> Right | SHR -> Sharp Right | U -> UTurn | SHL -> Sharp Left | L -> Left | KL -> Keep Left
//Takes in the previous direction and next direction, and based on these parameters, it tells the user what direction they are going in.
// First element in the pair is the previous direction and the second element is the next direction.
extern std::unordered_map<std::pair<std::string, std::string>, std::string, boost::hash<std::pair<std::string, std::string>>> direction_umap;

   


/****************************End Data Structure Declarations *****************************/

/**************************** Data Structure Helper Function Declarations *****************************/
void setting_up_intersections();
void setting_up_POI();
void setting_up_street_seg();
void setting_up_features();
void setting_up_street_intersection_length();
void setting_up_find_intersection_of_street();
void setting_up_get_OSMNodeTagValue();
void setting_up_find_partial_street_names();
void setting_up_street_seg_of_intersection();
void arrowhead(double x_coor, double y_coor, double x_coor2, double y_coor2, StreetSegmentIdx Street_seg_idx, std::string Which_arrows);
/**************************** End of Data Structure Helper Function Declarations *****************************/


/***************************** Helper Functions****************************************/
void arrowhead(double x_coor, double y_coor, double x_coor2, double y_coor2, StreetSegmentIdx Street_seg_idx, std::string Which_arrows){
    //finds change in x and y
    double deltax2 = x_coor2-x_coor;
    double deltay2 = y_coor2-y_coor;
    // finds the total length (hypotenuse)
    double total_len = pow(deltax2, 2) + pow(deltay2,2);
    total_len = pow(total_len , 0.5);
    
    double street_angle;
    //Finds the street angle
    if(deltax2 != 0){
        //finds street angle in radians
        street_angle = atan(deltay2/deltax2)*180/M_PI;
    }
    else{
        street_angle = 0;
    }
    

    //sets the length of the arrowhead
    double lenofarrowhead = 10;

    //Uses cosine law to find the final side of the triangle
    double newside = sqrt((lenofarrowhead*lenofarrowhead + total_len*total_len - 2*total_len*lenofarrowhead* cos(45*M_PI/180)));

    //uses sine law to find the angle in the triangle
    double subangle;
    if(newside != 0){
        subangle = asin(lenofarrowhead*sin(45*M_PI/180)/newside)*180/M_PI;
    }
    else{
        subangle = 0;
    }
    
    //Finds the change in x and y for the arrowheads with trig. Each arrow head is broken down into 2 triangles -> 4 total triangles. 
    double angle_of_new_triangle = street_angle - subangle;
    double deltay_of_new_side = sin(angle_of_new_triangle*M_PI/180) * newside/ sin(90*M_PI/180);
    double deltax_of_new_side = cos(angle_of_new_triangle*M_PI/180) * newside/ sin(90*M_PI/180);

    double angle_of_new_triangle2 = street_angle + subangle;
    double deltay_of_new_side2 = sin(angle_of_new_triangle2*M_PI/180) * newside/ sin(90*M_PI/180);
    double deltax_of_new_side2 = cos(angle_of_new_triangle2*M_PI/180) * newside/ sin(90*M_PI/180);
    
    double angle_of_new_triangle3 = street_angle + 45;
    double deltay_of_new_side3 = sin(angle_of_new_triangle3*M_PI/180) * lenofarrowhead;
    double deltax_of_new_side3 = cos(angle_of_new_triangle3*M_PI/180) * lenofarrowhead;

    double angle_of_new_triangle4 = street_angle - 45;
    double deltay_of_new_side4 = sin(angle_of_new_triangle4*M_PI/180) * lenofarrowhead;
    double deltax_of_new_side4 = cos(angle_of_new_triangle4*M_PI/180) * lenofarrowhead;

    //initalize
    double arrowhead1x;
    double arrowhead1y;
    double arrowhead2x;
    double arrowhead2y;
    double arrowhead3x;
    double arrowhead3y;
    double arrowhead4x;
    double arrowhead4y;

    
    //if the line is vertical
    if(abs(deltax2) < 1){
        //if y1 < y2
        if(y_coor < y_coor2){
            //If you want the arrow heads for both sides
            if(Which_arrows == "both"){
                Street_segments_INFO[Street_seg_idx].to_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor + lenofarrowhead / sqrt(2), y_coor2 - lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor - lenofarrowhead / sqrt(2),y_coor2 - lenofarrowhead / sqrt(2))));
                Street_segments_INFO[Street_seg_idx].from_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor + lenofarrowhead / sqrt(2),y_coor + lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor - lenofarrowhead / sqrt(2),y_coor + lenofarrowhead / sqrt(2))));
            }
            //if you want the arrowhead for the to point
            else if(Which_arrows == "to"){
                Street_segments_INFO[Street_seg_idx].to_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor + lenofarrowhead / sqrt(2), y_coor2 - lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor - lenofarrowhead / sqrt(2),y_coor2 - lenofarrowhead / sqrt(2))));
            }
            //if you want the arrowhead for the from point
            else if(Which_arrows == "from"){
                Street_segments_INFO[Street_seg_idx].from_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor + lenofarrowhead / sqrt(2),y_coor + lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor - lenofarrowhead / sqrt(2),y_coor + lenofarrowhead / sqrt(2))));
            }
        }
        //if y2 <y1
        else if(y_coor > y_coor2){
            //if you want the arrowhead for both sides
            if(Which_arrows == "both"){
                Street_segments_INFO[Street_seg_idx].to_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor + lenofarrowhead / sqrt(2), y_coor2 + lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor - lenofarrowhead / sqrt(2),y_coor2 + lenofarrowhead / sqrt(2))));
                Street_segments_INFO[Street_seg_idx].from_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor + lenofarrowhead / sqrt(2),y_coor - lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor - lenofarrowhead / sqrt(2),y_coor - lenofarrowhead / sqrt(2))));
            }
            //if you want the arrowheads for the to point
            else if(Which_arrows == "to"){
                Street_segments_INFO[Street_seg_idx].to_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor + lenofarrowhead / sqrt(2), y_coor2 + lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor - lenofarrowhead / sqrt(2),y_coor2 + lenofarrowhead / sqrt(2))));
            }
            //if you want the arrowhead for the from point
            else if(Which_arrows == "from"){
                Street_segments_INFO[Street_seg_idx].from_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor + lenofarrowhead / sqrt(2),y_coor - lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor - lenofarrowhead / sqrt(2),y_coor - lenofarrowhead / sqrt(2))));
            }
        }
    }
    //if the line is horizontal
    else if(abs(deltay2) < 1){
        //if y1 < y2
        if(x_coor < x_coor2){
            //If you want the arrow heads for both sides
            if(Which_arrows == "both"){
                Street_segments_INFO[Street_seg_idx].to_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor2 - lenofarrowhead / sqrt(2), y_coor + lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor2 - lenofarrowhead / sqrt(2),y_coor - lenofarrowhead / sqrt(2))));
                Street_segments_INFO[Street_seg_idx].from_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor + lenofarrowhead / sqrt(2),y_coor + lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor + lenofarrowhead / sqrt(2),y_coor - lenofarrowhead / sqrt(2))));
            }
            //if you want the arrowhead for the to point
            else if(Which_arrows == "to"){
                Street_segments_INFO[Street_seg_idx].to_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor2 - lenofarrowhead / sqrt(2), y_coor + lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor2 - lenofarrowhead / sqrt(2),y_coor - lenofarrowhead / sqrt(2))));
            }
            //if you want the arrowhead for the from point
            else if(Which_arrows == "from"){
                Street_segments_INFO[Street_seg_idx].from_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor + lenofarrowhead / sqrt(2),y_coor + lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor + lenofarrowhead / sqrt(2),y_coor - lenofarrowhead / sqrt(2))));
            }
        }
        //if y2 <y1
        else if(x_coor > x_coor2){
            //if you want the arrowhead for both sides
            if(Which_arrows == "both"){
                Street_segments_INFO[Street_seg_idx].to_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor2 + lenofarrowhead / sqrt(2), y_coor + lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor2 + lenofarrowhead / sqrt(2),y_coor - lenofarrowhead / sqrt(2))));
                Street_segments_INFO[Street_seg_idx].from_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor - lenofarrowhead / sqrt(2),y_coor + lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor - lenofarrowhead / sqrt(2),y_coor - lenofarrowhead / sqrt(2))));
            }
            //if you want the arrowheads for the to point
            else if(Which_arrows == "to"){
                Street_segments_INFO[Street_seg_idx].to_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor2 + lenofarrowhead / sqrt(2), y_coor + lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor2 + lenofarrowhead / sqrt(2),y_coor - lenofarrowhead / sqrt(2))));
            }
            //if you want the arrowhead for the from point
            else if(Which_arrows == "from"){
                Street_segments_INFO[Street_seg_idx].from_arrowhead.push_back(std::make_pair(ezgl::point2d(x_coor - lenofarrowhead / sqrt(2),y_coor + lenofarrowhead / sqrt(2) ), ezgl::point2d(x_coor - lenofarrowhead / sqrt(2),y_coor - lenofarrowhead / sqrt(2))));
            }
        }
    }
    //if x2 < x1
    else if( x_coor2 < x_coor){
        //To -> from arrowheads
        arrowhead1x = x_coor2 + deltax_of_new_side;
        arrowhead1y = y_coor2 + deltay_of_new_side;
        arrowhead2x = x_coor2 + deltax_of_new_side2;
        arrowhead2y = y_coor2 + deltay_of_new_side2;
        //From -> to arrowheads
        arrowhead3x = x_coor2 + deltax_of_new_side3;
        arrowhead3y = y_coor2 + deltay_of_new_side3;
        arrowhead4x = x_coor2 + deltax_of_new_side4;
        arrowhead4y = y_coor2 + deltay_of_new_side4;
        //if both, set up arrowheads for to and from
        if(Which_arrows == "both"){
            Street_segments_INFO[Street_seg_idx].to_arrowhead.push_back(std::make_pair(ezgl::point2d(arrowhead3x,arrowhead3y), ezgl::point2d(arrowhead4x,arrowhead4y)));
            Street_segments_INFO[Street_seg_idx].from_arrowhead.push_back(std::make_pair(ezgl::point2d(arrowhead1x,arrowhead1y), ezgl::point2d(arrowhead2x,arrowhead2y)));
        }
        //set up arowheads for to
        else if(Which_arrows == "to"){
            Street_segments_INFO[Street_seg_idx].to_arrowhead.push_back(std::make_pair(ezgl::point2d(arrowhead3x,arrowhead3y), ezgl::point2d(arrowhead4x,arrowhead4y)));
        }
        //sets up arrowheads for from
        else if(Which_arrows == "from"){
            Street_segments_INFO[Street_seg_idx].from_arrowhead.push_back(std::make_pair(ezgl::point2d(arrowhead1x,arrowhead1y), ezgl::point2d(arrowhead2x,arrowhead2y)));
        }
    }
    // if x1 < x2
    else{
        //From -> to arrowheads
        arrowhead1x = x_coor + deltax_of_new_side;
        arrowhead1y = y_coor + deltay_of_new_side;
        arrowhead2x = x_coor + deltax_of_new_side2;
        arrowhead2y = y_coor + deltay_of_new_side2;
        //To -> from arrowheads
        arrowhead3x = x_coor + deltax_of_new_side3;
        arrowhead3y = y_coor + deltay_of_new_side3;
        arrowhead4x = x_coor + deltax_of_new_side4;
        arrowhead4y = y_coor + deltay_of_new_side4;
        //if you want arrowheads for both to and from
        if(Which_arrows == "both"){
            Street_segments_INFO[Street_seg_idx].to_arrowhead.push_back(std::make_pair(ezgl::point2d(arrowhead1x,arrowhead1y), ezgl::point2d(arrowhead2x,arrowhead2y)));
            Street_segments_INFO[Street_seg_idx].from_arrowhead.push_back(std::make_pair(ezgl::point2d(arrowhead3x,arrowhead3y), ezgl::point2d(arrowhead4x,arrowhead4y)));
        }
        //if you want arrowheads for only to
        else if(Which_arrows == "to"){
            Street_segments_INFO[Street_seg_idx].to_arrowhead.push_back(std::make_pair(ezgl::point2d(arrowhead1x,arrowhead1y), ezgl::point2d(arrowhead2x,arrowhead2y)));
        }
        //if you want arrowheads for only from
        else if(Which_arrows == "from"){ 
            Street_segments_INFO[Street_seg_idx].from_arrowhead.push_back(std::make_pair(ezgl::point2d(arrowhead3x,arrowhead3y), ezgl::point2d(arrowhead4x,arrowhead4y)));
        }
    }
}
/***************************** Ending Helper Functions****************************************/

/*****************************Setting up Map Data Structures****************************************/
//Data Structure to set up all intersections of the map
void setting_up_intersections(){

    // nodes.resize(getNumIntersections());
    //Looping through all the intersections...
    for (int id = 0; id < getNumIntersections(); id++){
        //gets the x and y coordinates from the intersection's longitude and latitude values.
        double x_coor =   x_from_lon(getIntersectionPosition(id).longitude());
        double y_coor =   y_from_lat(getIntersectionPosition(id).latitude());
        // puts the coordinates together and also gets the name of that intersection
        intersections[id].xandy_coor_intersection = ezgl::point2d(x_coor,y_coor);
        intersections[id].name = getIntersectionName(id);

        // nodes[id].bestTime = std::numeric_limits<double>::infinity();
    }
}

//Data Structure to set up the POIs of the map
void setting_up_POI(){
    // resize the struct of POI
    POI_INFO.resize(getNumPointsOfInterest());
    // Loop through the POIs...
    for(int POI_idx = 0; POI_idx < getNumPointsOfInterest(); POI_idx++){
        // gets the name, position and coordinate of that POI
        POI_INFO[POI_idx].POI_name = getPOIName(POI_idx);
        POI_INFO[POI_idx].POI_position = getPOIPosition(POI_idx);
        double x_coor =   x_from_lon(getPOIPosition(POI_idx).longitude());
        double y_coor =   y_from_lat(getPOIPosition(POI_idx).latitude());
        POI_INFO[POI_idx].xandy_coor_POI = ezgl::point2d(x_coor,y_coor);
    }
}

// Data Structure to set up all of the street segments of the map
void setting_up_street_seg(){
    // resize the struct of street segments
    Street_segments_INFO.resize(getNumStreetSegments());
    // Loop through the street segments...
    for(int Street_seg_idx = 0; Street_seg_idx < getNumStreetSegments(); Street_seg_idx++){
        // gets the OSMID of each street segment
        Street_segments_INFO[Street_seg_idx].ID = getStreetSegmentInfo(Street_seg_idx).wayOSMID;
        // if there's a one way street...
        if(getStreetSegmentInfo(Street_seg_idx).oneWay){
            // If the the from longitude is less than the to longitude...
            if(x_from_lon(intersections.at(getStreetSegmentInfo(Street_seg_idx).from).position.longitude()) < x_from_lon(intersections.at(getStreetSegmentInfo(Street_seg_idx).to).position.longitude())){
                std::string text;
                // gets the street name of that ID...
                text.append(getStreetName(getStreetSegmentInfo(Street_seg_idx).streetID));
                // If street name is not 'unknown...
                if(getStreetName(getStreetSegmentInfo(Street_seg_idx).streetID)!="<unknown>"){
                    // Add an arrow to show one-way street
                    text.append(" --->");
                }
                // Adds the street to struct
                Street_segments_INFO[Street_seg_idx].Street_seg_name = text;
            }
            else{
                std::string text;
                //Add arrow in the opposite direction...
                text.append(getStreetName(getStreetSegmentInfo(Street_seg_idx).streetID));
                if(getStreetName(getStreetSegmentInfo(Street_seg_idx).streetID)!="<unknown>"){ 
                    text.append(" <---");    
                }
                Street_segments_INFO[Street_seg_idx].Street_seg_name = text;
            }
        }
        else {
            //Adds street name to street segment struct
            Street_segments_INFO[Street_seg_idx].Street_seg_name = getStreetName(getStreetSegmentInfo(Street_seg_idx).streetID);
        }
        // adds the number of curve points, speed limit along with the from and to coordinates for each street segment into the struct
        Street_segments_INFO[Street_seg_idx].num_curve_points = getStreetSegmentInfo(Street_seg_idx).numCurvePoints;
        Street_segments_INFO[Street_seg_idx].speed_limit = getStreetSegmentInfo(Street_seg_idx).speedLimit;
        Street_segments_INFO[Street_seg_idx].from = ezgl::point2d(x_from_lon(intersections.at(getStreetSegmentInfo(Street_seg_idx).from).position.longitude()),y_from_lat(intersections.at(getStreetSegmentInfo(Street_seg_idx).from).position.latitude()));
        Street_segments_INFO[Street_seg_idx].to = ezgl::point2d(x_from_lon(intersections.at(getStreetSegmentInfo(Street_seg_idx).to).position.longitude()),y_from_lat(intersections.at(getStreetSegmentInfo(Street_seg_idx).to).position.latitude()));
        
        //find the change in x-coordinatex and y-coordinates
        double deltax = x_from_lon(intersections.at(getStreetSegmentInfo(Street_seg_idx).to).position.longitude()) - x_from_lon(intersections.at(getStreetSegmentInfo(Street_seg_idx).from).position.longitude());
        double deltay = y_from_lat(intersections.at(getStreetSegmentInfo(Street_seg_idx).to).position.latitude()) - y_from_lat(intersections.at(getStreetSegmentInfo(Street_seg_idx).from).position.latitude());
        
        // If there are curve points...
        if(getStreetSegmentInfo(Street_seg_idx).numCurvePoints > 0){
            
            // For all of the curve points...
            for(int curve_num = 0; curve_num < getStreetSegmentInfo(Street_seg_idx).numCurvePoints; curve_num++){
                // gets the curve points and its x and y coordinates and pushes it into the struct.
                Street_segments_INFO[Street_seg_idx].curvepoints.push_back(getStreetSegmentCurvePoint(Street_seg_idx,curve_num ));
                double x_coor = x_from_lon(getStreetSegmentCurvePoint(Street_seg_idx,curve_num).longitude());
                double y_coor = y_from_lat(getStreetSegmentCurvePoint(Street_seg_idx,curve_num).latitude());
                Street_segments_INFO[Street_seg_idx].Street_seg_points_xy.push_back(ezgl::point2d(x_coor,y_coor));
            }
            double longest_segment = 0;
            // Looping through the curve points...
            for(int curve_num = 0; curve_num < getStreetSegmentInfo(Street_seg_idx).numCurvePoints-1; curve_num++){
                // gets the x and y coordinates of both 'to' and 'from', along with the distance between the coordinates
                double x_coor = x_from_lon(getStreetSegmentCurvePoint(Street_seg_idx,curve_num).longitude());
                double y_coor = y_from_lat(getStreetSegmentCurvePoint(Street_seg_idx,curve_num).latitude());
                double x_coor2 = x_from_lon(getStreetSegmentCurvePoint(Street_seg_idx,curve_num+1).longitude());
                double y_coor2 = y_from_lat(getStreetSegmentCurvePoint(Street_seg_idx,curve_num+1).latitude());
                double deltax2 = x_coor2-x_coor;
                double deltay2 = y_coor2-y_coor;
                // finds the total length of the curve points
                double total_len = pow(deltax2, 2) + pow(deltay2,2);
                total_len = pow(total_len , 0.5);
                // Checks if it is the longest street segment...
                if(longest_segment < total_len){
                    // finds the location of where the location text will be drawn
                    Street_segments_INFO[Street_seg_idx].draw_text = ezgl::point2d(x_coor,y_coor);
                    if(deltax2 != 0){
                        Street_segments_INFO[Street_seg_idx].angle = atan(deltay2/deltax2)*180/3.14;
                    }
                    else{
                        Street_segments_INFO[Street_seg_idx].angle = 0;
                    }
                }
            }
        }
        else{
            // Finds the angle of the sreet ID
            if(deltax != 0){
                Street_segments_INFO[Street_seg_idx].angle = atan(deltay/deltax)*180/3.14;
            }
            else{
                Street_segments_INFO[Street_seg_idx].angle = 0;
            }
            // finds the location of where the location text will be drawn.
            Street_segments_INFO[Street_seg_idx].draw_text = ezgl::point2d(x_from_lon(intersections.at(getStreetSegmentInfo(Street_seg_idx).to).position.longitude()) + deltax/2, y_from_lat(intersections.at(getStreetSegmentInfo(Street_seg_idx).to).position.latitude()) + deltay/2);
        }  
        if(getStreetSegmentInfo(Street_seg_idx).numCurvePoints ==0 ){
            //calculate inputs for function
            double x_coor = x_from_lon(intersections.at(getStreetSegmentInfo(Street_seg_idx).from).position.longitude());
            double y_coor = y_from_lat(intersections.at(getStreetSegmentInfo(Street_seg_idx).from).position.latitude());
            double x_coor2 =  x_from_lon(intersections.at(getStreetSegmentInfo(Street_seg_idx).to).position.longitude());
            double y_coor2 = y_from_lat(intersections.at(getStreetSegmentInfo(Street_seg_idx).to).position.latitude()) ;
            //finds arrowheads
            arrowhead(x_coor,y_coor,x_coor2,y_coor2,Street_seg_idx, "both");          
        }
        //if its a curved street
        else if(getStreetSegmentInfo(Street_seg_idx).numCurvePoints > 0 ){
            //calculate x and y coordinates
            //x2 and y2 is for the to point
            double x_coor2 = x_from_lon(intersections.at(getStreetSegmentInfo(Street_seg_idx).to).position.longitude());
            double y_coor2 = y_from_lat(intersections.at(getStreetSegmentInfo(Street_seg_idx).to).position.latitude());

            //x1 and y1 is for the last curved point
            double x_coor =  x_from_lon(getStreetSegmentCurvePoint(Street_seg_idx,getStreetSegmentInfo(Street_seg_idx).numCurvePoints-1).longitude());;
            double y_coor = y_from_lat(getStreetSegmentCurvePoint(Street_seg_idx,getStreetSegmentInfo(Street_seg_idx).numCurvePoints-1).latitude());
            arrowhead(x_coor,y_coor,x_coor2,y_coor2,Street_seg_idx, "to");      
            
            //calculating the dx and dy for the from point and the first curved point
            //x and y for from point
            x_coor = x_from_lon(intersections.at(getStreetSegmentInfo(Street_seg_idx).from).position.longitude());
            y_coor = y_from_lat(intersections.at(getStreetSegmentInfo(Street_seg_idx).from).position.latitude());

            //x and y for the first curved point
            x_coor2 =  x_from_lon(getStreetSegmentCurvePoint(Street_seg_idx,0).longitude());;
            y_coor2 = y_from_lat(getStreetSegmentCurvePoint(Street_seg_idx,0).latitude());
            arrowhead(x_coor,y_coor,x_coor2,y_coor2,Street_seg_idx, "from");             
        }
    }
    

}

// Data structure to set up the features of the map
void setting_up_features(){
    // Gets the number of features
    Feature_INFO.resize(getNumFeatures());
    // Loops through all of the features...
    for(int Feature_idx = 0; Feature_idx < getNumFeatures(); Feature_idx++){
        // gets the ID, name, feature points and type of feature
        Feature_INFO[Feature_idx].Feature_ID = getFeatureOSMID(Feature_idx);
        Feature_INFO[Feature_idx].Feature_name = getFeatureName(Feature_idx);
        Feature_INFO[Feature_idx].Feature_num_points = getNumFeaturePoints(Feature_idx);
        Feature_INFO[Feature_idx].Feature_type = getFeatureType(Feature_idx);
        // Looping through the number of feature points...
        for(int Feature_point_idx = 0; Feature_point_idx < getNumFeaturePoints(Feature_idx); Feature_point_idx++ ){
            // gets the feature points id, and point id and puts it into the struct.
            Feature_INFO[Feature_idx].Feature_points.push_back(getFeaturePoint(Feature_idx, Feature_point_idx));
            // gets the x and y coordinates of that feature point and puts it into the struct
            double x_coor = x_from_lon(getFeaturePoint(Feature_idx, Feature_point_idx).longitude());
            double y_coor = y_from_lat(getFeaturePoint(Feature_idx, Feature_point_idx).latitude());
            Feature_INFO[Feature_idx].Feature_points_xy.push_back(ezgl::point2d(x_coor,y_coor));
        }
        // If the number of feature points is more than 1, and the x and y coordinates is the same for the front and back of the vector...
        if((Feature_INFO[Feature_idx].Feature_points_xy.size() > 1) && (Feature_INFO[Feature_idx].Feature_points_xy.front() == Feature_INFO[Feature_idx].Feature_points_xy.back())){
            Feature_INFO[Feature_idx].area_closed = true;
        }
        else{
            Feature_INFO[Feature_idx].area_closed = false;
        }

        //setting up feature vectors
        if(Feature_INFO[Feature_idx].Feature_type == LAKE){
            lakes.push_back(Feature_INFO[Feature_idx]);
        }
        else if(Feature_INFO[Feature_idx].Feature_type == ISLAND){
            islands.push_back(Feature_INFO[Feature_idx]);
        }
        else if(Feature_INFO[Feature_idx].Feature_type == PARK){
            parks.push_back(Feature_INFO[Feature_idx]);
        }
        else if(Feature_INFO[Feature_idx].Feature_type == GREENSPACE){
            greenspaces.push_back(Feature_INFO[Feature_idx]);
        }
        else if(Feature_INFO[Feature_idx].Feature_type == GOLFCOURSE){
            golfCourses.push_back(Feature_INFO[Feature_idx]);
        }
        else if(Feature_INFO[Feature_idx].Feature_type == GLACIER){
            glaciers.push_back(Feature_INFO[Feature_idx]);
        }
        else if(Feature_INFO[Feature_idx].Feature_type == BEACH){
            beaches.push_back(Feature_INFO[Feature_idx]);
        }
        else if(Feature_INFO[Feature_idx].Feature_type == RIVER){
            rivers.push_back(Feature_INFO[Feature_idx]);
        }
        else if(Feature_INFO[Feature_idx].Feature_type == STREAM){
            streams.push_back(Feature_INFO[Feature_idx]);
        }
        else if(Feature_INFO[Feature_idx].Feature_type == BUILDING){
            buildings.push_back(Feature_INFO[Feature_idx]);
        }
        else{
            unknowns.push_back(Feature_INFO[Feature_idx]);
        }
    }
}

// Data structure to set up street intersection length
void setting_up_street_intersection_length(){
    street_seg_length.assign(getNumStreetSegments(),0);
    for(int r=0;r< getNumStreetSegments(); r++){
        // get coordinates of street segment intersections
        IntersectionIdx from = getStreetSegmentInfo(r).from;
        IntersectionIdx to = getStreetSegmentInfo(r).to;
        // get number of curve points
        int numCurves = getStreetSegmentInfo(r).numCurvePoints;
        double length = 0;
        double tempLength = 0;
        LatLon curveIdx;
        
        // get longitude and latitude of street segment intersections
        LatLon fromCo = getIntersectionPosition(from);
        LatLon toCo = getIntersectionPosition(to);
        // if there are no curve points...
        if (numCurves == 0){
            length = findDistanceBetweenTwoPoints(fromCo, toCo);
        } else {
            //for loop to add up distances between curve points
            for (int i = 0; i < numCurves - 1; i++){
                tempLength = findDistanceBetweenTwoPoints(getStreetSegmentCurvePoint(r, i), getStreetSegmentCurvePoint(r, i+1));
                length = length + tempLength;
            }
            // adds the distance from the start of an intersection to the closest curve point
            curveIdx = getStreetSegmentCurvePoint(r, 0);
            tempLength = findDistanceBetweenTwoPoints(fromCo, curveIdx);
            length = length + tempLength;
            // adds the distance from the start of an intersection to the closest curve point
            curveIdx = getStreetSegmentCurvePoint(r, numCurves - 1);
            tempLength = findDistanceBetweenTwoPoints(curveIdx, toCo);
            length = length + tempLength;
        }
        //Adds length of index r into the vector
        street_seg_length.at(r) = length;
    }
}

// Data structure for findStreetSegmentsOfIntersection
void setting_up_street_seg_of_intersection(){
    std::vector<StreetSegmentIdx> Empty = {};
    std::vector<StreetSegmentIdx> streetsegmentholder = {};
    //Initializing vector and placing empty vectors in the vector (Empty)
    street_segs.assign(getNumIntersections(), Empty);
    //looping through all intersections
    for(int i=0; i < getNumIntersections(); i++){
        //reseting streetsegmentholder to an empty vector 
        streetsegmentholder = Empty;
        //looping through all street segments connected to the intersection
        for(int j=0; j < getNumIntersectionStreetSegment(i); j++){
            //placing street segments into streetsegmentholder 
            streetsegmentholder.push_back(getIntersectionStreetSegment(i, j));
        }   
        //placing vector into data structure
        street_segs.at(i) = streetsegmentholder;
    }
}

// Data structure for find IntersectionsOfStreet
void setting_up_find_intersection_of_street(){
    std::vector<IntersectionIdx> Emptytwo = {};
    street_inter.assign(getNumIntersections(), Emptytwo);
    //Finds all of the intersections of a street, including the duplicates
    for (int i = 0; i < getNumStreetSegments(); i++){
        street_inter.at(getStreetSegmentInfo(i).streetID).push_back(getStreetSegmentInfo(i).from);
        street_inter.at(getStreetSegmentInfo(i).streetID).push_back(getStreetSegmentInfo(i).to);
    }
    // Loops through to remove all duplicate intersections
    for(int i=0; i<getNumStreets();i++){
        std::vector <IntersectionIdx> result = street_inter.at(i);
        std::set <IntersectionIdx> temp (result.begin(), result.end());
        result = std::vector <IntersectionIdx> (temp.begin(), temp.end());
        street_inter.at(i) = result;
    }
}

//Data Structure for getOSMNodeTagValue. Sets up an unordered map for each node
void setting_up_get_OSMNodeTagValue(){
    // Loops through the number of nodes and links id and node number...
    for( int i=0; i < getNumberOfNodes(); i++){
        OSMNodeMap.insert(std::make_pair(getNodeByIndex(i)->id(), i));
    }
    // Loops through the OSMWays...
    for(int i = 0; i < getNumberOfWays(); i++){\
        // Checks if the OSMWay is closed (1) and open (0), and pairs it with the OSMWay ID...
        if(isClosedWay(getWayByIndex(i))){
            OSMWay.insert(std::make_pair(getWayByIndex(i)->id(), 1));
        }
        else{
            // 
            OSMWay.insert(std::make_pair(getWayByIndex(i)->id(), 0));
        }
    }    
}

//Data structures for findStreetIDsFromPartialStreetName()
void setting_up_find_partial_street_names(){
    for(int i = 0; i < getNumStreets(); i++){
        std::string name = getStreetName(i);
        std::transform(name.begin(), name.end(),name.begin(), ::toupper);
        name = removeSpaces(name);
        //map if we are given one letter names
        singleLetterStreetName[name[0]].push_back(i);
        // // other map
        // partialStreetName[name[0]][name[1]].push_back(std::make_pair(name, i));
        partialStreetName[name].push_back(i);
    }
}
/*******************************Setting up Map Data Structures******************************/

/***************************** Loading & Closing Map***************************************/
/* loadMap will load all the .streets and .osm files from StreetsDatabaseAPI.h and 
   OSMDatabaseAPI.h, respectively. loadMap will also load any data structures that
   are created for the functions below. 

   Speed Requirement: None */
bool loadMap(std::string map_streets_database_filename) {
    std::cout << "loadMap: " << map_streets_database_filename << std::endl;
    
    // Checks if the .street file loaded successfully
    bool load_successful = loadStreetsDatabaseBIN(map_streets_database_filename);
    // Puts size of file into 'size'
    int size = map_streets_database_filename.size();
    
    // If .street file cannot be opened...
    if (load_successful != true){
        return false;
    } else {
        std::string OSMString = map_streets_database_filename;  //Puts file name into OSMString
        OSMString.replace(size - 11, 11, "osm.bin");    // Replaces .street.bin with osm.bin
        load_successful = loadOSMDatabaseBIN(OSMString);    //Checks if .osm file can be loaded in
    }

    //find the maximum and minimum latitude and longitude
    max_lat = getIntersectionPosition(0).latitude();
    min_lat = max_lat;
    max_lon = getIntersectionPosition(0).longitude();
    min_lon =  max_lon;
    intersections.resize(getNumIntersections());
    
    //setting intersection positions to correct lat and lon
    for (int id = 0; id < getNumIntersections(); id++){
        intersections[id].position = getIntersectionPosition(id);        
        max_lat = std::max(max_lat, intersections[id].position.latitude());
        min_lat = std::min(min_lat, intersections[id].position.latitude());
        max_lon = std::max(max_lon, intersections[id].position.longitude());
        min_lon = std::min(min_lon, intersections[id].position.longitude());
    }
    latitudeAverage = (max_lat + min_lat)/2;

    //Creating threads for different aspects of the map (intersections, POI, etc.)
    std::thread threadone(setting_up_intersections);
    std::thread threadtwo(setting_up_POI);
    std::thread threadthree(setting_up_street_seg);
    std::thread threadfour(setting_up_features);
    std::thread threadfive(setting_up_street_intersection_length);
    std::thread threadsix(setting_up_street_seg_of_intersection);
    std::thread threadseven(setting_up_find_intersection_of_street);
    std::thread threadeight(setting_up_get_OSMNodeTagValue);
    std::thread threadnine(setting_up_find_partial_street_names);

    //All threads are 'joined', so that it waits for other threads to finish running.
    threadone.join();
    threadtwo.join();
    threadthree.join();
    threadfour.join();
    threadfive.join();
    threadsix.join();
    threadseven.join();
    threadeight.join();
    threadnine.join();
    
    //Data Structure for findStreetSegmentTravelTime.
    street_segment_time.assign(getNumStreetSegments(),0);
    for(int r = 0; r < getNumStreetSegments(); r++){
        //Initializes variable time, and gets needed data
        double time = 0;
        double length = findStreetSegmentLength(r);
        float limit = getStreetSegmentInfo(r).speedLimit;
        double inf = std::numeric_limits<double>::infinity();
        if(limit == 0){
            street_segment_time.at(r) = inf;
        } else {
        //Calculation
        time = length/limit;
        street_segment_time.at(r) = time;
        }
    }

    //Data Structure for findStreetLength.
    streetlength.assign(getNumStreets(),0);
    for(int i=0;i< getNumStreetSegments(); i++){
        //Loops through all street segments and adds the length to the respective street
        streetlength.at(getStreetSegmentInfo(i).streetID) = streetlength.at(getStreetSegmentInfo(i).streetID) + findStreetSegmentLength(i);
    }
    
    //loops through all the OSM nodes
    for(int OSMnodenum = 0; OSMnodenum < getNumberOfNodes(); OSMnodenum++){
        //gets the OSMNode and the OSMNode ID
        const OSMNode *e = getNodeByIndex(OSMnodenum); 
        OSMID OSMNODEid = e->id();   
        //if the OSMNodeTag is a subway entrance...
        if(getOSMNodeTagValue(OSMNODEid, "railway") == "subway_entrance"){
            //gets the coordinates of that node
            double x_coor = x_from_lon(getNodeCoords(e).longitude());
            double y_coor =   y_from_lat(getNodeCoords(e).latitude());
            //adds the coordinates into the data structure
            Subway_INFO.push_back({x_coor,y_coor});
        }
        //if the OSMNodeTag is a library...
        if(getOSMNodeTagValue(OSMNODEid, "amenity") == "library"){
            //gets the coordinates of that node
            double x_coor = x_from_lon(getNodeCoords(e).longitude());
            double y_coor =   y_from_lat(getNodeCoords(e).latitude());
            //adds the coordinates into the data structure
            Library_INFO.push_back({x_coor,y_coor});
        } 
        //if the OSMNodeTag is a fast food restaurant...
        if(getOSMNodeTagValue(OSMNODEid, "amenity") == "fast_food"){
            //gets the coordinates of that node
            double x_coor = x_from_lon(getNodeCoords(e).longitude());
            double y_coor =   y_from_lat(getNodeCoords(e).latitude());
            //adds the coordinates into the data structure
            FastFood_INFO.push_back({x_coor,y_coor});
        }
        //if the OSMNodeTag is a restaurant...
        if(getOSMNodeTagValue(OSMNODEid, "amenity") == "restaurant"){
            //gets the coordinates of that node
            double x_coor = x_from_lon(getNodeCoords(e).longitude());
            double y_coor =   y_from_lat(getNodeCoords(e).latitude());
            //adds the coordinates into the data structure
            Restaurant_INFO.push_back({x_coor,y_coor});
        }

    }
    return load_successful;
}

/*Closes maps and data structures
  Speed Requirement: Moderate */
void closeMap() {
    //Clean-up your map related data structures here
    closeStreetDatabase();
    closeOSMDatabase();
    
    std::cout << "Closing map\n";
    
    //closing all the data structures we created
    std::vector<double>().swap(streetlength);
    std::vector<double>().swap(street_seg_length);
    std::vector<double>().swap(street_segment_time);
    std::vector<std::vector<StreetSegmentIdx>>().swap(street_segs);
    std::vector<std::vector< IntersectionIdx >>().swap(street_inter);
    std::unordered_map<OSMID, int>().swap(OSMNodeMap);
    std::unordered_map<OSMID, int>().swap(OSMWay);
    std::map<std::string, std::vector<StreetIdx>>().swap(partialStreetName);
    std::unordered_map<char, std::vector<StreetIdx>>().swap(singleLetterStreetName);
    /*
    intersections.clear();
    POI_INFO.clear();
    Street_segments_INFO.clear();
    Feature_INFO.clear();
    Subway_INFO.clear();
    */
    std::vector<intersection_data>().swap(intersections);
    std::vector <POI_data>().swap(POI_INFO);
    std::vector <Street_segment_data>().swap(Street_segments_INFO);
    std::vector <Feature_data>().swap(Feature_INFO);
    std::vector <ezgl::point2d>().swap(Subway_INFO);
    std::vector<ezgl::point2d>().swap(Library_INFO);
    std::vector<ezgl::point2d>().swap(FastFood_INFO);
    std::vector<ezgl::point2d>().swap(Restaurant_INFO);
    
    std::vector<Feature_data>().swap(lakes);
    std::vector<Feature_data>().swap(islands);
    std::vector<Feature_data>().swap(parks);
    std::vector<Feature_data>().swap(greenspaces);
    std::vector<Feature_data>().swap(golfCourses);
    std::vector<Feature_data>().swap(glaciers);
    std::vector<Feature_data>().swap(beaches);
    std::vector<Feature_data>().swap(rivers);
    std::vector<Feature_data>().swap(streams);
    std::vector<Feature_data>().swap(buildings);
    std::vector<Feature_data>().swap(unknowns);

    std::vector<StreetIdx>().swap(street_INFO);
    std::vector<int>().swap(prevHighlightedIntersections);

    centerStreet.clear();

    
    centerIntersection.clear();
    centerIntersection2.clear();


    Directions_to_2_from.clear();
    Directions_NESW.clear();
    street_seg_path.clear();    
}

/*****************************End Loading & Closing Map***********************************/


/**********************************Street Segments****************************************/
// Returns the distance between two (latitude,longitude) coordinates in meters
// Speed Requirement: Moderate
double findDistanceBetweenTwoPoints(LatLon point_1, LatLon point_2){
    double distance;
    double x1,x2,y1,y2 = 0;
    
    // Calculates latitude average and converts it to radians
    double latitudeAvg = ((point_1.latitude() + point_2.latitude())/2)*kDegreeToRadian;
    // Calculates longitude and latitude respectively and converts the coordinates to radians
    x1 = kEarthRadiusInMeters*point_1.longitude()*cos(latitudeAvg)*kDegreeToRadian;
    y1 = kEarthRadiusInMeters*point_1.latitude()*kDegreeToRadian;
    x2 = kEarthRadiusInMeters*point_2.longitude()*cos(latitudeAvg)*kDegreeToRadian;
    y2 = kEarthRadiusInMeters*point_2.latitude()*kDegreeToRadian;
    // Calculates the total distance between 2 points
    distance = sqrt(pow((y2 - y1), 2) + pow((x2-x1),2));
    
    return distance;
}

// Returns the length of the given street segment in meters.
//Information Loaded from the street_seg_length data structure
// Speed Requirement: Moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    return street_seg_length.at(street_segment_id);
}
// Returns the travel time to drive from one end of a street segment 
// to the other, in seconds, when driving at the speed limit
// Information Loaded from the street_segment_time data structure
// Speed Requirement: High 
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    return street_segment_time.at(street_segment_id);
}

/*******************************End Street Segments*************************************/


/**********************************Intersections***************************************/
/*Returns all intersections reachable by traveling down one street segment 
  from the given intersection.
  Some Information Loaded from the street_segs data structure
  Speed Requirement: High */
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id){
    int numStreetSegments = getNumIntersectionStreetSegment(intersection_id);
    //vector containing street segments connected to intersection
    std::vector<StreetSegmentIdx> streetSegments = findStreetSegmentsOfIntersection(intersection_id);
    std::vector<IntersectionIdx> adjacentIntersections;

    for (int i = 0; i < numStreetSegments; i++){
        //if street segment is 1 way, add the intersection ID of "to" into the vector if it isn't the original intersection
        if(getStreetSegmentInfo(streetSegments[i]).oneWay && getStreetSegmentInfo(streetSegments[i]).to != intersection_id){
            adjacentIntersections.push_back(getStreetSegmentInfo(streetSegments[i]).to);
        }
        //if not 1 way, add the next intersection to a vector list
        else if(getStreetSegmentInfo(streetSegments[i]).oneWay == false ){
            if(getStreetSegmentInfo(streetSegments[i]).to == intersection_id){
                adjacentIntersections.push_back(getStreetSegmentInfo(streetSegments[i]).from);
            }
            else{
                adjacentIntersections.push_back(getStreetSegmentInfo(streetSegments[i]).to);
            }
        }
    }
    std::vector<IntersectionIdx> newVect = RemoveDuplicate(adjacentIntersections);
    return newVect;
}

/*Loops through all intersections and calculates the distance using findDistanceBetweenTwoPoints()
  Keeps track and returns the shortest distance
  Speed Requirement: None */
IntersectionIdx findClosestIntersection(LatLon my_position){
    int numIntersections = getNumIntersections();
    //set initial closest intersection to be first intersection
    IntersectionIdx closest = 0;
    int distance = findDistanceBetweenTwoPoints(getIntersectionPosition(0), my_position);
    
    //loop through all intersections
    for(int i = 1; i < numIntersections; i++){
        //if an intersection is closer, make that the closest
        if(findDistanceBetweenTwoPoints(getIntersectionPosition(i), my_position) < distance){
            closest = i;
            distance = findDistanceBetweenTwoPoints(getIntersectionPosition(i), my_position);
        }
    }
    return closest;
}

/* Returns the street segments that connect to the given intersection 
   Information Loaded from the street_segs data structure
   Speed Requirement: High */
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){
    return street_segs.at(intersection_id);
}

/* Returns all intersections along the given street.
   Information Loaded from the street_inter data structure
   Speed Requirement: High*/
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
    return street_inter.at(street_id);
}

/* Return all intersection ids at which the two given streets intersect
   This function will typically return one intersection id for streets
   that intersect and a length 0 vector for streets that do not. For unusual 
   curved streets it is possible to have more than one intersection at which 
   two streets cross.
   Information Loaded from the street_inter data structure
   Speed Requirement: High */
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(StreetIdx street_id1, StreetIdx street_id2){
    std::vector<IntersectionIdx>vect1;
    std::vector<IntersectionIdx>vect2;
    std::vector<IntersectionIdx>result;
    
    //gets all intersections of each street
    vect1 = findIntersectionsOfStreet(street_id1);
    vect2 = findIntersectionsOfStreet(street_id2);
    
    //compares the intersections and only keeps the ones in common in between them
    std::set_intersection(vect1.begin(), vect1.end(), vect2.begin(), vect2.end(), std::back_inserter(result));
    return result;
}

/*********************************End Intersections*************************************/


/*************************************Streets******************************************/
/* Returns all street ids corresponding to street names that start with the given prefix
   Information Loaded from the singleLetterStreetName and partialStreetName data structure
   Speed Requirement: High*/
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){
    std::vector<StreetIdx> streetNames;
    //checks if prefix is empty
    if(street_prefix.length()==0){
        return streetNames;
    }

    //puts prefix in all caps
    std::string upper_street_prefix = street_prefix;
    std::transform(upper_street_prefix.begin(), upper_street_prefix.end(),upper_street_prefix.begin(), ::toupper);
    upper_street_prefix = removeSpaces(upper_street_prefix);

    if(upper_street_prefix.length() == 1){
        //if single letter doesn't exist
        if(singleLetterStreetName.find(upper_street_prefix[0]) == singleLetterStreetName.end()){
            return streetNames;
        }
        //returns vector pertaining to a single character
        streetNames = singleLetterStreetName.at(upper_street_prefix[0]);
        return (streetNames);
    }

    //iterator, set to lowest bound for partial street map matching prefix given
    auto it = partialStreetName.lower_bound(upper_street_prefix);

    //for loop that continues until at end of partial street map and cannot find matching prefix
    for(; (it->first).find(upper_street_prefix) == 0 && it != partialStreetName.end(); it++){
        //append second value in partial street map to end of vector
        streetNames.insert(streetNames.end(), (it->second).begin(), (it->second).end());
    }

    return streetNames;
}

/* Returns the length of a given street in meters
   Information Loaded from the streetlength data structure
   Speed Requirement: High */
double findStreetLength(StreetIdx street_id){
    return streetlength.at(street_id);
}

/* Returns the nearest point of interest of the given type (e.g. "restaurant") 
   to the given position
   Speed Requirement: None */
POIIdx findClosestPOI(LatLon my_position, std::string POItype){
    bool first_found = false;
    double closest_distance = 0;
    double current_dis = 0;
    POIIdx closest_type = 0;

    //loops though all the POI
    for(int i=0; i < getNumPointsOfInterest(); i++){
        //if this is the first POI found with proper type
        if(first_found == false && POItype == getPOIType(i)){
            //set closest found to the first one found
            closest_distance = findDistanceBetweenTwoPoints(my_position , getPOIPosition(i));
            closest_type = i;
            first_found = true;
        }
        //if first is already found and new POI is found chack type and distance, if closer set new closest to POI found
        else if(POItype == getPOIType(i)){
            //finds distance of current POI
            current_dis = findDistanceBetweenTwoPoints(my_position , getPOIPosition(i));
            //compares the current distance to the previous found largest distance
            if(current_dis < closest_distance){
                closest_distance = current_dis;
                closest_type = i;
            }
        }      
    }
    return closest_type;
}

/*************************************End Street***************************************/


/*****************************************OSM******************************************/
/*Returns the area of the given closed feature in square meters
  Information Loaded from the OSMWay data structure
  Speed Requirement: Moderate */
double findFeatureArea(FeatureIdx feature_id){
    std::vector<LatLon> allpoints;
    LatLon reference_point;
    double totalarea = 0;
    double currentarea = 0;

    //converting feature id into a OSMid
    TypedOSMID featureid = getFeatureOSMID(feature_id);
    
    //uses the data structure created in loadMap(). Explanation of the unordered map in loadMap() but TLDR key-->0 means the polygon is open
    if (OSMWay.find(featureid) != OSMWay.end()){
        //finds the value loaded at a certain key. If the value is 0, the polygon is open -> return 0
        if(OSMWay[featureid] == 0){
            return 0;
        }
    }
    //add all the points in the feature into a vector
    for(int j=0; j < getNumFeaturePoints(feature_id); j++){
        allpoints.push_back(getFeaturePoint(feature_id, j));
    }
    //Uses the first point as a reference point when calculating feature area
    reference_point = allpoints.at(0);

    //loop through all the points in the feature
    for(std::vector<LatLon>::iterator iter = allpoints.begin(); iter+1 < allpoints.end(); iter++ ){
        //the complicated formula below is just the trapezoid formula
        currentarea = (findDistance_in_x_dir(*iter, reference_point) + findDistance_in_x_dir(*(iter+1),reference_point))*0.5* (findDistance_in_y_dir(*iter,  *(iter+1)));
        totalarea = totalarea + currentarea;
    }
    //calculate trapezoid for the last point and first point
    currentarea = (findDistance_in_x_dir(allpoints.back(), reference_point) + findDistance_in_x_dir(allpoints.front(), reference_point))*0.5* (findDistance_in_y_dir(allpoints.front(),  allpoints.back()));
    totalarea = totalarea + currentarea;
    //ensures the area is positive since it can sometimes return negative
    totalarea= abs(totalarea);
    return totalarea;
}

/*Return the value associated with this key on the specified OSMNode.
  Information Loaded from the OSMNodeMap data structure
  Speed Requirement: High*/
std::string getOSMNodeTagValue (OSMID OSMid, std::string key){
    int index =0;
    //search the ordered map with a key. If key is found, return value(Node Index Number)
    if (OSMNodeMap.find(OSMid) != OSMNodeMap.end()){
        index = OSMNodeMap[OSMid];
    }
    //if key is not found, return empty string
    else{
        return "";
    }
    //Loop through all the tags at the node
    for(int j=0;j<getTagCount(getNodeByIndex(index)); j++){
        //if we find matching key, return value
        if(getTagPair(getNodeByIndex(index), j).first == key){
            return getTagPair(getNodeByIndex(index), j).second;
        }
    }
    //if key isn't found
    return "";
}
/***************************************End OSM****************************************/