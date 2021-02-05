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

/*
 * m1.cpp Declaration Menu
 * Function  1.1: vector<IntersectionIdx> findAdjacentInters(IntersectionIdx intersection_id);
 *          1.2: vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id);
 *          1.3: vector<string> findStreetNamesOfIntersection();
 *
 *          2.1: LatLonBounds findStreetBoundingBox(StreetIdx street_id);
 *          2.2: vector<IntersectionIdx> findIntersectionsOfTwoStreets(pair<StreetIdx,StreetIdx> street_ids);
 *          2.3: vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id);
 *          2.4  vector<StreetIdx> findStreetIdsFromPartialStreetName(string street_prefix);
 *
 *          3.1: double findDistanceBetweenTwoPoints(pair<LatLon, LatLon> points);
 *          3.2: double findStreetSegmentLength (StreetSegmentIdx street_segment_id);
 *          3.3: double findStreetLength (StreetIdx street_id);
 *          3.4: double findStreetSegmentTravelTime (StreetSegmentIdx street_segment_id);
 *          3.5: double findFeatureArea (FeatureIdx feature_id);
 *
 *          4.1: IntersectionIdx findClosestIntersection(LatLon my_position);
 *          4.2:POIIdx findClosestPOI(LatLon my_position, string POIname);
 * DataStructure
 *      Structure 1: vector <vector<StreetSegmentIdx>> intersectListOfStreetSegs;
 *
 *      Structure 2: vector<StreetInfo> StreetInfoList;
 *                      struct StreetInfo{
 *                          vector<StreetSegmentIdx> StreetInfoOfStreetSegsList;
 *                          set<IntersectionIdx> StreetInfoOfIntersectsList;
 *                      };
 *
 *      Structure 3: vector<vector<vector<IntersectionIdx>>> StreetXStreetIntersectsList;
 *
 *      Structure 4: struct StreetNameTree{CharNode root;};
 *                      struct CharNode{
 *                          std::vector<StreetIdx> curPrefixStreetsList;
 *                          CharNode* nextChar[256];
 *                      };
 */


/*Global Structure Define Begin*/

/**
 * Structure 1
 * <br> IntersectionList of StreetSegments [Direct Func: 1.1]
 */
std::vector <std::vector<StreetSegmentIdx>> intersectListOfStreetSegs;




/**
 * Sub Structure 2
 * <br> StreetInformation contains Segment vector & Intersection Set
 */
struct StreetInfo{
    std::vector<StreetSegmentIdx> StreetInfoOfStreetSegsList;
    std::set<IntersectionIdx> StreetInfoOfIntersectsList;
};
/**
 * Structure 2
 * <br> StreetInformation Vector of all streets [Func: 2.3 & 3.3]
 */
std::vector<StreetInfo> StreetInfoList;

//streetTreeOfStreetName
/**
 * Structure 3
 * <br> StreetIndex X StreetIndex double Array (Special) [Func: 2.2]
 */
std::vector<std::vector<std::vector<IntersectionIdx>>> StreetXStreetIntersectsList;

//
/**
 * Structure 4: 256CharNodeTree (Special) [Func: 2.4]
 * <br> Freed during Close Map
 */
struct CharNode{
    std::vector<StreetIdx> curPrefixStreetsList;
    CharNode* nextChar[256];
};
struct StreetNameTree{
    CharNode root;
};
/*Global Structure Define End*/

/*Global Structure Load Helper Begin*/
/**
 * Load Map Helper
 * <br> Construct DataStructure 1 IntersectionList of StreetSegments
 * @return build Successful boolean type, false if empty List
 */
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

/*Global Structure Load Helper End*/

/* Other Helper Begin */
template<typename Type>
/**
 * DataStructure Helper Function:<br>
 * Transfer Set into vector using copy
 * @tparam Type
 * @param source Set<Type>
 * @return destination Vector<Type>
 */
std::vector<Type> SetToVecTransferHelper(const std::set<Type> & srcSet){
    std::vector<Type> destVec(srcSet.size());
    std::copy(srcSet.begin(), srcSet.end(), destVec.begin());
    return destVec;
}

/* Other Helper End */

/**
 * LoadMap Function: <br>
 * loadMap will be called with the name of the file that stores the "layer-2"
 * map data accessed through StreetsDatabaseAPI.
 * Call loadStreetsDatabaseBIN with this filename to initialize the
 * layer 2 (StreetsDatabase) API.<br>
 * <br> If need data from the lower layer 1 (OSM Data) you will also need to
 * initialize the layer 1 OSMDatabaseAPI by calling loadOSMDatabaseBIN.
 * @param map_streets_database_filename (string)
 * @return loadMap Successful (bool)
 */
bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = false; //Indicates whether the map has loaded 
                                  //successfully

    std::cout << "loadMap: " << map_streets_database_filename << std::endl;

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
 * Function 1.1: <br>
 * Returns all intersections reachable by traveling down one street segment
 * from the given intersection (hint: you can't travel the wrong way on a
 * 1-way street) <br>
 * <br> Speed Requirement --> high
 * @param intersection_id
 * @return List Of Adjacent IntersectionIndex
 */
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id){

    //Declare AdjIntersection List
    std::set<IntersectionIdx> adjIntersectList;

    //Find SegmentNumber Exist in current intersection
    int SegsNum = intersectListOfStreetSegs[intersection_id].size();

    //Loop through StreetSegs of current intersection
    for(int curSegNum=0; curSegNum < SegsNum; curSegNum++) {

        //Save current SegInfo
        int curSegIdx = intersectListOfStreetSegs[intersection_id][curSegNum];
        StreetSegmentInfo curSegInfo = getStreetSegmentInfo(curSegIdx);
        IntersectionIdx idFrom = curSegInfo.from;
        IntersectionIdx idTo = curSegInfo.to;

        //Determine Segment OneWay
        if(curSegInfo.oneWay == false) {
            //Save id Differ of current intersection
            if (intersection_id == idFrom) {
                adjIntersectList.insert(idTo);
            } else if (intersection_id == idTo) {
                adjIntersectList.insert(idFrom);
            }
        }
        else{
            //Save if idFrom is current intersection
            if(intersection_id == idFrom){
                adjIntersectList.insert(idTo);
            }
        }
    }
    return SetToVecTransferHelper(adjIntersectList);
}

/**
 * Function 1.2: <br>
 * Returns the street segments that connect to the given intersection <br>
 * <br> Speed Requirement --> high
 * @param checking specific intersection_id
 * @return List Of StreetSegmentIndex of Specific Intersection
 */
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){
    return intersectListOfStreetSegs[intersection_id];
}


/**
 * Function 1.3: <br>
 * Returns the street names at the given intersection (includes duplicate
 * street names in the returned vector) <br>
 * <br> Speed Requirement --> high
 * @param intersection_id
 * @return
 */
std::vector<std::string> findStreetNamesOfIntersection(IntersectionIdx intersection_id){
    return {};
}

/**
 * Function 2.1:
 * Tranverse Through Intersection Belong to Specific Street and Save
 * MaxLat, MaxLon, MinLat, MinLon value into data Type LatLonBounds.<br>
 * <br> Speed Requirement --> none
 * @param street_id
 * @return Return an Retangular Area LatLonBounds that consist of
 * MinLatLon & MaxLatLon
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


