//
// Created by cheny811 on 2021-04-09.
//

/// Structure Area
struct IntersectNaviInfo{
    StreetSegmentIdx reachingEdge = -1;
    double bestTime = DBL_MAX;
    bool isTravel = false;
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

                double curTravelTime;
                LatLon fromPos = getIntersectionPosition(toIntersect);
                LatLon toPos = getIntersectionPosition(intersect_id_destination);
                double curDistance = findDistanceBetweenTwoPoints(std::make_pair(fromPos, toPos));
                double kVal = curDistance/60;
                if(currIntersectId != intersect_id_start &&
                   SegsInfoList[currStSegsId].segInfo.streetID !=
                   SegsInfoList[IntersectNaviInfoList[currIntersectId].reachingEdge].segInfo.streetID){
                    curTravelTime = IntersectNaviInfoList[currIntersectId].bestTime + curSegInfo.time + turn_penalty;

                    WaveFront.push(WaveElem(toIntersect, currStSegsId, curTravelTime, kVal+curTravelTime));
                }
                else{
                    curTravelTime = IntersectNaviInfoList[currIntersectId].bestTime + curSegInfo.time;
                    WaveFront.push(WaveElem(toIntersect, currStSegsId, curTravelTime, kVal+curTravelTime));
                }
            }
            IntersectNaviInfoList[currIntersectId].isTravel = true;
        }
    }
    return false;
}