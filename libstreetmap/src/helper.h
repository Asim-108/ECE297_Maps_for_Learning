/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file_header.h to edit this template
 */

/* 
 * File:   newFile.h
 * Author: soonghiu
 *
 * Created on February 10, 2023, 4:18 p.m.
 */
#pragma once

#ifndef HELPER_H
#define HELPER_H
#include <vector>
#include <string>
#include <utility>
#include <math.h>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "m1.h"

//used to see if an intersection is repeated twice in the vector
bool duplicate(const std::vector<IntersectionIdx> &vect, IntersectionIdx intersec);

//uses duplicate() to see if there is a duplicate element and removes it
std::vector<IntersectionIdx> RemoveDuplicate(const std::vector<IntersectionIdx> &vect);

//used to see if a street is repeated twice in a vector
bool duplicateStreet(const std::vector<StreetIdx> &vect, StreetIdx street);

//remove duplicate streets in a vector
//This removes the duplicates completely. {1,2,2,3,4} -> {1,3,4}
std::vector<StreetIdx> removeDuplicateStreets(const std::vector<StreetIdx> &vect);

//helper functions to find distances between 2 points in x direction specifically for findFeatureArea() function
double findDistance_in_x_dir(LatLon point_1, LatLon point_2);

//helper functions to find distances between 2 points in x direction specifically for findFeatureArea() function
double findDistance_in_y_dir(LatLon point_1, LatLon point_2);

//helper function to remove all spaces in a given string
std::string removeSpaces(std::string input);
#endif /* NEWFILE_H */
