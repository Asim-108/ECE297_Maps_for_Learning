/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file_header.cc to edit this template
 */

/* 
 * File:   newFile.cpp
 * Author: soonghiu
 * 
 * Created on February 10, 2023, 4:18 p.m.
 */

#include "helper.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <utility>
#include <vector>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

//used to see if an intersection is repeated twice in the vector
bool duplicate(const std::vector<IntersectionIdx> &vect, IntersectionIdx intersec){
    for (int i = 0; i < vect.size(); i++){
        if (vect.at(i) == intersec)
            return true;
    }
    return false;
}

//uses duplicate() to see if there is a duplicate element and removes it
std::vector<IntersectionIdx> RemoveDuplicate(const std::vector<IntersectionIdx> &vect){
    std::vector<IntersectionIdx> newVect;
    for (int i = 0; i < vect.size(); i++){
        if (duplicate(newVect, vect.at(i)) == false)
            newVect.push_back(vect.at(i));
    }
    return newVect;
}


//checks if a vector has 2 of the same elements in it
bool duplicateStreet(const std::vector<StreetIdx> &vect, StreetIdx street){
    for (int i = 0; i < vect.size(); i++){
        if (vect.at(i) == street)
            return true;
    }
    return false;
}

//Uses the duplicateStreet() function to find and remove duplicate elements. 
std::vector<StreetIdx> removeDuplicateStreets(const std::vector<StreetIdx> &vect){
    std::vector<StreetIdx> newVect;
    for (int i = 0; i < vect.size(); i++){
        if (duplicateStreet(newVect, vect.at(i)) == false)
            newVect.push_back(vect.at(i));
    }
    return newVect;
}

//helper functions to find distances between 2 points in x direction specifically for findFeatureArea() function
double findDistance_in_x_dir(LatLon point_1, LatLon point_2){
    double distance;
    double x1,x2= 0;
    double latitudeAverage = cos(((point_1.latitude() + point_2.latitude())/2)*kDegreeToRadian)*kDegreeToRadian;
    x1 = kEarthRadiusInMeters*point_1.longitude()*latitudeAverage;
    x2 = kEarthRadiusInMeters*point_2.longitude()*latitudeAverage;
    distance = x1-x2;
    return distance;
}

//helper functions to find distances between 2 points in x direction specifically for findFeatureArea() function
double findDistance_in_y_dir(LatLon point_1, LatLon point_2){
    double distance;
    double y1,y2 = 0;
    y1 = kEarthRadiusInMeters*point_1.latitude()*kDegreeToRadian;
    y2 = kEarthRadiusInMeters*point_2.latitude()*kDegreeToRadian;
    distance = y2-y1;
    return distance;
}

//this helper function removes all spaces in a string
std::string removeSpaces(std::string input){
  input.erase(std::remove(input.begin(),input.end(),' '),input.end());
  return input;
}
