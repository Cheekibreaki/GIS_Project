//
// Created by cheny811 on 2021-03-27.
//
#include "m3.h"
#include "m1.h"
#include "DBstruct.h"
#include <queue>

struct IntersectNaviInfo{
    StreetSegmentIdx reachingEdge = -1;
    double bestTime = std::numeric_limits<double>::max();
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
double computePathTravelTime(const std::vector<StreetSegmentIdx>& path, const double turn_penalty){
    return -1;
}

bool NaviInfoHelper(
        const IntersectionIdx intersect_id_start,
        const IntersectionIdx intersect_id_destination){

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

                // Save To INTERSECTIONINFO into the WaveFront
                // Initalize with WaveElem("curIntersect", "reachingEdge", "travelTime")
                WaveFront.push(WaveElem(currIntersectId,
                                        currStSegsId,
                                        IntersectNaviInfoList[currIntersectId].bestTime + curSegInfo.time));
            }
        }

    }
    return false;
}

std::vector<StreetSegmentIdx> backTracing(const IntersectionIdx intersect_id_start,
                                          const IntersectionIdx intersect_id_destination){

    std::deque<StreetSegmentIdx> path;

    auto curIntersectId = intersect_id_destination;
    auto previousEdge = IntersectNaviInfoList[curIntersectId].reachingEdge;

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

std::vector<StreetSegmentIdx> findPathBetweenIntersections(
        const IntersectionIdx intersect_id_start,
        const IntersectionIdx intersect_id_destination,
        const double turn_penalty){
    std::cout << "Running Navigation" << std::endl;
    IntersectNaviInfoList.resize(getNumIntersections());

    bool pathExist = NaviInfoHelper(intersect_id_start, intersect_id_destination);
    if(!pathExist) return {};
    IntersectNaviInfoList.clear();

    return backTracing(intersect_id_start, intersect_id_destination);
}
