
#include "NaviHelper.h"
#include "m4.h"
#include "m1.h"
#include <queue>
#include <list>
#include <set>
#include <cstdlib>
#include <ctime>

std::map<int, std::map<int,PathInfo>> PathStorage;
std::vector<IntersectionIdx> DeliveryInfo;

void MultiDest_Dijkstra_method(const float turn_penalty);
void MultiDest_Dijkstra(std::set<IntersectionIdx> relatedIntersect,
                        const IntersectionIdx intersect_id_start, const double turn_penalty);
std::list<int> Greedy_Method(int delivSize, int depotSize);

std::list<int> twoOptSwap(std::list<int> srcPath, int delivSize){
    auto optModifyPath = srcPath;
    bool legal = false;
    while(!legal){
        auto itr = optModifyPath.begin(), itr1 = optModifyPath.begin();
        int cutPos = rand() % (optModifyPath.size()-1);

        std::advance(itr, cutPos);
        std::advance(itr1,cutPos+1);

        if(*itr != (*itr1) - delivSize){
            std::swap(*itr, *itr1);
            legal = true;
        }
    }
}

std::vector<CourierSubPath> travelingCourier(
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<int>& depots,
        const float turn_penalty){


    /// Step 0: assemble all Intersections and Pass into MultiStart_Dij
    for(auto temp : deliveries){
        DeliveryInfo.push_back(temp.pickUp);
    }
    for(auto temp : deliveries){
        DeliveryInfo.push_back(temp.dropOff);
    }
    for(auto temp : depots){
        DeliveryInfo.push_back(temp);
    }

    std::cout <<"Delivery Num:" <<deliveries.size()<<"\n";
    std::cout <<"Depot Num:" <<depots.size()<<"\n";
    std::cout <<"DEliverInfo Size " << DeliveryInfo.size()<<"\n";
    for(int temp: DeliveryInfo){
        std::cout<< temp <<" ";
    }std::cout <<"\n";

    /// Step 1: MultiDest Dyjestra Method
    MultiDest_Dijkstra_method(turn_penalty);

    // Throw expection of Delivery Point cannot be Reach
    for(int idx = 0; idx < deliveries.size()*2; idx++){
        if(PathStorage[DeliveryInfo[idx]][DeliveryInfo[0]].travelTime == DBL_MAX) return{};
    }

    /// Step 2: Greedy Algo || MultiStart
    std::list<int> greedyPath = Greedy_Method(deliveries.size(), depots.size());
    if(greedyPath.empty()) return {};

    /// Step 3: 2/3 OPTs With Time Restriction
    // Generate Random Seed
    srand(time(NULL));

    std::multimap<double, std::list<int>> optPathList;

    // Cut a random Position of greedyPath






    auto itr = optPathList.begin();
    while(itr != optPathList.end() || itr->totalTravelTime < saveOptPath.totalTravelTime){
        itr++;
    }
    if(itr == optPathList.end()){
        optPathList.push_back(saveOptPath);
    }
    else if(itr->totalTravelTime >= saveOptPath.totalTravelTime){
        optPathLIst.insert(itr, saveOptPath);
    }


    /// Step 4: cast list into CourierPath
    std::vector<CourierSubPath> courierPath;
    std::vector<int> tempGreedyPath(greedyPath.begin(), greedyPath.end());

    for(int i = 0; i < tempGreedyPath.size() - 1; i++){
        CourierSubPath tempPath;
        tempPath.start_intersection = DeliveryInfo[tempGreedyPath[i]];
        tempPath.end_intersection = DeliveryInfo[tempGreedyPath[i+1]];
        tempPath.subpath = PathStorage[tempPath.start_intersection][tempPath.end_intersection].curPath;
        courierPath.push_back(tempPath);
    }

    /// Step 5: Free the Global Value
    for(auto itr = PathStorage.begin(); itr!= PathStorage.end(); itr++){
        itr->second.clear();
    }
    PathStorage.clear();
    DeliveryInfo.clear();

    return courierPath;
}

/// Greedy Algo
std::list<int> Greedy_Method(int delivSize, int depotSize){

    std::list<int> greedyPath;

    double minTravelTime = DBL_MAX;
    int startingDepot;
    int nextIntersect = -1;

    // Find the closest starting Depot to Pickup
    for(int curPickup = 0; curPickup < delivSize; curPickup++){
        for(int curDepot = delivSize*2; curDepot < DeliveryInfo.size(); curDepot++){
            if(minTravelTime > PathStorage[DeliveryInfo[curDepot]][DeliveryInfo[curPickup]].travelTime){
                minTravelTime = PathStorage[DeliveryInfo[curDepot]][DeliveryInfo[curPickup]].travelTime;
                startingDepot = curDepot;
                nextIntersect = curPickup;
            }
        }
    }
    greedyPath.push_back(startingDepot);
    greedyPath.push_back(nextIntersect);



    // nextIntersect is the first pickup choosed from closest depot
    std::set<int> unpicked;
    std::set<int> undroped;

    // Insert all unpicked except for first pickUp
    for(int i = 0; i < delivSize; i++){
        if(i == nextIntersect) continue;
        unpicked.insert(i);
    }
    // Insert first drop Off
    undroped.insert(nextIntersect + delivSize);


    // Search through all Pickup & dropOff
    while(!unpicked.empty() || !undroped.empty()){

        // Initalize Local Variables
        minTravelTime = DBL_MAX;
        nextIntersect = -1;
        const int previIntersect = greedyPath.back();

        // Find the Closest PickUp/DropOff
        for(int curPickUp : unpicked){
            if(minTravelTime > PathStorage[DeliveryInfo[previIntersect]][DeliveryInfo[curPickUp]].travelTime){
                minTravelTime = PathStorage[DeliveryInfo[previIntersect]][DeliveryInfo[curPickUp]].travelTime;
                nextIntersect = curPickUp;
            }
        }
        for(int curDropOff : undroped){
            if(minTravelTime > PathStorage[DeliveryInfo[previIntersect]][DeliveryInfo[curDropOff]].travelTime){
                minTravelTime = PathStorage[DeliveryInfo[previIntersect]][DeliveryInfo[curDropOff]].travelTime;
                nextIntersect = curDropOff;
            }
        }

        // Dynamic Unpick/Undrop
        if(nextIntersect < delivSize){
            unpicked.erase(nextIntersect);
            undroped.insert(nextIntersect + delivSize);
        }else{
            undroped.erase(nextIntersect);
        }

        greedyPath.push_back(nextIntersect);
    }

    int endingDepot;
    minTravelTime = DBL_MAX;
    for(int curDepot = delivSize*2; curDepot < DeliveryInfo.size(); curDepot++){
        if(minTravelTime > PathStorage[DeliveryInfo[curDepot]][DeliveryInfo[nextIntersect]].travelTime){
            minTravelTime = PathStorage[DeliveryInfo[curDepot]][DeliveryInfo[nextIntersect]].travelTime;
            endingDepot = curDepot;
        }
    }
    greedyPath.push_back(endingDepot);
    return greedyPath;
}

///MultiStart Dijkstra Method
void MultiDest_Dijkstra(std::set<IntersectionIdx> relatedIntersect,
                        const IntersectionIdx intersect_id_start, const double turn_penalty){

    // Temparay Structure
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
                PathInfo tempPath;
                tempPath.travelTime = IntersectNaviInfoList[currIntersectId].bestTime;
                tempPath.curPath = backTracing(intersect_id_start, currIntersectId, IntersectNaviInfoList);
                PathStorage[intersect_id_start][currIntersectId] = tempPath;
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


    for(auto currIntersectId : relatedIntersect){
        PathInfo tempPath;
        tempPath.travelTime = DBL_MAX;
        PathStorage[intersect_id_start][currIntersectId] = tempPath;
    }

}

void MultiDest_Dijkstra_method(const float turn_penalty){
    std::set<IntersectionIdx> relatedIntersect(DeliveryInfo.begin(), DeliveryInfo.end());
    std::vector<IntersectionIdx> indexList(relatedIntersect.begin(),relatedIntersect.end());
#pragma omp parallel for
    for(int i = 0; i < indexList.size(); i++){
        MultiDest_Dijkstra(relatedIntersect, indexList[i], turn_penalty);
    }
}
