//
// Created by cheny811 on 2021-03-27.
//
#include "m3.h"
#include "m1.h"
#include "DBstruct.h"
#include <queue>
#include <deque>
#include <float.h>
#include <utility>

struct IntersectNaviInfo{
    StreetSegmentIdx reachingEdge = -1;
    double bestTime = DBL_MAX;
    bool isTravel = false;
    double bestCost = DBL_MAX;
};
std::vector<IntersectNaviInfo> IntersectNaviInfoList;

double lineDistance;

struct WaveElem{
    int IntersectId;
    int reachingEdge;
    double travelTime;
    double cost;

    WaveElem (int curI, int rE, double tT, double cT) {
        IntersectId = curI;
        reachingEdge = rE;
        travelTime = tT;
        cost = cT;

    }
};



double computePathTravelTime(const std::vector<StreetSegmentIdx>& path, const double turn_penalty){
    double totalTurnPenalty = 0;
    if(path.size() > 2){
        for(int i=0 ; i < path.size()-1 ; i++ ){
            if(SegsInfoList[path[i]].segInfo.streetID != SegsInfoList[path[i+1]].segInfo.streetID){
                totalTurnPenalty+=turn_penalty;
            }
        }
    }
    double totalTraveTime = 0;
    for(auto curStSegId : path){
        totalTraveTime += SegsInfoList[curStSegId].time;
    }

    return totalTraveTime + totalTurnPenalty;
}

bool NaviInfoHelper(const IntersectionIdx intersect_id_start,
                    const IntersectionIdx intersect_id_destination,
                    const double turn_penalty);
std::vector<StreetSegmentIdx> backTracing(const IntersectionIdx intersect_id_start,
                                          const IntersectionIdx intersect_id_destination);


std::vector<StreetSegmentIdx> findPathBetweenIntersections(
        const IntersectionIdx intersect_id_start,
        const IntersectionIdx intersect_id_destination,
        const double turn_penalty){

    LatLon fromPos = getIntersectionPosition(intersect_id_start);
    LatLon toPos = getIntersectionPosition(intersect_id_destination);
    lineDistance = findDistanceBetweenTwoPoints(std::make_pair(fromPos, toPos));

    IntersectNaviInfoList.resize(getNumIntersections());

    std::vector<StreetSegmentIdx> path;
    bool pathExist = NaviInfoHelper(intersect_id_start, intersect_id_destination, turn_penalty);
    if(pathExist) path = backTracing(intersect_id_start, intersect_id_destination);

    IntersectNaviInfoList.clear();
    return path;
}

bool NaviInfoHelper(
        const IntersectionIdx intersect_id_start,
        const IntersectionIdx intersect_id_destination,
        const double turn_penalty){

    auto cmp = [](WaveElem lhs, WaveElem rhs){
        return (lhs.cost) > (rhs.cost);
    };
    std::priority_queue<WaveElem, std::vector<WaveElem>, decltype(cmp)> WaveFront(cmp);

    WaveFront.push(WaveElem(intersect_id_start, -1, 0, 0));

    while(!WaveFront.empty()){
        WaveElem currWave = WaveFront.top();
        WaveFront.pop();

        IntersectionIdx currIntersectId = currWave.IntersectId;

        if(currWave.cost < IntersectNaviInfoList[currIntersectId].bestCost){

            IntersectNaviInfoList[currIntersectId].reachingEdge = currWave.reachingEdge;
            IntersectNaviInfoList[currIntersectId].bestTime = currWave.travelTime;
            IntersectNaviInfoList[currIntersectId].bestCost = currWave.cost;

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

                double curTravelTime;
                LatLon fromPos = getIntersectionPosition(toIntersect);
                LatLon toPos = getIntersectionPosition(intersect_id_destination);
                double y1 = fromPos.latitude()*kDegreeToRadian;

                double y2 = toPos.latitude()*kDegreeToRadian;

                double latAvg=((y1+y2)/2);
                double x1 =fromPos.longitude() * cos(latAvg)* kDegreeToRadian;
                double x2 =toPos.longitude()* cos(latAvg)* kDegreeToRadian;
                double curDistanceTime = (abs(kEarthRadiusInMeters*(y2-y1)) + abs(kEarthRadiusInMeters*(x2-x1)))/SegsInfoList[toIntersect].segInfo.speedLimit;

                double kVal ;
                if(currIntersectId != intersect_id_start &&
                SegsInfoList[currStSegsId].segInfo.streetID !=
                SegsInfoList[IntersectNaviInfoList[currIntersectId].reachingEdge].segInfo.streetID){
                    curTravelTime = IntersectNaviInfoList[currIntersectId].bestTime + curSegInfo.time + turn_penalty;
                    kVal= curDistanceTime/100 +curTravelTime;
                    WaveFront.push(WaveElem(toIntersect, currStSegsId, curTravelTime, kVal));
                }
                else{
                    curTravelTime = IntersectNaviInfoList[currIntersectId].bestTime + curSegInfo.time;
                    kVal= curDistanceTime/100 +curTravelTime;
                    WaveFront.push(WaveElem(toIntersect, currStSegsId, curTravelTime, kVal));
                }
            }
            IntersectNaviInfoList[currIntersectId].isTravel = true;
        }
    }
    return false;
}

std::vector<StreetSegmentIdx> backTracing(const IntersectionIdx intersect_id_start,const IntersectionIdx intersect_id_destination){

    std::deque<StreetSegmentIdx> path;
    auto curIntersectId = intersect_id_destination;
    std::cout << IntersectNaviInfoList[curIntersectId].bestTime << std::endl;

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