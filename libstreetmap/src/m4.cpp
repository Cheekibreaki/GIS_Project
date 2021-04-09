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


void MultiDest_Dijkstra_Method(std::vector<IntersectionIdx>& relatedIntersect, const double turn_penalty);

void MultiDest_Dijkstra(const IntersectionIdx               fromIntersect,
                        std::vector<IntersectionIdx>&       relatedIntersect,
                        const double                        turn_penalty);

std::vector<StreetSegmentIdx> backTracing(const IntersectionIdx intersect_id_start,
                                          const IntersectionIdx intersect_id_destination,
                                          const std::vector<IntersectNaviInfo>&     IntersectNaviInfoList);



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


    /// Step 3: 2/3 OPTs With Time Restriction
    for(auto itr = PathStorage.begin(); itr!= PathStorage.end(); itr++){
        itr->second.clear();
    }
    PathStorage.clear();
    return{};
}

///MultiStart Dyjestra Method (NOT CHANGED YET)
void MultiDest_Dijkstra_Method(std::vector<IntersectionIdx>& relatedIntersect, const double turn_penalty){

    std::vector<IntersectionIdx> testPoint={21123, 51659, 60880, 6038};
    /*for(auto curIntersect: relatedIntersect){
        MultiDest_Dijkstra(curIntersect, relatedIntersect, turn_penalty);
    }*/

    MultiDest_Dijkstra(53127, testPoint, 15);
    testPoint.clear();
}

void MultiDest_Dijkstra(const IntersectionIdx               intersect_id_start,
                        std::vector<IntersectionIdx>&       relatedIntersect,
                        const double                        turn_penalty){

    std::vector<IntersectNaviInfo> IntersectNaviInfoList;
    IntersectNaviInfoList.resize(getNumIntersections());

    auto cmp = [](WaveElem lhs, WaveElem rhs){
        return (lhs.travelTime) > (rhs.travelTime);
    };
    std::priority_queue<WaveElem, std::vector<WaveElem>, decltype(cmp)> WaveFront(cmp);

    WaveFront.push(WaveElem(intersect_id_start, -1, 0));

    while(!WaveFront.empty()){
        WaveElem currWave = WaveFront.top();
        WaveFront.pop();

        IntersectionIdx currIntersectId = currWave.IntersectId;

        if(currWave.travelTime < IntersectNaviInfoList[currIntersectId].bestTime){

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

                while(!WaveFront.empty()){
                    WaveFront.pop();
                }
                IntersectNaviInfoList.clear();
                return;
            }


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

                if(currIntersectId != intersect_id_start &&
                   SegsInfoList[currStSegsId].segInfo.streetID !=
                   SegsInfoList[IntersectNaviInfoList[currIntersectId].reachingEdge].segInfo.streetID){
                    double curTravelTime = IntersectNaviInfoList[currIntersectId].bestTime + curSegInfo.time + turn_penalty;
                    WaveFront.push(WaveElem(toIntersect, currStSegsId, curTravelTime));
                }
                else{
                    double curTravelTime = IntersectNaviInfoList[currIntersectId].bestTime + curSegInfo.time;
                    WaveFront.push(WaveElem(toIntersect, currStSegsId, curTravelTime));
                }
            }
            IntersectNaviInfoList[currIntersectId].isTravel = true;
        }
    }

    IntersectNaviInfoList.clear();
}

std::vector<StreetSegmentIdx> backTracing(const IntersectionIdx intersect_id_start,
                                          const IntersectionIdx intersect_id_destination,
                                          const std::vector<IntersectNaviInfo>&     IntersectNaviInfoList){

    std::deque<StreetSegmentIdx> path;
    auto curIntersectId = intersect_id_destination;

    auto previousEdge = IntersectNaviInfoList[curIntersectId].reachingEdge;
    std::cout<< IntersectNaviInfoList[curIntersectId].bestTime<<std::endl;

    while(previousEdge != -1){
        path.push_front(previousEdge);

        auto fromIntersect = SegsInfoList[previousEdge].segInfo.from;
        auto toIntersect = SegsInfoList[previousEdge].segInfo.to;

        if(toIntersect == curIntersectId){
            curIntersectId = fromIntersect;
        }else{
            curIntersectId = toIntersect;
        }
        previousEdge = IntersectNaviInfoList[curIntersectId].reachingEdge;
    }

    return std::vector<StreetSegmentIdx>(path.begin(), path.end());
}