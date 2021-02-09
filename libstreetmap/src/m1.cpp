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
#include <unordered_set>
#include <algorithm>
#include <cmath>

using namespace std;
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
 *      Structure 1: vector <vector<StreetSegmentIdx>> IntersectListOfStreetSegs; (Func 1.2)
 *
 *      Structure 2: vector<StreetInfo> StreetInfoList;
 *                      struct StreetInfo{
 *                          vector<StreetSegmentIdx> StreetInfoOfStreetSegsList;            (Func 2.3)
 *                          set<IntersectionIdx> StreetInfoOfIntersectsList;                (Func 3.3)
 *                      };
 *
 *      Structure 3: vector<vector<vector<IntersectionIdx>>> StreetXStreetIntersectsList;   (Func 2.2)
 *
 *      Structure 4: struct StreetNameTree{CharNode root;};                                 (Func 2.4)
 *                      struct CharNode{
 *                          vector<StreetIdx> curPrefixStreetsList;
 *                          CharNode* nextChar[256];
 *                      };
 */


/*Global Structure Define Begin*/

/**
 * Structure 1
 * <br> IntersectionList of StreetSegments [Direct Func: 1.1]
 */
std::vector <std::vector<StreetSegmentIdx>> IntersectListOfStreetSegs;
vector<std::pair<bool,vector<std::string>>> IntersectionListOfStreetName;
/**
 * Load all streetSegs of one intersection in to intersection list
 */
void LoadStructure1(){
    IntersectListOfStreetSegs.resize(getNumIntersections());
    for (int curIntersect = 0; curIntersect < getNumIntersections(); curIntersect++) {
        for (int segNum = 0; segNum < getNumIntersectionStreetSegment(curIntersect); segNum++) {
            IntersectListOfStreetSegs[curIntersect].push_back(getIntersectionStreetSegment(curIntersect, segNum));
        }
    }
}



/**
 * Structure 2
 * <br> StreetInformation contains Segment vector & Intersection Set
 */
std::vector<std::vector<StreetSegmentIdx>> StreetListOfSegs;
void LoadStructure2(){
    StreetListOfSegs.resize(getNumStreets());
    for(int curSegIdx=0;curSegIdx<getNumStreetSegments();curSegIdx++){
        StreetIdx curStreetIdx = getStreetSegmentInfo(curSegIdx).streetID;
        StreetListOfSegs[curStreetIdx].push_back(curSegIdx);
    }
}
/**
 * Structure 3
 * <br> StreetInformation contains Segment vector & Intersection Set
 */
std::vector<std::set<IntersectionIdx>> StreetListOfIntersects;
void LoadStructure3(){
    StreetListOfIntersects.resize(getNumStreets());
    StreetSegmentInfo segInfo;
    StreetIdx curStreetIdx;
    for(int curIntersectIdx = 0; curIntersectIdx < getNumIntersections(); curIntersectIdx++) {
        std::vector<StreetSegmentIdx> segsIdxList = IntersectListOfStreetSegs[curIntersectIdx];
        for (int i= 0 ; i< segsIdxList.size();i++ ) {
            segInfo = getStreetSegmentInfo(segsIdxList[i]);
            curStreetIdx = segInfo.streetID;

            StreetListOfIntersects[curStreetIdx].insert(segInfo.from);
            StreetListOfIntersects[curStreetIdx].insert(segInfo.to);
            //
        }
    }
}
//
/**
 * Structure 4: 256CharNodeTree (Special) [Func: 2.4]
 * <br> Freed during Close Map
 */
struct CharNode{
    std::vector<StreetIdx> curPrefixStreetsList;
    CharNode* nextChar[256];
};
struct CharTree{
    CharNode* root;
}StNameTreeForPrefix;

/**
 * setCorrect Boundary for Char(-128 - 127) into (0 - 255)
 * @param char
 * @return
 */
int CharToInt(char c){
    return (((unsigned)c)&0xff);
}
/**
 * Intsert a String into the CharNodeTree
 * @param curStName
 * @param street_id
 */
void insertNameToTree(std::string curStName, StreetIdx street_id){
    CharNode* cptr = StNameTreeForPrefix.root;
    for(int charIdx = 0; charIdx < curStName.length(); charIdx++){
        int charDec = CharToInt(curStName[charIdx]);
        if(cptr->nextChar[charDec] == nullptr){
            cptr->nextChar[charDec] = new CharNode();
        }
        cptr = cptr -> nextChar[charDec];
        cptr ->curPrefixStreetsList.push_back(street_id);
    }
}
/**
 * Modify Name with trim space & lowercase
 * @param srcName
 * @return modified Name
 */
std::string modifyName(std::string srcName){
    std::string name = srcName;
    name.erase(remove(name.begin(), name.end(), ' '), name.end());
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    return name;
}

/**
 * Load CharNodeTree of all StreetName with insertName function
 */
void LoadStructure4(){
    StNameTreeForPrefix.root = new CharNode();
    int totalStNum = getNumStreets();
    for(int curStIdx = 0; curStIdx < totalStNum; curStIdx++){
        std::string stName = getStreetName(curStIdx);
        stName = modifyName(stName);
        insertNameToTree(stName, curStIdx);
    }
}
/**
 * clear all dynamic allocated CharNode using Recursion
 * @param myRoot
 */
void ClearStructure4(CharNode* myRoot){
    if(myRoot == nullptr){
        return;
    }
    for(int i=0; i<256; i++){
        ClearStructure4(myRoot->nextChar[i]);
    }
    delete myRoot;
    myRoot = nullptr;
}

/*Global Structure Define End*/

/* Other Helper Begin */
template<typename Type>
/**
 * DataStructure Helper Function:<br>
 * Transfer Set into vector using copy
 * @tparam Type
 * @param source Set<Type>
 * @return destination Vector<Type>
 */
std::vector<Type> SetToVec(const std::set<Type> & srcSet){
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
    LoadStructure1();
    LoadStructure2();
    LoadStructure3();
    LoadStructure4();
    load_successful = true; //Make sure this is updated to reflect whether
                            //loading the map succeeded or failed

    IntersectionListOfStreetName.resize(getNumIntersections());

    return load_successful;
}

void closeMap() {
    //Clean-up your map related data structures here

    // call this API to close the currently opened map
    closeStreetDatabase();
    IntersectListOfStreetSegs.clear();
    IntersectionListOfStreetName.clear();
    StreetListOfIntersects.clear();
    StreetListOfIntersects.clear();
    StreetListOfSegs.clear();
    // clear the data structure for searching street names
    ClearStructure4(StNameTreeForPrefix.root);
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
    std::set<IntersectionIdx> adjIntersectSet;
    int segsTotal = IntersectListOfStreetSegs[intersection_id].size();

    //Loop through StreetSegs of current intersection
    for(int segNum=0; segNum < segsTotal; segNum++) {

        //Save current SegInfo
        int curSegIdx = IntersectListOfStreetSegs[intersection_id][segNum];
        StreetSegmentInfo curSegInfo = getStreetSegmentInfo(curSegIdx);
        IntersectionIdx idFrom = curSegInfo.from;
        IntersectionIdx idTo = curSegInfo.to;

        //Determine Segment OneWay
        if(!curSegInfo.oneWay) {
            //Save id Differ of current intersection
            if (intersection_id == idFrom) {
                adjIntersectSet.insert(idTo);
            } else if (intersection_id == idTo) {
                adjIntersectSet.insert(idFrom);
            }
        }
        else{
            //Save if idFrom is current intersection
            if(intersection_id == idFrom){
                adjIntersectSet.insert(idTo);
            }
        }
    }
    return SetToVec<IntersectionIdx>(adjIntersectSet);
}

/**
 * Function 1.2: <br>
 * Returns the street segments that connect to the given intersection <br>
 * <br> Speed Requirement --> high
 * @param checking specific intersection_id
 * @return List Of StreetSegmentIndex of Specific Intersection
 */
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){
    return IntersectListOfStreetSegs[intersection_id];
}

/**
 * Function 1.3: <br>
 * Returns the street names at the given intersection (includes duplicate
 * street names in the returned vector) <br>
 * <br> Speed Requirement --> high
 * @param intersection_id
 * @return Vector of StreetNames
 */
std::vector<std::string> findStreetNamesOfIntersection(IntersectionIdx intersection_id){
    if(!IntersectionListOfStreetName[intersection_id].first){
        IntersectionListOfStreetName[intersection_id].first = true;
        int segsTotal = IntersectListOfStreetSegs[intersection_id].size();
        for(int segNum = 0; segNum < segsTotal; segNum++){
            int curSegIdx = IntersectListOfStreetSegs[intersection_id][segNum];
            StreetSegmentInfo curSegInfo = getStreetSegmentInfo(curSegIdx);

            std::string tempName = getStreetName(curSegInfo.streetID);
            IntersectionListOfStreetName[intersection_id].second.push_back(tempName);
        }
    }
    return IntersectionListOfStreetName[intersection_id].second;
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
    return SetToVec(StreetListOfIntersects[street_id]);
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
    std::string prefix = modifyName(street_prefix);
    CharNode* cptr = StNameTreeForPrefix.root;
    for(int charIdx = 0; charIdx < prefix.length(); charIdx++){
        int charDec = CharToInt(prefix[charIdx]);
        if(cptr->nextChar[charDec] == nullptr) {
            return {};
        }
        cptr = cptr -> nextChar[charDec];
    }
    return cptr -> curPrefixStreetsList;
}

/**
 * Function: 3.1
 * <br> Returns the distance between two (lattitude,longitude) coordinates in meters
 * <br> Speed Requirement --> Moderate
 * @param Pass in "From, To" position in LatLon pair
 * @return Distance in double
*/
double findDistanceBetweenTwoPoints(std::pair<LatLon, LatLon> points){
    double y1 = points.first.latitude()*kDegreeToRadian;
    double y2 = points.second.latitude()*kDegreeToRadian;
    double latAvg=(y1+y2)/2;
    double x1 =points.first.longitude() * cos(latAvg)*kDegreeToRadian;
    double x2 =points.second.longitude()* cos(latAvg)*kDegreeToRadian;
    return (kEarthRadiusInMeters*sqrt(pow(y2-y1,2.0) + pow(x2-x1,2.0)));
}
/**
 * Function 3.2
 * <br> Returns the length of the given street segment in meters Speed Requirement --> moderate
 * <br> Speed Requirement -->Moderate
 * <br> @param street_segment_id
 * @return SegmentLength
 */
double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    StreetSegmentInfo streetSegmentInfo = getStreetSegmentInfo(street_segment_id);
    IntersectionIdx idTo=streetSegmentInfo.to;
    IntersectionIdx idFrom=streetSegmentInfo.from;
    LatLon toLatLon = getIntersectionPosition(idTo);
    LatLon fromLatLon = getIntersectionPosition(idFrom);
    int numCurvePoints = streetSegmentInfo.numCurvePoints;
    double distance = 0;
    if(numCurvePoints==0) {
        distance = findDistanceBetweenTwoPoints(std::make_pair(toLatLon, fromLatLon));
    }else{
        LatLon curFirstLatLon=fromLatLon;
        LatLon curSecondLatLon=getStreetSegmentCurvePoint(street_segment_id,0);
        for(int curCurvePointNum=0;curCurvePointNum<numCurvePoints;curCurvePointNum++){
            if (curCurvePointNum!=0) {
                curFirstLatLon = curSecondLatLon;
                curSecondLatLon = curSecondLatLon=getStreetSegmentCurvePoint(street_segment_id,curCurvePointNum);;
            }
            distance+= findDistanceBetweenTwoPoints(std::make_pair(curFirstLatLon , curSecondLatLon));
        }
        distance+=findDistanceBetweenTwoPoints(std::make_pair(toLatLon,curSecondLatLon));
    }
    return distance;
}

/**
 * Function 3.3
 * <br> Returns the length of a given street in meters
 * <br> Speed Requirement --> high
 * @param street_id
 * @return
 */
double findStreetLength(StreetIdx street_id){
    double totalLength=0;
    for(StreetSegmentIdx curSegIdx : StreetListOfSegs[street_id]){
        totalLength += findStreetSegmentLength(curSegIdx);
    }
    return totalLength;//???? why/3 works
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
    float  speed=getStreetSegmentInfo(street_segment_id).speedLimit;
    double length=findStreetSegmentLength(street_segment_id);
    double time=(length/speed);
    return time;
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


