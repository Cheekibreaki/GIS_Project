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
#include "DBstruct.h"
#include <limits>
#include <OSMDatabaseAPI.h>

//using namespace std;
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
 * DataStructure: (For More Detail See DBstruct.h)
 *          vector <vector<StreetSegmentIdx>> IntersectListOfSegsList
 *          vector<LatLon> IntersectListOfInfo
 *
 *          vector<pair<bool,vector<string>>> IntersectListOfStName
 *
 *          vector<vector<StreetSegmentIdx>> StreetListOfSegsList
 *          vector<StreetSegmentInfo> SegListSegInfo
 *          vector<pair<double,double>> SegListOfLenAndTime
 *
 *          vector<set<IntersectionIdx>> StreetListOfIntersectsList
 *
 *          struct CharTree
 *
 *          unordered_map<string, vector<POIIdx>> POINameListOfPOIsList
 */
/*Global Structure Load Begin*/
double avg_lat;
double min_lat, max_lat, min_lon, max_lon;

///Struct Name Used

std::vector <StrSeg_Info> SegsInfoList;

std::unordered_map<std::string,std::vector<StreetSegmentIdx>> SegmentTypeList;

std::vector<intersect_info> IntersectInfoList;

std::vector <std::vector<StreetSegmentIdx>> IntersectListOfSegsList;
std::vector<LatLon> IntersectListOfLatLon;
std::vector<std::pair<bool,std::vector<std::string>>> IntersectListOfStName;
std::vector<std::vector<StreetSegmentIdx>> StreetListOfSegsList;
std::vector<std::set<IntersectionIdx>> StreetListOfIntersectsList;
std::unordered_map<std::string, std::vector<POIIdx>> POINameListOfPOIsList;
CharTree StNameTreeForPrefix;
std::vector<naturalFeature> NaturalFeatureList;
std::map<FeatureType, std::vector<FeatureIdx>> PolyFeatureList;
std::map<FeatureType, std::vector<FeatureIdx>> LineFeatureList;
/// CharTree "StNameTreeForPrefix" included

/// CharTree Member function & used Function
void CharTree::clear(){
    if(root== nullptr){
        return;
    }
    clearHelper(root);
}

void CharTree::clearHelper(CharNode* myRoot){
    if(myRoot == nullptr){
        return;
    }
    for(int i=0; i<256; i++){
        clearHelper(myRoot->nextChar[i]);
    }
    delete myRoot;
    myRoot = nullptr;
}

void CharTree::insertNameToTree(std::string curStName, StreetIdx street_id){
    CharNode* cptr = root;
    for(int charIdx = 0; charIdx < curStName.length(); charIdx++){
        int charDec = (curStName[charIdx]&0xff);
        if(cptr->nextChar[charDec] == nullptr){
            cptr->nextChar[charDec] = new CharNode();
        }
        cptr = cptr -> nextChar[charDec];
        cptr ->curPrefixStreetsList.push_back(street_id);
    }
}
std::string modifyName(std::string srcName){
    std::string name = srcName;
    name.erase(remove(name.begin(), name.end(), ' '), name.end());
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    return name;
}
void calc_avg_lat(){
    min_lat = IntersectListOfLatLon[0].latitude();
    max_lat = min_lat;
    min_lon = IntersectListOfLatLon[0].longitude();
    max_lon = min_lon;

    for(IntersectionIdx id = 0; id < IntersectListOfLatLon.size(); id++){

        min_lat = std::min(min_lat, IntersectListOfLatLon[id].latitude());
        max_lat = std::max(max_lat, IntersectListOfLatLon[id].latitude());
        min_lon = std::min(min_lon, IntersectListOfLatLon[id].longitude());
        max_lon = std::max(max_lon, IntersectListOfLatLon[id].longitude());
    }

    avg_lat = (min_lat + max_lat) / 2;
}


double x_from_lon(float lon){
    return lon * kDegreeToRadian * kEarthRadiusInMeters * std::cos(avg_lat * kDegreeToRadian);
}
double y_from_lat(float lat){
    return lat * kDegreeToRadian * kEarthRadiusInMeters;
}
double lon_from_x(float x){
    return x / (kDegreeToRadian * kEarthRadiusInMeters * std::cos(avg_lat * kDegreeToRadian));
}
double lat_from_y(float y){
    return y / kDegreeToRadian / kEarthRadiusInMeters;
}
ezgl::point2d LatLon_to_point2d(LatLon curLatLon){

    return {x_from_lon(curLatLon.longitude()),y_from_lat(curLatLon.latitude())};

}


/* Load Helper Start */
void LoadNaturalFeatureList(){
    NaturalFeatureList.resize(getNumFeatures());
    for(FeatureIdx feature_id=0; feature_id<getNumFeatures() ; feature_id++){
        NaturalFeatureList[feature_id].name=getFeatureName(feature_id);
        NaturalFeatureList[feature_id].type=getFeatureType(feature_id);
        NaturalFeatureList[feature_id].polyList.resize(getNumFeaturePoints(feature_id));

            for(int i= 0; i < getNumFeaturePoints(feature_id); i++){
                LatLon temp=getFeaturePoint(feature_id,i);
                double x= x_from_lon(temp.longitude());
                double y= y_from_lat(temp.latitude());
                NaturalFeatureList[feature_id].polyList[i]= ezgl::point2d(x,y);
            }
        if(NaturalFeatureList[feature_id].polyList[0]
        == NaturalFeatureList[feature_id].polyList[getNumFeaturePoints(feature_id)-1]
        && NaturalFeatureList[feature_id].polyList.size()>1){
            NaturalFeatureList[feature_id].isPoly=true;
        }

    }
}
void LoadNaturalFeatureTypeList(){

    //Initalize LineFeatureList
    //Initalize PolyFeatureList

    /*PolyFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(UNKNOWN,{}));
    PolyFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(PARK,{}));
    PolyFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(BEACH,{}));
    PolyFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(LAKE,{}));
    PolyFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(RIVER,{}));
    PolyFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(ISLAND,{}));
    PolyFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(BUILDING,{}));
    PolyFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(GREENSPACE,{}));
    PolyFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(GOLFCOURSE,{}));
    PolyFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(STREAM,{}));

    LineFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(UNKNOWN,{}));
    LineFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(PARK,{}));
    LineFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(BEACH,{}));
    LineFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(LAKE,{}));
    LineFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(RIVER,{}));
    LineFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(ISLAND,{}));
    LineFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(BUILDING,{}));
    LineFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(GREENSPACE,{}));
    LineFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(GOLFCOURSE,{}));
    LineFeatureList.insert(std::pair<FeatureType,std::vector<FeatureIdx>>(STREAM,{}));*/

    for(int feature_id=0; feature_id < getNumFeatures(); feature_id++ ) {
        if (NaturalFeatureList[feature_id].isPoly) {

            PolyFeatureList[NaturalFeatureList[feature_id].type].push_back(feature_id);

        } else {

            LineFeatureList[NaturalFeatureList[feature_id].type].push_back(feature_id);

        }

    }



}


void LoadIntersectInfoList(){
    IntersectInfoList.resize(getNumIntersections());

    for(IntersectionIdx id = 0; id < IntersectListOfLatLon.size(); id++){
        IntersectInfoList[id].curPosXY = LatLon_to_point2d(IntersectListOfLatLon[id]);
        IntersectInfoList[id].name = getIntersectionName(id);
    }
}

void LoadIntersectListOfInfo(){
    //resize all IntersecionList to amount of Intersection
    IntersectListOfSegsList.resize(getNumIntersections());
    IntersectListOfLatLon.resize(getNumIntersections());

    //go through all IntersectionId
    for (int curIntersect = 0; curIntersect < getNumIntersections(); curIntersect++) {

        IntersectListOfLatLon[curIntersect] = getIntersectionPosition(curIntersect);

        //load SegmentList of current intersection
        for (int segNum = 0; segNum < getNumIntersectionStreetSegment(curIntersect); segNum++) {
            IntersectListOfSegsList[curIntersect].push_back(getIntersectionStreetSegment(curIntersect, segNum));
        }
    }
}



void LoadStructurePackage(){
    //resize all List before loaded
    StreetListOfSegsList.resize(getNumStreets());

    SegsInfoList.resize(getNumStreetSegments());



    for(int curSegIdx=0;curSegIdx<getNumStreetSegments();curSegIdx++){

        SegsInfoList[curSegIdx].segInfo = getStreetSegmentInfo(curSegIdx);

        double length = findStreetSegmentLength(curSegIdx);
        double speed = SegsInfoList[curSegIdx].segInfo.speedLimit;
        SegsInfoList[curSegIdx].length = length;
        SegsInfoList[curSegIdx].time = (length/speed);
        SegsInfoList[curSegIdx].toXY=LatLon_to_point2d(IntersectListOfLatLon[SegsInfoList[curSegIdx].segInfo.from]);
        SegsInfoList[curSegIdx].fromXY=LatLon_to_point2d(IntersectListOfLatLon[SegsInfoList[curSegIdx].segInfo.to]);

        StreetIdx curStreetIdx = SegsInfoList[curSegIdx].segInfo.streetID;
        StreetListOfSegsList[curStreetIdx].push_back(curSegIdx);

    }
}
void LoadIntersectListOfStName(){
    IntersectListOfStName.resize(getNumIntersections());
    //use function findStreetNameOfIntersection
}

void LoadStreetListOfIntersectsList(){
    StreetListOfIntersectsList.resize(getNumStreets());
    StreetSegmentInfo segInfo;
    StreetIdx curStreetIdx;
    for(int curIntersectIdx = 0; curIntersectIdx < getNumIntersections(); curIntersectIdx++) {
        std::vector<StreetSegmentIdx> segsIdxList = IntersectListOfSegsList[curIntersectIdx];
        for (int i= 0 ; i< segsIdxList.size();i++ ) {
            segInfo = getStreetSegmentInfo(segsIdxList[i]);
            curStreetIdx = segInfo.streetID;

            StreetListOfIntersectsList[curStreetIdx].insert(segInfo.from);
            StreetListOfIntersectsList[curStreetIdx].insert(segInfo.to);
            //
        }
    }
}

void LoadStNameTreeForPrefix(){
    StNameTreeForPrefix.root = new CharNode();
    int totalStNum = getNumStreets();
    for(int curStIdx = 0; curStIdx < totalStNum; curStIdx++){
        std::string stName = getStreetName(curStIdx);
        stName = modifyName(stName);
        StNameTreeForPrefix.insertNameToTree(stName, curStIdx);
    }
}

void LoadPOINameListOfPOIsList(){
    for(POIIdx curPOI = 0; curPOI < getNumPointsOfInterest(); curPOI++ ){
        POINameListOfPOIsList[getPOIName(curPOI)].push_back(curPOI);
    }
}
//void LoadTypeListOfSegsList(std::string OSMpath){
//    loadOSMDatabaseBIN(OSMpath);
//    for(int segIdx=0; segIdx<SegsInfoList.size();segIdx++){
//        OSMID OSM=SegsInfoList[segIdx].segInfo.wayOSMID;
//        int OSM_64=uint64_t(OSM);
//        const OSMWay *curWay = getWayByIndex(OSM_64);
////        for(unsigned j=0;j<getTagCount(curWay);j++) {
////            std::pair<std::string, std::string> tagPair = getTagPair(curWay, j);
////            if(tagPair.first=="highway"&&tagPair.second=="motorway"){
////                SegmentTypeList["motorway"].push_back(segIdx);
////                break;
////            }
////            else if(tagPair.first=="highway"&&tagPair.second=="trunk"){
////                SegmentTypeList["trunk"].push_back(segIdx);
////                break;
////            }
////            else if(tagPair.first=="highway"&&tagPair.second=="primary"){
////                SegmentTypeList["primary"].push_back(segIdx);
////                break;
////            }
////            else if(tagPair.first=="highway"&&tagPair.second=="secondary"){
////                SegmentTypeList["secondary"].push_back(segIdx);
////                break;
////            }
////            else if(tagPair.first=="highway"&&tagPair.second=="tertiary"){
////                SegmentTypeList["tertiary"].push_back(segIdx);
////                break;
////            }
////            else if(tagPair.first=="highway"&&tagPair.second=="unclassified"){
////                SegmentTypeList["unclassified"].push_back(segIdx);
////                break;
////            }
////            else if(tagPair.first=="highway"&&tagPair.second=="residential"){
////                SegmentTypeList["residential"].push_back(segIdx);
////                break;
////            }else{
////                SegmentTypeList["unknown"].push_back(segIdx);
////                break;
////            }
////
////        }
//    }
//    closeOSMDatabase();
//}
///Load Helper End


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

    //Load Function Called

    LoadIntersectListOfInfo();
    calc_avg_lat();

    LoadStructurePackage();

    LoadIntersectListOfStName();

    LoadStreetListOfIntersectsList();

    LoadStNameTreeForPrefix();

    LoadPOINameListOfPOIsList();

    //LoadTypeListOfSegsList(map_streets_database_filename);

    LoadIntersectInfoList();

    LoadNaturalFeatureList();

    LoadNaturalFeatureTypeList();

    load_successful = true; //Make sure this is updated to reflect whether
    //loading the map succeeded or failed

    return load_successful;
}

void closeMap() {
    //Clean-up your map related data structures here

    // call this API to close the currently opened map
    closeStreetDatabase();

    IntersectListOfSegsList.clear();
    IntersectListOfLatLon.clear();
    IntersectListOfStName.clear();

    StreetListOfIntersectsList.clear();
    StreetListOfSegsList.clear();

    SegsInfoList.clear();

    StNameTreeForPrefix.clear();

    POINameListOfPOIsList.clear();

    IntersectInfoList.clear();

    NaturalFeatureList.clear();

    PolyFeatureList.clear();
    LineFeatureList.clear();
}

/// Tested Functions implemtation from m1.h

/// 1.1: vector<IntersectionIdx> findAdjacentInters(IntersectionIdx intersection_id);
/// 1.2: vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id);
/// 1.3: vector<string> findStreetNamesOfIntersection();

/// 2.1: LatLonBounds findStreetBoundingBox(StreetIdx street_id);
/// 2.2: vector<IntersectionIdx> findIntersectionsOfTwoStreets(pair<StreetIdx,StreetIdx> street_ids);
/// 2.3: vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id);
/// 2.4  vector<StreetIdx> findStreetIdsFromPartialStreetName(string street_prefix);

/// 3.1: double findDistanceBetweenTwoPoints(pair<LatLon, LatLon> points);
/// 3.2: double findStreetSegmentLength (StreetSegmentIdx street_segment_id);
/// 3.3: double findStreetLength (StreetIdx street_id);
/// 3.4: double findStreetSegmentTravelTime (StreetSegmentIdx street_segment_id);
/// 3.5: double findFeatureArea (FeatureIdx feature_id);

/// 4.1: IntersectionIdx findClosestIntersection(LatLon my_position);
/// 4.2:POIIdx findClosestPOI(LatLon my_position, string POIname);

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
    std::unordered_set<IntersectionIdx> adjIntersectSet;
    int segsTotal = IntersectListOfSegsList[intersection_id].size();

    //Loop through StreetSegs of current intersection
    for(int segNum=0; segNum < segsTotal; segNum++) {

        //Save current SegInfo
        int curSegIdx = IntersectListOfSegsList[intersection_id][segNum];
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
    return std::vector<IntersectionIdx> (adjIntersectSet.begin(),adjIntersectSet.end());
}

/**
 * Function 1.2: <br>
 * Returns the street segments that connect to the given intersection <br>
 * <br> Speed Requirement --> high
 * @param checking specific intersection_id
 * @return List Of StreetSegmentIndex of Specific Intersection
 */
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){
    return IntersectListOfSegsList[intersection_id];
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
    if(!IntersectListOfStName[intersection_id].first){
        IntersectListOfStName[intersection_id].first = true;
        int segsTotal = IntersectListOfSegsList[intersection_id].size();
        for(int segNum = 0; segNum < segsTotal; segNum++){
            int curSegIdx = IntersectListOfSegsList[intersection_id][segNum];
            StreetSegmentInfo curSegInfo = getStreetSegmentInfo(curSegIdx);

            std::string tempName = getStreetName(curSegInfo.streetID);
            IntersectListOfStName[intersection_id].second.push_back(tempName);
        }
    }
    return IntersectListOfStName[intersection_id].second;
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
    std::vector<IntersectionIdx> allIntersections=findIntersectionsOfStreet(street_id);
    double maxLatitude = -400.0;
    double maxLongitude = -400.0;
    double minLatitude = 400.0;
    double minLongitude = 400.0;

    for(int curIdx = 0 ;curIdx < allIntersections.size() ;curIdx++ ){
        IntersectionIdx curIntersection = findIntersectionsOfStreet(street_id)[curIdx];
        LatLon position = IntersectListOfLatLon[curIntersection];
        if(maxLatitude < position.latitude()){
            maxLatitude = position.latitude();
        }
        if(maxLongitude < position.longitude()){
            maxLongitude = position.longitude();
        }
        if(minLatitude > position.latitude()){
            minLatitude = position.latitude();

        }
        if(minLongitude > position.longitude()){
            minLongitude = position.longitude();
        }

    }
    std::vector<StreetSegmentIdx> AllStreetSegments = StreetListOfSegsList[street_id];
    for(int curSeg = 0; curSeg < AllStreetSegments.size(); curSeg++){
        int curveNum= SegsInfoList[AllStreetSegments[curSeg]].segInfo.numCurvePoints;
        for(int i = 0 ; i< curveNum ; i++){
            LatLon position = getStreetSegmentCurvePoint(AllStreetSegments[curSeg],i);

            if(maxLatitude < position.latitude()){
                maxLatitude = position.latitude();
            }
            if(maxLongitude < position.longitude()){
                maxLongitude = position.longitude();
            }
            if(minLatitude > position.latitude()){
                minLatitude = position.latitude();

            }
            if(minLongitude > position.longitude()){
                minLongitude = position.longitude();
            }

        }

    }

    empty.max=LatLon(maxLatitude,maxLongitude);
    empty.min=LatLon(minLatitude,minLongitude);


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
    std::vector<IntersectionIdx> StoreIntersections;
    StoreIntersections.resize(StreetListOfIntersectsList[street_ids.first].size());

    std::set_intersection(StreetListOfIntersectsList[street_ids.first].begin(),
                          StreetListOfIntersectsList[street_ids.first].end(),
                          StreetListOfIntersectsList[street_ids.second].begin(),
                          StreetListOfIntersectsList[street_ids.second].end()
            ,StoreIntersections.begin());
    StoreIntersections.erase(remove(StoreIntersections.begin(),StoreIntersections.end(),0)
            ,StoreIntersections.end());
    return StoreIntersections;
}

/**
 * Function 2.3
 * <br> Returns all intersections along the a given street
 * <br> Speed Requirement --> high
 * @param street_id
 * @return
 */
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
    return std::vector<IntersectionIdx>(StreetListOfIntersectsList[street_id].begin(),
                                        StreetListOfIntersectsList[street_id].end());
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
        int charDec = (prefix[charIdx]&0xff);
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
    double latAvg=((y1+y2)/2);
    double x1 =points.first.longitude() * cos(latAvg)* kDegreeToRadian;
    double x2 =points.second.longitude()* cos(latAvg)* kDegreeToRadian;
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
    StreetSegmentInfo streetSegmentInfo = SegsInfoList[street_segment_id].segInfo;
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
                curSecondLatLon=getStreetSegmentCurvePoint(street_segment_id,curCurvePointNum);
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
    for(StreetSegmentIdx curSegIdx : StreetListOfSegsList[street_id]){
        totalLength += findStreetSegmentLength(curSegIdx);
    }
    return totalLength;
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
    return SegsInfoList[street_segment_id].time;
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
    std::cout.precision(17);
    int featPtNum = getNumFeaturePoints(feature_id);
    LatLon firstPointLatLon = getFeaturePoint(feature_id, 0);
    LatLon lastPointLatLon = getFeaturePoint(feature_id, featPtNum-1);
    double area = 0;
    if(firstPointLatLon == lastPointLatLon) {
        std::vector<double> xList = std::vector<double>(featPtNum);
        std::vector<double> yList = std::vector<double>(featPtNum);
        double latAvg = 0;
        //Save yList
        for (int pointNum = 0; pointNum < featPtNum; pointNum++) {
            double tempLat = getFeaturePoint(feature_id, pointNum).latitude();
            yList[pointNum] = tempLat * kDegreeToRadian;
            latAvg += tempLat;
        }

        latAvg = (latAvg / featPtNum) * kDegreeToRadian;
        //Save xList
        for (int pointNum = 0; pointNum < featPtNum; pointNum++) {
            double tempLon = getFeaturePoint(feature_id, pointNum).longitude();
            xList[pointNum] = tempLon * cos(latAvg)* kDegreeToRadian;

        }
        std::vector<double> xRef = std::vector<double>(featPtNum);
        std::vector<double> yRef = std::vector<double>(featPtNum);
        xRef[0] = xRef[featPtNum-1] = 0;
        yRef[0] = yRef[featPtNum-1] = 0;
        for(int i=1; i < featPtNum-1; i++){
            xRef[i] = kEarthRadiusInMeters * (xList[i] - xList[0]);
            yRef[i] = kEarthRadiusInMeters * (yList[i] - yList[0]);
        }
        double temp1=0, temp2=0;
        for (int i = 1; i <= featPtNum - 3; i++) {
            double cur = xRef[i] * yRef[i+1];
            temp1 += cur;
        }
        for (int i = 1; i <= featPtNum - 3; i++) {
            double cur = yRef[i] * xRef[i+1];
            temp2 += cur;
        }
        area = (temp1 - temp2)/2;
        if(area < 0) area*=-1;
    }
    return area;
}



/**
 * Function 4.1
 * <br> Returns the nearest intersection to the given position
 * <br> Speed Requirement --> none
 * @param my_position
 * @return
 */
IntersectionIdx findClosestIntersection(LatLon my_position){
    IntersectionIdx closestIntersection = -1;
    double closestDistance = std::numeric_limits<double>::max();
    for(IntersectionIdx curIntersectIdx = 0; curIntersectIdx < IntersectListOfLatLon.size(); curIntersectIdx++){
        double curDistance = findDistanceBetweenTwoPoints
                (std::make_pair(IntersectListOfLatLon[curIntersectIdx], my_position));

        if(curDistance < closestDistance){
            closestDistance = curDistance;
            closestIntersection = curIntersectIdx;
        }
    }
    return closestIntersection;
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
    std::vector<POIIdx> POIList = POINameListOfPOIsList.at(POIname);
    if(POIList.empty()){
        return -1;
    }
    double minDistance = std::numeric_limits<double>::max();
    POIIdx closestPOIIdx = -1;
    for(POIIdx curPOI : POIList){
        double curDistance = findDistanceBetweenTwoPoints(std::make_pair(getPOIPosition(curPOI), my_position));
        if(curDistance < minDistance){
            minDistance = curDistance;
            closestPOIIdx = curPOI;
        }
    }
    return closestPOIIdx;
}


