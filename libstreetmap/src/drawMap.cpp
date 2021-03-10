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

void draw_streetSeg_Normal(ezgl::renderer *g);
void draw_streetSeg_OSM(ezgl::renderer *g);
void draw_streetSeg(ezgl::renderer *g);
void draw_naturalFeature(ezgl::renderer *g);
void draw_legend(ezgl::renderer *g);

void highlight_intersection(ezgl::renderer *g);
void highlight_streetseg(ezgl::renderer *g);
void highlight_poi(ezgl::renderer *g);

void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_mouse_move(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_key_press(ezgl::application *application, GdkEventKey *event, char *key_name);
void initial_setup(ezgl::application *application, bool new_window);
void TextInput_Enter_Key_action(GtkWidget *, gpointer data);
std::string highlight_twoStreet_Intersection(std::string& firstStName, std::string& secondStName);
StreetIdx check_StreetIdx_PartialStN(std::string& partialName);
void clear_Intersect_highlight();


void drawLabelList(ezgl::renderer *g, std::vector<ezgl::point2d> point_list, std::string png_path);
void drawLineHelper(ezgl::renderer *g ,int R,int G,int B, int D, std::vector<StreetSegmentIdx> StrIDList,double width);

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
    draw_naturalFeature(g);
    //draw_streetSeg(g);
    highlight_intersection(g);
    highlight_streetseg(g);
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

void drawLineHelper(ezgl::renderer *g,int R,int G,int B, int D, std::vector<StreetSegmentIdx> strIDList,double width){
    if(strIDList.empty()==true){
        return;
    }
    for(int curSeg = 0; curSeg<strIDList.size(); curSeg++) {
        g->set_color(R,G,B,D);
        g->set_line_width(width);
        int segIdx = strIDList[curSeg];
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
    return;
}
void draw_streetSeg(ezgl::renderer *g){
    //Optional:For OSM
    //draw_streetSeg_OSM(g);

}
///Find osm for further modification Not Finished
void draw_streetSeg_Normal(ezgl::renderer *g){

}
void draw_streetSeg_OSM(ezgl::renderer *g) {
    std::unordered_map<std::string,std::vector<StreetSegmentIdx>>::const_iterator got;
    got= SegmentTypeList.find("level1");//unknown
    if(got!=SegmentTypeList.end()) {
        drawLineHelper(g, 255,64,0,255, got->second, 2);
    }
    got= SegmentTypeList.find("level2");//unknown
    if(got!=SegmentTypeList.end()) {
        drawLineHelper(g, 51, 51, 51, 255, got->second, 2);
    }
    got= SegmentTypeList.find("level3");//unknown
    if(got!=SegmentTypeList.end()) {
        drawLineHelper(g, 51, 51, 51, 255, got->second, 2);
    }
    got= SegmentTypeList.find("level4");//unknown
    if(got!=SegmentTypeList.end()) {
        drawLineHelper(g, 51, 51, 51, 255, got->second, 2);
    }
    got= SegmentTypeList.find("pedestrian");//unknown
    if(got!=SegmentTypeList.end()) {
        drawLineHelper(g, 51, 51, 51, 255, got->second, 2);
    }
    got= SegmentTypeList.find("service");//unknown
    if(got!=SegmentTypeList.end()) {
        drawLineHelper(g, 51, 51, 51, 255, got->second, 2);
    }
    got= SegmentTypeList.find("unknown");//unknown
    if(got!=SegmentTypeList.end()) {
        drawLineHelper(g, 51, 51, 51, 255, got->second, 2);
    }
    got= SegmentTypeList.find("bus");//unknown
    if(got!=SegmentTypeList.end()) {
        drawLineHelper(g, 51, 51, 51, 255, got->second, 2);
    }
    //    std::vector<StreetSegmentIdx> level1List= SegmentTypeList.at("level1");//residential
//    drawLineHelper(g,230,230,230,255,level1List,2);
//    std::vector<StreetSegmentIdx> level2List= SegmentTypeList.at("level2");//large residential
//    drawLineHelper(g,255,255,255,255,level2List,2);
//    std::vector<StreetSegmentIdx> level3List= SegmentTypeList.at("level3");//major road
//    drawLineHelper(g,0,255,0,255,level3List,2);
//    std::vector<StreetSegmentIdx> level4List= SegmentTypeList.at("level4");//highway
//    drawLineHelper(g,255,255,77,255,level4List,2);
//    std::vector<StreetSegmentIdx> pedestrianList= SegmentTypeList.at("pedestrian");//pedestrain
//    drawLineHelper(g,102,0,255,255,pedestrianList,2);
//    std::vector<StreetSegmentIdx> serviceList= SegmentTypeList.at("service");//service
//    drawLineHelper(g,217,217,217,255,serviceList,2);
//    std::vector<StreetSegmentIdx> unknownList= SegmentTypeList.at("unknown");//unknown
//    drawLineHelper(g,51,51,51,255,unknownList,2);
//    std::vector<StreetSegmentIdx> busList;
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
void clear_Intersect_highlight(){
    for(auto & intersectInfo: IntersectInfoList){
        intersectInfo.highlight = false;
    }
    for(auto & segsInfo : SegsInfoList){
        segsInfo.highlight = false;
    }
    /*for(auto & poiInfo : PoiInfoList){
        poiInfo.highlight = false;
    }*/
}

void highlight_intersection(ezgl::renderer *g){
    ezgl::surface *png_surface = ezgl::renderer::load_png("libstreetmap/resources/labels/pin_point.png");

    for(auto & intersectInfo : IntersectInfoList){
        if(intersectInfo.highlight){
            g->draw_surface(png_surface, intersectInfo.curPosXY);
        }
    }
    ezgl::renderer::free_surface(png_surface);
}

void highlight_streetseg(ezgl::renderer *g){
    for(auto & intersectInfo : IntersectInfoList){
        if(intersectInfo.highlight){

        }
    }
}
void highlight_poi(ezgl::renderer *g){

}
/*User interaction*/


void act_on_mouse_move(ezgl::application *application, GdkEventButton *event, double x, double y){

}
void act_on_key_press(ezgl::application *application, GdkEventKey *event, char *key_name){

}

void act_on_mouse_press(ezgl::application* app, GdkEventButton* event, double x, double y){
    clear_Intersect_highlight();

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



void TextInput_Enter_Key_action(GtkWidget *, gpointer data){
    auto app = static_cast<ezgl::application *>(data);
    GtkEntry* text_Entry = (GtkEntry* ) app->get_object("TextInput");

    const char * text = gtk_entry_get_text(text_Entry);

    std::string str = text;
    int idx = str.find("&");


    //Find Two Street Intersection
    if(idx != -1){
        std::string firstStreet = str.substr(0, idx);
        std::string secondStreet = str.substr(idx+1, str.size());

        std::string errorMessage = highlight_twoStreet_Intersection(firstStreet, secondStreet);

        gtk_entry_set_text(text_Entry, (firstStreet+" & "+secondStreet).c_str());

        if(!errorMessage.empty()){
            app->update_message(errorMessage);
        }else{
            app->update_message((firstStreet+" and "+secondStreet+"Intersection found"));
        }
    }
    else{
        auto StreetIdxList = findStreetIdsFromPartialStreetName(str);
        if(StreetIdxList.empty()){
            app->update_message("Street Name Not Found");
            return;
        }
        StreetIdx curStId = StreetIdxList[0];
        gtk_entry_set_text(text_Entry, getStreetName(curStId).c_str());

        for(auto segIdx : StreetListOfSegsList[curStId]){
            SegsInfoList[segIdx].highlight = true;
        }
    }
    app->refresh_drawing();
}

void TextInput_Change_action(GtkEntry* entry, gchar* /*preedit*/, gpointer user_data){

}
/*Supportive Helper Functions*/



/*Legend*/
void calcLegendLength(ezgl::renderer *g){
    // Calculate LegendLength
    // (visibleWorld.right-visibleWorld.left)/(visibleScreen.right-visibleScreen.left) * 100
    // [m/100 pixel]
    ezgl::rectangle currentScreen = g->get_visible_screen();
    ezgl::rectangle currentWorld = g->get_visible_world();

    legendLength = 100 * (currentWorld.right()-currentWorld.left())/(currentScreen.right()-currentScreen.left());
}

/*PNG helper*/
void drawLabelList(ezgl::renderer *g, std::vector<ezgl::point2d> point_list, std::string png_path){
    ezgl::surface *png_surface = ezgl::renderer::load_png(png_path.c_str());

    for(int i=0; i< point_list.size(); i++){
        g->draw_surface(png_surface, point_list.at(i));
    }

    ezgl::renderer::free_surface(png_surface);
}



std::string highlight_twoStreet_Intersection(std::string& firstStName, std::string& secondStName){

    clear_Intersect_highlight();

    StreetIdx firstStreetIdx = check_StreetIdx_PartialStN(firstStName);
    StreetIdx secondStreetIdx = check_StreetIdx_PartialStN(secondStName);

    if(firstStreetIdx == -1){
        if(secondStreetIdx != -1) secondStName = getStreetName(secondStreetIdx);
        return "Name of First Street No Found";
    }
    if(secondStreetIdx == -1){
        firstStName = getStreetName(firstStreetIdx);
        return "Name of Second Street No Found";
    }

    firstStName = getStreetName(firstStreetIdx);
    secondStName = getStreetName(secondStreetIdx);


    auto IntersectList = findIntersectionsOfTwoStreets(std::make_pair(firstStreetIdx, secondStreetIdx));

    if(IntersectList.empty()){
        return "Intersection No Found";
    }

    for(int id : IntersectList){
        IntersectInfoList[id].highlight = true;
    }

    return "";
}
StreetIdx check_StreetIdx_PartialStN(std::string& partialName){
    auto tempStreetIdxList = findStreetIdsFromPartialStreetName(partialName);
    if(tempStreetIdxList.empty()) return -1;
    return tempStreetIdxList[0];
}
