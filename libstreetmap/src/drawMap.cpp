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

bool DisplayOSM;
bool DisplayPOI;
/**
 * True == Day, False == Night
 */
bool DisplayColor;

void draw_main_canvas (ezgl::renderer *g);

void draw_streetSeg_Normal(ezgl::renderer *g);
void draw_streetSeg_OSM(ezgl::renderer *g);
void draw_streetSeg(ezgl::renderer *g);
void draw_naturalFeature(ezgl::renderer *g);
void draw_legend(ezgl::renderer *g);

std::vector<StreetSegmentIdx> highlightStSegList;
std::vector<IntersectionIdx> highlightIntersectList;
void highlight_intersection(ezgl::renderer *g);
void highlight_streetseg(ezgl::renderer *g);
void highlight_poi(ezgl::renderer *g);

void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y);
void initial_setup(ezgl::application *application, bool new_window);

// Singal Callback Functions
void gtk_Switch_set_OSM_display (GtkWidget */*widget*/, GdkEvent */*event*/, gpointer user_data);
void gtk_ToogleButton_set_Display_Color (GtkToggleButton * /*togglebutton*/, gpointer user_data);
void gtk_CheckButton_set_POI_display (GtkToggleButton */*togglebutton*/, gpointer user_data);
void gtk_ComboBoxText_Reload_Map (GtkComboBox */*widget*/, gpointer user_data);
void gtk_ComboBoxText_Change_Search_Mode(GtkComboBox */*widget*/, gpointer user_data);
void gtk_Entry_search_icon (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data);
/// For both Entry key and Find button
void gtk_Entry_search_Enter_Key(GtkWidget *wid, gpointer data);
StreetIdx check_StreetIdx_PartialStN(std::string& partialName);

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
                    NULL, NULL);
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
    if(highlightIntersectList.empty()) {
        return;
    }
    ezgl::surface *png_surface = ezgl::renderer::load_png("libstreetmap/resources/labels/pin_point.png");

    for(auto intersectId : highlightIntersectList){
        g->draw_surface(png_surface, IntersectInfoList[intersectId].curPosXY);
    }
    ezgl::renderer::free_surface(png_surface);
}

void highlight_streetseg(ezgl::renderer *g){
    // DrawSomething

}
void highlight_poi(ezgl::renderer *g){

}
/*User interaction*/
void act_on_mouse_press(ezgl::application* app, GdkEventButton* event, double x, double y){
    highlightIntersectList.clear();

    LatLon pos = LatLon(lat_from_y(y),lon_from_x(x));
    int id = findClosestIntersection(pos);

    std::cout << "Closest Intersection: "<< IntersectInfoList[id].name << "\n";

    highlightIntersectList.push_back(id);

    app->refresh_drawing();
}

void gtk_Switch_set_OSM_display (GtkWidget */*widget*/, GdkEvent */*event*/, gpointer user_data){
    auto app = static_cast<ezgl::application *>(user_data);
    if(DisplayOSM){
        DisplayOSM = false;
        app->update_message("CLOSING OSM PLEASE WAIT.........");

        app->update_message("CLOSING OSM FINISHED");

    }else{
        DisplayOSM = true;
        app->update_message("LOADING OSM PLEASE WAIT.........");

        app->update_message("LOADING OSM FINISHED");
    }
    app->refresh_drawing();
}

void initial_setup(ezgl::application *application, bool new_window){
    DisplayColor = true;
    DisplayPOI = false;
    DisplayOSM = false;
    searchMode = "Select MODE ...";

    g_signal_connect(
            application->get_object("ChangeMap"),
            "changed",
            G_CALLBACK(gtk_ComboBoxText_Reload_Map),
            application
    );

    g_signal_connect(
            application->get_object("FuncMode"),
            "changed",
            G_CALLBACK(gtk_ComboBoxText_Change_Search_Mode),
            application
    );


    /* Three Same Signal execute same function */
    g_signal_connect(
            application->get_object("UserInput"),
            "activate",
            G_CALLBACK(gtk_Entry_search_Enter_Key),
            application
    );

    g_signal_connect(
            application->get_object("UserInput"),
            "icon-press",
            G_CALLBACK(gtk_Entry_search_icon),
            application
    );
    g_signal_connect(
            application->get_object("Find"),
            "clicked",
            G_CALLBACK(gtk_Entry_search_Enter_Key),
            application
    );



    g_signal_connect(
            application->get_object("DisplayOSM"),
            "button-press-event",
            G_CALLBACK(gtk_Switch_set_OSM_display),
            application
    );

    g_signal_connect(
            application->get_object("DisplayColor"),
            "toggled",
            G_CALLBACK(gtk_ToogleButton_set_Display_Color),
            application
    );
    g_signal_connect(
            application->get_object("DisplayPOI"),
            "toggled",
            G_CALLBACK(gtk_CheckButton_set_POI_display),
            application
    );
}

void gtk_ComboBoxText_Reload_Map (GtkComboBox */*widget*/, gpointer user_data){
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
    ezgl::rectangle new_world = ezgl::rectangle{{x_from_lon(min_lon),y_from_lat(min_lat)},
                                                {x_from_lon(max_lon),y_from_lat(max_lat)}};
    app->change_canvas_world_coordinates("MainCanvas", new_world);
    app->refresh_drawing();

}
void gtk_ComboBoxText_Change_Search_Mode(GtkComboBox */*widget*/, gpointer user_data){
    auto app = static_cast<ezgl::application *>(user_data);
    auto* combo_Box = (GtkComboBoxText * ) app->get_object("FuncMode");
    searchMode = (std::string)gtk_combo_box_text_get_active_text(combo_Box);
}
void gtk_Entry_search_icon (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data){
    gtk_Entry_search_Enter_Key(NULL, user_data);
}
void gtk_Entry_search_Enter_Key(GtkWidget *wid, gpointer data){
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
void gtk_ToogleButton_set_Display_Color (GtkToggleButton * /*togglebutton*/, gpointer user_data){
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
void gtk_CheckButton_set_POI_display (GtkToggleButton */*togglebutton*/, gpointer user_data){
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