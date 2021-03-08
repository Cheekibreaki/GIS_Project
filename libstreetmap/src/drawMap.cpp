//
// Created by cheny811 on 2021-03-03.
//

#include <iostream>

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"

#include "m1.h"
#include "DBstruct.h"
#include "m2.h"

//struct naturalFeature{
//    std::string name;
//    FeatureType type;
//    std::vector<ezgl::point2d> polyList;
//    bool isPoly = false;
//};

//std::vector<naturalFeature> naturalFeatureList;

float legendLength;
void calcLegendLength(ezgl::renderer *g);

double avg_lat = 0;
double min_lat, max_lat, min_lon, max_lon;
void calc_avg_lat();

std::vector<intersect_info> IntersectInfoList;
void InitDrawingStructure();
void CloseDrawingStructure();

void draw_main_canvas (ezgl::renderer *g);
void draw_intersection(ezgl::renderer *g);
void draw_streetSeg(ezgl::renderer *g);
void draw_naturalFeature(ezgl::renderer *g);
void draw_legend(ezgl::renderer *g);
void drawLabelList(ezgl::renderer *g, std::vector<ezgl::point2d> point_list, std::string png_path);

void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_mouse_move(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_key_press(ezgl::application *application, GdkEventKey *event, char *key_name);
void initial_setup(ezgl::application *application, bool new_window);

//void set_up_naturalFeature();

//void set_up_naturalFeature(){
//    naturalFeatureList.resize(getNumFeatures());
//    for(FeatureIdx feature_id=0; feature_id<getNumFeatures() ; feature_id++){
//        naturalFeatureList[feature_id].name=getFeatureName(feature_id);
//        naturalFeatureList[feature_id].type=getFeatureType(feature_id);
//        naturalFeatureList[feature_id].polyList.resize(getNumFeaturePoints(feature_id));
//
//            for(int i= 0; i < getNumFeaturePoints(feature_id); i++){
//                LatLon temp=getFeaturePoint(feature_id,i);
//                double x= x_from_lon(temp.longitude());
//                double y= y_from_lat(temp.latitude());
//                naturalFeatureList[feature_id].polyList[i]= ezgl::point2d(x,y);
//            }
//        if(findFeatureArea(feature_id)!=-1 && naturalFeatureList[feature_id].polyList.size()>1){
//            naturalFeatureList[feature_id].isPoly=true;
//        }
//    }
//
//}
void InitDrawingStructure(){

    calc_avg_lat();
    LoadIntersectInfoList();

}
void CloseDrawingStructure(){

    IntersectInfoList.clear();

}

void drawMap(){

    //set_up_naturalFeature();
    InitDrawingStructure();


    ezgl::application::settings settings;
    settings.main_ui_resource   =   "libstreetmap/resources/main.ui";
    settings.window_identifier  =   "MainWindow";
    settings.canvas_identifier  =   "MainCanvas";

    ezgl::application application(settings);

    //set init coordinate system
    ezgl::rectangle initial_world = ezgl::rectangle{{x_from_lon(min_lon),y_from_lat(min_lat)},
                                  {x_from_lon(max_lon),y_from_lat(max_lat)}};



    application.add_canvas("MainCanvas", draw_main_canvas, initial_world,ezgl::GREY_55);



    application.run(initial_setup, act_on_mouse_press,
                    act_on_mouse_move, act_on_key_press);

    CloseDrawingStructure();
}



/*Render drawing main Canvas*/

void draw_main_canvas(ezgl::renderer *g){
    draw_intersection(g);
    draw_naturalFeature(g);
    draw_streetSeg(g);
    draw_legend(g);
}

void draw_legend(ezgl::renderer *g){
    g->set_coordinate_system(ezgl::SCREEN);

    g->set_color(255, 255, 255, 100);
    g->fill_rectangle({10, 10}, {130, 30});

    g->set_color(0, 0, 0, 255);
    g->set_line_width(2);
    g->draw_line({20, 25}, {120, 25});
    g->draw_line({20, 25}, {20, 20});
    g->draw_line({120, 25}, {120, 20});

    calcLegendLength(g);
    std::string legendText = std::to_string(legendLength);

    g->draw_text({70,18},legendText);

    g->set_coordinate_system(ezgl::WORLD);
}

///Find osm for further modification Not Finished
void draw_streetSeg(ezgl::renderer *g) {

    for(int segIdx = 0; segIdx<SegListSegInfo.size(); segIdx++){
        double curSegSpeed = SegListSegInfo[segIdx].speedLimit;
        if( legendLength > 1000 && (curSegSpeed-13.888) <0.1 && (curSegSpeed-13.888)>0.0){
            continue;
        }else{
            g->set_color(255,255,255,255);
            if(curSegSpeed > 16.7){
                g->set_color(250,226,211,255);
            }
        }

        ezgl::point2d fromPos=IntersectInfoList[SegListSegInfo[segIdx].from].curPosXY;
        ezgl::point2d toPos=IntersectInfoList[SegListSegInfo[segIdx].to].curPosXY;



        int numCurvePoints = SegListSegInfo[segIdx].numCurvePoints;
        if(numCurvePoints != 0){
            //start of fromPos connect first curvePoint
            ezgl::point2d lastCurvePos = fromPos;

            //for loop through all curvePoint
            for(int curCurvePointNum=0;curCurvePointNum < numCurvePoints;curCurvePointNum++){
                ezgl::point2d tempCurvePos = LatLon_to_point2d(getStreetSegmentCurvePoint(segIdx,curCurvePointNum));

                g->draw_line(tempCurvePos,lastCurvePos);
                lastCurvePos = tempCurvePos;
            }
            //draw the last curvePoint to toPos
            g->draw_line(lastCurvePos,toPos);

        }else{
            g->draw_line(fromPos,toPos);
        }
    }
}
void draw_naturalFeature(ezgl::renderer *g){
    for(FeatureIdx feature_id=0; feature_id<getNumFeatures() ; feature_id++){

        std::vector<ezgl::point2d>polyList;
        polyList.resize(getNumFeaturePoints(feature_id));

        for(int i= 0; i < getNumFeaturePoints(feature_id); i++){
            LatLon temp=getFeaturePoint(feature_id,i);
            double x= x_from_lon(temp.longitude());
            double y= y_from_lat(temp.latitude());
            polyList[i]= ezgl::point2d(x,y);
        }

        if(getFeatureType(feature_id)==0) {
            g->set_color(255,228,225);
        }
        if(getFeatureType(feature_id)==1){
            g->set_color(128,128,0);
        }
        if(getFeatureType(feature_id)==2){
            g->set_color(251,239,199);
        }
        if(getFeatureType(feature_id)==3){
            g->set_color(185,208,251);
        }
        if(getFeatureType(feature_id)==4){
            g->set_color(185,208,251);
        }
        if(getFeatureType(feature_id)==5){
            g->set_color(105,105,105);
        }
        if(getFeatureType(feature_id)==6){
            g->set_color(214,214,214);
        }
        if(getFeatureType(feature_id)==7){
            g->set_color(206,222,175);
        }
        if(getFeatureType(feature_id)==8){
            g->set_color(50,205,50);
        }

        if(findFeatureArea(feature_id)!=-1 && polyList.size()>1) {
            if (legendLength < 300 && getFeatureType(feature_id) == 6){
                g->fill_poly(polyList);
        }
            else {g->fill_poly(polyList);}

        }
        if(polyList[0]!=polyList[getNumFeaturePoints(feature_id)-1]) {
                if(getFeatureType(feature_id)!=6) {
                    for (int i = 0; i < polyList.size() - 1; i++) {
                        g->draw_line({polyList[i].x, polyList[i].y}, {polyList[i + 1].x, polyList[i + 1].y});
                    }
                }else if(getFeatureType(feature_id)==6 && legendLength<300){
                    for (int i = 0; i < polyList.size() - 1; i++) {
                        g->draw_line({polyList[i].x, polyList[i].y}, {polyList[i + 1].x, polyList[i + 1].y});
                    }
                }
        }

        }



}
void draw_intersection(ezgl::renderer *g){
    for(IntersectionIdx id = 0; id < IntersectListOfLatLon.size(); id++){
        float x = IntersectInfoList[id].curPosXY.x;
        float y = IntersectInfoList[id].curPosXY.y;

        if(IntersectInfoList[id].highlight){
            g->set_color(ezgl::RED);
        }else{
            g->set_color(ezgl::CYAN);
        }

        float width = 5;
        float height = width;
        g->fill_rectangle({x, y},{x + width, y + height});
    }

    /*std::vector<ezgl::point2d> tempList;
    for(IntersectionIdx id = 0; id < 10; id++){
        tempList.push_back(IntersectInfoList[id].curPosXY);
    }
    drawLabelList(g, tempList, "libstreetmap/resources/labels/pin_pos.png");*/
}

/*User interaction*/


void act_on_mouse_move(ezgl::application *application, GdkEventButton *event, double x, double y){

}
void act_on_key_press(ezgl::application *application, GdkEventKey *event, char *key_name){

}
void initial_setup(ezgl::application *application, bool new_window){

}

void act_on_mouse_press(ezgl::application* app, GdkEventButton* event, double x, double y){
    std::cout << "Mouse clicked at (" <<x<< "," <<y<< ")\n";
    LatLon pos = LatLon(lat_from_y(y),lon_from_x(x));
    int id = findClosestIntersection(pos);

    std::cout << "Closest Intersection: "<< IntersectInfoList[id].name << "\n";
    for(int i=0;i<IntersectListOfSegsList[id].size();i++)
    std::cout << "Closest Seg speed "<< getStreetSegmentInfo(IntersectListOfSegsList[id][i]).speedLimit << "\n";
    IntersectInfoList[id].highlight = true;

    app->refresh_drawing();
}
/*Supportive Helper Functions*/


void calc_avg_lat(){
    min_lat = IntersectListOfLatLon[0].latitude();
    max_lat = min_lat;
    min_lon = IntersectListOfLatLon[0].longitude();
    max_lon = min_lon;

    for(IntersectionIdx id = 0; id < IntersectListOfLatLon.size(); id++){

        min_lat = std::min(min_lat, IntersectListOfLatLon[id].latitude());
        max_lat = std::max(max_lat, IntersectListOfLatLon[id].latitude());
        min_lon = std::min(min_lon, IntersectListOfLatLon[id].longitude());
        max_lon = std::max(max_lon, IntersectListOfLatLon[id].longitude());
    }

    avg_lat = (min_lat + max_lat) / 2;
}

void LoadIntersectInfoList(){
    IntersectInfoList.resize(getNumIntersections());

    for(IntersectionIdx id = 0; id < IntersectListOfLatLon.size(); id++){
        IntersectInfoList[id].curPosXY = LatLon_to_point2d(IntersectListOfLatLon[id]);
        IntersectInfoList[id].name = getIntersectionName(id);
    }
}

double x_from_lon(float lon){
    return lon * kDegreeToRadian * kEarthRadiusInMeters * std::cos(avg_lat * kDegreeToRadian);
}
double y_from_lat(float lat){
    return lat * kDegreeToRadian * kEarthRadiusInMeters;
}
double lon_from_x(float x){
    return x / (kDegreeToRadian * kEarthRadiusInMeters * std::cos(avg_lat * kDegreeToRadian));
}
double lat_from_y(float y){
    return y / kDegreeToRadian / kEarthRadiusInMeters;
}
ezgl::point2d LatLon_to_point2d(LatLon curLatLon){

    return {x_from_lon(curLatLon.longitude()),y_from_lat(curLatLon.latitude())};

}

void calcLegendLength(ezgl::renderer *g){
    // Calculate LegendLength
    // (visibleWorld.right-visibleWorld.left)/(visibleScreen.right-visibleScreen.left) * 100
    // [m/100 pixel]
    ezgl::rectangle currentScreen = g->get_visible_screen();
    ezgl::rectangle currentWorld = g->get_visible_world();

    legendLength = 100 * (currentWorld.right()-currentWorld.left())/(currentScreen.right()-currentScreen.left());
}

void drawLabelList(ezgl::renderer *g, std::vector<ezgl::point2d> point_list, std::string png_path){
    ezgl::surface *png_surface = ezgl::renderer::load_png(png_path.c_str());

    for(int i=0; i< point_list.size(); i++){
        g->draw_surface(png_surface, point_list.at(i));
    }

    ezgl::renderer::free_surface(png_surface);
}

