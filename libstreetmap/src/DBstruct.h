#pragma once //protects against multiple inclusions of this header file
#include <vector>
#include <set>
#include <map>
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
/*
 * @struct structure Point of Segment Information
 * @content     length
 * <br>         time
 * <br>         segInfo
 * <br>         highlight
 */
struct StrSeg_Info{
    double length;
    double time;
    ezgl::point2d fromXY;
    ezgl::point2d toXY;
    StreetSegmentInfo segInfo;
    bool highlight = false;
};

struct naturalFeature{
    std::string name;
    FeatureType type;
    std::vector<ezgl::point2d> polyList;
    bool isPoly = false;
};

//external std::vector<>
extern std::map<FeatureType, std::vector<FeatureIdx>> PolyFeatureList;
extern std::map<FeatureType, std::vector<FeatureIdx>> LineFeatureList;


/* External structures */
extern std::vector<naturalFeature> NaturalFeatureList;
extern std::vector <OSMID> OSMWayofOSMIDList;
/**
 * Segment type contains all belonged segment ID
 */
extern std::unordered_map<std::string,std::vector<StreetSegmentIdx>> SegmentTypeList;

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
 * segment List contains Length and Travel time
 * SegInformation (OSMID, from, to, oneWay, numCurvPoints, speedLimit, streetID)
 * and highlight
 */
extern std::vector <StrSeg_Info> SegsInfoList;
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
extern std::vector <poi_info> PoiInfoList;




/* Load Functions Start */
void LoadOSMWayofOSMIDList();
void LoadNaturalFeatureTypeList();
void LoadNaturalFeatureList();
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

void LoadTypeListOfSegsList(std::string OSMpath);

/* Supportive Func */


extern double avg_lat;
extern double min_lat, max_lat, min_lon, max_lon;

/**
 * Modify string return for trim space & lowercase
 * @param srcName
 * @return modified Name
 */
std::string modifyName(std::string srcName);
/* func and Structure implemented in drawMap */
void calc_avg_lat();
double x_from_lon(float lon);
double y_from_lat(float lat);
double lon_from_x(float x);
double lat_from_y(float y);
ezgl::point2d LatLon_to_point2d(LatLon curLatLon);


/* Supportive Func END */