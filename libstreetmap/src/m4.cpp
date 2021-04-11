
#include "NaviHelper.h"
#include "m4.h"
#include "m1.h"
#include <queue>
#include <set>

std::map<int, std::map<int,PathInfo>> PathStorage;
void MultiDest_Dijkstra(const IntersectionIdx intersect_id_start,
                        const std::set<IntersectionIdx>& allIntersects,
                        const double turn_penalty);


std::vector<CourierSubPath> travelingCourier(
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<int>& depots,
        const float turn_penalty){

    /// Step 1: MultiDest Dyjestra Method

    // assemble all Intersections and Pass into MultiStart_Dij
    std::set<IntersectionIdx> allIntersect;
    for(auto temp : deliveries){
        allIntersect.insert(temp.pickUp);
        allIntersect.insert(temp.dropOff);
    }
    allIntersect.insert(depots.begin(), depots.end());

    for(auto curIntersect : allIntersect){
        MultiDest_Dijkstra(curIntersect, allIntersect, turn_penalty);
    }

    /// Step 2: Greedy Algo || MultiStart


    /// Step 3: 2/3 OPTs With Time Restriction


    /// Step 4: Free the Global Value
    for(auto itr = PathStorage.begin(); itr!= PathStorage.end(); itr++){
        itr->second.clear();
    }
    PathStorage.clear();
    return{};
}

///MultiStart Dijkstra Method

void MultiDest_Dijkstra(const IntersectionIdx intersect_id_start,
                        const std::set<IntersectionIdx>& allIntersects,
                        const double turn_penalty){

    // Temparay Structure
    std::set<IntersectionIdx> relatedIntersect = allIntersects;
    std::vector<IntersectNaviInfo> IntersectNaviInfoList(getNumIntersections());

    // Temparay Structure: WaveFront Initialization
    auto cmp = [](WaveElem lhs, WaveElem rhs){
        return (lhs.travelTime) > (rhs.travelTime);
    };
    std::priority_queue<WaveElem, std::vector<WaveElem>, decltype(cmp)> WaveFront(cmp);

    // Push The First Wave Element
    WaveFront.push(WaveElem(intersect_id_start, -1, 0));

    while(!WaveFront.empty()){
        WaveElem currWave = WaveFront.top();
        WaveFront.pop();

        IntersectionIdx currIntersectId = currWave.IntersectId;

        if(currWave.travelTime < IntersectNaviInfoList[currIntersectId].bestTime){

            IntersectNaviInfoList[currIntersectId].reachingEdge = currWave.reachingEdge;
            IntersectNaviInfoList[currIntersectId].bestTime = currWave.travelTime;

            // Find it currentIntersect is one of the relatedIntersect
            auto itr = relatedIntersect.find(currIntersectId);

            // If yes Then remove from list and save the Path information to Storage
            if(itr != relatedIntersect.end()){
                PathStorage[intersect_id_start][currIntersectId].travelTime = IntersectNaviInfoList[currIntersectId].bestTime;
                PathStorage[intersect_id_start][currIntersectId].curPath  = backTracing(intersect_id_start, currIntersectId, IntersectNaviInfoList);
                relatedIntersect.erase(itr);
            }

            //If the related Intersect list is empty then return from Dystra
            if(relatedIntersect.empty()){
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

                double curTravelTime;
                if(currIntersectId != intersect_id_start &&
                   SegsInfoList[currStSegsId].segInfo.streetID !=
                   SegsInfoList[IntersectNaviInfoList[currIntersectId].reachingEdge].segInfo.streetID){
                    curTravelTime = IntersectNaviInfoList[currIntersectId].bestTime + curSegInfo.time + turn_penalty;
                    WaveFront.push(WaveElem(toIntersect, currStSegsId, curTravelTime));
                }
                else{
                    curTravelTime = IntersectNaviInfoList[currIntersectId].bestTime + curSegInfo.time;
                    WaveFront.push(WaveElem(toIntersect, currStSegsId, curTravelTime));
                }
            }
            IntersectNaviInfoList[currIntersectId].isTravel = true;
        }
    }
}
