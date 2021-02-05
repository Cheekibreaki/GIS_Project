/* 
 * Copyright 2021 University of Toronto
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
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include <vector>
#include <set>

/***** Function Naming Begin *****/
/*
Function    1.1: std::vector<IntersectionIdx> findAdjacentInters(IntersectionIdx intersection_id);
            1.2: std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id);
            1.3: std::vector<std::string> findStreetNamesOfIntersection();

            2.1: LatLonBounds findStreetBoundingBox(StreetIdx street_id);
            2.2: std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(std::pair<StreetIdx, StreetIdx> street_ids);
            2.3: std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id);
            2.4 std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix);

            3.1: double findDistanceBetweenTwoPoints(std::pair<LatLon, LatLon> points);
            3.2: double findStreetSegmentLength (StreetSegmentIdx street_segment_id);
            3.3: double findStreetLength (StreetIdx street_id);
            3.4: double findStreetSegmentTravelTime (StreetSegmentIdx street_segment_id);
            3.5: double findFeatureArea (FeatureIdx feature_id);

            4.1: IntersectionIdx findClosestIntersection(LatLon my_position);
            4.2:POIIdx findClosestPOI(LatLon my_position, std::string POIname);
 */
/***** Function Naming End *****/


/***** Global Structure Define Begin   *****/

//Structure 1: IntersectionIndex -> StreetSegmentIndex[Func: 1.1]
std::vector <std::vector<StreetSegmentIdx>> intersectionStreetSegments;

bool IntersectonToMultiseg(bool load_successful){
if(load_successful==true) {

    load_successful=false;
    intersectionStreetSegments.resize(getNumIntersections());

    for (int curSegmentNumber = 0; curSegmentNumber < getNumIntersections(); curSegmentNumber++) {
        for (int curSegmentNumber = 0;
             curSegmentNumber < getNumIntersectionStreetSegment(curSegmentNumber); curSegmentNumber++) {
            StreetSegmentIdx streetSegmentIdx = getIntersectionStreetSegment(curSegmentNumber, curSegmentNumber);
            intersectionStreetSegments[curSegmentNumber].push_back(streetSegmentIdx);
        }

    }
    load_successful=true;
}
return load_successful;
}

//Structure 2: StreetIndex -> IntersectionIndex & StreetSegmentIndex [Func: 2.3 & 3.3]

//Structure 3: StreetIndex X StreetIndex double Array (Special) [Func: 2.2]

//Structure 4: 256CharNodeTree(Special) [Func: 2.4]

/***** Global Structure Define End     *****/


// loadMap will be called with the name of the file that stores the "layer-2"
// map data accessed through StreetsDatabaseAPI: the street and intersection 
// data that is higher-level than the raw OSM data). 
// This file name will always end in ".streets.bin" and you 
// can call loadStreetsDatabaseBIN with this filename to initialize the
// layer 2 (StreetsDatabase) API.
// If you need data from the lower level, layer 1, API that provides raw OSM
// data (nodes, ways, etc.) you will also need to initialize the layer 1 
// OSMDatabaseAPI by calling loadOSMDatabaseBIN. That function needs the 
// name of the ".osm.bin" file that matches your map -- just change 
// ".streets" to ".osm" in the map_streets_database_filename to get the proper
// name.
bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = false; //Indicates whether the map has loaded 
                                  //successfully

    std::cout << "loadMap: " << map_streets_database_filename << std::endl;

    //
    // Load your map related data structures here.
    //
    load_successful=IntersectonToMultiseg( load_successful);
    

    load_successful = true; //Make sure this is updated to reflect whether
                            //loading the map succeeded or failed

    return load_successful;
}

void closeMap() {
    //Clean-up your map related data structures here
    
}
// Returns the distance between two (lattitude,longitude) coordinates in meters
// Speed Requirement --> moderate
double findDistanceBetweenTwoPoints(std::pair<LatLon, LatLon> points){
    return 0;
}

// Returns the length of the given street segment in meters
// Speed Requirement --> moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    return 0;
}

// Returns the travel time to drive from one end of a street segment in
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    return 0;
}

// Returns the nearest intersection to the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position){
    return 0;
}

// Returns the street segments that connect to the given intersection
// Speed Requirement --> high
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){
    return {};
}

// Returns the street names at the given intersection (includes duplicate
// street names in the returned vector)
// Speed Requirement --> high
std::vector<std::string> findStreetNamesOfIntersection(IntersectionIdx intersection_id){
    return {};
}

// Returns all intersections reachable by traveling down one street segment
// from the given intersection (hint: you can't travel the wrong way on a
// 1-way street)
// the returned vector should NOT contain duplicate intersections
// Speed Requirement --> high
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id){
    return {};
}

// Returns all intersections along the a given street
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
    return {};
}

// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual
// curved streets it is possible to have more than one intersection at which
// two streets cross.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(std::pair<StreetIdx, StreetIdx> street_ids){
    return {};
}

// Returns all street ids corresponding to street names that start with the
// given prefix
// The function should be case-insensitive to the street prefix.
// The function should ignore spaces.
//  For example, both "bloor " and "BloOrst" are prefixes to
// "Bloor Street East".
// If no street names match the given prefix, this routine returns an empty
// (length 0) vector.
// You can choose what to return if the street prefix passed in is an empty
// (length 0) string, but your program must not crash if street_prefix is a
// length 0 string.
// Speed Requirement --> high
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){
    return {};
}

// Returns the length of a given street in meters
// Speed Requirement --> high
double findStreetLength(StreetIdx street_id){
    return 0;
}

// Return the smallest rectangle that contains all the intersections and
// curve points of the given street (i.e. the min,max lattitude
// and longitude bounds that can just contain all points of the street).
// Speed Requirement --> none
LatLonBounds findStreetBoundingBox(StreetIdx street_id){
    LatLonBounds empty;
    return empty;
}

// Returns the nearest point of interest of the given name to the given position
// Speed Requirement --> none
POIIdx findClosestPOI(LatLon my_position, std::string POIname){
    return 0;
}

// Returns the area of the given closed feature in square meters
// Assume a non self-intersecting polygon (i.e. no holes)
// Return 0 if this feature is not a closed polygon.
// Speed Requirement --> moderate
double findFeatureArea(FeatureIdx feature_id){
    return 0;
}
