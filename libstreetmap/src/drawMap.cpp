//test
// Created by cheny811 on 2021-03-03.
//

#include <iostream>

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <math.h>
#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "DBstruct.h"
#include <OSMDatabaseAPI.h>

float legendLength;

bool DisplayOSM = false;
bool DisplayPOI = false;
bool is_osm_Loaded = false;
/**
 * True == Day, False == Night
 */
bool DisplayColor = true;

void draw_main_canvas (ezgl::renderer *g);

void setSegColor_Normal(int tempSegType, ezgl::renderer *g);
void setSegColor_OSM(int tempSegType, ezgl::renderer *g);
void setFeatureColor(int tempFeatureType, ezgl::renderer *g);

void draw_streetSeg_Normal(ezgl::renderer *g);
void draw_streetSeg_OSM(ezgl::renderer *g);
void draw_streetSeg_controller(ezgl::renderer *g);
void draw_naturalFeature(ezgl::renderer *g);
void draw_legend(ezgl::renderer *g);
void draw_POI(ezgl::renderer *g);
void draw_oneWay(ezgl::renderer *g);


StreetIdx highlightStreet = -1;
std::vector<ezgl::point2d> highlightIntersectList;
std::vector<ezgl::point2d> highlightPOIList;
std::vector<ezgl::point2d> highlightMousePress;

double turn_penalty = 15;
IntersectionIdx lastClickIntersection = -1;
std::vector<StreetSegmentIdx> highlightNaviRoute;

void highlight_clear();
void highlight_mouse_press(ezgl::renderer *g);
void highlight_intersection(ezgl::renderer *g);
void highlight_street(ezgl::renderer *g);
void highlight_poi(ezgl::renderer *g);



void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y);
void press_INTERSECT(ezgl::application* app, GdkEventButton* event, const ezgl::point2d & mousePos, const LatLon & pos);
void press_POI(ezgl::application* app, GdkEventButton* event, const ezgl::point2d & mousePos);
void press_NAVIGATION(ezgl::application* app, GdkEventButton* event, const ezgl::point2d & mousePos, const LatLon & pos);

void initial_setup(ezgl::application *application, bool new_window);

std::string font;

// Singal Callback Functions
void Switch_set_OSM_display (GtkWidget */*widget*/, GdkEvent */*event*/, gpointer user_data);
void ToogleButton_set_Display_Color (GtkToggleButton * /*togglebutton*/, gpointer user_data);
void CheckButton_set_POI_display (GtkToggleButton */*togglebutton*/, gpointer user_data);
void ComboBoxText_Reload_Map (GtkComboBox */*widget*/, gpointer user_data);
void ComboBoxText_Change_Search_Mode(GtkComboBox */*widget*/, gpointer user_data);
void Entry_search_icon (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data);

void Dialog_Box_NaviRooteDisplay(ezgl::application *application);
void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);

std::string searchMode = "Select MODE ...";
void Entry_search_Controller(GtkWidget *wid, gpointer data);
void search_Mode_INTERSECT(ezgl::application* app, GtkEntry * text_Entry, std::string text);
void search_Mode_POI(ezgl::application* app, GtkEntry * text_Entry, std::string text);
void search_Mode_STREET(ezgl::application* app, GtkEntry * text_Entry, std::string text);
void search_Mode_TWOSTREET(ezgl::application* app, GtkEntry * text_Entry, std::string text);
void search_Mode_NAVIGATION(ezgl::application* app, GtkEntry * text_Entry, std::string text);

StreetIdx checkFirst_StreetIdx_PartialStN(std::string& partialName);
IntersectionIdx checkFirst_IntersectIdx_PartialIntersect(std::string& partialName);

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
    g->format_font(font,ezgl::font_slant::normal, ezgl::font_weight::normal);
    drawNightColor(g);
    calcLegendLength(g);
    draw_naturalFeature(g);
    draw_streetSeg_controller(g);
    draw_street_Name(g);

    highlight_intersection(g);
    highlight_poi(g);
    //asdasdas
    if(legendLength<1000){
        g->format_font("monospace",ezgl::font_slant::normal, ezgl::font_weight::normal);
        draw_oneWay(g);
        g->format_font(font,ezgl::font_slant::normal, ezgl::font_weight::normal);
    }
    highlight_street(g);


    draw_legend(g);
    draw_POI(g);

    drawLineHelper(g, highlightNaviRoute);

    highlight_mouse_press(g);
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
    if(legendLength<1000){
        for(auto StIdx = 0; StIdx < StreetListOfSegsList.size(); StIdx++){
            std::string StName = getStreetName(StIdx);
            if(StName!="<unknown>"){
                for(auto SegIdx : StreetListOfSegsList[StIdx]){
                    if(DisplayColor){
                        g->set_color(ezgl::BLACK);
                    }else{
                        g->set_color(ezgl::WHITE);
                    }
                    g->set_font_size(12);
                    ezgl::point2d midPoint = (SegsInfoList[SegIdx].toXY+SegsInfoList[SegIdx].fromXY) * ezgl::point2d(0.5,0.5);
                    //g->draw_text(midPoint,StName,legendLength,legendLength);
                    ezgl::point2d fromPos = SegsInfoList[SegIdx].fromXY;
                    ezgl::point2d toPos = SegsInfoList[SegIdx].toXY;
                    double length=SegsInfoList[SegIdx].length*0.8;
                    if(length>25){
                        ezgl::rectangle seg_Boundary(fromPos,fromPos+ezgl::point2d(length,length));
                        double fromX=fromPos.x;
                        double fromY=fromPos.y;
                        double toX=toPos.x;
                        double toY=toPos.y;
                        int numCurvePoints = SegsInfoList[SegIdx].segInfo.numCurvePoints;
                        if(numCurvePoints==0){
                            double angle = SegsInfoList[SegIdx].angle;
                            if((toY-fromY)>0&&(toX-fromX)>0){
                                if(angle<45){
                                    g->set_text_rotation(angle);
                                    g->draw_text(midPoint,StName,seg_Boundary.width(),seg_Boundary.height());
                                }else{
                                    g->set_text_rotation(angle);
                                    g->draw_text(midPoint,StName,seg_Boundary.width()+50,seg_Boundary.height());
                                }
                            }else if((toY-fromY)>0&&(toX-fromX)<0){
                                if(angle<45){
                                    g->set_text_rotation(angle);
                                    g->draw_text(midPoint,StName,seg_Boundary.width(),seg_Boundary.height());
                                }else{
                                    g->set_text_rotation(angle);
                                    g->draw_text(midPoint,StName,seg_Boundary.width()+50,seg_Boundary.height());
                                }
                            }else if((toY-fromY)<0&&(toX-fromX)<0){
                                if(angle>-45){
                                    g->set_text_rotation(angle);
                                    g->draw_text(midPoint,StName,seg_Boundary.width(),seg_Boundary.height());
                                }else{
                                    g->set_text_rotation(angle);
                                    g->draw_text(midPoint,StName,seg_Boundary.width()+50,seg_Boundary.height());
                                }
                            }else if((toY-fromY)<0&&(toX-fromX)>0){
                                if(angle>-45){
                                    g->set_text_rotation(angle);
                                    g->draw_text(midPoint,StName,seg_Boundary.width(),seg_Boundary.height());
                                }else{
                                    g->set_text_rotation(angle);
                                    g->draw_text(midPoint,StName,seg_Boundary.width()+50,seg_Boundary.height());
                                }
                            }
                        }
                    }

                }
            }
        }
    }
}

void draw_legend(ezgl::renderer *g){
    g->set_text_rotation(0);
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
            if(DisplayColor)    g->set_color(255, 255, 255);
            else                g->set_color(105, 121, 128);
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
    if(legendLength<300){
        g->set_color(100,100,100);
        for(int segIdx=0;segIdx<SegsInfoList.size();segIdx++){
            if(SegsInfoList[segIdx].segInfo.oneWay && findStreetSegmentLength(segIdx) > 100){
                ezgl::point2d fromPos = SegsInfoList[segIdx].fromXY;
                ezgl::point2d toPos = SegsInfoList[segIdx].toXY;
                double fromX=fromPos.x;
                double fromY=fromPos.y;
                double toX=toPos.x;
                double toY=toPos.y;
                int numCurvePoints = SegsInfoList[segIdx].segInfo.numCurvePoints;
                if(numCurvePoints==0){
                    double angle = SegsInfoList[segIdx].angle;
                    if((toY-fromY)>0&&(toX-fromX)>0){
                        g->set_text_rotation(angle);
                        g->draw_text(fromPos,"⟶");
                    }else if((toY-fromY)>0&&(toX-fromX)<0){
                        g->set_text_rotation(angle);
                        g->draw_text(fromPos,"⟵");
                    }else if((toY-fromY)<0&&(toX-fromX)<0){
                        g->set_text_rotation(angle);
                        g->draw_text(fromPos,"⟵");
                    }else if((toY-fromY)<0&&(toX-fromX)>0){
                        g->set_text_rotation(angle);
                        g->draw_text(fromPos,"⟶");
                    }
                }

            }
        }
    }
}
void drawLineHelper(ezgl::renderer *g,std::vector<StreetSegmentIdx> strIDList){
    if(strIDList.empty()){
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
        drawLineHelper(g,got->second);
    }
    got= SegmentTypeList_OSM.find("level4");
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_OSM(OSM_level4,g);
        drawLineHelper(g,got->second);
    }
    got= SegmentTypeList_OSM.find("pedestrian");
    if(got!=SegmentTypeList_OSM.end()) {
        setSegColor_OSM(OSM_pedestrian,g);
        drawLineHelper(g,got->second);
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
}
void setSegColor_Normal(int tempSegType, ezgl::renderer *g) {
    switch (tempSegType) {
        case Normal_level1:
            if(DisplayColor)    g->set_color(240, 240, 240);
            else                g->set_color(100, 110, 110);
            g->set_line_width((1 / legendLength) * 1000);
            break;
        case Normal_level2:
            if(DisplayColor)    g->set_color(240, 240, 240);
            else                g->set_color(100, 110, 110);
            g->set_line_width((1.5 / legendLength) * 1000);
            break;
        case Normal_level3:
            if(DisplayColor)    g->set_color(255, 255, 255);
            else                g->set_color(167, 183, 190);
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
        default: break;
    }
}

void setSegColor_OSM(int tempSegType, ezgl::renderer *g){
    switch(tempSegType){
        case OSM_level1:
            if(DisplayColor)    g->set_color(240, 240, 240);
            else                g->set_color(100, 110, 110);
            g->set_line_width((1/legendLength)*1000);
            break;
        case OSM_level2:
            if(DisplayColor)    g->set_color(240, 240, 240);
            else                g->set_color(100, 110, 110);
            g->set_line_width((1.5/legendLength)*1000);
            break;
        case OSM_level3:
            if(DisplayColor)    g->set_color(255, 255, 255);
            else                g->set_color(167, 183, 190);
            g->set_line_width((2/legendLength)*1000);
            break;
        case OSM_level4: g->set_color(255, 204, 0);
            g->set_line_width((2/legendLength)*1000);
            break;
        case OSM_pedestrian:
            if(DisplayColor)    g->set_color(240, 240, 240);
            else                g->set_color(100, 110, 110);
            g->set_line_width((1/legendLength)*1000);
            break;
        case OSM_service:
            if(DisplayColor)    g->set_color(240, 240, 240);
            else                g->set_color(100, 110, 110);
            g->set_line_width((1/legendLength)*1000);
            break;
        case OSM_unknown:
            if(DisplayColor)    g->set_color(240, 240, 240);
            else                g->set_color(100, 110, 110);
            g->set_line_width((1/legendLength)*1000);
            break;
        case OSM_bus:
            g->set_color(100,100,255);
            g->set_line_width((1.5/legendLength)*1000);
            break;
        default: break;
    }
}
void setFeatureColor(int tempFeatureType, ezgl::renderer *g){
    if(DisplayColor) {
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
            default:break;
        }
    }
    else if(!DisplayColor) {
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
                g->set_color(25, 25, 25,255);
                break;
            case BUILDING:
                g->set_color(50, 57, 70);
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
            default:break;
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
            if (std::string(PoiInfoList[tempList[idx]].icon_day) != "noIcon") {
                if(DisplayColor) {
                    ezgl::surface *png_surface = ezgl::renderer::load_png(PoiInfoList[tempList[idx]].icon_day);
                    g->draw_surface(png_surface, PoiInfoList[tempList[idx]].curPosXY);
                    ezgl::renderer::free_surface(png_surface);
                g->set_font_size(10);
                g->set_color(63,71,70);
                g->draw_text({PoiInfoList[tempList[idx]].curPosXY.x + 5, PoiInfoList[tempList[idx]].curPosXY.y + 10},
                             getPOIName(tempList[idx]));
                }else if (!DisplayColor) {
                    ezgl::surface *png_surface = ezgl::renderer::load_png(PoiInfoList[tempList[idx]].icon_night);
                    g->draw_surface(png_surface, PoiInfoList[tempList[idx]].curPosXY);
                    ezgl::renderer::free_surface(png_surface);
                    g->set_font_size(10);
                    g->set_color(ezgl::WHITE);
                    g->draw_text({PoiInfoList[tempList[idx]].curPosXY.x + 5, PoiInfoList[tempList[idx]].curPosXY.y + 10},
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
                }else if(!DisplayColor){
                    g->set_color(168, 168, 168, 120);
                    g->fill_arc(PoiInfoList[tempList[idx]].curPosXY, 7, 0, 360);

                    g->set_font_size(10);
                    g->set_color(ezgl::WHITE);
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
            ezgl::point2d temp = pos + ezgl::point2d(legendLength*0.05, legendLength*0.05);
            g->set_color(ezgl::BLUE);
            g->draw_rectangle(pos, temp);
        }
    }
}

void highlight_street(ezgl::renderer *g){
    g->set_color(255,0,0,200);
    g->set_line_width(10);
    if(highlightStreet != -1)
        drawLineHelper(g,StreetListOfSegsList[highlightStreet]);
}

void highlight_poi(ezgl::renderer *g){
    if(highlightPOIList.empty()) return;
    if(searchMode == "POI"){
        for(auto pos : highlightPOIList){
            ezgl::point2d temp = pos + ezgl::point2d(legendLength*0.05, legendLength*0.05);
            g->set_color(ezgl::BLUE);
            g->draw_rectangle(pos, temp);
        }
    }
}
void highlight_mouse_press(ezgl::renderer *g){
    drawLabelList(g,highlightMousePress, "libstreetmap/resources/labels/pin_point.png");
}
void highlight_clear(){
    highlightMousePress.clear();
    highlightIntersectList.clear();
    highlightPOIList.clear();
    highlightNaviRoute.clear();
    highlightStreet = -1;
    lastClickIntersection = -1;
}


/*User interaction*/
void act_on_mouse_press(ezgl::application* app, GdkEventButton* event, double x, double y){
    ezgl::point2d mousePos(x,y);
    LatLon pos = LatLon(lat_from_y(y),lon_from_x(x));

    if(event->button == 3){
        highlight_clear();
        app->update_message("Highlighted Cleared");
        app->refresh_drawing();
        return;
    }

    if(searchMode == "Select MODE ..."){
        app->update_message("Please Select Mode Before Searching ...");
        return;
    }

    if(searchMode == "INTERSECT"){
        press_INTERSECT(app, event, mousePos, pos);
    }

    if(searchMode == "POI"){
        press_POI(app, event, mousePos);

    }
    if(searchMode == "NAVIGATION"){
        press_NAVIGATION(app, event, mousePos, pos);
    }
    app->refresh_drawing();
}
void press_INTERSECT(ezgl::application* app, GdkEventButton* event, const ezgl::point2d & mousePos, const LatLon & pos){
    int id = findClosestIntersection(pos);
    std::cout << "Closest Intersection ID: "<< id << "\tName: "<< IntersectInfoList[id].name << "\n";
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
void press_POI(ezgl::application* app, GdkEventButton* event, const ezgl::point2d & mousePos){
    if(event->button == 1){
        highlightMousePress.clear();

        if(!highlightPOIList.empty()){
            ezgl::point2d closest;
            double closestLength = 9999;
            for(auto point : highlightPOIList){
                double tempLength = calc_distance_point2d(mousePos, point);
                if(tempLength < closestLength){
                    closestLength = tempLength;
                    closest = point;
                }
            }
            highlightMousePress.push_back(closest);
        }
    }
}
void press_NAVIGATION(ezgl::application* app, GdkEventButton* event, const ezgl::point2d & mousePos, const LatLon & pos){
    if(event->button == 1){
        int id = findClosestIntersection(pos);
        app->update_message("Closest Intersection: " + IntersectInfoList[id].name);
        highlightMousePress.push_back(IntersectInfoList[id].curPosXY);
        if(lastClickIntersection != -1){
            auto tempList = findPathBetweenIntersections(lastClickIntersection, id, turn_penalty);

            Dialog_Box_NaviRooteDisplay(app);

            highlightNaviRoute.insert(highlightNaviRoute.end(),tempList.begin(),tempList.end());

            lastClickIntersection = id;
        }else{
            lastClickIntersection = id;
        }
    }
}

void initial_setup(ezgl::application *application, bool /*new_window*/){
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
    font = "monospace";
    if(text == "Beijing, China"){
        font = "Noto Sans CJK SC";
        map_path = "/cad2/ece297s/public/maps/beijing_china.streets.bin";
    }
    if(text == "Cairo, Egypt"){
        map_path = "/cad2/ece297s/public/maps/cairo_egypt.streets.bin";
    }
    if(text == "Cape-Town, South-Africa"){
        map_path = "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin";
    }
    if(text == "Golden-Horseshoe, Canada"){
        map_path = "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin";
    }
    if(text == "Hamilton, Canada"){
        map_path = "/cad2/ece297s/public/maps/hamilton_canada.streets.bin";
    }
    if(text == "Hong-Kong, China"){
        font = "Noto Sans CJK SC";
        map_path = "/cad2/ece297s/public/maps/hong-kong_china.streets.bin";
    }
    if(text == "Iceland"){
        map_path = "/cad2/ece297s/public/maps/iceland.streets.bin";
    }
    if(text == "Interlaken, Switzerland"){
        map_path = "/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin";
    }
    if(text == "London, England"){
        map_path = "/cad2/ece297s/public/maps/london_england.streets.bin";
    }
    if(text == "Moscow, Russia"){
        map_path = "/cad2/ece297s/public/maps/moscow_russia.streets.bin";
    }
    if(text == "New-Delhi, India"){
        map_path = "/cad2/ece297s/public/maps/new-delhi_india.streets.bin";
    }
    if(text == "New-York, USA"){
        map_path = "/cad2/ece297s/public/maps/new-york_usa.streets.bin";
    }
    if(text == "Rio-De-Janeiro, Brazil"){
        map_path = "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin";
    }
    if(text == "Saint-Helena"){
        map_path = "/cad2/ece297s/public/maps/saint-helena.streets.bin";
    }
    if(text == "Singapore"){
        map_path = "/cad2/ece297s/public/maps/singapore.streets.bin";
    }
    if(text == "Sydney, Australia"){
        map_path = "/cad2/ece297s/public/maps/sydney_australia.streets.bin";
    }
    if(text == "Tehran, Iran"){
        map_path = "/cad2/ece297s/public/maps/tehran_iran.streets.bin";
    }
    if(text == "Tokyo, Japan"){
        font = "Noto Sans CJK SC";
        map_path = "/cad2/ece297s/public/maps/tokyo_japan.streets.bin";
    }
    if(text == "Toronto, Canada"){
        map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
    }

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

    auto* text_Entry = (GtkEntry* ) app->get_object("UserInput");
    gtk_entry_set_text(text_Entry, "");

    auto* combo_Box = (GtkComboBoxText * ) app->get_object("FuncMode");
    searchMode = (std::string)gtk_combo_box_text_get_active_text(combo_Box);
    app->refresh_drawing();
}

void Entry_search_icon (GtkEntry */*entry*/, GtkEntryIconPosition /*icon_pos*/, GdkEvent */*event*/, gpointer user_data){
    Entry_search_Controller(NULL, user_data);
}
void Entry_search_Controller(GtkWidget */*wid*/, gpointer data){
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
    if(searchMode == "NAVIGATION"){
        search_Mode_NAVIGATION(app, text_Entry, text);
    }
    app->refresh_drawing();
}
void Switch_set_OSM_display (GtkWidget */*widget*/, GdkEvent */*event*/, gpointer user_data){
    auto app = static_cast<ezgl::application *>(user_data);

    auto* switch_osm = (GtkSwitch *) app->get_object("DisplayOSM");

    //std::cout << gtk_switch_get_state(switch_osm) << std::endl;

    if(gtk_switch_get_state(switch_osm)){
        DisplayOSM = false;
        app->update_message("Disable OSM");
    }else{
        DisplayOSM = true;

        if(is_osm_Loaded){
            app->update_message("OSM ALREADY LOADED");
        }else{
            is_osm_Loaded = true;
            loadOSMDatabaseBIN(osm_file_path);
            LoadOSMWayofOSMIDList();
            LoadTypeListOfSegsList_OSM(osm_file_path);
            app->update_message("LOADING OSM FINISHED");
        }
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
void search_Mode_INTERSECT(ezgl::application* app, GtkEntry * /*text_Entry*/, std::string text){
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
}

void search_Mode_POI(ezgl::application* app, GtkEntry * text_Entry, std::string text){
    // ReadFrom POI Tree
    auto POIIdxList = POINameTree.getIdList(text);
    if(POIIdxList.empty()){
        app->update_message("POI Name Not Found");
        return;
    }
    gtk_entry_set_text(text_Entry, PoiInfoList[POIIdxList[0]].name.c_str());
    for(auto poiId : POINameListOfPOIsList.at(PoiInfoList[POIIdxList[0]].name)){
        app->update_message("Displaying POI" + PoiInfoList[POIIdxList[0]].name);
        highlightPOIList.push_back(PoiInfoList[poiId].curPosXY);
    }
    //highlightIntersectList.push_back();
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

    StreetIdx firstStreetIdx = checkFirst_StreetIdx_PartialStN(firstStreet);
    StreetIdx secondStreetIdx = checkFirst_StreetIdx_PartialStN(secondStreet);

    if(firstStreetIdx == -1){
        if(secondStreetIdx != -1)secondStreet = getStreetName(secondStreetIdx);
        app->update_message("Name of First Street No Found");
        gtk_entry_set_text(text_Entry, (firstStreet+" & "+secondStreet).c_str());
        return;
    }
    if(secondStreetIdx == -1){
        firstStreet = getStreetName(firstStreetIdx);
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
    for(auto IntersectIdx : tempIntersectList){
        highlightMousePress.push_back(IntersectInfoList[IntersectIdx].curPosXY);
    }

    gtk_entry_set_text(text_Entry, (firstStreet+" & "+secondStreet).c_str());
}

void search_Mode_NAVIGATION(ezgl::application* app, GtkEntry * text_Entry, std::string text){
    int idx = text.find('/');
    if(idx == -1){
        app->update_message("NAVIGATION MODE needs / to seperate two INTERSECTIONs");
        return;
    }

    std::string firstIntersect = text.substr(0, idx);
    std::string secondIntersect = text.substr(idx+1, text.size());

    std::cout << firstIntersect + " " + secondIntersect << std::endl;

    IntersectionIdx firstIntersectIdx = checkFirst_IntersectIdx_PartialIntersect(firstIntersect);
    IntersectionIdx secondIntersectIdx = checkFirst_IntersectIdx_PartialIntersect(secondIntersect);

    if(firstIntersectIdx == -1 && secondIntersectIdx == -1){
        app->update_message("Both intersectionName no Found");
        return;
    }
    if(firstIntersectIdx == -1){
        app->update_message("First intersectionName no Found");
        secondIntersect = getIntersectionName(secondIntersectIdx);
        gtk_entry_set_text(text_Entry, (firstIntersect+" & "+secondIntersect).c_str());
        return;
    }
    if(secondIntersectIdx == -1){
        app->update_message("Second intersectionName no Found");
        firstIntersect = getIntersectionName(firstIntersectIdx);
        gtk_entry_set_text(text_Entry, (firstIntersect+" & "+secondIntersect).c_str());
        return;
    }
    firstIntersect = getIntersectionName(firstIntersectIdx);
    secondIntersect = getIntersectionName(secondIntersectIdx);
    gtk_entry_set_text(text_Entry, (firstIntersect+" & "+secondIntersect).c_str());

    // Excute Navigation Process
    highlightNaviRoute = findPathBetweenIntersections(firstIntersectIdx, secondIntersectIdx, turn_penalty);
    Dialog_Box_NaviRooteDisplay(app);


}

void Dialog_Box_NaviRooteDisplay(ezgl::application *application){
    // Update the status bar message
    application->update_message("Dialog_Box Creating.....");

    GObject *window;         // the parent window over which to add the dialog
    GtkWidget *content_area; // the content area of the dialog
    GtkWidget *label;        // the label we will create to display a message in the contentarea
    GtkWidget *dialog;       // the dialog box we will create

    // get a pointer to the main application window
    window = application->get_object(application->get_main_window_id().c_str());

    dialog = gtk_dialog_new_with_buttons("Test Dialog",
                                         (GtkWindow*) window,
                                         GTK_DIALOG_MODAL,
                                         ("UP"),
                                         GTK_RESPONSE_ACCEPT,
                                         ("DOWN"),
                                         GTK_RESPONSE_REJECT,
                                         NULL);

    // Create a label and attach it to the content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new("Simple Dialog With a Label. Press OK or CANCEL too close.");
    gtk_container_add(GTK_CONTAINER(content_area), label);

    // The main purpose of this is to show dialog??s child widget, label
    gtk_widget_show_all(dialog);

    g_signal_connect(
            GTK_DIALOG(dialog),
            "response",
            G_CALLBACK(on_dialog_response),
            NULL
    );

    // Redraw the main canvas
    application->refresh_drawing();
}
void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data){
    // For demonstration purposes, this will show the enum name and int value of the button that was pressed
    std::cout << "response is ";
    switch(response_id) {
        case GTK_RESPONSE_ACCEPT:
            std::cout << "GTK_RESPONSE_ACCEPT ";
            break;
        case GTK_RESPONSE_DELETE_EVENT:
            std::cout << "GTK_RESPONSE_DELETE_EVENT (i.e. ??X?? button) ";
            break;
        case GTK_RESPONSE_REJECT:
            std::cout << "GTK_RESPONSE_REJECT ";
            break;
        default:
            std::cout << "UNKNOWN ";
            break;
    }
    std::cout << "(" << response_id << ")\n";
    // This will cause the dialog to be destroyed and close.
    // without this line the dialog remains open unless the
    // response_id is GTK_RESPONSE_DELETE_EVENT which
    // automatically closes the dialog without the following line.
    //gtk_widget_destroy(GTK_WIDGET (dialog));
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

StreetIdx checkFirst_StreetIdx_PartialStN(std::string& partialName){
    auto tempStreetIdxList = findStreetIdsFromPartialStreetName(partialName);
    if(tempStreetIdxList.empty()) return -1;
    return tempStreetIdxList[0];
}

IntersectionIdx checkFirst_IntersectIdx_PartialIntersect(std::string& partialName){
    auto tempIntersectIdxList = IntersectNameTree.getIdList(partialName);
    if(tempIntersectIdxList.empty()) return -1;
    return tempIntersectIdxList[0];
}