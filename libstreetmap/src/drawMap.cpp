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

std::string searchMode;

void draw_main_canvas (ezgl::renderer *g);
void draw_streetSeg(ezgl::renderer *g);
void draw_naturalFeature(ezgl::renderer *g);
void draw_legend(ezgl::renderer *g);

std::vector<StreetSegmentIdx> highlightStSegList;
std::vector<IntersectionIdx> highlightIntersectList;
void highlight_intersection(ezgl::renderer *g);
void highlight_streetseg(ezgl::renderer *g);
void highlight_poi(ezgl::renderer *g);

void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_mouse_move(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_key_press(ezgl::application *application, GdkEventKey *event, char *key_name);
void initial_setup(ezgl::application *application, bool new_window);

void ComboBox_Change_Search_Mode(GtkComboBox */*widget*/, gpointer user_data);
void TextInput_Enter_Key_action(GtkWidget *, gpointer data);
void TextInput_Change_action(GtkEntry* entry, gchar* /*preedit*/, gpointer user_data);

std::string highlight_twoStreet_Intersection(std::string& firstStName, std::string& secondStName);
StreetIdx check_StreetIdx_PartialStN(std::string& partialName);
void clear_Intersect_highlight();


void drawLabelList(ezgl::renderer *g, const std::vector<ezgl::point2d>& point_list, const std::string& png_path);
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
        auto curType = (FeatureType)curIndex;
        tempFeatureList = PolyFeatureList[curType];
        for(int i : tempFeatureList){

            g->fill_poly(NaturalFeatureList[i].polyList);
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

void highlight_intersection(ezgl::renderer *g){
    ezgl::surface *png_surface = ezgl::renderer::load_png("libstreetmap/resources/labels/pin_point.png");

    for(auto intersectId : highlightIntersectList){
        g->draw_surface(png_surface, IntersectInfoList[intersectId].curPosXY);
    }
    ezgl::renderer::free_surface(png_surface);
}

void highlight_streetseg(ezgl::renderer *g){

}
void highlight_poi(ezgl::renderer *g){

}
/*User interaction*/


void act_on_mouse_move(ezgl::application *application, GdkEventButton *event, double x, double y){

}
void act_on_key_press(ezgl::application *application, GdkEventKey *event, char *key_name){

}

void act_on_mouse_press(ezgl::application* app, GdkEventButton* event, double x, double y){
    highlightIntersectList.clear();

    LatLon pos = LatLon(lat_from_y(y),lon_from_x(x));
    int id = findClosestIntersection(pos);

    std::cout << "Closest Intersection: "<< IntersectInfoList[id].name << "\n";

    highlightIntersectList.push_back(id);

    app->refresh_drawing();
}



void initial_setup(ezgl::application *application, bool new_window){
    searchMode = "Select MODE ...";

    g_signal_connect(
            application->get_object("TextInput"),
            "activate",
            G_CALLBACK(TextInput_Enter_Key_action),
            application
    );
    /*g_signal_connect(
            application->get_object("TextInput"),
            "changed",
            G_CALLBACK(TextInput_Change_action),
            application
    );*/
    g_signal_connect(
            application->get_object("ComboBox"),
            "changed",
            G_CALLBACK(ComboBox_Change_Search_Mode),
            application
    );
}

void ComboBox_Change_Search_Mode(GtkComboBox */*widget*/, gpointer user_data){
    auto app = static_cast<ezgl::application *>(user_data);
    auto* combo_Box = (GtkComboBoxText * ) app->get_object("ComboBox");
    searchMode = (std::string)gtk_combo_box_text_get_active_text(combo_Box);
}

void TextInput_Enter_Key_action(GtkWidget *, gpointer data){
    // Catch User Invalid Input
    // Set Highlight Object & tell map to reDraw
    auto app = static_cast<ezgl::application *>(data);

    // Check if user select One Search MODE
    if(searchMode == "Select MODE ..."){
        app->update_message("Please Select Mode Before Searching ...");
        return;
    }

    // Get User input Text
    auto* text_Entry = (GtkEntry* ) app->get_object("TextInput");
    std::string text = (std::string)gtk_entry_get_text(text_Entry);

    if(searchMode == "STREET"){
        auto StreetIdxList = findStreetIdsFromPartialStreetName(text);
        if(StreetIdxList.empty()){
            app->update_message("Street Name Not Found");
            return;
        }
        StreetIdx curStId = StreetIdxList[0];
        gtk_entry_set_text(text_Entry, getStreetName(curStId).c_str());

        highlightStSegList.clear();
        highlightStSegList = StreetListOfSegsList[curStId];

        app->update_message("Street: " + getStreetName(curStId) + " Highlighted");
    }

    if(searchMode == "TWOSTREET"){
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

        highlightIntersectList.clear();
        highlightIntersectList = findIntersectionsOfTwoStreets(std::make_pair(firstStreetIdx, secondStreetIdx));

        if(highlightIntersectList.empty()){
            app->update_message("Intersection No Found");
            gtk_entry_set_text(text_Entry, (firstStreet+" & "+secondStreet).c_str());
            return;
        }

        gtk_entry_set_text(text_Entry, (firstStreet+" & "+secondStreet).c_str());
    }


    app->refresh_drawing();
}

void TextInput_Change_action(GtkEntry* entry, gchar* /*preedit*/, gpointer user_data){
    // after key in TextBar been pressed, reload the Textview widget (no need to render)
    // g_connected in the init
    auto app = static_cast<ezgl::application *>(user_data);
    auto* text_Entry = (GtkEntry* ) app->get_object("TextInput");

    const char * text = gtk_entry_get_text(text_Entry);

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