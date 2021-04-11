
#include "NaviHelper.h"
#include "m4.h"
#include "m1.h"
#include <queue>
#include <set>
///This is a new line
std::map<int, std::map<int,PathInfo>> PathStorage;
void MultiDest_Dijkstra(const IntersectionIdx intersect_id_start,
                        const std::set<IntersectionIdx>& allIntersects,
                        const double turn_penalty);


void Greedy_Method(const std::vector<DeliveryInf>& deliveries,
                   const std::vector<int>& depots){
    std::vector<CourierSubPath> courierPaths;


    double minFirstTime = DBL_MAX;
    IntersectionIdx firstIntersect;//start Depot
    int firstId;//pickup or drop off Id
    IntersectionIdx secondIntersect;//1st pickUp
    int secondId;//pickup or drop off Id

    for(int curDepot=delivSize*2; curDepot < DeliveryInfo.size(); curDepot++){
        for(int curPickup=0; curPickup < delivSize; curPickup++){
            /*IntersectionIdx tempDepotIdx = DeliveryInfo[curDepot];
            IntersectionIdx tempDelivIdx = DeliveryInfo[curPickup];
            double tempMinTime = PathStorage[tempDepotIdx][tempDelivIdx].travelTime;
            if(minFirstTime>tempMinTime){
                firstIntersect=tempDepotIdx;
                secondIntersect=tempDelivIdx;
                minFirstTime=tempMinTime;
            }
        }
    }
   // greedyPath.push_back(firstIntersect);
   // greedyPath.push_back(secondIntersect);


//    std::map<int, IntersectionIdx> unpicked;
//    std::map<int, IntersectionIdx> undroped;
//
//    //Insert all unpicked except for first pickUp
//    for(int i=0;i<deliveries.size();i++){
//        if(i!=secondId){
//            std::pair <int,IntersectionIdx> temp=std::make_pair(i,deliveries[i].pickUp);
//            unpicked.insert(temp);
//        }
//    }
//    //Insert first drop Off
//    std::pair <int,IntersectionIdx> firstDropOff=std::make_pair(secondId,deliveries[secondId].dropOff);
//    undroped.insert(firstDropOff);
//
//    //Switch intsections
//    //firstIntersect=secondIntersect;
//
//    int delivCounter=deliveries.size();
//    while(delivCounter!=0){
//        firstIntersect=secondIntersect;
//        bool isdroped=false;
//        double minPathTime = DBL_MAX;
//        for(auto it=undroped.begin();it!=undroped.end();it++){
//            double tempTime;
//            int index=it->first;
//            IntersectionIdx curIntersectIdx=it->second;
//            std::map<int, PathInfo>temp =PathStorage[firstIntersect];
//            tempTime = temp[curIntersectIdx].travelTime;
//            if(tempTime<minPathTime){
//                secondId=index;
//                secondIntersect=curIntersectIdx;
//                minPathTime=tempTime;
//                intersectPath=temp[curIntersectIdx].curPath;
//            }
//        }
//        for(auto it=undroped.begin();it!=unpicked.end();it++){
//            double tempTime;
//            int index=it->first;
//            IntersectionIdx curIntersectIdx=it->second;
//            std::map<int, PathInfo>temp =PathStorage[firstIntersect];
//            tempTime = temp[curIntersectIdx].travelTime;
//            if(tempTime<minPathTime){
//                secondId=index;
//                delivCounter--;
//                secondIntersect=curIntersectIdx;
//                isdroped= true;
//                minPathTime=tempTime;
//                intersectPath=temp[curIntersectIdx].curPath;
//            }
//        }
//        if(isdroped==true){
//            delivCounter--;//need multiple
//            undroped.erase(secondId);
//        }else{
//            unpicked.erase(secondId);//need multiple
//            std::pair <int,IntersectionIdx> temp=std::make_pair(secondId,deliveries[secondId].dropOff);
//            undroped.insert(temp);//need multiple
//        }
//    }
}

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
    Greedy_Method(deliveries, depots);

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
