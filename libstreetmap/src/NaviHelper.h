//
// Created by cheny811 on 2021-04-10.
//

#pragma once //protects against multiple inclusions of this header file
#include "DBstruct.h"
#include <float.h>
#include <utility>
#include <map>

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

extern std::map<int, std::map<int,PathInfo>> PathStorage;

std::vector<StreetSegmentIdx> backTracing(const IntersectionIdx intersect_id_start,
                                          const IntersectionIdx intersect_id_destination,
                                          const std::vector<IntersectNaviInfo>& IntersectNaviInfoList);