//
// Created by cheny811 on 2021-03-27.
//
#include "m3.h"
#include "m1.h"
#include "DBstruct.h"
#include <queue>

struct IntersectNaviInfo{
    StreetSegmentIdx reachingEdge;
    double bestTime;
};
std::vector<IntersectNaviInfo> IntersectNaviInfoList;

struct WaveElem{
    int curIntersect;
    int reachingEdge;
    double travelTime;
    WaveElem (int curI, int rE, double tT) {
        curIntersect = curI;
        reachingEdge = rE;
        travelTime = tT;
    }
};

double computePathTravelTime(const std::vector<StreetSegmentIdx>& path, const double turn_penalty){
    return -1;
}

std::vector<StreetSegmentIdx> findPathBetweenIntersections(const IntersectionIdx intersect_id_start,
        const IntersectionIdx intersect_id_destination, const double turn_penalty){
    return {};
}
