//test
// Created by cheny811 on 2021-03-03.
//

#include <iostream>

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <math.h>
#include "m1.h"
#include "DBstruct.h"
#include "m2.h"
#include <OSMDatabaseAPI.h>

float legendLength;

bool DisplayOSM;
bool DisplayPOI;
bool is_osm_Loaded;
/**
 * True == Day, False == Night
 */
bool DisplayColor = true;

void draw_main_canvas (ezgl::renderer *g);

void setSegColor_Normal(int tempSegType, ezgl::renderer *g);
void setSegColor_OSM(int tempSegType, ezgl::renderer *g);

void draw_streetSeg_Normal(ezgl::renderer *g);
void draw_streetSeg_OSM(ezgl::renderer *g);
void draw_streetSeg_controller(ezgl::renderer *g);
void draw_naturalFeature(ezgl::renderer *g);
void draw_legend(ezgl::renderer *g);
void draw_POI(ezgl::renderer *g);
void draw_oneWay(ezgl::renderer *g);


StreetIdx highlightStreet = -1;
std::vector<StreetSegmentIdx> highlightStSegList;
std::vector<ezgl::point2d> highlightIntersectList;
std::vector<ezgl::point2d> highlightMousePress;

void highlight_mouse_press(ezgl::renderer *g);
void highlight_intersection(ezgl::renderer *g);
void highlight_streetseg(ezgl::renderer *g);
void highlight_poi(ezgl::renderer *g);



void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y);
void initial_setup(ezgl::application *application, bool new_window);

// Singal Callback Functions
void Switch_set_OSM_display (GtkWidget */*widget*/, GdkEvent */*event*/, gpointer user_data);
void ToogleButton_set_Display_Color (GtkToggleButton * /*togglebutton*/, gpointer user_data);
void CheckButton_set_POI_display (GtkToggleButton */*togglebutton*/, gpointer user_data);
void ComboBoxText_Reload_Map (GtkComboBox */*widget*/, gpointer user_data);
void ComboBoxText_Change_Search_Mode(GtkComboBox */*widget*/, gpointer user_data);
void Entry_search_icon (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data);



std::string searchMode;
void Entry_search_Controller(GtkWidget *wid, gpointer data);
void search_Mode_INTERSECT(ezgl::application* app, GtkEntry * text_Entry, std::string text);
void search_Mode_POI(ezgl::application* app, GtkEntry * text_Entry, std::string text);
void search_Mode_STREET(ezgl::application* app, GtkEntry * text_Entry, std::string text);
void search_Mode_TWOSTREET(ezgl::application* app, GtkEntry * text_Entry, std::string text);

StreetIdx check_StreetIdx_PartialStN(std::string& partialName);

void calc_screen_fit(ezgl::application* app, ezgl::rectangle& setScreen);
double calc_distance_point2d(ezgl::point2d first, ezgl::point2d second);
double calc_two_POI_distance(POIIdx POI_first, POIIdx POI_second);
void calcLegendLength(ezgl::renderer *g);

void drawLabelList(ezgl::renderer *g, const std::vector<ezgl::point2d>& point_list, const std::string& png_path);
void drawLineHelper(ezgl::renderer *g ,std::vector<StreetSegmentIdx>StrIDList);
void drawLineHelper_highway(ezgl::renderer *g ,std::vector<StreetSegmentIdx>StrIDList);
void drawNightColor(ezgl::renderer *g);



void drawMap(){
    ezgl::application::settings settings;
    settings.main_ui_resource   =   "libstreetmap/resources/main.ui";
    settings.window_identifier  =   "MainWindow";
    settings.canvas_identifier  =   "MainCanvas";

    ezgl::application application(settings);

    //set init coordinate system
    ezgl::rectangle initial_world = ezgl::rectangle{{x_from_lon(min_lon),y_from_lat(min_lat)},
                                                    {x_from_lon(max_lon),y_from_lat(max_lat)}};


    ezgl::color backgroundColor = ezgl::color(ezgl::WHITE);
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world,backgroundColor);



    application.run(initial_setup, act_on_mouse_press,
                    NULL, NULL);
}

//pullsdqdfqsdd

/*Render drawing main Canvas*/
void draw_street_Name(ezgl::renderer *g);
void draw_main_canvas(ezgl::renderer *g){
    drawNightColor(g);
    calcLegendLength(g);
    draw_naturalFeature(g);
    draw_streetSeg_controller(g);
    //draw_street_Name(g);

    highlight_intersection(g);
    //asdasdas
    if(legendLength<500){
        //draw_oneWay(g);
    }
    if(highlightStreet != -1){
        highlight_streetseg(g);
    }
    highlight_mouse_press(g);
    draw_legend(g);
    draw_POI(g);
//    draw_POI_text(g);
}

void drawNightColor(ezgl::renderer *g){
    ezgl::rectangle temp=g->get_visible_world();
    if(DisplayColor){
        g->set_color(230,230,230,255);
        g->fill_rectangle(temp.bottom_left(),temp.top_right());
    }else if(!DisplayColor){
        g->set_color(25,25,25,255);
        g->fill_rectangle(temp.bottom_left(),temp.top_right());
    }

}

void draw_street_Name(ezgl::renderer *g){
    for(auto StIdx = 0; StIdx < StreetListOfSegsList.size(); StIdx++){
        std::string StName = getStreetName(StIdx);
        for(auto SegIdx : StreetListOfSegsList[StIdx]){
            g->set_color(ezgl::BLACK);
            g->set_font_size(8);
            ezgl::point2d midPoint = (SegsInfoList[SegIdx].toXY+SegsInfoList[SegIdx].fromXY) * ezgl::point2d(0.5,0.5);
            g->draw_text(midPoint,StName,legendLength,legendLength);


        }
    }
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

    std::string legendText = std::to_string(legendLength);

    g->draw_text({70,18},legendText);

    g->set_coordinate_system(ezgl::WORLD);
}
void drawLineHelper_highway(ezgl::renderer *g,std::vector<StreetSegmentIdx> strIDList){
    if(strIDList.empty()){
        return;
    }
    for(int curSeg = 0; curSeg<strIDList.size(); curSeg++) {
        int segIdx = strIDList[curSeg];
        double speed=SegsInfoList[segIdx].segInfo.speedLimit;
        if(speed<=20){
            g->set_color(232, 232, 232);
        }else{
            g->set_color(204, 202, 55);
        }
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

void draw_oneWay(ezgl::renderer *g){
    g->set_color(0,0,0);
    for(int segIdx=0;segIdx<SegsInfoList.size();segIdx++){
        if(SegsInfoList[segIdx].segInfo.oneWay==true&&findStreetSegmentLength(segIdx)>100){
            ezgl::point2d fromPos = SegsInfoList[segIdx].fromXY;
            ezgl::point2d toPos = SegsInfoList[segIdx].toXY;

            int numCurvePoints = SegsInfoList[segIdx].segInfo.numCurvePoints;
            if (numCurvePoints != 0) {
                //start of fromPos connect first curvePoint
                ezgl::point2d lastCurvePos = fromPos;
                //for loop through all curvePoint
                for (int curCurvePointNum = 0; curCurvePointNum < numCurvePoints; curCurvePointNum++) {
                    ezgl::point2d tempCurvePos = LatLon_to_point2d(getStreetSegmentCurvePoint(segIdx, curCurvePointNum));
                    if(curCurvePointNum==numCurvePoints-1){
                    }
                    lastCurvePos = tempCurvePos;
                }
            } else {
                g->draw_text(fromPos,"⟶");
            }
        }
    }
}
void drawLineHelper(ezgl::renderer *g,std::vector<StreetSegmentIdx> strIDList){
    if(strIDList.empty()==true){
        return;
    }

    for(int curSeg = 0; curSeg<strIDList.size(); curSeg++) {
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
}
void draw_streetSeg_controller(ezgl::renderer *g){
    if(DisplayOSM){
        draw_streetSeg_OSM(g);
    }else{
        draw_streetSeg_Normal(g);
    }
}

///Find osm for further modification Not Finished
void draw_streetSeg_Normal(ezgl::renderer *g){
    std::unordered_map<std::string,std::vector<StreetSegmentIdx>>::const_iterator got;
    got= SegmentTypeList_Normal.find("level1");//unknown
    g->set_line_cap(ezgl::line_cap::round);
    if(got!=SegmentTypeList_Normal.end()) {
        setSegColor_Normal(Normal_level1,g);
        drawLineHelper(g,got->second);
    }
    got= SegmentTypeList_Normal.find("level2");//unknown
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_Normal(Normal_level2,g);
        drawLineHelper(g,got->second);
    }
    got= SegmentTypeList_Normal.find("level3");//unknown
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_Normal(Normal_level3,g);
        drawLineHelper(g,got->second);
    }
    got= SegmentTypeList_Normal.find("level4");//unknown
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_Normal(Normal_level4,g);
        drawLineHelper_highway(g,got->second);
    }
    got= SegmentTypeList_Normal.find("level5");//unknown
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_Normal(Normal_level5,g);
        drawLineHelper_highway(g,got->second);
    }
}
void draw_streetSeg_OSM(ezgl::renderer *g) {
    g->set_line_cap(ezgl::line_cap::round);
    std::unordered_map<std::string,std::vector<StreetSegmentIdx>>::const_iterator got;
    got= SegmentTypeList_OSM.find("level1");
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_OSM(OSM_level1,g);
        drawLineHelper(g,got->second);
    }
    got= SegmentTypeList_OSM.find("level2");
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_OSM(OSM_level2,g);
        drawLineHelper(g,got->second);
    }
    got= SegmentTypeList_OSM.find("level3");
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_OSM(OSM_level3,g);
    }
    got= SegmentTypeList_OSM.find("level4");
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_OSM(OSM_level4,g);
    }
    got= SegmentTypeList_OSM.find("pedestrian");
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_OSM(OSM_pedestrian,g);
    }
    got= SegmentTypeList_OSM.find("service");
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_OSM(OSM_service,g);
        drawLineHelper(g,got->second);
    }
    got= SegmentTypeList_OSM.find("unknown");
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_OSM(OSM_unknown,g);
        drawLineHelper(g,got->second);
    }
    got= SegmentTypeList_OSM.find("bus");
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_OSM(OSM_bus,g);
        drawLineHelper(g,got->second);
    }

//    std::vector<StreetSegmentIdx> level1List= SegmentTypeList_OSM.at("level1");//residential
//    drawLineHelper(g,230,230,230,255,level1List,2);
//    std::vector<StreetSegmentIdx> level2List= SegmentTypeList_OSM.at("level2");//large residential
//    drawLineHelper(g,255,255,255,255,level2List,2);
//    std::vector<StreetSegmentIdx> level3List= SegmentTypeList_OSM.at("level3");//major road
//    drawLineHelper(g,0,255,0,255,level3List,2);
//    std::vector<StreetSegmentIdx> level4List= SegmentTypeList_OSM.at("level4");//highway
//    drawLineHelper(g,255,255,77,255,level4List,2);
//    std::vector<StreetSegmentIdx> pedestrianList= SegmentTypeList_OSM.at("pedestrian");//pedestrain
//    drawLineHelper(g,102,0,255,255,pedestrianList,2);
//    std::vector<StreetSegmentIdx> serviceList= SegmentTypeList_OSM.at("service");//service
//    drawLineHelper(g,217,217,217,255,serviceList,2);
//    std::vector<StreetSegmentIdx> unknownList= SegmentTypeList_OSM.at("unknown");//unknown
//    drawLineHelper(g,51,51,51,255,unknownList,2);
//    std::vector<StreetSegmentIdx> busList;
}
void setSegColor_Normal(int tempSegType, ezgl::renderer *g) {
    switch (tempSegType) {
        case Normal_level1:
            g->set_color(230, 230, 230);
            g->set_line_width((1 / legendLength) * 1000);
            break;
        case Normal_level2:
            g->set_color(230, 230, 230);
            g->set_line_width((1.5 / legendLength) * 1000);
            break;
        case Normal_level3:
            g->set_color(255, 255, 255);
            g->set_line_width((2 / legendLength) * 500);
            break;
        case Normal_level4:
            g->set_color(255, 204, 0);
            g->set_line_width((2/ legendLength) * 500);
            break;
        case Normal_level5:
            g->set_color(255, 204, 0);
            g->set_line_width((2/ legendLength) * 500);//((3 / legendLength) * 1000);
            break;
    }
}

void setSegColor_OSM(int tempSegType, ezgl::renderer *g){
    switch(tempSegType){
        case OSM_level1: g->set_color(255,255,255,145);
            g->set_line_width((1/legendLength)*1000);
            break;
        case OSM_level2: g->set_color(255,225,225);
            g->set_line_width((2/legendLength)*1000);
            break;
        case OSM_level3: g->set_color(255,204,0);
            g->set_line_width((3/legendLength)*1000);
            break;
        case OSM_level4: g->set_color(255,204,0);
            g->set_line_width((3/legendLength)*1000);
            break;
        case OSM_pedestrian: g->set_color(255,204,0);
            g->set_line_width((3/legendLength)*1000);
            break;
        case OSM_service: g->set_color(255,204,0);
            g->set_line_width((3/legendLength)*1000);
            break;
        case OSM_unknown: g->set_color(255,204,0);
            g->set_line_width((3/legendLength)*1000);
            break;
        case OSM_bus: g->set_color(255,204,0);
            g->set_line_width((3/legendLength)*1000);
            break;
    }
}
void setFeatureColor(int tempFeatureType, ezgl::renderer *g){
    if(DisplayColor==true) {
        switch (tempFeatureType) {
            case UNKNOWN:
                g->set_color(255, 228, 225);
                break;
            case PARK:
                g->set_color(148, 176, 117);
                break;
            case BEACH:
                g->set_color(251, 239, 199);
                break;
            case LAKE:
                g->set_color(185, 208, 251);
                break;
            case RIVER:
                g->set_color(185, 208, 251);
                break;
            case ISLAND:
                g->set_color(230, 230, 230);
                break;
            case BUILDING:
                g->set_color(205, 205, 205);
                break;
            case GREENSPACE:
                g->set_color(206, 222, 175);
                break;
            case GOLFCOURSE:
                g->set_color(148, 176, 117);
                break;
            case STREAM:
                g->set_color(185, 208, 251);
                break;
        }
    }
    else if(DisplayColor==false) {
        switch (tempFeatureType) {
            case UNKNOWN:
                g->set_color(255, 228, 225);
                break;
            case PARK:
                g->set_color(38, 46, 45);
                break;
            case BEACH:
                g->set_color(44, 38, 46);
                break;
            case LAKE:
                g->set_color(36, 43, 54);
                break;
            case RIVER:
                g->set_color(36, 43, 54);
                break;
            case ISLAND:
                g->set_color(230, 230, 230);
                break;
            case BUILDING:
                g->set_color(176, 196, 222);
                break;
            case GREENSPACE:
                g->set_color(140, 185, 161);
                break;
            case GOLFCOURSE:
                g->set_color(140, 185, 161);
                break;
            case STREAM:
                g->set_color(36, 43, 54);
                break;
        }
    }
}

void draw_naturalFeature(ezgl::renderer *g){
    std::vector<FeatureIdx> tempFeatureList;

    for(int curIndex = UNKNOWN; curIndex <= STREAM; curIndex++){
        setFeatureColor(curIndex, g);
        auto curType = (FeatureType)curIndex;
        tempFeatureList = PolyFeatureList[curType];
        for(int i : tempFeatureList){
            if(curIndex!=BUILDING){
                g->fill_poly(NaturalFeatureList[i].polyList);
            }else{
                if(legendLength<500){
                    g->fill_poly(NaturalFeatureList[i].polyList);
                }
            }

        }

        tempFeatureList = LineFeatureList[curType];
        for(int i : tempFeatureList){

            for(int pointIdx= 0; pointIdx < NaturalFeatureList[i].polyList.size() - 1; pointIdx++) {
                g->draw_line(NaturalFeatureList[i].polyList[pointIdx],
                             NaturalFeatureList[i].polyList[pointIdx + 1]);
            }
        }

    }
}

void draw_POI(ezgl::renderer *g) {
    std::vector<POIIdx> tempList = {};
    if(DisplayPOI) {
        bool NeedPush = false;

    for (int idx = 0; idx < PoiInfoList.size(); idx++) {

        ezgl::rectangle temp = g->get_visible_world();

        if (temp.left() < PoiInfoList[idx].curPosXY.x &&
            temp.bottom() < PoiInfoList[idx].curPosXY.y &&
            temp.right() > PoiInfoList[idx].curPosXY.x &&
            temp.top() > PoiInfoList[idx].curPosXY.y) {

            if (tempList.empty()) {
                tempList.push_back(idx);
            } else {
                for (int i = 0; i < tempList.size(); ++i) {
                    if (calc_two_POI_distance(tempList[i], idx) > legendLength * 1) {
                        NeedPush = true;

                    } else {
                        NeedPush = false;
                        break;
                    }
                }
                if (NeedPush == true) {
                    tempList.push_back(idx);
                }
            }
        }
    }

    if(!tempList.empty()) {
        for (int idx = 0; idx < tempList.size(); idx++) {
            if (PoiInfoList[tempList[idx]].icon_day != "noIcon") {
                if(DisplayColor) {
                    ezgl::surface *png_surface = ezgl::renderer::load_png(PoiInfoList[tempList[idx]].icon_day);
                    g->draw_surface(png_surface, PoiInfoList[tempList[idx]].curPosXY);
                    ezgl::renderer::free_surface(png_surface);
                g->set_font_size(10);
                g->set_color(63,71,70);
                g->draw_text({PoiInfoList[tempList[idx]].curPosXY.x + 3, PoiInfoList[tempList[idx]].curPosXY.y + 6},
                             getPOIName(tempList[idx]));
                }else if (!DisplayColor) {
                    ezgl::surface *png_surface = ezgl::renderer::load_png(PoiInfoList[tempList[idx]].icon_night);
                    g->draw_surface(png_surface, PoiInfoList[tempList[idx]].curPosXY);
                    ezgl::renderer::free_surface(png_surface);
                    g->set_font_size(10);
                    g->set_color(ezgl::BLACK);
                    g->draw_text({PoiInfoList[tempList[idx]].curPosXY.x + 3, PoiInfoList[tempList[idx]].curPosXY.y + 5},
                                 getPOIName(tempList[idx]));
                }
            } else {
                if(DisplayColor){
                    g->set_color(168, 168, 168, 120);
                    g->fill_arc(PoiInfoList[tempList[idx]].curPosXY, 7, 0, 360);

                    g->set_font_size(10);
                    g->set_color(ezgl::BLACK);
                    g->draw_text({PoiInfoList[tempList[idx]].curPosXY.x, PoiInfoList[tempList[idx]].curPosXY.y + 5},
                                 getPOIName(tempList[idx]));
                }

            }

        }

    }
}

}

void highlight_intersection(ezgl::renderer *g){
    if(highlightIntersectList.empty()) return;
    if(searchMode == "INTERSECT"){
        for(auto pos : highlightIntersectList){
            ezgl::point2d temp = pos + ezgl::point2d(legendLength*0.01, legendLength*0.01);
            g->set_color(ezgl::BLUE);
            g->draw_rectangle(pos, temp);
        }
    }
}

void highlight_streetseg(ezgl::renderer *g){
    g->set_color(255,0,0,200);
    g->set_line_width(10);

    drawLineHelper(g,StreetListOfSegsList[highlightStreet]);
}

void highlight_poi(ezgl::renderer *g){

}
void highlight_mouse_press(ezgl::renderer *g){
    drawLabelList(g,highlightMousePress, "libstreetmap/resources/labels/pin_point.png");
}
void highlight_clear(){
    highlightIntersectList.clear();
    highlightStSegList.clear();
    highlightStreet = -1;
}


/*User interaction*/
void act_on_mouse_press(ezgl::application* app, GdkEventButton* event, double x, double y){
    ezgl::point2d mousePos(x,y);
    LatLon pos = LatLon(lat_from_y(y),lon_from_x(x));
    int id = findClosestIntersection(pos);

    std::cout << "Closest Intersection: "<< IntersectInfoList[id].name << "\n";

    if(searchMode == "Select MODE ..."){
        app->update_message("Please Select Mode Before Searching ...");
        return;
    }

    if(searchMode == "INTERSECT"){
        if(event->button == 1){
            highlightMousePress.clear();

            if(!highlightIntersectList.empty()){
                ezgl::point2d closest;
                double closestLength = 9999;
                for(auto point : highlightIntersectList){
                    double tempLength = calc_distance_point2d(mousePos, point);
                    if(tempLength < closestLength){
                        closestLength = tempLength;
                        closest = point;
                    }
                }
                highlightMousePress.push_back(closest);
            }
            else{
                highlightMousePress.push_back(IntersectInfoList[id].curPosXY);
            }
            app->update_message("Closest Intersection: " + IntersectInfoList[id].name);
        }
    }

    app->refresh_drawing();
}


void initial_setup(ezgl::application *application, bool new_window){
    DisplayColor = true;
    DisplayPOI = false;
    DisplayOSM = false;
    is_osm_Loaded = false;
    searchMode = "Select MODE ...";

    g_signal_connect(
            application->get_object("ChangeMap"),
            "changed",
            G_CALLBACK(ComboBoxText_Reload_Map),
            application
    );

    g_signal_connect(
            application->get_object("FuncMode"),
            "changed",
            G_CALLBACK(ComboBoxText_Change_Search_Mode),
            application
    );


    /* Three Same Signal execute same function */
    g_signal_connect(
            application->get_object("UserInput"),
            "activate",
            G_CALLBACK(Entry_search_Controller),
            application
    );

    g_signal_connect(
            application->get_object("UserInput"),
            "icon-press",
            G_CALLBACK(Entry_search_icon),
            application
    );
    g_signal_connect(
            application->get_object("Find"),
            "clicked",
            G_CALLBACK(Entry_search_Controller),
            application
    );



    g_signal_connect(
            application->get_object("DisplayOSM"),
            "button-press-event",
            G_CALLBACK(Switch_set_OSM_display),
            application
    );

    g_signal_connect(
            application->get_object("DisplayColor"),
            "toggled",
            G_CALLBACK(ToogleButton_set_Display_Color),
            application
    );
    g_signal_connect(
            application->get_object("DisplayPOI"),
            "toggled",
            G_CALLBACK(CheckButton_set_POI_display),
            application
    );
}

void ComboBoxText_Reload_Map (GtkComboBox */*widget*/, gpointer user_data){
    auto app = static_cast<ezgl::application *>(user_data);
    auto* combo_Box = (GtkComboBoxText * ) app->get_object("ChangeMap");
    std::string text = (std::string)gtk_combo_box_text_get_active_text(combo_Box);
    std::cout <<text <<std::endl;
    std::string map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
    if(text == "Beijing, China")            map_path = "/cad2/ece297s/public/maps/beijing_china.streets.bin";
    if(text == "Cairo, Egypt")              map_path = "/cad2/ece297s/public/maps/cairo_egypt.streets.bin";
    if(text == "Cape-Town, South-Africa")   map_path = "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin";
    if(text == "Golden-Horseshoe, Canada")  map_path = "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin";
    if(text == "Hamilton, Canada")          map_path = "/cad2/ece297s/public/maps/hamilton_canada.streets.bin";
    if(text == "Hong-Kong, China")          map_path = "/cad2/ece297s/public/maps/hong-kong_china.streets.bin";
    if(text == "Iceland")                   map_path = "/cad2/ece297s/public/maps/iceland.streets.bin";
    if(text == "Interlaken, Switzerland")   map_path = "/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin";
    if(text == "London, England")           map_path = "/cad2/ece297s/public/maps/london_england.streets.bin";
    if(text == "Moscow, Russia")            map_path = "/cad2/ece297s/public/maps/moscow_russia.streets.bin";
    if(text == "New-Delhi, India")          map_path = "/cad2/ece297s/public/maps/new-delhi_india.streets.bin";
    if(text == "New-York, USA")             map_path = "/cad2/ece297s/public/maps/new-york_usa.streets.bin";
    if(text == "Rio-De-Janeiro, Brazil")    map_path = "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin";
    if(text == "Saint-Helena")              map_path = "/cad2/ece297s/public/maps/saint-helena.streets.bin";
    if(text == "Singapore")                 map_path = "/cad2/ece297s/public/maps/singapore.streets.bin";
    if(text == "Sydney, Australia")         map_path = "/cad2/ece297s/public/maps/sydney_australia.streets.bin";
    if(text == "Tehran, Iran")              map_path = "/cad2/ece297s/public/maps/tehran_iran.streets.bin";
    if(text == "Tokyo, Japan")              map_path = "/cad2/ece297s/public/maps/tokyo_japan.streets.bin";
    if(text == "Toronto, Canada")           map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";

    closeMap();
    loadMap(map_path);

    highlightStreet = -1;
    DisplayOSM = false;
    is_osm_Loaded = false;
    searchMode = "Select MODE ...";

    gtk_switch_set_active((GtkSwitch *)app->get_object("DisplayOSM"), false);
    gtk_combo_box_set_active((GtkComboBox *)app->get_object("FuncMode"), 0);
    gtk_toggle_button_set_active((GtkToggleButton *)app->get_object("DisplayColor"), false);

    ezgl::rectangle new_world = ezgl::rectangle{{x_from_lon(min_lon),y_from_lat(min_lat)},
                                                {x_from_lon(max_lon),y_from_lat(max_lat)}};
    app->change_canvas_world_coordinates("MainCanvas", new_world);
    app->refresh_drawing();
}

void ComboBoxText_Change_Search_Mode(GtkComboBox */*widget*/, gpointer user_data){
    highlight_clear();
    auto app = static_cast<ezgl::application *>(user_data);
    auto* combo_Box = (GtkComboBoxText * ) app->get_object("FuncMode");
    searchMode = (std::string)gtk_combo_box_text_get_active_text(combo_Box);
    app->refresh_drawing();
}

void Entry_search_icon (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data){
    Entry_search_Controller(NULL, user_data);
}
void Entry_search_Controller(GtkWidget *wid, gpointer data){
    highlight_clear();
    // Catch User Invalid Input
    // Set Highlight Object & tell map to reDraw
    // Check if user select One Search MODE
    auto app = static_cast<ezgl::application *>(data);

    if(searchMode == "Select MODE ..."){
        app->update_message("Please Select Mode Before Searching ...");
        return;
    }

    // Get User input Text
    auto* text_Entry = (GtkEntry* ) app->get_object("UserInput");
    std::string text = (std::string)gtk_entry_get_text(text_Entry);
    if(searchMode == "INTERSECT"){
        search_Mode_INTERSECT(app, text_Entry, text);
    }
    if(searchMode == "POI"){
        search_Mode_POI(app, text_Entry, text);
    }
    if(searchMode == "STREET"){
        search_Mode_STREET(app, text_Entry, text);
    }
    if(searchMode == "TWOSTREET"){
        search_Mode_TWOSTREET(app, text_Entry, text);
    }
    app->refresh_drawing();
}
void Switch_set_OSM_display (GtkWidget */*widget*/, GdkEvent */*event*/, gpointer user_data){
    auto app = static_cast<ezgl::application *>(user_data);
    if(DisplayOSM){
        DisplayOSM = false;
        app->update_message("CLOSING OSM PLEASE WAIT.........");

        app->update_message("CLOSING OSM FINISHED");

    }else{
        DisplayOSM = true;
        app->update_message("LOADING OSM PLEASE WAIT.........");

        osm_file_path.replace(osm_file_path.end()-11,osm_file_path.end(),"osm.bin");

        if(is_osm_Loaded){
            app->update_message("OSM ALREADY LOADED");
        }else{
            is_osm_Loaded = true;
            loadOSMDatabaseBIN(osm_file_path);
            LoadOSMWayofOSMIDList();
            LoadTypeListOfSegsList_OSM(osm_file_path);
        }

        app->update_message("LOADING OSM FINISHED");

    }
    app->refresh_drawing();
}
void ToogleButton_set_Display_Color (GtkToggleButton * /*togglebutton*/, gpointer user_data){
    auto app = static_cast<ezgl::application *>(user_data);
    auto* colorButton = (GtkButton *)app->get_object("DisplayColor");
    auto* DayImg = (GtkWidget *)app->get_object("DayImg");
    auto* NightImg = (GtkWidget *)app->get_object("NightImg");
    if(DisplayColor){
        DisplayColor = false;
        app->update_message("Switching to Night Mode");
        gtk_button_set_image(colorButton, NightImg);
    }else{
        DisplayColor = true;
        app->update_message("Switching to Day Mode");
        gtk_button_set_image(colorButton, DayImg);
    }
    app->refresh_drawing();
}

void CheckButton_set_POI_display (GtkToggleButton */*togglebutton*/, gpointer user_data){
    auto app = static_cast<ezgl::application *>(user_data);
    if(DisplayPOI){
        DisplayPOI = false;
        app->update_message("POI Display disabled");
    }
    else{
        DisplayPOI = true;
        app->update_message("POI Display enabled");
    }
    app->refresh_drawing();
}


/*Supportive Helper Functions*/
void search_Mode_INTERSECT(ezgl::application* app, GtkEntry * text_Entry, std::string text){
    // ReadFrom Intersect Tree
    auto IntersectIdxList = IntersectNameTree.getIdList(text);
    if(IntersectIdxList.empty()){
        app->update_message("Intersect Name Not Found");
        return;
    }
    for(auto IntersectIdx : IntersectIdxList){
        app->update_message("Displaying Intersection");
        highlightIntersectList.push_back(IntersectInfoList[IntersectIdx].curPosXY);
    }
    //highlightIntersectList.push_back();
}

void search_Mode_POI(ezgl::application* app, GtkEntry * text_Entry, std::string text){
    // ReadFrom POI Tree

}

void search_Mode_STREET(ezgl::application* app, GtkEntry * text_Entry, std::string text){
    auto StreetIdxList = findStreetIdsFromPartialStreetName(text);
    if(StreetIdxList.empty()){
        app->update_message("Street Name Not Found");
        return;
    }
    highlightStreet = StreetIdxList[0];
    gtk_entry_set_text(text_Entry, getStreetName(highlightStreet).c_str());
    app->update_message("Street: " + getStreetName(highlightStreet) + " Highlighted");


    LatLonBounds minmax = findStreetBoundingBox(highlightStreet);
    ezgl::point2d minPoint = LatLon_to_point2d(minmax.min);
    ezgl::point2d maxPoint = LatLon_to_point2d(minmax.max);
    ezgl::rectangle setScreen(minPoint,maxPoint);

    calc_screen_fit(app, setScreen);
}

void search_Mode_TWOSTREET(ezgl::application* app, GtkEntry * text_Entry, std::string text){
    int idx = text.find('&');
    if(idx == -1){
        app->update_message("TwoStreetIntersect & no found");
        return;
    }

    std::string firstStreet = text.substr(0, idx);
    std::string secondStreet = text.substr(idx+1, text.size());

    StreetIdx firstStreetIdx = check_StreetIdx_PartialStN(firstStreet);
    StreetIdx secondStreetIdx = check_StreetIdx_PartialStN(secondStreet);

    if(firstStreetIdx == -1){
        if(secondStreetIdx != -1)secondStreet = getStreetName(secondStreetIdx);
        app->update_message("Name of First Street No Found");
        gtk_entry_set_text(text_Entry, (firstStreet+" & "+secondStreet).c_str());
        return;
    }
    if(secondStreetIdx == -1){
        firstStreet = getStreetName(firstStreetIdx);
        gtk_entry_set_text(text_Entry, (firstStreet+" & "+secondStreet).c_str());
        app->update_message("Name of Second Street No Found");
        gtk_entry_set_text(text_Entry, (firstStreet+" & "+secondStreet).c_str());
        return;
    }

    firstStreet = getStreetName(firstStreetIdx);
    secondStreet = getStreetName(secondStreetIdx);

    auto tempIntersectList = findIntersectionsOfTwoStreets(std::make_pair(firstStreetIdx, secondStreetIdx));

    if(tempIntersectList.empty()){
        app->update_message("Intersection No Found");
        gtk_entry_set_text(text_Entry, (firstStreet+" & "+secondStreet).c_str());
        return;
    }

    gtk_entry_set_text(text_Entry, (firstStreet+" & "+secondStreet).c_str());
}


void calc_screen_fit(ezgl::application* app, ezgl::rectangle& setScreen){
    auto initScreen = app->get_renderer()->get_visible_screen();

    double possibleWidth = setScreen.height()/initScreen.height()*initScreen.width();
    if(setScreen.width() < possibleWidth){
        double widthDiffer = possibleWidth - setScreen.width();
        setScreen.m_first.x -= (widthDiffer/2);
        setScreen.m_second.x += (widthDiffer/2);
    }
    double possibleHeight = setScreen.width()/initScreen.width()*initScreen.height();
    if(setScreen.height() < possibleHeight){
        double heightDiffer = possibleHeight - setScreen.height();
        setScreen.m_first.y -= (heightDiffer/2);
        setScreen.m_second.y += (heightDiffer/2);
    }
    ezgl::zoom_fit(app->get_canvas("MainCanvas"),setScreen);
}
double calc_distance_point2d(ezgl::point2d first, ezgl::point2d second){
    ezgl::rectangle temp(first, second);
    return sqrt(temp.width()*temp.width() + temp.height() * temp.height());
}

double calc_two_POI_distance(POIIdx POI_first, POIIdx POI_second){

    ezgl::point2d POI1=PoiInfoList[POI_first].curPosXY;
    ezgl::point2d POI2=PoiInfoList[POI_second].curPosXY;

    return calc_distance_point2d(POI1, POI2);
}
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
void drawLabelList(ezgl::renderer *g, const std::vector<ezgl::point2d>& point_list, const std::string& png_path){
    ezgl::surface *png_surface = ezgl::renderer::load_png(png_path.c_str());

    for(auto & i : point_list){
        g->draw_surface(png_surface, i);
    }

    ezgl::renderer::free_surface(png_surface);
}

StreetIdx check_StreetIdx_PartialStN(std::string& partialName){
    auto tempStreetIdxList = findStreetIdsFromPartialStreetName(partialName);
    if(tempStreetIdxList.empty()) return -1;
    return tempStreetIdxList[0];
}