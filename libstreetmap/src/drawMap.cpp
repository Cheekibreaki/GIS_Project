//
// Created by cheny811 on 2021-03-03.
//
#include "m2.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "m1.cpp"

void draw_main_canvas (ezgl::renderer *g);

void drawMap(){
    ezgl::application::settings settings;
    settings.main_ui_resource   =   "libstreetmap/resources/main.ui";
    settings.window_identifier  =   "MainWindow";
    settings.canvas_identifier  =   "MainCanvas";

    ezgl::application application(settings);

    ezgl::rectangle initial_world{{0,0},{1000,1000}};

    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);



    application.run(nullptr, nullptr,
                    nullptr, nullptr);
}

void draw_main_canvas(ezgl::renderer *g){
    g->draw_rectangle({0, 0},{1000, 1000});

}