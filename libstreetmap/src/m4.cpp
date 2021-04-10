//
// Created by cheny811 on 2021-04-09.
//

#include "DBstruct.h"
#include "m4.h"
#include "m1.h"
#include <queue>
#include <float.h>
#include <map>

/// Structure Area
struct IntersectNaviInfo{
    StreetSegmentIdx reachingEdge = -1;
    double bestTime = DBL_MAX;
    bool isTravel = false;
};

struct WaveElem{
    int IntersectId;
    int reachingEdge;
    double travelTime;
    WaveElem (int curI, int rE, double tT) {
        IntersectId = curI;
        reachingEdge = rE;
        travelTime = tT;
    }
};

struct PathInfo{
    double travelTime;
    std::vector<StreetSegmentIdx> curPath;
};

std::map<int, std::map<int, PathInfo>> PathStorage;
//std::map<int, std::map<int, PathInfo>> PathStorage;


void MultiDest_Dijkstra_Method(std::vector<IntersectionIdx>& relatedIntersect, const double turn_penalty);

void MultiDest_Dijkstra(const IntersectionIdx               fromIntersect,
                        std::vector<IntersectionIdx>&       relatedIntersect,
                        const double                        turn_penalty);

std::vector<StreetSegmentIdx> backTracing(const IntersectionIdx intersect_id_start,
                                          const IntersectionIdx intersect_id_destination,
                                          std::vector<IntersectNaviInfo>&     IntersectNaviInfoList);



/// Intergrate Function
std::vector<CourierSubPath> travelingCourier(
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<int>& depots,
        const float turn_penalty){

    /// Step 1: MultiDest Dyjestra Method
    // assemble all Intersections and Pass into MultiStart_Dij
    std::vector<IntersectionIdx> allIntersect;
    for(auto temp : deliveries){
        allIntersect.push_back(temp.pickUp);
        allIntersect.push_back(temp.dropOff);
    }
    allIntersect.insert(allIntersect.end(), depots.begin(), depots.end());

    MultiDest_Dijkstra_Method(allIntersect, turn_penalty);

    /// Step 2: Greedy Algo || MultiStart
    std::vector<CourierSubPath> courierPaths;


    double minFirstTime = 999999999999999;
    IntersectionIdx firstIntersect;
    int firstId;//pickup or drop off Id
    IntersectionIdx secondIntersect;
    int secondId;//pickup or drop off Id
    std::vector<StreetSegmentIdx> intersectPath;
    for(auto tempDepot : depots){
        for(auto tempDeliv : deliveries){
            std::unordered_map<int, std::unordered_map<int, PathInfo>>::const_iterator gotTempDepot = PathStorage.find (tempDepot);
            std::unordered_map<int, PathInfo>::const_iterator gotTempDeliv = gotTempDepot->second.find(tempDeliv.pickUp);
            double tempMinTime = gotTempDeliv->second.travelTime;
            if(minFirstTime>tempMinTime){
                firstIntersect=tempDepot;
                secondIntersect=tempDeliv.pickUp;
                intersectPath=gotTempDeliv->second.curPath;
                minFirstTime=tempMinTime;
            }
        }
    }
    struct CourierSubPath tempCourierSubPath = {firstIntersect,secondIntersect,intersectPath};
    courierPaths.push_back(tempCourierSubPath);



    std::unordered_map<int, IntersectionIdx> unpicked;
    std::unordered_map<int, IntersectionIdx> undroped;


    for(int i=0;i<deliveries.size();i++){
        if(i!=secondId){
            std::pair <int,IntersectionIdx> temp=std::make_pair(i,deliveries[i].pickUp);
            unpicked.insert(temp);
        }
    }
    std::pair <int,IntersectionIdx> firstDropOff=std::make_pair(secondId,deliveries[secondId].dropOff);
    undroped.insert(firstDropOff);

    int delivCounter=deliveries.size();
    while(delivCounter!=0){
        bool isdroped=false;
        double minPathTime=999999999999;
        for(auto it=undroped.begin();it!=undroped.end();it++){
            double tempTime;
            int index=it->first;

            //unfinished

            if(tempTime<minPathTime){
                minPathTime=tempTime;
            }
        }
        for(int i=0;i<unpicked.size();i++){
            double tempTime;

            //unfinished


            if(tempTime<minPathTime){
                delivCounter--;
                isdroped= true;
                minPathTime=tempTime;
            }
        }



    }




//    while(countDelivLeft!=0){
//        firstIntersect=secondIntersect;
//        for(tempDeliv : deliveries) {
//            std::unordered_map<std::string, double>::const_iterator gotTempFirst = PathStorage.find(firstIntersect);
//            std::unordered_map<std::string, double>::const_iterator gotTempPick = got->second.find(tempDeliv.pickUp);
//            std::unordered_map<std::string, double>::const_iterator gotTempDrop = got->second.find(tempDeliv.DropOff);
//            int tempPickMinTime = gotTempPick->second.travelTime;
//            int tempDropMinTime = gotTempDrop ->second.travelTime;
//            if (minTime > tempMinTime) {
//                firstIntersect = tempDepot;
//                secondIntersect = tempDeliv.pickUp;
//                intersectPath = gotTempDeliv->second.curPath;
//            }
//        }
//    }




    /// Step 3: 2/3 OPTs With Time Restriction
    return{};
}

///MultiStart Dyjestra Method (NOT CHANGED YET)
void MultiDest_Dijkstra_Method(std::vector<IntersectionIdx>& relatedIntersect, const double turn_penalty){

    for(auto curIntersect: relatedIntersect){
        MultiDest_Dijkstra(curIntersect, relatedIntersect, turn_penalty);
    }

}

void MultiDest_Dijkstra(const IntersectionIdx               fromIntersect,
                        std::vector<IntersectionIdx>&       relatedIntersect,
                        const double                        turn_penalty){

    std::vector<IntersectNaviInfo> IntersectNaviInfoList;
    IntersectNaviInfoList.resize(getNumIntersections());

    auto cmp = [](WaveElem lhs, WaveElem rhs){return (lhs.travelTime) > (rhs.travelTime);};
    std::priority_queue<WaveElem, std::vector<WaveElem>, decltype(cmp)> WaveFront(cmp);

    WaveFront.push(WaveElem(fromIntersect, -1, 0));

    while(!WaveFront.empty()){

        WaveElem currWave = WaveFront.top();
        WaveFront.pop();

        IntersectionIdx currIntersectId = currWave.IntersectId;

        if(currWave.travelTime < IntersectNaviInfoList[currIntersectId].bestTime){
            // Store THe bestTime into curretnIntersect
            IntersectNaviInfoList[currIntersectId].reachingEdge = currWave.reachingEdge;
            IntersectNaviInfoList[currIntersectId].bestTime = currWave.travelTime;


            for(auto itr = relatedIntersect.begin();itr != relatedIntersect.end(); itr++){
                if(*itr==currIntersectId){
                    relatedIntersect.erase(itr);
                    PathStorage[fromIntersect][currIntersectId].travelTime
                                                = IntersectNaviInfoList[currIntersectId].bestTime;
                    PathStorage[fromIntersect][currIntersectId].curPath
                                                = backTracing(fromIntersect, currIntersectId, IntersectNaviInfoList);
                }
            }
            if(relatedIntersect.empty()){
                return;
            }

            // find All streetSegment of Current Intersection
            auto tempStSegsList = findStreetSegmentsOfIntersection(currIntersectId);

            // each outEdge of currNode
            for(auto currStSegsId : tempStSegsList){
                const StrSeg_Info & curSegInfo = SegsInfoList[currStSegsId];

                IntersectionIdx toIntersect;

                // Check From and To & ONEWAY
                if(curSegInfo.segInfo.oneWay){
                    if(curSegInfo.segInfo.to == currIntersectId){
                        continue;
                    }else{
                        toIntersect = curSegInfo.segInfo.to;
                    }
                }else{
                    if(curSegInfo.segInfo.to == currIntersectId){
                        toIntersect = curSegInfo.segInfo.from;
                    }else{
                        toIntersect = curSegInfo.segInfo.to;
                    }
                }

                if(IntersectNaviInfoList[toIntersect].isTravel) continue;
                // Save To INTERSECTIONINFO into the WaveFront
                // Initalize with WaveElem("curIntersect", "reachingEdge", "travelTime")

                if(currStSegsId != IntersectNaviInfoList[currIntersectId].reachingEdge
                   && currIntersectId != intersect_id_start){
                    WaveFront.push(WaveElem(toIntersect, currStSegsId,
                                            IntersectNaviInfoList[currIntersectId].bestTime + curSegInfo.time + turn_penalty));
                }else{
                    WaveFront.push(WaveElem(toIntersect, currStSegsId,
                                            IntersectNaviInfoList[currIntersectId].bestTime + curSegInfo.time));
                }
            }
            IntersectNaviInfoList[currIntersectId].isTravel = true;
        }

    }

    IntersectNaviInfoList.clear();
}