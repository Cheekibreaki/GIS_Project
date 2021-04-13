
#include "NaviHelper.h"
#include "m4.h"
#include "m1.h"
#include <queue>
#include <list>
#include <set>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#define TIME_LIMIT 50

std::map<int, std::map<int,PathInfo>> PathStorage;
std::vector<IntersectionIdx> DeliveryInfo;

void MultiDest_Dijkstra_method(const float turn_penalty);
void MultiDest_Dijkstra(std::set<IntersectionIdx> relatedIntersect,
                        const IntersectionIdx intersect_id_start, const double turn_penalty);
std::list<int> Greedy_Method(int delivSize, int depotSize);


std::list<int> twoOpt(const std::list<int>& srcPath, int delivSize);
double check_path_time(std::list<int> Path);
bool check_legal(std::list<int> Path, int delivSize);
int find_closest_depot(int checkIntersect, int delivSize);

std::vector<CourierSubPath> travelingCourier(
        const std::vector<DeliveryInf>& deliveries,
        const std::vector<int>& depots,
        const float turn_penalty){

    srand(time(NULL));
    auto startTime=std::chrono::high_resolution_clock::now();

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
    std::cout <<"Running MultiDest....\n";
    MultiDest_Dijkstra_method(turn_penalty);

    // Throw expection of Delivery Point cannot be Reach
    for(int idx = 0; idx < deliveries.size()*2; idx++){
        if(PathStorage[DeliveryInfo[idx]][DeliveryInfo[0]].travelTime == DBL_MAX) return{};
    }

    /// Step 2: Greedy Algo
    std::cout <<"Running greedyAlgo....\n";
    std::list<int> greedyPath = Greedy_Method(deliveries.size(), depots.size());

    /// Step 3: 2/3 OPTs With Time Restriction
    //std::multimap<double, std::list<int>> optPathList;
    std::list<int> optPath(greedyPath);
    optPath.pop_back();
    optPath.pop_front();

    double cost = check_path_time(optPath);
    std::cout <<"GreedyCost: "<<cost <<"\n";
    double T = TIME_LIMIT;
    //bool noChange = false;

    while(T > 47 /*&& !noChange*/){
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto wallClock = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - startTime);
        T = TIME_LIMIT - wallClock.count();
        std::cout <<"TIME LEFT:" <<T<<"\n";
        /*auto modifyPath = twoOpt(optPath, deliveries.size());
        double modifyCost = check_path_time(modifyPath);
        double deltaCost = modifyCost - cost;
        if(modifyCost < cost *//*|| rand() % 2 < exp(-deltaCost/(T0.9))*//*){
            optPath = modifyPath;
            cost = modifyCost;
        }*/
        for(int i=0;i<10000;i++){
            for(int j =0; j<1000; j++){}
        }
    }
    std::cout <<"OptCost: "<<cost <<"\n";

    int startingDepot= find_closest_depot(optPath.front(), deliveries.size());
    int endingDepot = find_closest_depot(optPath.back(), deliveries.size());

    optPath.push_front(startingDepot);
    optPath.push_back(endingDepot);

    /// Step 4: cast list into CourierPath
    std::vector<CourierSubPath> courierPath;
    auto itr = optPath.begin();
    std::advance(itr, 1);
    int lastNum = optPath.front();
    for(; itr != optPath.end(); itr++){
        CourierSubPath tempPath;
        tempPath.start_intersection = DeliveryInfo[lastNum];
        tempPath.end_intersection = DeliveryInfo[*itr];
        tempPath.subpath = PathStorage[tempPath.start_intersection][tempPath.end_intersection].curPath;
        courierPath.push_back(tempPath);
        lastNum = *itr;
    }

    /// Step 5: Free the Global Value
    for(auto & i : PathStorage){
        i.second.clear();
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

        int closestDepot = find_closest_depot(curPickup, delivSize);

        if(minTravelTime > PathStorage[DeliveryInfo[closestDepot]][DeliveryInfo[curPickup]].travelTime){
            minTravelTime = PathStorage[DeliveryInfo[closestDepot]][DeliveryInfo[curPickup]].travelTime;
            startingDepot = closestDepot;
            nextIntersect = curPickup;
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


    greedyPath.push_back(find_closest_depot(nextIntersect, delivSize));
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




// Useful Helper Functions
std::list<int> twoOpt(const std::list<int>& srcPath, int delivSize){
    std::list<int> optModifyPath, midPath;

    bool legal = false;
    while(!legal){
        optModifyPath.clear();
        optModifyPath = srcPath;

        auto cutItr1 = optModifyPath.begin(), cutItr2 = optModifyPath.begin();

        int cutPos1 = rand() % (optModifyPath.size()-1);
        int cutPos2 = rand() % (optModifyPath.size() - cutPos1 - 1) + cutPos1+2;

        std::advance(cutItr1, cutPos1);
        std::advance(cutItr2, cutPos2);

        midPath.splice(midPath.begin(), optModifyPath, cutItr1, cutItr2);

        midPath.reverse();
        legal = check_legal(midPath, delivSize);

        optModifyPath.splice(cutItr2, midPath);
    }
    return optModifyPath;
}

double check_path_time(std::list<int> Path){
    double totalTravelTime = 0;
    auto itr = Path.begin();
    std::advance(itr, 1);
    int lastNum = Path.front();
    for(; itr != Path.end(); itr++){
        totalTravelTime += PathStorage[DeliveryInfo[lastNum]][DeliveryInfo[*itr]].travelTime;
        lastNum = *itr;
    }
    return totalTravelTime;
}

bool check_legal(std::list<int> Path, int delivSize){
    for(auto itr = Path.begin(); itr != Path.end(); itr++) {
        if(*itr >= delivSize){
            for(auto itr1 = itr; itr1 != Path.end(); itr1++){
                if(*itr-delivSize == *itr1){
                    return false;
                }
            }
        }
    }
    return true;
}

int find_closest_depot(int checkIntersect, int delivSize){
    double minTravelTime = DBL_MAX;
    int endingDepot;
    for(int curDepot = delivSize*2; curDepot < DeliveryInfo.size(); curDepot++){
        if(minTravelTime > PathStorage[DeliveryInfo[curDepot]][DeliveryInfo[checkIntersect]].travelTime){
            minTravelTime = PathStorage[DeliveryInfo[curDepot]][DeliveryInfo[checkIntersect]].travelTime;
            endingDepot = curDepot;
        }
    }
    return endingDepot;
}