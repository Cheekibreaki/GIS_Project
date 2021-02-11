#pragma once //protects against multiple inclusions of this header file
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cmath>
/**
 * Intersection List of StreetSegments List (streetSegments belongs to Current Intersection)
 */
std::vector <std::vector<StreetSegmentIdx>> IntersectListOfSegsList;
/**
 * Intersection List of LatitudeLongitude (LatLon belongs to Current Intersection)
 */
std::vector<LatLon> IntersectListOfLatLon;
/**
 * Load all streetSegments and LatLon of current intersection in to relative list
 */
void LoadIntersectListOfSegAndLatLon();



/**
 * Intersection List of StreetNames List (streetNames belong to Current Intersection)
 */
std::vector<std::pair<bool,std::vector<std::string>>> IntersectListOfStName;
/**
 * Only resize List
 * Loaded during func findStreetNamesOfIntersection
 * <br>Save Data if not loaded using firstElement
 */
void LoadIntersectListOfStName();




/**
 * Package
 * <br>StreetSegment List of StreetSegmentInfo/ Length & TravelTime
 * <br>Street List of all StreetSegments belonged to current Street
 * <br>StreetInformation contains Segment vector & Intersection Set
 */
std::vector<std::vector<StreetSegmentIdx>> StreetListOfSegsList;
std::vector<StreetSegmentInfo> SegListSegInfo;
std::vector<std::pair<double,double>> SegListOfLenAndTime;
/**
 * Load StreetList -> SegList
 * <br>Load SegList -> SegInfo
 * <br>Load SegList -> Length & TravelTime
 */
void LoadStructurePackage();

/**
 * Street List Of all Intersections belong to current Street
 */
std::vector<std::set<IntersectionIdx>> StreetListOfIntersectsList;
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
    void clear(){
        if(root== nullptr){
            return;
        }
        clearHelper(root);
    };
    /**
     * clear all dynamic allocated CharNode using Recursion
     * @param myRoot
     */
    void clearHelper(CharNode* myRoot);
}StNameTreeForPrefix;
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
/**
 * Load CharNodeTree of all StreetName with insertName function
 */
void LoadStNameTreeForPrefix();
/**
 * Intsert a String into the CharNodeTree
 * @param curStName
 * @param street_id
 */
void insertNameToTree(std::string curStName, StreetIdx street_id){
    CharNode* cptr = StNameTreeForPrefix.root;
    for(int charIdx = 0; charIdx < curStName.length(); charIdx++){
        int charDec = (((unsigned)curStName[charIdx])&0xff);
        if(cptr->nextChar[charDec] == nullptr){
            cptr->nextChar[charDec] = new CharNode();
        }
        cptr = cptr -> nextChar[charDec];
        cptr ->curPrefixStreetsList.push_back(street_id);
    }
};
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
};

/**
 * POI Name List -> POI Index List
 * <br>Key: POI Name
 * <br>Value: List of POI Index
 */
std::unordered_map<std::string, std::vector<POIIdx>> POINameListOfPOIsList;
/**
 * Load POI Name List -> POI Index List
 */
void LoadPOINameListOfPOIsList();
