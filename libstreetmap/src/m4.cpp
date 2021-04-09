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
std::vector<IntersectNaviInfo> IntersectNaviInfoList;

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

std::map<IntersectionIdx, std::map<IntersectionIdx, std::vector<StreetSegmentIdx> curPath>> PathStorage;


/// Intergrate Function
std::vector<CourierSubPath> travelingCourier(
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<int>& depots,
        const float turn_penalty){

    /// Step 1: MultiStart Dyjestra Method

    /// Step 2: Greedy Algo

    /// Step 3: 2/3 OPTs With Time Restriction

}

///MultiStart Dyjestra Method (NOT CHANGED YET)
void MultiStart_Dijkstra(
        std::vector<IntersectionIdx> relatedIntersect,
        const double turn_penalty){

    for()

    IntersectNaviInfoList.resize(getNumIntersections());

    bool pathExist = NaviInfoHelper(intersect_id_start, intersect_id_destination, turn_penalty);
    if(!pathExist) return {};
    IntersectNaviInfoList.clear();

    auto temp = backTracing(intersect_id_start, intersect_id_destination);

    std::cout << "Total TravelTime + TURNING pen" << computePathTravelTime(temp, turn_penalty) << std::endl;

    return temp;
}
bool NaviInfoHelper(
        const IntersectionIdx intersect_id_start,
        const IntersectionIdx intersect_id_destination,
        const double turn_penalty){

    auto cmp = [](WaveElem lhs, WaveElem rhs){return (lhs.travelTime) > (rhs.travelTime);};
    std::priority_queue<WaveElem, std::vector<WaveElem>, decltype(cmp)> WaveFront(cmp);

    WaveFront.push(WaveElem(intersect_id_start, -1, 0));

    while(!WaveFront.empty()){

        WaveElem currWave = WaveFront.top();
        WaveFront.pop();

        IntersectionIdx currIntersectId = currWave.IntersectId;

        if(currWave.travelTime < IntersectNaviInfoList[currIntersectId].bestTime){

            IntersectNaviInfoList[currIntersectId].reachingEdge = currWave.reachingEdge;
            IntersectNaviInfoList[currIntersectId].bestTime = currWave.travelTime;

            if(currIntersectId == intersect_id_destination){
                return true;
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
    return false;
}