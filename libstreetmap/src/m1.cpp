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
/**
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
std::vector <std::vector<StreetSegmentIdx>> intersectListOfStreetSegs;

//intersectListOfStreetSegs
//streetListOfStreetSegs
//StreetListOfIntersects
//streetXStreetOfIntersects
//streetTreeOfStreetName

bool LoadHelperIntersectListOfStreetSegs(){

    //Set Size of intersectList
    intersectListOfStreetSegs.resize(getNumIntersections());

    //Loop through All intersections (column)
    for (int curIntersect = 0; curIntersect < getNumIntersections(); curIntersect++) {

        //Loop through individual intersection's all connected StreetSegs
        for (int curSegNum = 0; curSegNum < getNumIntersectionStreetSegment(curIntersect); curSegNum++) {

            //Using DataBaseAPI func getIntersectStreetSeg(IntersectIdx, SegNum)
            //push into intersectionListOfStreetSegs
            intersectListOfStreetSegs[curIntersect].push_back(getIntersectionStreetSegment(curIntersect, curSegNum));
        }
    }

    //Check Structure Created
    if(intersectListOfStreetSegs.empty()) {
        return false;
    }
    else{
        return true;
    }
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
    // Load StreetDataBase Structure from BIN
    load_successful = loadStreetsDatabaseBIN(map_streets_database_filename);
    if(!load_successful) return false;

    // Load IntersectListOfStreetSegs
    load_successful = LoadHelperIntersectListOfStreetSegs();
    if(!load_successful) return false;


    load_successful = true; //Make sure this is updated to reflect whether
                            //loading the map succeeded or failed

    return load_successful;
}

void closeMap() {
    //Clean-up your map related data structures here
    
}
/**
 * Function 1.1
 * <br> Returns all intersections reachable by traveling down one street segment
 * <br> from the given intersection (hint: you can't travel the wrong way on a
 * <br> 1-way street)
 * <br> the returned vector should NOT contain duplicate intersections
 * <br> Speed Requirement --> high
 * @param intersection_id
 * @return
 */
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id){
    return {};
}

/**
 * Function 1.2
 * <br> Returns the street segments that connect to the given intersection
 * <br> Speed Requirement --> high
 * @param intersection_id
 * @return
 */
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){
    return intersectListOfStreetSegs[intersection_id];
}


/**
 * Function 1.3
 * <br> Returns the street names at the given intersection (includes duplicate
 * <br> street names in the returned vector)
 * <br> Speed Requirement --> high
 * @param intersection_id
 * @return
 */
std::vector<std::string> findStreetNamesOfIntersection(IntersectionIdx intersection_id){
    return {};
}

/**
 * Function 2.1
 * <br> Return the smallest rectangle that contains all the intersections and
 * <br> curve points of the given street (i.e. the min,max lattitude
 * <br> and longitude bounds that can just contain all points of the street).
 * <br> Speed Requirement --> none
 * @param street_id
 * @return
 */
LatLonBounds findStreetBoundingBox(StreetIdx street_id){
    LatLonBounds empty;
    return empty;
}

/**
 * Function 2.2
 * <br> Return all intersection ids at which the two given streets intersect
 * <br> This function will typically return one intersection id for streets
 * <br> that intersect and a length 0 vector for streets that do not. For unusual
 * <br> curved streets it is possible to have more than one intersection at which
 * <br> two streets cross.
 * <br> Speed Requirement --> high
 * @param street_ids
 * @return
 */
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(std::pair<StreetIdx, StreetIdx> street_ids){
    return {};
}

/**
 * Function 2.3
 * <br> Returns all intersections along the a given street
 * <br> Speed Requirement --> high
 * @param street_id
 * @return
 */
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
    return {};
}

/**
 * Function 2.4
 * <br> Returns all street ids corresponding to street names that start with the
 * <br> given prefix
 * <br> The function should be case-insensitive to the street prefix.
 * <br> The function should ignore spaces.
 * <br> For example, both "bloor " and "BloOrst" are prefixes to
 * <br> "Bloor Street East".
 * <br> If no street names match the given prefix, this routine returns an empty
 * <br> (length 0) vector.
 * <br> You can choose what to return if the street prefix passed in is an empty
 * <br> (length 0) string, but your program must not crash if street_prefix is a
 * <br> length 0 string.
 * <br> Speed Requirement --> high
 * @param street_prefix
 * @return
 */
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){
    return {};
}

/**
 * Function: 3.1
 * <br> Returns the distance between two (lattitude,longitude) coordinates in meters
 * <br> Speed Requirement --> Moderate
 * @param Pass in "From, To" position in LatLon pair
 * @return Distance in double
*/
double findDistanceBetweenTwoPoints(std::pair<LatLon, LatLon> points){
    return 0;
}
/**
 * Function 3.2
 * <br> Returns the length of the given street segment in meters Speed Requirement --> moderate
 * <br> Speed Requirement -->Moderate
 * <br> @param street_segment_id
 * @return SegmentLength
 */
double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    return 0;
}

/**
 * Function 3.3
 * <br> Returns the length of a given street in meters
 * <br> Speed Requirement --> high
 * @param street_id
 * @return
 */
double findStreetLength(StreetIdx street_id){
    return 0;
}

/**
 * Function 3.4
 * <br> Returns the travel time to drive from one end of a street segment in
 * <br> to the other, in seconds, when driving at the speed limit
 * <br> Note: (time = distance/speed_limit)
 * <br> Speed Requirement --> high
 * @param street_segment_id
 * @return
 */
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    return 0;
}

/**
 * Function 3.5
 * <br> Returns the area of the given closed feature in square meters
 * <br> Assume a non self-intersecting polygon (i.e. no holes)
 * <br> Return 0 if this feature is not a closed polygon.
 * <br> Speed Requirement --> moderate
 * @param feature_id
 * @return AreaSize in double
 */
double findFeatureArea(FeatureIdx feature_id){
    return 0;
}

/**
 * Function 4.1
 * <br> Returns the nearest intersection to the given position
 * <br> Speed Requirement --> none
 * @param my_position
 * @return
 */
IntersectionIdx findClosestIntersection(LatLon my_position){
    return 0;
}

/**
 * Function 4.2
 * <br> Returns the nearest point of interest of the given name to the given position
 * <br> Speed Requirement --> none
 * @param my_position
 * @param POIname
 * @return closest POIIdx
 */
POIIdx findClosestPOI(LatLon my_position, std::string POIname){
    return 0;
}


