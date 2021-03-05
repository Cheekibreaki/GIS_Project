//
// Created by cheny811 on 2021-03-03.
//

#include <iostream>

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"

#include "m1.h"
#include "DBstruct.h"
#include "m2.h"


double avg_lat = 0;
std::vector<intersect_info> IntersectInfoList;

void draw_main_canvas (ezgl::renderer *g);
void draw_intersection(ezgl::renderer *g);
void draw_streetSeg(ezgl::renderer *g);
void draw_naturalFeature(ezgl::renderer *g);
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);



void drawMap(){


    double min_lat = IntersectListOfLatLon[0].latitude();
    double max_lat = min_lat;
    double min_lon = IntersectListOfLatLon[0].longitude();
    double max_lon = min_lon;

    for(IntersectionIdx id = 0; id < IntersectListOfLatLon.size(); id++){

        min_lat = std::min(min_lat, IntersectListOfLatLon[id].latitude());
        max_lat = std::max(max_lat, IntersectListOfLatLon[id].latitude());
        min_lon = std::min(min_lon, IntersectListOfLatLon[id].longitude());
        max_lon = std::max(max_lon, IntersectListOfLatLon[id].longitude());
    }

    avg_lat = (min_lat + max_lat) / 2;


    IntersectInfoList.resize(getNumIntersections());

    for(IntersectionIdx id = 0; id < IntersectListOfLatLon.size(); id++){
        IntersectInfoList[id].curPosXY.x = x_from_lon(IntersectListOfLatLon[id].longitude());
        IntersectInfoList[id].curPosXY.y = y_from_lat(IntersectListOfLatLon[id].latitude());
        IntersectInfoList[id].name = getIntersectionName(id);
    }


    ezgl::application::settings settings;
    settings.main_ui_resource   =   "libstreetmap/resources/main.ui";
    settings.window_identifier  =   "MainWindow";
    settings.canvas_identifier  =   "MainCanvas";

    ezgl::application application(settings);

    //set init coordinate system
    ezgl::rectangle initial_world{{x_from_lon(min_lon),y_from_lat(min_lat)},
                                  {x_from_lon(max_lon),y_from_lat(max_lat)}};

    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);



    application.run(nullptr, act_on_mouse_click,
                    nullptr, nullptr);
}

void draw_main_canvas(ezgl::renderer *g){

    //draw_intersection(g);

    draw_streetSeg(g);
    draw_naturalFeature(g);
}

void draw_streetSeg(ezgl::renderer *g){

}
void draw_naturalFeature(ezgl::renderer *g){

}
void draw_intersection(ezgl::renderer *g){
    for(IntersectionIdx id = 0; id < IntersectListOfLatLon.size(); id++){
        float x = IntersectInfoList[id].curPosXY.x;
        float y = IntersectInfoList[id].curPosXY.y;

        if(IntersectInfoList[id].highlight){
            g->set_color(ezgl::RED);
        }else{
            g->set_color(ezgl::GREY_55);
        }

        //g->get_visible_screen();

        float width = 5;
        float height = width;
        g->fill_rectangle({x, y},{x + width, y + height});
    }
}

void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y){
    std::cout << "Mouse clicked at (" <<x<< "," <<y<< ")\n";
    LatLon pos = LatLon(lat_from_y(y),lon_from_x(x));
    int id = findClosestIntersection(pos);

    std::cout << "Closest Intersection: "<< IntersectInfoList[id].name << "\n";
    IntersectInfoList[id].highlight = true;

    app->refresh_drawing();
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
