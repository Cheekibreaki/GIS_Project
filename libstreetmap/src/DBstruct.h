#pragma once //protects against multiple inclusions of this header file
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cmath>

#include "StreetsDatabaseAPI.h"

/**
 * Intersection List of StreetSegments List (streetSegments belongs to Current Intersection)
 */
extern std::vector <std::vector<StreetSegmentIdx>> IntersectListOfSegsList;


/**
 * Intersection List of Info include LatLon & IntersectName
 */
extern std::vector<LatLon> IntersectListOfLatLon;
/**
 * Load all streetSegments and LatLon and IntersectName of current intersection in to relative list
 */
void LoadIntersectListOfInfo();

/**
 * Intersection List of StreetNames List (streetNames belong to Current Intersection)
 * <br> !!!!!!Notice this structure is not preloaded, need to use with function "findStreetNamesOfIntersection"
 */
extern std::vector<std::pair<bool,std::vector<std::string>>> IntersectListOfStName;
/**
 * Only resize List
 * Loaded during func findStreetNamesOfIntersection
 * <br>Save Data if not loaded using firstElement
 */
void LoadIntersectListOfStName();

/*
 * Package
 * <br>StreetSegment List of StreetSegmentInfo/ Length & TravelTime
 * <br>Street List of all StreetSegments belonged to current Street
 * <br>StreetInformation contains Segment vector & Intersection Set
 */
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
 * Load StreetList -> SegList
 * <br>Load SegList -> SegInfo
 * <br>Load SegList -> Length & TravelTime
 */
void LoadStructurePackage();

/**
 * Street List Of all Intersections belong to current Street
 */
extern std::vector<std::set<IntersectionIdx>> StreetListOfIntersectsList;
/**
 * Load Street List -> Intersection List
 */
void LoadStreetListOfIntersectsList();


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
extern CharTree StNameTreeForPrefix;

/**
 * Load CharNodeTree of all StreetName with insertName function
 */
void LoadStNameTreeForPrefix();

/**
 * Modify Name with trim space & lowercase
 * @param srcName
 * @return modified Name
 */
std::string modifyName(std::string srcName);


/**
 * POI Name List -> POI Index List
 * <br>Key: POI Name
 * <br>Value: List of POI Index
 */
extern std::unordered_map<std::string, std::vector<POIIdx>> POINameListOfPOIsList;
/**
 * Load POI Name List -> POI Index List
 */
void LoadPOIListOfLatLonsList();


///temparay func and Structure used in drawMap
double x_from_lon(float lon);
double y_from_lat(float lat);
double lon_from_x(float x);
double lat_from_y(float y);



struct position_XY {
    double x;
    double y;
};
struct intersect_info{
    position_XY curPosXY;
    std::string name;
    bool highlight;
};
extern std::vector<intersect_info> IntersectInfoList;