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
double x_from_lon(float lon);
double y_from_lat(float lat);
double lon_from_x(float x);
double lat_from_y(float y);

void draw_main_canvas (ezgl::renderer *g);



void drawMap(){



    double min_lat = IntersectListOfIntersectInfo[0].position.latitude();
    double max_lat = min_lat;
    double min_lon = IntersectListOfIntersectInfo[0].position.longitude();
    double max_lon = min_lon;

    for(IntersectionIdx id = 0; id < IntersectListOfIntersectInfo.size(); id++){
        std::cout << IntersectListOfIntersectInfo[id].position.latitude() <<" " <<IntersectListOfIntersectInfo[id].position.longitude() << std::endl;

        min_lat = std::min(min_lat, IntersectListOfIntersectInfo[id].position.latitude());
        max_lat = std::max(max_lat, IntersectListOfIntersectInfo[id].position.latitude());
        min_lon = std::min(min_lon, IntersectListOfIntersectInfo[id].position.longitude());
        max_lon = std::max(max_lon, IntersectListOfIntersectInfo[id].position.longitude());
    }

    avg_lat = (min_lat + max_lat) / 2;

    ezgl::application::settings settings;
    settings.main_ui_resource   =   "libstreetmap/resources/main.ui";
    settings.window_identifier  =   "MainWindow";
    settings.canvas_identifier  =   "MainCanvas";

    ezgl::application application(settings);

    //set init coordinate system
    ezgl::rectangle initial_world{{x_from_lon(min_lon),y_from_lat(min_lat)},
                                  {x_from_lon(max_lon),y_from_lat(max_lat)}};

    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);



    application.run(nullptr, nullptr,
                    nullptr, nullptr);
}

void draw_main_canvas(ezgl::renderer *g){
    g->set_color(ezgl::GREY_55);

    for(IntersectionIdx id = 0; id < IntersectListOfIntersectInfo.size(); id++){
        float x = x_from_lon(IntersectListOfIntersectInfo[id].position.longitude());
        float y = y_from_lat(IntersectListOfIntersectInfo[id].position.latitude());

        //std::cout<<x<<" "<<y<<std::endl;

        float width = 100;
        float height = width;
        g->fill_rectangle({x, y},{x + width, y + height});
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
