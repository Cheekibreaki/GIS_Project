#pragma once //protects against multiple inclusions of this header file
#include <vector>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <cmath>
/**
 * Structure 1
 */
std::vector <std::vector<StreetSegmentIdx>> IntersectListOfSegsList;
std::vector<LatLon> IntersectListOfLatLon;
/**
 * Load all streetSegs of one intersection in to intersection list
 */
void LoadIntersectListOfSegAndLatLon();

//Loaded during func findStreetNamesOfIntersection
//Save Data if not loaded using firstElement
std::vector<std::pair<bool,std::vector<std::string>>> IntersectListOfStName;
void LoadIntersectListOfStName();

/**
 * Package 2
 * <br> StreetInformation contains Segment vector & Intersection Set
 */
std::vector<std::vector<StreetSegmentIdx>> StreetListOfSegsList;
std::vector<StreetSegmentInfo> SegListSegInfo;
std::vector<std::pair<double,double>> SegListOfLenAndTime;
void LoadStructurePackage();

/**
 * Structure 3
 * <br> StreetInformation contains Segment vector & Intersection Set
 */
std::vector<std::set<IntersectionIdx>> StreetListOfIntersectsList;
void LoadStreetListOfIntersectsList();


struct CharNode{
    std::vector<StreetIdx> curPrefixStreetsList;
    CharNode* nextChar[256];
};
/**
 * Structure 4: 256CharNodeTree (Special) [Func: 2.4]
 * <br> Freed during Close Map
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
void insertNameToTree(std::string curStName, StreetIdx street_id);
/**
 * Modify Name with trim space & lowercase
 * @param srcName
 * @return modified Name
 */
std::string modifyName(std::string srcName);