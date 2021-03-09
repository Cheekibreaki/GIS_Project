//test
// Created by cheny811 on 2021-03-03.
//

#include <iostream>

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"

#include "m1.h"
#include "DBstruct.h"
#include "m2.h"

float legendLength;
void calcLegendLength(ezgl::renderer *g);


void draw_main_canvas (ezgl::renderer *g);
void draw_intersection(ezgl::renderer *g);
void draw_streetSeg(ezgl::renderer *g);
void draw_naturalFeature(ezgl::renderer *g);
void draw_legend(ezgl::renderer *g);


void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_mouse_move(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_key_press(ezgl::application *application, GdkEventKey *event, char *key_name);
void initial_setup(ezgl::application *application, bool new_window);
void TextInput_Enter_Key_action(GtkWidget *, gpointer data);

void drawLabelList(ezgl::renderer *g, std::vector<ezgl::point2d> point_list, std::string png_path);
void drawLineHelper(ezgl::renderer *g ,int R,int G,int B, int D, std::vector<StreetSegmentIdx>StrIDList,double width);

void drawMap(){

    ezgl::application::settings settings;
    settings.main_ui_resource   =   "libstreetmap/resources/main.ui";
    settings.window_identifier  =   "MainWindow";
    settings.canvas_identifier  =   "MainCanvas";

    ezgl::application application(settings);

    //set init coordinate system
    ezgl::rectangle initial_world = ezgl::rectangle{{x_from_lon(min_lon),y_from_lat(min_lat)},
                                                    {x_from_lon(max_lon),y_from_lat(max_lat)}};


    ezgl::color backgroundColor = ezgl::color(220,220,220,255);

    application.add_canvas("MainCanvas", draw_main_canvas, initial_world,backgroundColor);



    application.run(initial_setup, act_on_mouse_press,
                    act_on_mouse_move, act_on_key_press);
}



/*Render drawing main Canvas*/

void draw_main_canvas(ezgl::renderer *g){
    draw_intersection(g);
    draw_naturalFeature(g);
    //draw_streetSeg(g);
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

void drawLineHelper(ezgl::renderer *g,int R,int G,int B, int D, std::vector<StreetSegmentIdx>StrIDList,double width){
    for(int curSeg = 0; curSeg<StrIDList.size(); curSeg++) {
        g->set_color(R,G,B,D);
        g->set_line_width(width);
        int segIdx = StrIDList[curSeg];
        ezgl::point2d fromPos = SegsInfoList[segIdx].fromXY;
        ezgl::point2d toPos = SegsInfoList[segIdx].toXY;


        int numCurvePoints = SegsInfoList[segIdx].segInfo.numCurvePoints;
        if (numCurvePoints != 0) {
            //start of fromPos connect first curvePoint
            ezgl::point2d lastCurvePos = fromPos;

            //for loop through all curvePoint
            for (int curCurvePointNum = 0; curCurvePointNum < numCurvePoints; curCurvePointNum++) {
                ezgl::point2d tempCurvePos = LatLon_to_point2d(getStreetSegmentCurvePoint(segIdx, curCurvePointNum));

                g->draw_line(tempCurvePos, lastCurvePos);
                lastCurvePos = tempCurvePos;
            }
            //draw the last curvePoint to toPos
            g->draw_line(lastCurvePos, toPos);

        } else {
            g->draw_line(fromPos, toPos);
        }
    }
}
///Find osm for further modification Not Finished
void draw_streetSeg(ezgl::renderer *g) {
    ezgl::renderer *k=g;
    //std::vector<POIIdx> POIList = POINameListOfPOIsList.at(POIname);
    std::vector<StreetSegmentIdx> MotorWaySegsList= SegmentTypeList.at("motorway");
    g->set_color(255,255,255,255);
    drawLineHelper(k,255,255,255,255,MotorWaySegsList,2);
}
void setFeatureColor(int tempFeatureType, ezgl::renderer *g){
    switch(tempFeatureType){
        case UNKNOWN:       g->set_color(255,228,225);  break;
        case PARK:          g->set_color(148,176,117);  break;
        case BEACH:         g->set_color(251,239,199);  break;
        case LAKE:          g->set_color(185,208,251);  break;
        case RIVER:         g->set_color(185,208,251);  break;
        case ISLAND:        g->set_color(230,230,230);  break;
        case BUILDING:      g->set_color(205,205,205);  break;
        case GREENSPACE:    g->set_color(206,222,175);  break;
        case GOLFCOURSE:    g->set_color(148,176,117);  break;
        case STREAM:        g->set_color(185,208,251);  break;
    }
}

void draw_naturalFeature(ezgl::renderer *g){
    std::vector<FeatureIdx> tempFeatureList;

    for(int curIndex = UNKNOWN; curIndex <= STREAM; curIndex++){
        setFeatureColor(curIndex, g);
        FeatureType curType = (FeatureType)curIndex;
        tempFeatureList = PolyFeatureList[curType];
        for(int i = 0; i< tempFeatureList.size(); i++){

            g->fill_poly(NaturalFeatureList[tempFeatureList[i]].polyList);
        }

        tempFeatureList = LineFeatureList[curType];
        for(int i = 0; i < tempFeatureList.size(); i++){

            for(int pointIdx= 0; pointIdx < NaturalFeatureList[tempFeatureList[i]].polyList.size() - 1; pointIdx++) {
                g->draw_line(NaturalFeatureList[tempFeatureList[i]].polyList[pointIdx],
                             NaturalFeatureList[tempFeatureList[i]].polyList[pointIdx + 1]);
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
    drawLabelList(g, tempList, "libstreetmap/resources/labels/pin_point.png");*/
}

/*User interaction*/


void act_on_mouse_move(ezgl::application *application, GdkEventButton *event, double x, double y){

}
void act_on_key_press(ezgl::application *application, GdkEventKey *event, char *key_name){

}


void act_on_mouse_press(ezgl::application* app, GdkEventButton* event, double x, double y){
    LatLon pos = LatLon(lat_from_y(y),lon_from_x(x));
    int id = findClosestIntersection(pos);

    std::cout << "Closest Intersection: "<< IntersectInfoList[id].name << "\n";

    IntersectInfoList[id].highlight = true;

    app->refresh_drawing();
}

void initial_setup(ezgl::application *application, bool new_window){
    application->update_message("EZGL Application");

    g_signal_connect(
            application->get_object("TextInput"),
            "activate",
            G_CALLBACK(TextInput_Enter_Key_action),
            application
    );

}

StreetIdx check_StreetIdx_PartialStN(std::string& partialName){
    auto tempStreetIdxList = findStreetIdsFromPartialStreetName(partialName);
    if(tempStreetIdxList.empty()) return -1;
    return tempStreetIdxList[0];
}

bool draw_twoStreet_Intersection(ezgl::renderer* g, std::string& firstStName, std::string& secondStName){

    StreetIdx firstStreetIdx = check_StreetIdx_PartialStN(firstStName);
    StreetIdx secondStreetIdx = check_StreetIdx_PartialStN(secondStName);

    if(firstStreetIdx == -1){
        std::cout << "Name of First Street No Found" << std::endl;
        return false;
    }
    if(secondStreetIdx == -1){
        std::cout << "Name of Second Street No Found" << std::endl;
        return false;
    }

    firstStName = getStreetName(firstStreetIdx);
    secondStName = getStreetName(secondStreetIdx);


    auto IntersectList = findIntersectionsOfTwoStreets(std::make_pair(firstStreetIdx, secondStreetIdx));

    if(IntersectList.empty()){
        std::cout << "Intersection No Found" << std::endl;
        return false;
    }
    /*for(auto a:IntersectList){
        std::cout << a<<std::endl;
    }*/
    std::vector<ezgl::point2d> tempPointList;

    for(int id : IntersectList){
        tempPointList.push_back(IntersectInfoList[id].curPosXY);
    }
    drawLabelList(g, tempPointList, "libstreetmap/resources/labels/pin_point.png");

    return true;
}

void TextInput_Enter_Key_action(GtkWidget *, gpointer data){
    auto app = static_cast<ezgl::application *>(data);
    GtkEntry* text_Entry = (GtkEntry* ) app->get_object("TextInput");

    const char * text = gtk_entry_get_text(text_Entry);

    std::string str = text;
    int idx = str.find("&");


    //Find Two Street Intersection
    /*if(idx != -1){
        std::string firstStreet = str.substr(0, idx);
        std::string secondStreet = str.substr(idx+1, str.size());

        bool nameFound = draw_twoStreet_Intersection(app->get_renderer(), firstStreet, secondStreet);

        gtk_entry_set_text(text_Entry, (firstStreet+"&"+secondStreet).c_str());

        if(!nameFound){
            app->update_message("Intersecion of two Street No Found");
        }else{
            app->update_message((firstStreet+" and "+secondStreet+"Intersection found"));
        }
    }*/
    /*else{
        auto IdxList = findStreetIdsFromPartialStreetName(str);
        if(IdxList.empty()){

            return;
        }
        gtk_entry_set_text(text_Entry, getStreetName(IdxList[0]).c_str());
    }*/
}

void TextInput_Change_action(GtkEntry* entry, gchar* /*preedit*/, gpointer user_data){

}
/*Supportive Helper Functions*/




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
