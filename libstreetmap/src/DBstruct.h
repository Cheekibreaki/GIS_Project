#pragma once //protects against multiple inclusions of this header file
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cmath>

#include "StreetsDatabaseAPI.h"
#include "ezgl/point.hpp"


/* External constants */


/* Support Lower Level Structures */


/**
 * Char Tree Node contains the next node List(pointer) & vector of current Prefix StreetID
 */
struct CharNode{
    std::vector<StreetIdx> curPrefixStreetsList;
    CharNode* nextChar[256];
};
/**
 * 256CharNodeTree (Node Include Vector)
 * <br> Freed during Close Map (#TREENAME#.close())
 */
struct CharTree{
    CharNode* root;
    /**
     * clear the CharTree
     */
    void clear();
    /**
     * clear all dynamic allocated CharNode using Recursion
     * @param myRoot
     */
    void clearHelper(CharNode* myRoot);
    /**
     * Intsert a String into the CharNodeTree
     * @param curStName
     * @param street_id
     */
    void insertNameToTree(std::string curStName, StreetIdx street_id);
};
/**
 * @struct structure intersection Information
 * @content     curPosXY
 * <br>         name
 * <br>         highlight
 */
struct intersect_info{
    ezgl::point2d curPosXY;
    std::string name;
    bool highlight = false;
};
/**
 * @struct structure Point of Interest Information
 * @content     curPosXY
 * <br>         name
 * <br>         type
 * <br>         highlight
 */
struct poi_info{
    ezgl::point2d curPosXY;
    std::string name;
    std::string type;
    bool highlight = false;
};






/* External structures */


/**
 * Intersection List of StreetSegments List (streetSegments belongs to Current Intersection)
 */
extern std::vector <std::vector<StreetSegmentIdx>> IntersectListOfSegsList;
/**
 * Intersection List of Info include LatLon & IntersectName
 */
extern std::vector<LatLon> IntersectListOfLatLon;
/**
 * Intersection List of StreetNames List (streetNames belong to Current Intersection)
 * <br> !!!!!!Notice this structure is not preloaded, need to use with function "findStreetNamesOfIntersection"
 */
extern std::vector<std::pair<bool,std::vector<std::string>>> IntersectListOfStName;
 /**
  * Street List contains all segment Id that belongs to indivual street
  */
extern std::vector<std::vector<StreetSegmentIdx>> StreetListOfSegsList;
/**
 * segment List contains Information(OSMID, from, to, oneWay, numCurvPoints, speedLimit, streetID )
 */
extern std::vector<StreetSegmentInfo> SegListSegInfo;
/**
 * segment List contains Length and Travel time
 */
extern std::vector<std::pair<double,double>> SegListOfLenAndTime;
/**
 * Street List Of all Intersections belong to current Street
 */
extern std::vector<std::set<IntersectionIdx>> StreetListOfIntersectsList;
/**
 * Partial Street Name Loaded, each node has a vector of current char's StreetName ID
 */
extern CharTree StNameTreeForPrefix;
/**
 * Unordered map of POI Name to find POI Index List, access by using NAME of string
 * <br>Key: POI Name
 * <br>Value: List of POI Index
 */
extern std::unordered_map<std::string, std::vector<POIIdx>> POINameListOfPOIsList;
/**
 * Intersection Information list contains lower level structure intersect_info (curPosXY, name, highlight)
 */
extern std::vector<intersect_info> IntersectInfoList;
/**
 * POI information list contains lower level structure poi_info (curPosXY, name, type, highlight)
 */
extern std::vector<poi_info> PoiInfoList;





/* Load Functions Start */

/**
 * Load all streetSegments and LatLon and IntersectName of current intersection in to relative list
 */
void LoadIntersectListOfInfo();
/**
 * Only resize List
 * Loaded during func findStreetNamesOfIntersection
 * <br>Save Data if not loaded using firstElement
 */
void LoadIntersectListOfStName();
/**
 * Load StreetList -> SegList
 * <br>Load SegList -> SegInfo
 * <br>Load SegList -> Length & TravelTime
 */
void LoadStructurePackage();
/**
 * Load Street List -> Intersection List
 */
void LoadStreetListOfIntersectsList();
/**
 * Load CharNodeTree of all StreetName with insertName function
 */
void LoadStNameTreeForPrefix();
/**
 * Load POI Name List -> POI Index List
 */
void LoadPOINameListOfPOIsList();
/**
 * Load Intersection Information
 */
void LoadIntersectInfoList();
/**
 * Load POI Information
 */
void LoadPoiInfoList();




/* Supportive Func */


/**
 * Modify string return for trim space & lowercase
 * @param srcName
 * @return modified Name
 */
std::string modifyName(std::string srcName);
/* func and Structure implemented in drawMap */
double x_from_lon(float lon);
double y_from_lat(float lat);
double lon_from_x(float x);
double lat_from_y(float y);
ezgl::point2d LatLon_to_point2d(LatLon curLatLon);


/* Supportive Func END */