
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

std::vector<CourierSubPath> create_courierPath(const std::list<int>& optPath);
void free_globals();

std::list<int> two_optPath_method(double timeLeft, const std::list<int>& greedyPath, int delivSize){
    std::list<int> optPath(greedyPath);

    auto startTime = std::chrono::high_resolution_clock::now();

    int num = optPath.size();
    double kVal = 0;
    if(num < 10){
        kVal = 0.1;
    }else if(num >=10 && num < 50){
        kVal = 0.2;
    }else if(num >=50 && num < 100){
        kVal = 0.5;
    }else if(num >=100){
        kVal = 0.9;
    }
    timeLeft *= kVal;

    double cost = check_path_time(optPath);
    //std::cout << "GreedyCost: " << cost << "\n";
    bool timeOut = false;

    while (!timeOut) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto wallClock = std::chrono::duration_cast < std::chrono::duration < double >> (currentTime - startTime);
        double T = wallClock.count();

        auto modifyPath = twoOpt(optPath, delivSize);
        double modifyCost = check_path_time(modifyPath);
        double deltaCost = modifyCost - cost;
        if (modifyCost < cost || rand()%2 < exp(-deltaCost * T)) {
            optPath = modifyPath;
            cost = modifyCost;
        }
        if (timeLeft < T) {
            timeOut = true;
        }
    }
    //std::cout << "OptCost: " << cost << "\n";
    return optPath;
}
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

//    std::cout <<"Delivery Num:" <<deliveries.size()<<"\n";
//    std::cout <<"Depot Num:" <<depots.size()<<"\n";
//    std::cout <<"DEliverInfo Size " << DeliveryInfo.size()<<"\n";

    /// Step 1: MultiDest Dyjestra Method
    MultiDest_Dijkstra_method(turn_penalty);

    // Throw expection of Delivery Point cannot be Reach
    for(int idx = 0; idx < deliveries.size()*2; idx++){
        if(PathStorage[DeliveryInfo[idx]][DeliveryInfo[0]].travelTime == DBL_MAX) return{};
    }

    /// Step 2: Greedy Algo
    std::list<int> greedyPath = Greedy_Method(deliveries.size(), depots.size());
    if(greedyPath.empty()) return{};
    if(greedyPath.size() == 4){
        auto couierPath = create_courierPath(greedyPath);
        free_globals();
        return couierPath;
    }

    auto endTime=std::chrono::high_resolution_clock::now();
    auto wallClock = std::chrono::duration_cast < std::chrono::duration < double >> (endTime - startTime);
    /// Step 3: 2/3 OPTs With Time Restriction
//    greedyPath.pop_back();
//    greedyPath.pop_front();
//
//    // TwoOptMethod
//    auto optPath = two_optPath_method(TIME_LIMIT - wallClock.count(), greedyPath, deliveries.size());
//
//    optPath.push_front(find_closest_depot(optPath.front(), deliveries.size()));
//    optPath.push_back(find_closest_depot(optPath.back(), deliveries.size()));
//
//    /// Step 4: cast list into CourierPath
    auto courierPath = create_courierPath(greedyPath);

    /// Step 5: Free the Global Value
    free_globals();

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

    PathStorage[intersect_id_start];

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

                PathStorage[intersect_id_start].insert(std::make_pair(currIntersectId, tempPath));

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

std::vector<CourierSubPath> create_courierPath(const std::list<int>& optPath){
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
    return courierPath;
}

void free_globals(){
    for(auto & i : PathStorage){
        i.second.clear();
    }
    PathStorage.clear();
    DeliveryInfo.clear();
}