/* 
 * Copyright 2023 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <math.h>
#include <cmath>
#include <iostream>
#include <ctype.h>
#include <string>
#include <sstream>
#include <vector>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <unordered_map>

#include <boost/functional/hash.hpp>


/**************************** Global Constants ********************************/
//rendering constants, adjust to change the range of the zoom level's (numbers calculated based on ratio of screen width and world_creen_width)
#define zoom_boundary_zero_one 19
#define zoom_boundary_one_two 7
#define zoom_boundary_two_three 4
#define zoom_boundary_three_four 2
#define zoom_boundary_four_five 0
/**************************** Global Constants End ********************************/

/**************************** Function Declarations ********************************/
void draw_main_canvas(ezgl::renderer *g);
void initial_setup (ezgl::application* application, bool /*new_window*/);
void act_on_key_press(ezgl::application* app, GdkEventKey* event, char* key_name);
void toggleDark(GtkWidget* /*widget*/, ezgl::application* application);
void toggleStreets(GtkWidget* /*widget*/, ezgl::application* application);
void toggleFeatures(GtkWidget* /*widget*/, ezgl::application* application);
void togglePOIs(GtkWidget* /*widget*/, ezgl::application* application);
void toggleSubways(GtkWidget* /*widget*/, ezgl::application* application);
void toggleLibraries(GtkWidget* /*widget*/, ezgl::application* application);
void toggleFood(GtkWidget* /*widget*/, ezgl::application* application);
double x_from_lon(float longitude);
double y_from_lat(float latitude);
double lon_from_x(double x);
double lat_from_y(double y);
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);
POIIdx findAnyClosestPOI(LatLon my_position);
void mapChangeInput(GtkEntry * /*entry*/, ezgl::application* application);
void callBackEnter(GtkWidget *widget, gpointer data);
void mainpointsofPOI();
std::vector<StreetIdx> checkPartialStreetName(std::string firstStreet);
std::vector<IntersectionIdx> checkIntersections(std::string firstStreet, std::string secondStreet);
void moveScreenStreet(ezgl::application* application);
void moveScreenIntersection(ezgl::application* application);
void seperateUserInputOfStreets(std::string userText, std::string *firstStreet, std::string *secondStreet);
void userSearchInput(GtkEntry * /*entry*/, ezgl::application* application);
void userTypingCompletion(GtkEntry* widget, ezgl::application* application);
void helpPage(GtkButton * /*button*/, ezgl::application* application);
void legendHelp(GtkButton * /*entry*/, ezgl::application* application);
void searchHelp(GtkButton * /*entry*/, ezgl::application* application);
void toggleHelp(GtkButton * /*entry*/, ezgl::application* application);
void distanceHelp(GtkButton * /*entry*/, ezgl::application* application);
void mapHelp(GtkButton * /*entry*/, ezgl::application* application);
void pathHelp(GtkButton * /*entry*/, ezgl::application* application);
void clickHelp(GtkButton * /*entry*/, ezgl::application* application);
void clickedPaths(GtkButton * /*button*/, ezgl::application* application);
void switchPathIntersections(GtkButton */*button*/, ezgl::application* application);
void directionDecisions(std::pair<IntersectionIdx, IntersectionIdx> intersection_pair);
std::string get_directions(std::vector <StreetSegmentIdx> streetseg_vector, int starting_intersectionIdx, int end_intersectionIdx);
void userSearchError(std::string errorMessage);
int rounding_int(int street_segment_length_as_int);
bool removeDuplicateName(std::string name);

/**************************** Function Declarations End ********************************/


/**************************** Data Structure Declarations ********************************/
//data structures for intersections
struct intersection_data {
    LatLon position;
    std::string name;
    ezgl::point2d xandy_coor_intersection;
    bool highlighted = false;
};
extern std::vector <intersection_data> intersections;

//data structures for POIs
struct POI_data {
    LatLon POI_position;
    std::string POI_name;
    ezgl::point2d xandy_coor_POI;
    bool highlighted = false;
};
extern std::vector <POI_data> POI_INFO;

//data structures for street segments
struct Street_segment_data {
    OSMID ID;
    std::string Street_seg_name;
    int num_curve_points;
    float speed_limit;
    ezgl::point2d from;
    ezgl::point2d to;
    std::vector <LatLon> curvepoints;
    std::vector <ezgl::point2d> Street_seg_points_xy;
    ezgl::point2d draw_text;
    double angle;
    std::vector <std::pair<ezgl::point2d,ezgl::point2d >> to_arrowhead;
    std::vector <std::pair<ezgl::point2d,ezgl::point2d >>  from_arrowhead;
};
extern std::vector <Street_segment_data> Street_segments_INFO;

//data structures for features
struct Feature_data {
    TypedOSMID Feature_ID;
    std::string Feature_name;
    int Feature_nun_points;
    FeatureType Feature_type;
    std::vector <LatLon> Feature_points;
    std::vector <ezgl::point2d> Feature_points_xy;
    bool area_closed;
};
extern std::vector <Feature_data> Feature_INFO;

//vector for streets
extern std::vector <StreetIdx> street_INFO;

//vector for subway stations
extern std::vector <ezgl::point2d> Subway_INFO;

//vector for libraries
extern std::vector <ezgl::point2d> Library_INFO;

//vector for fast food places
extern std::vector <ezgl::point2d> FastFood_INFO;

//vector for restaurants
extern std::vector <ezgl::point2d> Restaurant_INFO;

//vector for previously highlighted intersections
extern std::vector <int> prevHighlightedIntersections;

//external global variables
extern double latitudeAverage;
extern double min_lon;
extern double max_lon;
extern double min_lat;
extern double max_lat;

//used to print features in order on map
extern std::vector<Feature_data>lakes;
extern std::vector<Feature_data>islands;
extern std::vector<Feature_data>parks;
extern std::vector<Feature_data>greenspaces;
extern std::vector<Feature_data>golfCourses;
extern std::vector<Feature_data>glaciers;
extern std::vector<Feature_data>beaches;
extern std::vector<Feature_data>rivers;
extern std::vector<Feature_data>streams;
extern std::vector<Feature_data>buildings;
extern std::vector<Feature_data>unknowns;

//global vectors to retrieve street and intersection ids for search bar.
extern std::vector<StreetIdx> centerStreet;
extern std::vector<IntersectionIdx> centerIntersection;
extern std::vector<IntersectionIdx> centerIntersection2;

//vectors to help calculate the directions of a given path
extern std::vector <std::string> Directions_to_2_from;
extern std::vector <std::pair<std::string, std::string>> Directions_NESW;

//global vector of the path to be used elsewhere
extern std::vector <StreetSegmentIdx> street_seg_path;

//straight -> Straight | KR -> Keep Right | R -> Right | SHR -> Sharp Right | U -> UTurn | SHL -> Sharp Left | L -> Left | KL -> Keep Left
//Takes in the previous direction and next direction, and based on these parameters, it tells the user what direction they are going in.
// First element in the pair is the previous direction and the second element is the next direction.
std::unordered_map<std::pair<std::string, std::string>, std::string, boost::hash<std::pair<std::string, std::string>>> direction_umap = {
    {std::make_pair("N", "N"), "Straight"},
    {std::make_pair("N", "NE"), "KR"},
    {std::make_pair("N", "E"), "R"},
    {std::make_pair("N", "SE"), "SHR"},
    {std::make_pair("N", "S"), "U"},
    {std::make_pair("N", "SW"), "SHL"},
    {std::make_pair("N", "W"), "L"},
    {std::make_pair("N", "NW"), "KL"},
   
    {std::make_pair("NE", "N"), "KL"},
    {std::make_pair("NE", "NE"), "Straight"},
    {std::make_pair("NE", "E"), "KR"},
    {std::make_pair("NE", "SE"), "R"},
    {std::make_pair("NE", "S"), "SHR"},
    {std::make_pair("NE", "SW"), "U"},
    {std::make_pair("NE", "W"), "SHL"},
    {std::make_pair("NE", "NW"), "L"},
    
    {std::make_pair("E", "N"), "L"},
    {std::make_pair("E", "NE"), "KL"},
    {std::make_pair("E", "E"), "Straight"},
    {std::make_pair("E", "SE"), "KR"},
    {std::make_pair("E", "S"), "R"},
    {std::make_pair("E", "SW"), "SHR"},
    {std::make_pair("E", "W"), "U"},
    {std::make_pair("E", "NW"), "SHL"},

    {std::make_pair("SE", "N"), "SHL"},
    {std::make_pair("SE", "NE"), "L"},
    {std::make_pair("SE", "E"), "KL"},
    {std::make_pair("SE", "SE"), "Straight"},
    {std::make_pair("SE", "S"), "KR"},
    {std::make_pair("SE", "SW"), "R"},
    {std::make_pair("SE", "W"), "SHR"},
    {std::make_pair("SE", "NW"), "U"},

    {std::make_pair("S", "N"), "U"},
    {std::make_pair("S", "NE"), "SHL"},
    {std::make_pair("S", "E"), "L"},
    {std::make_pair("S", "SE"), "KL"},
    {std::make_pair("S", "S"), "Straight"},
    {std::make_pair("S", "SW"), "KR"},
    {std::make_pair("S", "W"), "R"},
    {std::make_pair("S", "NW"), "SHR"},

    {std::make_pair("SW", "N"), "SHR"},
    {std::make_pair("SW", "NE"), "U"},
    {std::make_pair("SW", "E"), "SHL"},
    {std::make_pair("SW", "SE"), "L"},
    {std::make_pair("SW", "S"), "KL"},
    {std::make_pair("SW", "SW"), "Straight"},
    {std::make_pair("SW", "W"), "KR"},
    {std::make_pair("SW", "NW"), "R"},

    {std::make_pair("W", "N"), "R"},
    {std::make_pair("W", "NE"), "SHR"},
    {std::make_pair("W", "E"), "U"},
    {std::make_pair("W", "SE"), "SHL"},
    {std::make_pair("W", "S"), "L"},
    {std::make_pair("W", "SW"), "KL"},
    {std::make_pair("W", "W"), "Straight"},
    {std::make_pair("W", "NW"), "KR"},

    {std::make_pair("NW", "N"), "KR"},
    {std::make_pair("NW", "NE"), "R"},
    {std::make_pair("NW", "E"), "SHR"},
    {std::make_pair("NW", "SE"), "U"},
    {std::make_pair("NW", "S"), "SHL"},
    {std::make_pair("NW", "SW"), "L"},
    {std::make_pair("NW", "W"), "KL"},
    {std::make_pair("NW", "NW"), "Straight"},
    };



/**************************** Data Structure Declarations End********************************/

/**************************** Global Variables ********************************/
bool darkMode = false;
bool streetsEnabled = true;
bool featuresEnabled = true;
bool POIsEnabled = true;
bool subwaysEnabled = true;
bool librariesEnabled = true;
bool foodEnabled = true;
int prevHighlightedPOI = 0;
std::vector <int> highlightedStreets;
int searchStatus = 1;
int clickStatus = 1;
std::string searchEntry;
int Map_location = 0;
int move = 0;
double detailZoom;
std::pair<IntersectionIdx, IntersectionIdx> Last_intersection_clicked;
int start_directions = 0;
int *searched_streets = new int(0);
GtkLabel *helpInfo;
ezgl::point2d mouseClickLoc(0,0);

ezgl::rectangle startingWorld;

/**************************** Global Variables End ********************************/


/**************************** Draw Map ********************************/
void drawMap() {
    //setting up EZGL window
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);

    //setting up initial canvas
    ezgl::rectangle initial_world({x_from_lon(min_lon), y_from_lat(min_lat)},
                  {x_from_lon(max_lon), y_from_lat(max_lat)});

    startingWorld = initial_world;
    
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);
    application.run(initial_setup, act_on_mouse_click, nullptr, nullptr);
}
/**************************** Draw Map End ********************************/

/**************************** EZGL Applications ********************************/
void initial_setup (ezgl::application* application, bool /*new_window*/){
    //create buttons for toggling layers and light/dark mode
    //gtk connection for Restaurants
    GObject* toggleRestaurant = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    toggleRestaurant = application->get_object("foodToggle");
    g_signal_connect(toggleRestaurant, "toggled", G_CALLBACK(toggleFood), application);
    
    //gtk connection for Libraries
    GObject* toggleLibrary = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    toggleLibrary = application->get_object("libraryToggle");
    g_signal_connect(toggleLibrary, "toggled", G_CALLBACK(toggleLibraries), application);
    
    //gtk connection for POI
    GObject* togglePOI = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    togglePOI = application->get_object("POIToggle");
    g_signal_connect(togglePOI, "toggled", G_CALLBACK(togglePOIs), application);
    
    //gtk connection for Features
    GObject* toggleFeature = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    toggleFeature = application->get_object("FeatureToggle");
    g_signal_connect(toggleFeature, "toggled", G_CALLBACK(toggleFeatures), application);
    
    //gtk connection for streets
    GObject* toggleStreet = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    toggleStreet = application->get_object("StreetToggle");
    g_signal_connect(toggleStreet, "toggled", G_CALLBACK(toggleStreets), application);
    
    //gtk connection for Subways
    GObject* toggleSubway = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    toggleSubway = application->get_object("SubwayToggle");
    g_signal_connect(toggleSubway, "toggled", G_CALLBACK(toggleSubways), application);

    //gtk connection for Dark Mode
    GObject* toggleNight = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    toggleNight = application->get_object("DarkToggle");
    g_signal_connect(toggleNight, "toggled", G_CALLBACK(toggleDark), application);
    
    //gtk connection for search bar
    GObject* searchBar = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    searchBar = application->get_object("searchBar");
    g_signal_connect(searchBar, "activate", G_CALLBACK(userSearchInput), application);
    g_signal_connect(searchBar, "changed", G_CALLBACK(userTypingCompletion), application);
    
    //gtk connection for different maps
    GObject* combo_box = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    combo_box = application->get_object("mapList");
    g_signal_connect(combo_box, "changed", G_CALLBACK(mapChangeInput), application);
    
    //gtk connection for help menu
    GObject* helpMenu = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    helpMenu = application->get_object("helpMenu");
    g_signal_connect(helpMenu, "clicked", G_CALLBACK(helpPage), application);
    
    //gtk connection for help pop-up
    GObject* helpWindow = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    helpWindow = application->get_object("helpPage");
    g_signal_connect(helpWindow, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), application);
    
    //gtk connection for help in colour-coding
    GObject* legend = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    legend = application->get_object("legendHelp");
    g_signal_connect(legend, "clicked", G_CALLBACK(legendHelp), application);
    
    //gtk connection for help in searching
    GObject* search = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    search = application->get_object("searchHelp");
    g_signal_connect(search, "clicked", G_CALLBACK(searchHelp), application);
    
    //gtk connection for help in toggling features
    GObject* toggle = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    toggle = application->get_object("toggleHelp");
    g_signal_connect(toggle, "clicked", G_CALLBACK(toggleHelp), application);
    
    //gtk connection for help in distance scale
    GObject* distance = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    distance = application->get_object("distanceHelp");
    g_signal_connect(distance, "clicked", G_CALLBACK(distanceHelp), application);
    
    //gtk connection for help in loading maps
    GObject* map = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    map = application->get_object("loadMapHelp");
    g_signal_connect(map, "clicked", G_CALLBACK(mapHelp), application);
    
    //gtk connection for help in finding paths
    GObject* path = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    path = application->get_object("pathHelp");
    g_signal_connect(path, "clicked", G_CALLBACK(pathHelp), application);
    
    //gtk connection for help in last clicked area
    GObject* click = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    click = application->get_object("clickHelp");
    g_signal_connect(click, "clicked", G_CALLBACK(clickHelp), application);
    
    //gtk connection for clicked intersection paths
    GObject* intersectionPaths = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    intersectionPaths = application->get_object("intersectionPaths");
    g_signal_connect(intersectionPaths, "clicked", G_CALLBACK(clickedPaths), application);
    
    //gtk connection for switching intersections
    GObject* switchPaths = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    switchPaths = application->get_object("switchPath");
    g_signal_connect(switchPaths, "clicked", G_CALLBACK(switchPathIntersections), application);
}
 
 //highlight clicked area and show info 
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y){
    (void)event;
    
    //gtk code to create dialogue box to show information on clicked location
    GtkWidget *dialog = gtk_dialog_new();
    GtkWidget *mouseLabel;
    GtkWidget *infoLabel;
    GtkWidget *POILabel;
    GtkWidget *container;
    gtk_window_set_title(GTK_WINDOW(dialog), "Information");
    gtk_widget_set_size_request(dialog, 200, 150);
    
    //gtk connection for input into dialogue box about mouse clicked location
    std::string mouse = "Mouse clicked at " + std::to_string(x) + ", " + std::to_string(y) + "\n";
    mouseLabel = gtk_label_new(mouse.c_str());
    container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(container), mouseLabel);
  
    //find closest intersection and POI
    LatLon pos = LatLon(lat_from_y(y), lon_from_x(x));
    int intId = findClosestIntersection(pos);
    int poiId = findAnyClosestPOI(pos);

    //Updating vector to keep track of last point clicked
    Last_intersection_clicked.first = Last_intersection_clicked.second;
    Last_intersection_clicked.second = intId;   

    //gtk connection for input into dialogue box about closest intersection
    std::string closestIntersection = "Closest Intersection: " + intersections[intId].name + "\n";
    infoLabel = gtk_label_new(closestIntersection.c_str());
    gtk_container_add(GTK_CONTAINER(container), infoLabel);
    
    //gtk connection for input into dialogue box about closest POI
    std::string closestPOI = "Closest Point of Interest: " + POI_INFO[poiId].POI_name + "\n";
    POILabel = gtk_label_new(closestPOI.c_str());
    gtk_container_add(GTK_CONTAINER(container), POILabel);

    //show dialogue box
    gtk_widget_show_all(dialog);

    //clear previous highlighted intersection and POI
    for(int intersectionNum = 0; intersectionNum < prevHighlightedIntersections.size(); intersectionNum++){
        intersections[prevHighlightedIntersections[intersectionNum]].highlighted = false;
    }
    POI_INFO[prevHighlightedPOI].highlighted = false;

    //update previous highlighted POI and intersection
    prevHighlightedIntersections.push_back(intId);
    prevHighlightedPOI = poiId;

    //store location of where mouse was clicked
    mouseClickLoc.x = x;
    mouseClickLoc.y = y;

    //update highlighted intersection and POI
    intersections[intId].highlighted = true;
    POI_INFO[poiId].highlighted = true;

    //update map drawing
    app->refresh_drawing();
}
/**************************** EZGL Applications End ********************************/

/**************************** Helper Functions ********************************/

//find the closest POI to location, regardless of type
POIIdx findAnyClosestPOI(LatLon my_position){
    int numPOI = getNumPointsOfInterest();
    //set initial distance to be first POI
    POIIdx closest = 0;
    int distance = findDistanceBetweenTwoPoints(getPOIPosition(0), my_position);

    //if next POI is closer, that is the closest POI
    for(int poiId = 0; poiId < numPOI; poiId++){
        if(findDistanceBetweenTwoPoints(getPOIPosition(poiId), my_position) < distance){
            closest = poiId;
            distance = findDistanceBetweenTwoPoints(getPOIPosition(poiId), my_position);
        }
    }

    return closest;

}

//toggle dark mode when button pressed
void toggleDark(GtkWidget* /*widget*/, ezgl::application* application){
    darkMode = !darkMode;
    application->refresh_drawing();
}

//toggle streets visibility when button pressed
void toggleStreets(GtkWidget* /*widget*/, ezgl::application* application){
    streetsEnabled = !streetsEnabled;
    application->refresh_drawing();
}

//toggle features visibility when button pressed
void toggleFeatures(GtkWidget* /*widget*/, ezgl::application* application){
    featuresEnabled = !featuresEnabled;
    application->refresh_drawing();
}

//toggle POI visibility when button pressed
void togglePOIs(GtkWidget* /*widget*/, ezgl::application* application){
    POIsEnabled = !POIsEnabled;
    application->refresh_drawing();
}

//toggle subway visibility when button pressed
void toggleSubways(GtkWidget* /*widget*/, ezgl::application* application){
    subwaysEnabled = !subwaysEnabled;
    application->refresh_drawing();  
}

//toggle libraries visibility when button pressed
void toggleLibraries(GtkWidget* /*widget*/, ezgl::application* application){
    librariesEnabled = !librariesEnabled;
    application->refresh_drawing();  
}

//toggle restaurant visibility when button pressed
void toggleFood(GtkWidget* /*widget*/, ezgl::application* application){
    foodEnabled = !foodEnabled;
    application->refresh_drawing();  
}

//Takes in an integer and rounds it to the nearest 50
int rounding_int(int street_segment_length_as_int){
    int street_len_approx = 50;
    //find out if length mod 50 is closer to closer to 0 or 50
    int rounding = street_segment_length_as_int%street_len_approx;

    //doesn't round if it is less than 50
    if(street_segment_length_as_int < 50){
        street_segment_length_as_int = street_segment_length_as_int;
    }
    //rounds down if it is closer to 0
    else if(rounding < street_len_approx/2){
        street_segment_length_as_int = street_segment_length_as_int - rounding;
    }
    //round up if its closer to 50
    else{
        street_segment_length_as_int = street_segment_length_as_int + street_len_approx - rounding;
    }
    return street_segment_length_as_int;
}

/**************************** Helper Functions End ********************************/

/**************************** Drawing ********************************/

void draw_main_canvas(ezgl::renderer *g){

    //get the size of the world and screen
    ezgl::rectangle world_coor = g->get_visible_world();
    ezgl::rectangle visible_screen = g->get_visible_screen();

    //changes the background based on which mode we're in
    if(darkMode){
        g->set_color(74, 74, 74);
        g->fill_rectangle(world_coor);
    }
    else{
        g->set_color(215,215,215);
        g->fill_rectangle(world_coor);
    }

    /********************************* Zoom Level *************************************/


    //calculate ratio between world and screen. Ratio will determine how zoomed in we are
    double zoom_number = world_coor.width()/visible_screen.width();

    //assigns the appropriate zoom level based on ratio. Higher zoom level means more zoomed in
    int zoom_level = 0; 
    if(zoom_number > zoom_boundary_zero_one){
        zoom_level = 0;
    }
    else if(zoom_number > zoom_boundary_one_two){
        zoom_level = 1;
    }
    else if(zoom_number > zoom_boundary_two_three){
        zoom_level = 2;
    }
    else if(zoom_number > zoom_boundary_three_four){
        zoom_level = 3;
    }
    else if(zoom_number > zoom_boundary_four_five){
        zoom_level = 4;
    }

    //zoom variable used for more detailed dynamic zooming
    detailZoom = 100 * world_coor.width()/visible_screen.width();
    /********************************* End Zoom *************************************/

    g->set_line_width(1);
    g->set_font_size(10);
    std::string current_font;
    /*
    0 = Toronto,Canada              - English   
    1 = beijing, china              - CJK
    2 = cairo, Egypt                - Arabic
    3 = cape-town, South-Africa     - English
    4 = golden-horseshoe, Canada    - English
    5 = hamilton, Canada            - English
    6 = hong-kong, China            - CJK
    7 = Iceland                     - English
    8 = interlaken, Switzerland     - English
    9 = kyiv, Ukraine               - Noto Sans 
    10 = london, England            - English
    11 = new-delhi, India           - English   
    12 = new-york,USA               - English
    13 = rio-de-janeiro, Brazil     - English   
    14 = saint-helena               - English
    15 = singapore                  - English
    16 = sydney, Australia          - English
    17 = tehran, Iran               - DejaVu
    18 = Tokyo, Japan               - CJK
    */

   //setting languages depending on map selected
    //CJK
    if (Map_location == 1 || Map_location == 6 || Map_location == 18){
        g->format_font("Noto Sans CJK SC", ezgl::font_slant::normal, ezgl::font_weight::normal, 10);
        current_font = "Noto Sans CJK SC";
    } 
    //Cairo, Egypt, and Tehran, Iran
    else if (Map_location == 2 || Map_location == 17){
        g->format_font("DejaVu Sans Mono", ezgl::font_slant::normal, ezgl::font_weight::normal, 10);
        current_font = "DejaVu Sans Mono";
    } 
    //kyiv, Ukraine
    else if (Map_location == 9){
        g->format_font("Noto Sans", ezgl::font_slant::normal, ezgl::font_weight::normal, 10);
        current_font = "Noto Sans";
    } 
    //Map_location 0,3,4,5,7,8,10,11,12,13,14,15,16
    else{
        g->format_font("", ezgl::font_slant::normal, ezgl::font_weight::normal, 10);
        current_font = "";
    }

    /********************************* Zoom-Checking *************************************/

    //zoomed out too far, reset zoom level to starting level
    if(world_coor.height() > startingWorld.height()){
        g->set_visible_world(startingWorld);
    }
    //zoomed in too far, zoom back a little bit
    else if(detailZoom < 1.5){
        ezgl::rectangle rect = g->get_visible_world();
        //points selected to "bounce" back a little bit and let user know they can't zoom in any more
        ezgl::point2d diffPoint(6.25, 7.5);       
        ezgl::point2d botLeft = rect.bottom_left();
        botLeft -= diffPoint;
        ezgl::point2d topRight = rect.top_right();
        topRight += diffPoint;

        ezgl::rectangle newRect(botLeft, topRight);
        g->set_visible_world(newRect);
    }


    /********************************* End Zoom-Checking *************************************/

    /********************************* Features *************************************/
    //resetting rotation and width
    g->set_text_rotation(0);
    g->set_line_width(1);
    //loop through all features, check there type and colour code them accordingly
    if(featuresEnabled){
        //print lakes
        for(int Feature_idx = 0; Feature_idx < lakes.size(); Feature_idx++){
            //set colour for feature
            if(darkMode){
                g->set_color(30, 63, 102);
            }
            else{
                g->set_color(85,206,255);
            }
            //draw feature if polygon
            if(lakes[Feature_idx].area_closed){
                g->fill_poly(lakes[Feature_idx].Feature_points_xy);
            }
            //draw feature if not polygon
            else{
                for(size_t feature_point_idx = 0; feature_point_idx < lakes[Feature_idx].Feature_points_xy.size()-1; feature_point_idx++){
                    g->draw_line(lakes[Feature_idx].Feature_points_xy[feature_point_idx], lakes[Feature_idx].Feature_points_xy[feature_point_idx+1]);
                }
            }
        }

        //print islands
        for(int Feature_idx = 0; Feature_idx < islands.size(); Feature_idx++){
            //set colour for feature
            if(darkMode){
                g->set_color(150, 108, 71);
            }
            else{
                g->set_color(255,168,92);
            }
            //draw feature if polygon
            if(islands[Feature_idx].area_closed){
                g->fill_poly(islands[Feature_idx].Feature_points_xy);
            }
            //draw feature if not polygon
            else{
                for(size_t feature_point_idx = 0; feature_point_idx < islands[Feature_idx].Feature_points_xy.size()-1; feature_point_idx++){
                    g->draw_line(islands[Feature_idx].Feature_points_xy[feature_point_idx], islands[Feature_idx].Feature_points_xy[feature_point_idx+1]);
                }
            }
        }

        //print parks
        for(int Feature_idx = 0; Feature_idx < parks.size(); Feature_idx++){
            //set colour for feature
            if(darkMode){
                g->set_color(0, 99, 10);
            }
            else{
                g->set_color(148,229,107);
            }
            //draw feature if polygon
            if(parks[Feature_idx].area_closed){
                g->fill_poly(parks[Feature_idx].Feature_points_xy);
            }
            //draw feature if not polygon
            else{
                for(size_t feature_point_idx = 0; feature_point_idx < parks[Feature_idx].Feature_points_xy.size()-1; feature_point_idx++){
                    g->draw_line(parks[Feature_idx].Feature_points_xy[feature_point_idx], parks[Feature_idx].Feature_points_xy[feature_point_idx+1]);
                }
            }
        }

        //print greenspaces
        for(int Feature_idx = 0; Feature_idx < greenspaces.size(); Feature_idx++){
            //set colour for feature
            if(darkMode){
                g->set_color(0, 99, 10);
            }
            else{
                g->set_color(148,229,107);
            }
            //draw feature if polygon
            if(greenspaces[Feature_idx].area_closed){
                g->fill_poly(greenspaces[Feature_idx].Feature_points_xy);
            }
            //draw feature if not polygon
            else{
                for(size_t feature_point_idx = 0; feature_point_idx < greenspaces[Feature_idx].Feature_points_xy.size()-1; feature_point_idx++){
                    g->draw_line(greenspaces[Feature_idx].Feature_points_xy[feature_point_idx], greenspaces[Feature_idx].Feature_points_xy[feature_point_idx+1]);
                }
            }
        }

        //print golfCourses
        for(int Feature_idx = 0; Feature_idx < golfCourses.size(); Feature_idx++){
            //set colour for feature
            if(darkMode){
                g->set_color(35,82,15);
            }
            else{
                g->set_color(74,217,88);
            }
            //draw feature if polygon
            if(golfCourses[Feature_idx].area_closed){
                g->fill_poly(golfCourses[Feature_idx].Feature_points_xy);
            }
            //draw feature if not polygon
            else{
                for(size_t feature_point_idx = 0; feature_point_idx < golfCourses[Feature_idx].Feature_points_xy.size()-1; feature_point_idx++){
                    g->draw_line(golfCourses[Feature_idx].Feature_points_xy[feature_point_idx], golfCourses[Feature_idx].Feature_points_xy[feature_point_idx+1]);
                }
            }
        }

        //print glaciers
        for(int Feature_idx = 0; Feature_idx < glaciers.size(); Feature_idx++){
            //set colour for feature
            if(darkMode){
                g->set_color(20, 102, 122);
            }
            else{
                g->set_color(205,240,255);
            }
            //draw feature if polygon
            if(glaciers[Feature_idx].area_closed){
                g->fill_poly(glaciers[Feature_idx].Feature_points_xy);
            }
            //draw feature if not polygon
            else{
                for(size_t feature_point_idx = 0; feature_point_idx < glaciers[Feature_idx].Feature_points_xy.size()-1; feature_point_idx++){
                    g->draw_line(glaciers[Feature_idx].Feature_points_xy[feature_point_idx], glaciers[Feature_idx].Feature_points_xy[feature_point_idx+1]);
                }
            }
        }

        //print beaches
        for(int Feature_idx = 0; Feature_idx < beaches.size(); Feature_idx++){
            //set colour for feature
            if(darkMode){
                g->set_color(184, 168, 145);
            }
            else{
                g->set_color(255,235,205);
            }
            //draw feature if polygon
            if(beaches[Feature_idx].area_closed){
                g->fill_poly(beaches[Feature_idx].Feature_points_xy);
            }
            //draw feature if not polygon
            else{
                for(size_t feature_point_idx = 0; feature_point_idx < beaches[Feature_idx].Feature_points_xy.size()-1; feature_point_idx++){
                    g->draw_line(beaches[Feature_idx].Feature_points_xy[feature_point_idx], beaches[Feature_idx].Feature_points_xy[feature_point_idx+1]);
                }
            }
        }

        //print rivers
        for(int Feature_idx = 0; Feature_idx < rivers.size(); Feature_idx++){
            //set colour for feature
            if(darkMode){
                g->set_color(30, 63, 102);
            }
            else{
                g->set_color(85,206,255);
            }
            //draw feature if polygon
            if(rivers[Feature_idx].area_closed){
                g->fill_poly(rivers[Feature_idx].Feature_points_xy);
            }
            //draw feature if not polygon
            else{
                for(size_t feature_point_idx = 0; feature_point_idx < rivers[Feature_idx].Feature_points_xy.size()-1; feature_point_idx++){
                    g->draw_line(rivers[Feature_idx].Feature_points_xy[feature_point_idx], rivers[Feature_idx].Feature_points_xy[feature_point_idx+1]);
                }
            }
        }

        //print streams
        for(int Feature_idx = 0; Feature_idx < streams.size(); Feature_idx++){
            //set colour for feature
            if(darkMode){
                g->set_color(30, 63, 102);
            }
            else{
                g->set_color(85,206,255);
            }
            //draw feature if polygon
            if(streams[Feature_idx].area_closed){
                g->fill_poly(streams[Feature_idx].Feature_points_xy);
            }
            //draw feature if not polygon
            else{
                for(size_t feature_point_idx = 0; feature_point_idx < streams[Feature_idx].Feature_points_xy.size()-1; feature_point_idx++){
                    g->draw_line(streams[Feature_idx].Feature_points_xy[feature_point_idx], streams[Feature_idx].Feature_points_xy[feature_point_idx+1]);
                }
            }
        }

        //print buildings
        for(int Feature_idx = 0; Feature_idx < buildings.size(); Feature_idx++){
            //set colour for feature
            if(darkMode){
                g->set_color(99, 99, 99);
            }
            else{
                g->set_color(169,169,169);
            }
            //draw buildings only if zoom level is greater than 3
            if(zoom_level > 3){
                //draw feature if polygon
                if(buildings[Feature_idx].area_closed){
                    g->fill_poly(buildings[Feature_idx].Feature_points_xy);
                }
                //draw feature if not polygon
                else{
                    for(size_t feature_point_idx = 0; feature_point_idx < buildings[Feature_idx].Feature_points_xy.size()-1; feature_point_idx++){
                        g->draw_line(buildings[Feature_idx].Feature_points_xy[feature_point_idx], buildings[Feature_idx].Feature_points_xy[feature_point_idx+1]);
                    }
                }
            }
        }
    }
    /********************************* End Features *************************************/



    /********************************* Street Segments *************************************/
    if(streetsEnabled){   
        for(size_t Street_seg_idx = 0; Street_seg_idx < Street_segments_INFO.size(); Street_seg_idx++){
            //differentiate between highways, major, minor, and highlighted streets
            //if street is highlighted
            if(std::find(highlightedStreets.begin(), highlightedStreets.end(), getStreetSegmentInfo(Street_seg_idx).streetID) != highlightedStreets.end()){
                if(zoom_level < 3){
                    g->set_line_width(3);
                }
                else{
                    g->set_line_width(6);
                }
                g->set_color(3, 252, 236);
            }
            //street is highway
            else if(Street_segments_INFO.at(Street_seg_idx).speed_limit > 22){
                if(zoom_level < 3){
                    g->set_line_width(4);
                }
                else{
                    g->set_line_width(6);
                }
                if(darkMode){
                    g->set_color(186,134,65);
                }
                else{
                    g->set_color(255, 221, 120);
                }
            }
            //street is major street
            else if(Street_segments_INFO.at(Street_seg_idx).speed_limit > 14){
                if(zoom_level < 2){
                    g->set_line_width(2);
                }
                else{
                    g->set_line_width(4);
                }
                if(darkMode){
                    g->set_color(153, 153, 153);
                }
                else{
                    g->set_color(255,228,196);
                }
            }
            //street is minor street
            else{
                if(zoom_level < 3){
                    g->set_line_width(3);
                }
                else{
                    g->set_line_width(3);
                }
                if(darkMode){
                    g->set_color(140, 140, 140);
                }
                else{
                    g->set_color(255,255,255);
                }
            }
            //only show major streets when the most zoomed out
            if((Street_segments_INFO.at(Street_seg_idx).speed_limit > 14) || std::find(highlightedStreets.begin(), highlightedStreets.end(), getStreetSegmentInfo(Street_seg_idx).streetID) != highlightedStreets.end()){
                //if the street is not curved
                if(Street_segments_INFO.at(Street_seg_idx).num_curve_points == 0){
                    //draw street
                    g->draw_line(Street_segments_INFO.at(Street_seg_idx).from, Street_segments_INFO.at(Street_seg_idx).to);
                }
                //if the street is curved
                else{
                    //draw the line for street.from -> first curved point
                    g->draw_line(Street_segments_INFO.at(Street_seg_idx).from, Street_segments_INFO[Street_seg_idx].Street_seg_points_xy[0]);

                    //loop through all the curved points and draw them
                    for(size_t street_seg_curve_points = 0; street_seg_curve_points < Street_segments_INFO[Street_seg_idx].Street_seg_points_xy.size()-1;street_seg_curve_points++){
                        g->draw_line(Street_segments_INFO[Street_seg_idx].Street_seg_points_xy[street_seg_curve_points], Street_segments_INFO[Street_seg_idx].Street_seg_points_xy[street_seg_curve_points+1]);
                    }
                    //draw the line for the last curved point -> street.to
                    g->draw_line(Street_segments_INFO[Street_seg_idx].Street_seg_points_xy[Street_segments_INFO[Street_seg_idx].Street_seg_points_xy.size()-1],Street_segments_INFO.at(Street_seg_idx).to);
                    
                }
            }
            //if the zoom level is greater than 1, draw all streets
            else if(zoom_level > 1){
                //for streets with no curved points
                if(Street_segments_INFO.at(Street_seg_idx).num_curve_points == 0){
                    //draw street
                    g->draw_line(Street_segments_INFO.at(Street_seg_idx).from, Street_segments_INFO.at(Street_seg_idx).to);
                }
                //if the street is curved
                else{
                    g->draw_line(Street_segments_INFO.at(Street_seg_idx).from, Street_segments_INFO[Street_seg_idx].Street_seg_points_xy[0]);
                    for(size_t street_seg_curve_points = 0; street_seg_curve_points < Street_segments_INFO[Street_seg_idx].Street_seg_points_xy.size()-1;street_seg_curve_points++){
                        g->draw_line(Street_segments_INFO[Street_seg_idx].Street_seg_points_xy[street_seg_curve_points], Street_segments_INFO[Street_seg_idx].Street_seg_points_xy[street_seg_curve_points+1]);
                    }
                    //draw the line for the last curved point -> street.to
                    g->draw_line(Street_segments_INFO[Street_seg_idx].Street_seg_points_xy[Street_segments_INFO[Street_seg_idx].Street_seg_points_xy.size()-1],Street_segments_INFO.at(Street_seg_idx).to);
                }
            }
        } 
    }
    /********************************* End Street Segments *************************************/

    

    /********************************* Intersections *************************************/
    //show intersections only if zoomed in and streets are enabled
    if(zoom_level >= 3 && streetsEnabled){
        for(size_t inter_id = 0; inter_id < intersections.size(); ++inter_id){
            double width = 4;
            double height = width;

            //if we want them highlighted, highlight them in yellow. If not, put them in red
            if(intersections[inter_id].highlighted){
                g->set_color(0, 255, 170);
            }
            else{
                g->set_color(255,154,152);
            }
            //draws the intersections. offset is used to center the intersection points on the intersection
            double offset = width/2;
            g->fill_rectangle({intersections[inter_id].xandy_coor_intersection.x-offset,intersections[inter_id].xandy_coor_intersection.y-offset}, width,height);
        }
    }
    /*********************************End Intersections*************************************/

    /********************************* Drawing Path *************************************/
    //reset colors and line width
    g->set_line_width(15);
    if(darkMode){
        g->set_color(145,26,26);
    }
    else{
        g->set_color(255,94,94);
    }
    //Checks if a there is path searched
    if(*searched_streets == 1){
        //Loop through the vector of path segments
        for(size_t Street_seg_idx = 0; Street_seg_idx < street_seg_path.size(); Street_seg_idx++){
            //if it's straight,,,
            if(Street_segments_INFO[street_seg_path[Street_seg_idx]].num_curve_points == 0){
                // draws road
                g->set_line_width(5);
                g->draw_line(Street_segments_INFO.at(street_seg_path[Street_seg_idx]).from,Street_segments_INFO.at(street_seg_path[Street_seg_idx]).to);
                //draws the arrows based on direction
                //if it is a to -> from street
                if(Directions_to_2_from[Street_seg_idx] == "to2from"){
                    g->draw_line(Street_segments_INFO.at(street_seg_path[Street_seg_idx]).from_arrowhead.front().first,Street_segments_INFO.at(street_seg_path[Street_seg_idx]).from);
                    g->draw_line(Street_segments_INFO.at(street_seg_path[Street_seg_idx]).from_arrowhead.front().second,Street_segments_INFO.at(street_seg_path[Street_seg_idx]).from);
                }
                //if it is a from -> to street
                else{
                    g->draw_line(Street_segments_INFO.at(street_seg_path[Street_seg_idx]).to_arrowhead.front().first,Street_segments_INFO.at(street_seg_path[Street_seg_idx]).to);
                    g->draw_line(Street_segments_INFO.at(street_seg_path[Street_seg_idx]).to_arrowhead.front().second,Street_segments_INFO.at(street_seg_path[Street_seg_idx]).to);
                }
            }
            //if it is a curved street
            else if(Street_segments_INFO[street_seg_path[Street_seg_idx]].num_curve_points > 0){
                //draws the curved street
                g->set_line_width(5); 
                g->draw_line(Street_segments_INFO.at(street_seg_path[Street_seg_idx]).from, Street_segments_INFO.at(street_seg_path[Street_seg_idx]).Street_seg_points_xy[0]);

                for(size_t street_seg_curve_points = 0; street_seg_curve_points < Street_segments_INFO[street_seg_path[Street_seg_idx]].Street_seg_points_xy.size()-1;street_seg_curve_points++){
                    g->draw_line(Street_segments_INFO[street_seg_path[Street_seg_idx]].Street_seg_points_xy[street_seg_curve_points], Street_segments_INFO[street_seg_path[Street_seg_idx]].Street_seg_points_xy[street_seg_curve_points+1]);
                }
                //draw the line for the last curved point -> street.to
                g->draw_line(Street_segments_INFO[street_seg_path[Street_seg_idx]].Street_seg_points_xy[Street_segments_INFO[street_seg_path[Street_seg_idx]].Street_seg_points_xy.size()-1],Street_segments_INFO.at(street_seg_path[Street_seg_idx]).to);
                //draws the arrow heads if it is a to -> from street
                if(Directions_to_2_from[Street_seg_idx] == "to2from" && Street_segments_INFO.at(Street_seg_idx).from_arrowhead.size() > 0){
                    g->draw_line(Street_segments_INFO.at(street_seg_path[Street_seg_idx]).from_arrowhead.front().first,Street_segments_INFO.at(street_seg_path[Street_seg_idx]).from);
                    g->draw_line(Street_segments_INFO.at(street_seg_path[Street_seg_idx]).from_arrowhead.front().second,Street_segments_INFO.at(street_seg_path[Street_seg_idx]).from);
                }           
                //draws the arrow heads if it is a from -> to street
                else if(Directions_to_2_from[Street_seg_idx] == "from2to" && Street_segments_INFO.at(Street_seg_idx).to_arrowhead.size() > 0){
                    g->draw_line(Street_segments_INFO.at(street_seg_path[Street_seg_idx]).to_arrowhead.front().first,Street_segments_INFO.at(street_seg_path[Street_seg_idx]).to);
                    g->draw_line(Street_segments_INFO.at(street_seg_path[Street_seg_idx]).to_arrowhead.front().second,Street_segments_INFO.at(street_seg_path[Street_seg_idx]).to);
                }

            }
        }
    }
    /********************************* End of Drawing Path *************************************/

    /********************************* Drawing POIs *************************************/
    //draw POIs only if they are enabled and detail zoom level is < 200
    if(POIsEnabled && detailZoom < 200){
        //reset rotation and text size    
        g->set_text_rotation(0);
        g->set_font_size(9);
        
        //set POI colour to orange, or yellow if highlighted
        for(size_t POI_idx = 0; POI_idx < POI_INFO.size(); POI_idx++){

            if(POI_INFO[POI_idx].highlighted){
                g->set_color(ezgl::YELLOW);
            }
            else{
                if(darkMode){
                    g->set_color(115,61,20);
                }
                else{
                    g->set_color(ezgl::ORANGE);
                }
            }
            //constants for drawing POI
            int radius = 5;
            int start_deg = 0;
            int end_deg = 360;

            //draw all if zoomed in close
            if(detailZoom < 60){
                g->fill_elliptic_arc(POI_INFO[POI_idx].xandy_coor_POI, radius,radius,start_deg,end_deg);
            }
            //draw every other if zoomed in not that close, or highlighted
            else if(detailZoom < 100){
                if (POI_idx % 2 == 0 || POI_INFO[POI_idx].highlighted){
                    g->fill_elliptic_arc(POI_INFO[POI_idx].xandy_coor_POI, radius,radius,start_deg,end_deg);
                }
            }
            //draw 1 in 3 if zoomed out further, or highlighted
            else if(POI_idx % 3 == 0 || POI_INFO[POI_idx].highlighted){
                g->fill_elliptic_arc(POI_INFO[POI_idx].xandy_coor_POI, radius,radius,start_deg,end_deg);
            }  
        }
    }
    /********************************* End POIs *************************************/
    
 
    /********************************* Drawing Subways *************************************/

    //only draw subways if they're enabled
    if(subwaysEnabled){
        //reset text rotation
        g->set_text_rotation(0);
        //draw if zoom level is greater than 0
        if(zoom_level > 0){
            //loads fonts for subway icons
            g->format_font("Emoji", ezgl::font_slant::normal, ezgl::font_weight::normal, 15);
            //adjust based on zoom level
            if(zoom_level > 2){
                g->set_font_size(25);
            }
            //constant for loading unicode associated with subway icon
            std::string Unicode_SubwayIcon = "\U0001f687";
            //draw subway icons for all subway entrances
            for(int subway_entrance_num = 0; subway_entrance_num < Subway_INFO.size(); subway_entrance_num++){
                g->draw_text(Subway_INFO[subway_entrance_num], Unicode_SubwayIcon);
            }
            //reset font after drawing subways
            g->format_font(current_font, ezgl::font_slant::normal, ezgl::font_weight::normal, 10);
        }
    }

    /********************************* End of Subways *************************************/

    /********************************* Drawing Libraries *************************************/
    //only draw libraries if they're enabled
    if(librariesEnabled){
        //reset text rotation
        g->set_text_rotation(0);
        //draw if zoom level is greater than 0
        if(zoom_level > 0){
            //loads fonts for library icons
            g->format_font("Emoji", ezgl::font_slant::normal, ezgl::font_weight::normal, 15);
            //adjust based on zoom level
            if(zoom_level > 2){
                g->set_font_size(25);
            }
            //constant for loading unicode associated with library icon
            std::string Unicode_LibraryIcon = "\U0001f4da";
            //draw subway icons for all libraries
            for(int libraryNum = 0; libraryNum < Library_INFO.size(); libraryNum++){
                g->draw_text(Library_INFO[libraryNum], Unicode_LibraryIcon);
            }
            //reset font after drawing libraries
            g->format_font(current_font, ezgl::font_slant::normal, ezgl::font_weight::normal, 10);
        }
    }

    /********************************* End of Libraries *************************************/

    /********************************* Drawing Food *************************************/
    // only draw restaurants if they're enabled
    if(foodEnabled){
        //reset text rotation
        g->set_text_rotation(0);
        //draw if zoom level is greater than 2
        if(zoom_level > 2){
            //loads fonts for food icons
            g->format_font("Emoji", ezgl::font_slant::normal, ezgl::font_weight::normal, 15);
            //adjust based on zoom level
            if(zoom_level > 2){
                g->set_font_size(25);
            }
            //constant for loading unicode associated with fast food icon
            std::string Unicode_FastFoodIcon = "\U0001F354";
            //draw fast food icons for all fast food places

            //zoom levels for restaurants
            for(int fastFoodNum = 0; fastFoodNum < FastFood_INFO.size(); fastFoodNum++){
                //drawing different amounts based off zoom level to avoid clutter
                if(detailZoom < 40){
                    g->draw_text(FastFood_INFO[fastFoodNum], Unicode_FastFoodIcon);
                }
                else if(detailZoom < 60){
                    if(fastFoodNum % 2 == 0){
                        g->draw_text(FastFood_INFO[fastFoodNum], Unicode_FastFoodIcon);
                    }
                }
                else if(detailZoom < 100){
                    if(fastFoodNum % 3 == 0){
                        g->draw_text(FastFood_INFO[fastFoodNum], Unicode_FastFoodIcon);
                    }
                }
                else if(detailZoom < 200){
                    if(fastFoodNum % 4 == 0){
                        g->draw_text(FastFood_INFO[fastFoodNum], Unicode_FastFoodIcon);
                    }
                }
                else{
                    if(fastFoodNum % 5 == 0){
                        g->draw_text(FastFood_INFO[fastFoodNum], Unicode_FastFoodIcon);
                    }
                }
            }

            //constant for loading unicode associated with restaurant icon
            std::string Unicode_RestaurantIcon = "\U0001F959";
            //draw restaurant icons for all restaurants
            for(int restaurantNum = 0; restaurantNum < Restaurant_INFO.size(); restaurantNum++){
                //drawing different amounts based off zoom level to avoid clutter
                if(detailZoom < 40){
                    g->draw_text(Restaurant_INFO[restaurantNum], Unicode_RestaurantIcon);
                }
                else if(detailZoom < 60){
                    if(restaurantNum % 2 == 0){
                        g->draw_text(Restaurant_INFO[restaurantNum], Unicode_RestaurantIcon);
                    }
                }
                else if(detailZoom < 100){
                    if(restaurantNum % 3 == 0){
                        g->draw_text(Restaurant_INFO[restaurantNum], Unicode_RestaurantIcon);
                    }
                }
                else if(detailZoom < 200){
                    if(restaurantNum % 4 == 0){
                        g->draw_text(Restaurant_INFO[restaurantNum], Unicode_RestaurantIcon);
                    }
                }
                else{
                    if(restaurantNum % 5 == 0){
                        g->draw_text(Restaurant_INFO[restaurantNum], Unicode_RestaurantIcon);
                    }
                }
                
            }

            //reset font after drawing food icons
            g->format_font(current_font, ezgl::font_slant::normal, ezgl::font_weight::normal, 10);
        }
    }

    /********************************* End of Food *************************************/

    /********************************* Mouse Clicked *************************************/
    if(!(world_coor.height() > startingWorld.height())){ 
        //draw circle on last placed clicked
        //set point colour to red
        if(darkMode){
            g->set_color(161,71,67);
        }
        else{
            g->set_color(232,102,97);
        }
        //dynamic sizing for pinned location based on detailed zoom level
        double radius = detailZoom / 11;
        //draw location mouse clicked
        g->fill_arc(mouseClickLoc, radius, 0, 360);
    }
    /********************************* End Mouse Clicked *************************************/

    /********************************* Drawing Text *************************************/
    //write all feature text if buildings visible
    if(featuresEnabled){
        if(zoom_level > 3){
            for(size_t Feature_idx = 0; Feature_idx < Feature_INFO.size(); Feature_idx++){ 
                if(Feature_INFO[Feature_idx].Feature_name != "<noname>"){
                    //set colour based on light/dark mode
                    if(darkMode){
                        g->set_color(ezgl::WHITE);
                    }
                    else{
                        g->set_color(ezgl::BLACK);
                    }
                    //set text rotation, draw text
                    g->set_text_rotation(0);
                    double feature_text =  250;

                    //if not building, write text
                    if(Feature_INFO[Feature_idx].Feature_type != BUILDING){
                        g->draw_text(Feature_INFO[Feature_idx].Feature_points_xy[0], Feature_INFO[Feature_idx].Feature_name,feature_text,feature_text);
                    }
                    //if building, write text based on zoom level
                    else{
                        //zoomed in all the way, write all text  
                         if(detailZoom < 60){
                            g->draw_text(Feature_INFO[Feature_idx].Feature_points_xy[0], Feature_INFO[Feature_idx].Feature_name,feature_text,feature_text);
                        }
                        //medium zoom in, write every other name
                        else if(detailZoom < 100){
                            if (Feature_idx % 2 == 0){
                                g->draw_text(Feature_INFO[Feature_idx].Feature_points_xy[0], Feature_INFO[Feature_idx].Feature_name,feature_text,feature_text);
                            }
                        }
                        //not zoomed in, every 3rd name
                        else if(Feature_idx % 3 == 0){
                            g->draw_text(Feature_INFO[Feature_idx].Feature_points_xy[0], Feature_INFO[Feature_idx].Feature_name,feature_text,feature_text);
                        }

                    } 
                }
            }
        }
        else if(zoom_level > 2){
            //loop through all features except buildings
            for(size_t Feature_idx = 0; Feature_idx < Feature_INFO.size(); Feature_idx++){ 
                if((Feature_INFO[Feature_idx].Feature_name != "<noname>") && Feature_INFO[Feature_idx].Feature_type != BUILDING){
                    //set colour based on light/dark mode
                    if(darkMode){
                        g->set_color(ezgl::WHITE);
                    }
                    else{
                        g->set_color(ezgl::BLACK);
                    }
                    //set text rotation, draw text
                    g->set_text_rotation(0);
                    double feature_text =  250;
                    g->draw_text(Feature_INFO[Feature_idx].Feature_points_xy[0], Feature_INFO[Feature_idx].Feature_name,feature_text,feature_text);
                }
            }
        }
        
    }
    //write text for POIs if zoomed maps are 
    if(POIsEnabled && detailZoom < 200){ 
        for(size_t POI_idx = 0; POI_idx < POI_INFO.size(); POI_idx++){
            //set colour based on mode
            if(darkMode){
                g->set_color(ezgl::WHITE);
            }
            else{
                g->set_color(ezgl::BLACK);
            }
            //POI calculating when to print POI names
            double len_of_str = POI_INFO.at(POI_idx).POI_name.length();
            double arbitrary_limit = 100;
            double arbitrary_multiplyer = 3;
            double print_when = arbitrary_limit - len_of_str*arbitrary_multiplyer;
            if(print_when < 50){
                print_when = 50;
            }
            //zoomed in all the way, print all POI names
            if(detailZoom < 60){
                g->draw_text(POI_INFO[POI_idx].xandy_coor_POI, POI_INFO.at(POI_idx).POI_name,print_when,print_when);
            }
            //medium zoom in, draw every other POI
            else if(detailZoom < 100){
                if (POI_idx % 2 == 0 || POI_INFO[POI_idx].highlighted){
                    g->draw_text(POI_INFO[POI_idx].xandy_coor_POI, POI_INFO.at(POI_idx).POI_name,print_when,print_when);
                }
            }
            //not zoomed in, every 3rd POI
            else if(POI_idx % 3 == 0 || POI_INFO[POI_idx].highlighted){
                g->draw_text(POI_INFO[POI_idx].xandy_coor_POI, POI_INFO.at(POI_idx).POI_name,print_when,print_when);
            }
        }
    }

    //drawing text for all streets
    if(streetsEnabled){
        //Loop through all street segments
        for(size_t Street_seg_idx = 0; Street_seg_idx < Street_segments_INFO.size(); Street_seg_idx++){
            // if it's highlighted or a highway, draw always...
            if((Street_segments_INFO.at(Street_seg_idx).speed_limit > 14) || std::find(highlightedStreets.begin(), highlightedStreets.end(), getStreetSegmentInfo(Street_seg_idx).streetID) != highlightedStreets.end()){
                //if it's a straight street...
                if(Street_segments_INFO.at(Street_seg_idx).num_curve_points == 0){
                    if(darkMode){
                        g->set_color(ezgl::WHITE);
                    }
                    else{
                        g->set_color(ezgl::BLACK);
                    }
                    // draws street names if the text size is appropriate based on zoom levels
                    double Street_seg_len = findStreetSegmentLength(Street_seg_idx);
                    double str_len_modifier = Street_segments_INFO[Street_seg_idx].Street_seg_name.length() * 5;
                    g->set_text_rotation((Street_segments_INFO[Street_seg_idx].angle));
                    g->draw_text(Street_segments_INFO[Street_seg_idx].draw_text, Street_segments_INFO[Street_seg_idx].Street_seg_name,Street_seg_len - str_len_modifier,Street_seg_len - str_len_modifier);
                }
                //if there is a curved street...
                else{
                    // draws street names if the text size is appropriate based on zoom levels
                    g->set_text_rotation(round(Street_segments_INFO[Street_seg_idx].angle));
                    double Street_seg_len = findStreetSegmentLength(Street_seg_idx);
                    double str_len_modifier = Street_segments_INFO[Street_seg_idx].Street_seg_name.length() * 5;
                    if(darkMode){
                        g->set_color(ezgl::WHITE);
                    }
                    else{
                        g->set_color(ezgl::BLACK);
                    }
                    g->draw_text(Street_segments_INFO[Street_seg_idx].draw_text, Street_segments_INFO[Street_seg_idx].Street_seg_name,Street_seg_len - str_len_modifier,Street_seg_len - str_len_modifier);
                }
            }
            //if we are at a certain zoom level, draw all of the streets
            else if(zoom_level > 0){
                //if the street is straight...
                if(Street_segments_INFO.at(Street_seg_idx).num_curve_points == 0){
                    // draws street names if the text size is appropriate based on zoom levels
                    g->set_text_rotation((Street_segments_INFO[Street_seg_idx].angle));                        
                    if(darkMode){
                        g->set_color(ezgl::WHITE);
                    }
                    else{
                        g->set_color(ezgl::BLACK);
                    }
                    double Street_seg_len = findStreetSegmentLength(Street_seg_idx);
                    double str_len_modifier = Street_segments_INFO[Street_seg_idx].Street_seg_name.length() * 5;
                    g->draw_text(Street_segments_INFO[Street_seg_idx].draw_text, Street_segments_INFO[Street_seg_idx].Street_seg_name,Street_seg_len - str_len_modifier,Street_seg_len - str_len_modifier);
                    
                }
                // if the street is curved...
                else{
                    if(darkMode){
                        g->set_color(ezgl::WHITE);
                    }
                    else{
                        g->set_color(ezgl::BLACK);
                    }
                    // draws street names if the text size is appropriate based on zoom levels
                    g->set_text_rotation((Street_segments_INFO[Street_seg_idx].angle));
                    double Street_seg_len = findStreetSegmentLength(Street_seg_idx);
                    double str_len_modifier = Street_segments_INFO[Street_seg_idx].Street_seg_name.length() * 5;
                    g->draw_text(Street_segments_INFO[Street_seg_idx].draw_text, Street_segments_INFO[Street_seg_idx].Street_seg_name,Street_seg_len - str_len_modifier,Street_seg_len - str_len_modifier);
                    
                }
            }    
        }
        /********************************* End of Drawing Text *************************************/
    }
   

    /********************************* ZOOM Scale *************************************/
    //zoom scaling
    double width = 100 * world_coor.width()/visible_screen.width();
    double width_offset = 20 * world_coor.width()/visible_screen.width();
    double height = 50 * world_coor.width()/visible_screen.width();
    double height_offset = 20 * world_coor.width()/visible_screen.width();
    double scale_offset_x = 20 * world_coor.width()/visible_screen.width();
    double scale_offset_y = 10 * world_coor.width()/visible_screen.width();
    double side_bar_height = 2 * world_coor.width()/visible_screen.width();
    double text_y_offset = 5 * world_coor.width()/visible_screen.width();
    
    //Sets up the canvas
    g->set_line_width(2);
    g->set_text_rotation(0);
    g->set_color(ezgl::WHITE);
    //Draws the white background
    g->fill_rectangle({world_coor.right() - width - width_offset, world_coor.bottom() + height - height_offset}, width, height);
    g->set_color(ezgl::BLACK);
    //draws the black border
    g->draw_rectangle({world_coor.right() - width - width_offset, world_coor.bottom() + height - height_offset}, width, height);
    //draws the actual distance scale
    g->draw_line({world_coor.right() - width - width_offset + scale_offset_x, world_coor.bottom() + height - height_offset+scale_offset_y}, {world_coor.right() - width_offset - scale_offset_x, world_coor.bottom() - height_offset + height + scale_offset_y});
    g->draw_line({world_coor.right() - width - width_offset + scale_offset_x, world_coor.bottom() + height - height_offset+scale_offset_y}, {world_coor.right() - width - width_offset + scale_offset_x, world_coor.bottom() - height_offset + height + scale_offset_y + side_bar_height});
    g->draw_line({world_coor.right() - width_offset - scale_offset_x , world_coor.bottom() + height - height_offset+scale_offset_y}, {world_coor.right() - width_offset - scale_offset_x, world_coor.bottom() - height_offset + height + scale_offset_y + side_bar_height});
    //draws the numbers on the distance scale
    g->draw_text({world_coor.right() - width - width_offset + scale_offset_x, world_coor.bottom() - height_offset + height + scale_offset_y + side_bar_height + text_y_offset}, "0");
    g->draw_text({world_coor.right() - width_offset - scale_offset_x, world_coor.bottom() - height_offset + height + scale_offset_y + side_bar_height + text_y_offset}, std::__cxx11::to_string((int)std::round(width)));
    /********************************* End Zoom Scale *************************************/
}

/******************************************Search Bar*********************************************/
// Function to separate the user's input to see if they are looking for a street or an intersection or a path between 2 intersections
void seperateUserInputOfStreets(std::string userText, std::string *firstStreet, std::string *secondStreet){
    // makes the user's input into lower case
    std::transform(userText.begin(), userText.end(), userText.begin(), ::tolower);
    int street_location;
   
    //Finds the location of 'and'
    street_location = userText.find(" and ");
    // If it doesn't exist...
    if (street_location == 0 || street_location >= userText.length()){
        //Finds the location of '+'
        street_location = userText.find(" + ");
        // If it doesn't exist..
        if (street_location == 0 || street_location >= userText.length()){
            //Finds the location of '&'
            street_location = userText.find(" & ");
            // If it doesn't exist..
            if (street_location == 0 || street_location >= userText.length()){
                // Looking for a street, so returns street name in the firstStreet.
                *firstStreet = userText;
                *secondStreet = "*";
                return;
            } else {
                // Stores the first and second street in separate strings
                *firstStreet = userText.substr(0, street_location);
                *secondStreet = userText.substr(street_location + 3);
                return;
            }
        } else {
            // Stores the first and second street in separate strings
            *firstStreet = userText.substr(0, street_location);
            *secondStreet = userText.substr(street_location + 3);
            return;
        }
    } else {
        // Stores the first and second street in separate strings
        *firstStreet = userText.substr(0, street_location);
        *secondStreet = userText.substr(street_location + 5);
        return;
    }
}

// Function to find the street ids of the street
std::vector<StreetIdx> checkPartialStreetName(std::string firstStreet){
    std::vector<StreetIdx> streetIds = findStreetIdsFromPartialStreetName(firstStreet);
    return streetIds;
}

// Function to find the intersection ids of the streets
std::vector<IntersectionIdx> checkIntersections(std::string firstStreet, std::string secondStreet){
    // Vectors to hold the intersections
    std::vector<IntersectionIdx> intersectionIds;
    std::vector<IntersectionIdx> tempIntersectionIds;
    // Clears vectors of any intersection Ids
    tempIntersectionIds.clear();
    intersectionIds.clear();
    // Finds the street ids of each street
    std::vector<StreetIdx> temp1 = findStreetIdsFromPartialStreetName(firstStreet);
    // If the street is empty...
    if(temp1.size() == 0){
        return intersectionIds;
    }
    std::vector<StreetIdx> temp2 = findStreetIdsFromPartialStreetName(secondStreet);
    // If the street is empty...
    if(temp2.size() == 0){
        return intersectionIds;
    }

    // Loops through all of the street Ids and finds all of the intersection ids and puts it in a vector
    for(auto firstStart = 0; firstStart < temp1.size(); firstStart++){
        for (auto secondStart = 0; secondStart < temp2.size(); secondStart++){
            tempIntersectionIds = findIntersectionsOfTwoStreets(temp1.at(firstStart), temp2.at(secondStart));
            intersectionIds.insert(intersectionIds.end(), tempIntersectionIds.begin(), tempIntersectionIds.end());
        }
    }
    return intersectionIds;
}

// Function to move the screen to a specific street
void moveScreenStreet(ezgl::application* application){
    // gets the first coordinate of the intersection of the first street.
    std::vector<IntersectionIdx> intersectionIds = findIntersectionsOfStreet(centerStreet.front());
    LatLon id_coor = getIntersectionPosition(intersectionIds.front());
    // gets the x and y coordinates
    double x_coor = x_from_lon(id_coor.longitude());
    double y_coor = y_from_lat(id_coor.latitude());
    // create length and width size of the screen.
    double length = 300;
    double width = 250;
    
    // gets the world coordinates and sets the screen
    ezgl::point2d bottomLeft(x_coor - length/2, y_coor - width/2);
    ezgl::point2d topRight(x_coor + length/2, y_coor + width/2);
    ezgl::rectangle screen(bottomLeft, topRight);
    application->get_renderer()->set_visible_world(screen);
    application->refresh_drawing();
}

// Function to move the screen to a specific intersection
void moveScreenIntersection(ezgl::application* application){
    // gets the first coordinate of the intersection
    LatLon id_coor = getIntersectionPosition(centerIntersection.front());
    // gets the x and y coordinates
    double x_coor = x_from_lon(id_coor.longitude());
    double y_coor = y_from_lat(id_coor.latitude());
    // create length and width size of the screen.
    double length = 300;
    double width = 250;
    
    // gets the world coordinates and sets the screen
    ezgl::point2d bottomLeft(x_coor - length/2, y_coor - width/2);
    ezgl::point2d topRight(x_coor + length/2, y_coor + width/2);
    ezgl::rectangle screen(bottomLeft, topRight);
    application->get_renderer()->set_visible_world(screen);
    application->refresh_drawing();
}

// Callback Function for the search bar
void userSearchInput(GtkEntry * /*entry*/, ezgl::application* application){
    GtkEntry* searchInput = GTK_ENTRY(application->get_object("searchBar"));
    std::string userText = gtk_entry_get_text(searchInput);
    std::string firstIntersection = gtk_entry_get_text(searchInput);
    std::string errorMessage;
    
    // Separates the user's input
    std::string firstStreet, secondStreet, thirdStreet, fourthStreet;
    int intersection_location;
    
    // Checks if the user is looking for a path between 2 intersections.
    intersection_location = userText.find("|");
    // If the user doesn't input 2 intersections...
    if (intersection_location == 0 || intersection_location >= userText.length()){
        seperateUserInputOfStreets(userText, &firstStreet, &secondStreet);
        thirdStreet = "*";
        fourthStreet = "*";
    //if the user inputs 2 intersections...
    } else {
        //Erases the second intersection from the user's input
        firstIntersection.erase(intersection_location, firstIntersection.length());
        //Checks if there is a space before the |
        //if there is a space before the |...
        if (firstIntersection.length() - 1 == ' '){
            //erases the space
            firstIntersection.erase(intersection_location, intersection_location + 1);
            //separates the street names
            seperateUserInputOfStreets(firstIntersection, &firstStreet, &secondStreet);            
        } else {
            seperateUserInputOfStreets(firstIntersection, &firstStreet, &secondStreet);
        }
        
        // checks if there is a space after the |
        if (userText.at(intersection_location + 1) != ' '){
            //erases the first intersection
            userText.erase(0, intersection_location + 1);
            //separates the street names
            seperateUserInputOfStreets(userText, &thirdStreet, &fourthStreet);
        } else {
            //erases the first intersection and the added space
            userText.erase(0, intersection_location + 1);
            //separates the street names
            seperateUserInputOfStreets(userText, &thirdStreet, &fourthStreet);
        }
    }
    
    // If the user inputs only one street...
    if(secondStreet == "*" && thirdStreet == "*" && fourthStreet == "*"){
        // gets the street ids of that street
        centerStreet = checkPartialStreetName(firstStreet);
        //check to see if vector is empty or not
        if(centerStreet.size() > 0){
            // highlights the street
            highlightedStreets = centerStreet;
            moveScreenStreet(application);
        } else {
            // Pushes invalid street message
            errorMessage = "Street Doesn't Exist. Try Again.";
            userSearchError(errorMessage);
        }
    } else if (thirdStreet == "*" && fourthStreet == "*" && secondStreet != "*"){
        // gets the intersection ids of the streets
        centerIntersection = checkIntersections(firstStreet, secondStreet);
        //check to see if vector is empty or not
        if(centerIntersection.size() > 0){
            //clear previously highlighted intersections
            for(int intersectionNum = 0; intersectionNum < prevHighlightedIntersections.size(); intersectionNum++){
                intersections[prevHighlightedIntersections[intersectionNum]].highlighted = false;
            }

            //highlight intersections
            for(int intersectionNum = 0; intersectionNum < centerIntersection.size(); intersectionNum++){
                prevHighlightedIntersections.push_back(centerIntersection[intersectionNum]);
                intersections[centerIntersection[intersectionNum]].highlighted = true;
            }
            moveScreenIntersection(application);
        } else {
            //pushes invalid intersection message
            errorMessage = "Intersection Doesn't Exist. Try Again.";
            userSearchError(errorMessage);
        }
    } else {
        //gets the intersection ids of both intersections inputted
        centerIntersection = checkIntersections(firstStreet, secondStreet);
        centerIntersection2 = checkIntersections(thirdStreet, fourthStreet);
        //if there are intersection ids for both intersections...
        if(centerIntersection.size() > 0 && centerIntersection2.size() > 0){
            //finds path and highlights here
            directionDecisions(std::make_pair(centerIntersection.front(), centerIntersection2.front()));
            //moves the camera to follow to the path on the map
            moveScreenIntersection(application);
            //If both intersections don't exist...
        } else if (centerIntersection.size() == 0 && centerIntersection2.size() == 0){
            errorMessage = "Both Intersections Entered Do Not Exist. Try Again.";
            userSearchError(errorMessage);
            //If the first intersection doesn't exist...
        } else if (centerIntersection.size() == 0){
            errorMessage = "First Intersection Entered Doesn't Exist. Try Again.";
            userSearchError(errorMessage);
        }else if (centerIntersection2.size() == 0){
            //If the second intersection doesn't exist...
            errorMessage = "Second Intersection Entered Doesn't Exist. Try Again.";
            userSearchError(errorMessage);
        }
    }
}

//Function to print out error messages
void userSearchError(std::string errorMessage){
    //Creates a dialog box
    GtkWidget *dialog = gtk_dialog_new();
    GtkWidget *error;
    GtkWidget *container;
    gboolean response;
    
    //sets up the window of the dialog box
    gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    gtk_widget_set_size_request(dialog, 100, 50);
    
    //Create button on dialog box
    gtk_dialog_add_button(GTK_DIALOG(dialog), "Close", 1);
    
    //stores error message into widget
    error = gtk_label_new(errorMessage.c_str());
    container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(container), error);
    
    //shows the error message to the user
    gtk_widget_show_all(dialog);
    
    //checks if the user presses on the close button
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    //if the user presses
    if (response){
        //closes the dialog box
        gtk_widget_destroy(dialog);
    }
}

//Function for auto-completion
void userTypingCompletion(GtkEntry *widget, ezgl::application* application){
    GtkListStore* list = GTK_LIST_STORE(application->get_object("dropDownSearch"));
    GtkTreeIter iter;
    std::vector<StreetIdx> streetInput;
    std::vector<IntersectionIdx> intersectionInput1;
    std::string first, second;
    
    //if the string is empty
    std::string input(gtk_entry_get_text(widget));
    if(input.length() == 0){
        return;
    }

    //clears the list
    gtk_list_store_clear(list);
    
    for (int interIds = 0; interIds < getNumIntersections(); interIds++){
        gtk_list_store_append(list, &iter);
        std::string interName = getIntersectionName(interIds);
        gtk_list_store_set(list, &iter, 0, interName.c_str(), -1);
    }
}
/**************************************End Search Bar************************************/

/**************************************Change Maps***************************************/
// Callback function to change between maps
void mapChangeInput(GtkEntry */*entry*/, ezgl::application* application){
    GtkComboBox* selection = GTK_COMBO_BOX(application->get_object("mapList"));
    Map_location = gtk_combo_box_get_active(selection);
    
    std::string map_path;
    
    if (Map_location == 0){
        map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
    } else if (Map_location == 1){
        map_path = "/cad2/ece297s/public/maps/beijing_china.streets.bin";
    } else if (Map_location == 2){
        map_path = "/cad2/ece297s/public/maps/cairo_egypt.streets.bin";
    } else if (Map_location == 3){
        map_path = "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin";
    } else if (Map_location == 4){
        map_path = "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin";
    } else if (Map_location == 5){
        map_path = "/cad2/ece297s/public/maps/hamilton_canada.streets.bin";
    } else if (Map_location == 6){
        map_path = "/cad2/ece297s/public/maps/hong-kong_china.streets.bin";
    } else if (Map_location == 7){
        map_path = "/cad2/ece297s/public/maps/iceland.streets.bin";
    } else if (Map_location == 8){
        map_path = "/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin";
    } else if (Map_location == 9){
        map_path = "/cad2/ece297s/public/maps/kyiv_ukraine.streets.bin";
    } else if (Map_location == 10){
        map_path = "/cad2/ece297s/public/maps/london_england.streets.bin";
    } else if (Map_location == 11){
        map_path = "/cad2/ece297s/public/maps/new-delhi_india.streets.bin";
    } else if (Map_location == 12){
        map_path = "/cad2/ece297s/public/maps/new-york_usa.streets.bin";
    } else if (Map_location == 13){
        map_path = "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin";
    } else if (Map_location == 14){
        map_path = "/cad2/ece297s/public/maps/saint-helena.streets.bin";
    } else if (Map_location == 15){
        map_path = "/cad2/ece297s/public/maps/singapore.streets.bin";
    } else if (Map_location == 16){
        map_path = "/cad2/ece297s/public/maps/sydney_australia.streets.bin";
    } else if (Map_location == 17){
        map_path = "/cad2/ece297s/public/maps/tehran_iran.streets.bin";
    } else if (Map_location == 18){
        map_path = "/cad2/ece297s/public/maps/tokyo_japan.streets.bin";
    }
    
    //Upon Map closing, reset the drawing of streets and the last point clicked to prevent seg faults
    *searched_streets = 0;
    Last_intersection_clicked.first = 0;
    Last_intersection_clicked.second = 1;

    // Closes previous map
    closeMap();
    // Loads new map
    loadMap(map_path);
    // Create a new world based on new x and y coordinates of the new map
    ezgl::rectangle new_world = ezgl::rectangle({x_from_lon(min_lon), y_from_lat(min_lat)},
                                                {x_from_lon(max_lon), y_from_lat(max_lat)});
    application->change_canvas_world_coordinates("MainCanvas", new_world);
    
    startingWorld = new_world;

    // Draws the new map
    application->refresh_drawing();
}
/************************************* End Change Maps******************************************/

/************************************* Help Page ******************************************/
//Help page for colour coding
void legendHelp(GtkButton * /*entry*/, ezgl::application* application){
    //gtk connection to the help window
    GtkWidget* help = GTK_WIDGET(application->get_object("helpPage"));
    helpInfo = GTK_LABEL(application->get_object("helpPageInformation"));
    //clears the text
    gtk_label_set_text(helpInfo, NULL);
    //sets the text and stores it into the label
    std::string legendInfo = "Points of Interest - List of important locations within the map that you are looking at. These include famous places, \nrestaurants and more, and are shown through orange "
            "dots on the map.     Orange (LM) and Brown (DM)\n\n"
            "Highlighting Points of Interest - highlighting the point of interests will turn the orange dot yellow    Yellow(LM & DM)\n\n"
            "Intersections - At every location where two streets cross each other, a red dot is placed    Salmon Pink (LM & DM)\n"
            "Streets - Each street is highlighted on the map, with most being streets that road vehicles pass through, along with \n"
            "alleys. Major streets on slightly thicker than minor streets and are also coloured differently.\n"
            "Major Streets  Bisque Beige (LM) and Manatee Grey (DM)     Minor Streets   White (LM) and Taupe Gray (DM)\n\n"
            "Highlighting Intersections/Streets - highlighting streets/intersections will turn the respective \n"
            "colour of the street/intersection teal     Teal(LM & DM)\n"
            "\nGreen Spaces - This includes parks and more except for golf courses   Pastel Green (LM) and Pakistan Green (DM)\n\n"
            "Golf Courses - These green spaces are highlighted in a different shade of green due to the\n large area that golf courses need   UFO Green (LM) and Lincoln Green (DM)\n\n"
            "Bodies of Water - This includes lakes, oceans, streams and rivers.     Maya Blue (LM) and Saint Patrick Blue (DM)\n\n"
            "Glaciers - Large accumulation of ice, snow and rocks that can be seen on the map   Light Cyan (LM) and Skobeloff Blue (DM)\n\n"
            "Beaches - Areas of sand that can be found on the map   Beige(LM) and Khaki (DM)\n\n"
            "Islands - Small areas of land surrounded by water      Sandy Brown (LM) and Pale Brown (DM)\n\n"
            "Buildings - Blocks of buildings, which can include shops, restaurants and residential locations    Grey (LM) and Light Grey (DM)\n\n"
            "Last Area Clicked - Clicking on the map creates a red dot on where you clicked. This feature is further introduced in 'Last Clicked Area'\n"
            "described in Find Path.   Terra Cotta (LM) and Smokey Topaz (DM)\n\n"
            "Highways - A major street/avenue that is intended for public use for vehicles      Yellow (LM) and Dark Yellow (DM)";
    gtk_label_set_text(helpInfo, legendInfo.c_str());
    //resizes the window
    gtk_window_resize(GTK_WINDOW(help), 100, 150);
}
//Help page for searching
void searchHelp(GtkButton * /*entry*/, ezgl::application* application){
    //gtk connection to the help window
    GtkWidget* help = GTK_WIDGET(application->get_object("helpPage"));
    helpInfo = GTK_LABEL(application->get_object("helpPageInformation"));
    //clears the text
    gtk_label_set_text(helpInfo, NULL);
    //sets the text and stores it into the label
    std::string searchInfo = "The search bar is able to search for streets and intersections. The user is able to search for a street,\n"
            "even with a partial street name. Using the user's input, all streets that have the partial street name will be highlighted\n"
            "teal.\n\nTo search for an intersection, the user will be able to input two streets by putting 'and, & or +' between two streets.\n"
            "For example, by typing 'Bay and Yonge' on the map of Toronto, the intersection(s) will be highlighted in teal. \n\nTo search for a path"
            "between two intersections, put a '|' in between. For example, by typing 'Bay and College | Yonge and Dundas' on the map of\nToronto, the"
            " quickest path between the two\n\nThe search bar also includes autocomplete. The user can use the arrow keys to navigate through the\n"
            "streets or intersections and when they find the street or intersection that they are looking for, the user can press enter and it will\n"
            "autocomplete onto the search bar.";
    gtk_label_set_text(helpInfo, searchInfo.c_str());
    //resizes the window
    gtk_window_resize(GTK_WINDOW(help), 100, 150);
}
//Help page for toggling
void toggleHelp(GtkButton * /*entry*/, ezgl::application* application){
    //gtk connection to the help window
    GtkWidget* help = GTK_WIDGET(application->get_object("helpPage"));
    helpInfo = GTK_LABEL(application->get_object("helpPageInformation"));
    //clears the text
    gtk_label_set_text(helpInfo, NULL);
    //sets the text and stores it into the label
    std::string toggleInfo = "On the right-hand side of the screen, there are multiple buttons that allows the users to toggle different \n"
                             "features to remove or show features on the map. These include toggling of dark mode, streets and intersections,\n"
                             "features, points of interests, libraries and restaurants.\n\n";
    gtk_label_set_text(helpInfo, toggleInfo.c_str());
     //resizes the window
    gtk_window_resize(GTK_WINDOW(help), 100, 150);
}
//Help page for distance scale
void distanceHelp(GtkButton * /*entry*/, ezgl::application* application){
    //gtk connection to the help window
    GtkWidget* help = GTK_WIDGET(application->get_object("helpPage"));
    helpInfo = GTK_LABEL(application->get_object("helpPageInformation"));
    //clears the text
    gtk_label_set_text(helpInfo, NULL);
    //sets the text and stores it into the label
    std::string distanceInfo = "The distance scale can be found on the bottom right of the screen. The scale is based on the zoom level of\n"
                                "your map, and is always measured in meters.";
    gtk_label_set_text(helpInfo, distanceInfo.c_str());
    //resizes the window
    gtk_window_resize(GTK_WINDOW(help), 100, 150);
}
//Help page for loading maps
void mapHelp(GtkButton * /*entry*/, ezgl::application* application){
    //gtk connection to the help window
    GtkWidget* help = GTK_WIDGET(application->get_object("helpPage"));
    helpInfo = GTK_LABEL(application->get_object("helpPageInformation"));
    //clears the text
    gtk_label_set_text(helpInfo, NULL);
    //sets the text and stores it into the label
    std::string mapInfo = "On the top right of the user's screen, underneath the search bar, there is an option to load different maps. By \n"
                          "clicking on the drop-down box, the user is able to click on a different city/country and look at the new map that\n"
                          "they would like to explore.";
    gtk_label_set_text(helpInfo, mapInfo.c_str());
    //resizes the window
    gtk_window_resize(GTK_WINDOW(help), 100, 150);
}
//Help page for finding paths
void pathHelp(GtkButton * /*entry*/, ezgl::application* application){
    //gtk connection to the help window
    GtkWidget* help = GTK_WIDGET(application->get_object("helpPage"));
    helpInfo = GTK_LABEL(application->get_object("helpPageInformation"));
    //clears the text
    gtk_label_set_text(helpInfo, NULL);
    //sets the text and stores it into the label
    std::string pathInfo = "On the right-hand side of the screen, there is a button called 'Clicked Paths'. When clicking on two intersections\n"
                           "and then clicking on this button, a pop-up page will appear asking the user if they would like to get directions\n"
                           "between these two paths.\n\n"
                           "If the user wants to swap the start and end point of the chosen path, clicking on a button on the right-hand side\n"
                           "of the screen called 'Switch Paths' will do this task and give the user the new directions.\n\n"
                           "To find a path using a search bar, the user can type in two intersections separated by '|'. For example, by searching\n"
                           "'Bay Street & College Street | Dundas Street West & Jarvis Street', the directions will be given to you along with the\n"
                           "highlighted path.";
    gtk_label_set_text(helpInfo, pathInfo.c_str());
    //resizes the window
    gtk_window_resize(GTK_WINDOW(help), 100, 150);
}
//Help page for last area clicked
void clickHelp(GtkButton * /*entry*/, ezgl::application* application){
    //gtk connection to the help window
    GtkWidget* help = GTK_WIDGET(application->get_object("helpPage"));
    helpInfo = GTK_LABEL(application->get_object("helpPageInformation"));
    //clears the text
    gtk_label_set_text(helpInfo, NULL);
    //sets the text and stores it into the label
    std::string clickInfo = "When clicking on the map, a salmon pink dot will be left on the place you last clicked. When clicking, information on the\n"
                            "clicked area will show. This includes the closest intersection, coordinates on the world map along with the closest point of\n"
                            "interest.\n\n"
                            "To find the path between two intersections, more information can be found in 'Find Path'";
    gtk_label_set_text(helpInfo, clickInfo.c_str());
    //resizes the window
    gtk_window_resize(GTK_WINDOW(help), 100, 150);
}
//Function to load the pop-up window of the help page
void helpPage(GtkButton * /*entry*/, ezgl::application* application){
    //connects the window to glade
    GtkWidget* help = GTK_WIDGET(application->get_object("helpPage"));
    gtk_window_set_title(GTK_WINDOW(help), "Help Page");
    //sets the initial size of the page
    gtk_widget_set_size_request(help, 150, 100);
    gtk_widget_show_all(help);
    
    //gtk connection to the label/content of the help page
    helpInfo = GTK_LABEL(application->get_object("helpPageInformation"));
    //Initial welcome page of the help page
    std::string introduction = "Welcome to Maps Around the World!\nClick on any of the buttons above to learn more about this map!";
    //sets the text
    gtk_label_set_text(helpInfo, introduction.c_str());
}

/************************************* End Help Page ******************************************/

/************************************* Clicked Intersections ******************************************/
//Function to shows the highlighted path
void clickedPaths(GtkButton * /*button*/, ezgl::application* application){
    //Goes to function to see if the user would like to see the directions to get from one intersection to the second intersection
    directionDecisions(Last_intersection_clicked);
    application->refresh_drawing();
}

//Function to switch the to and from
void switchPathIntersections(GtkButton */*button*/, ezgl::application* application){
    IntersectionIdx temp;
    //temp holds the first intersection
    temp = Last_intersection_clicked.first;
    //first intersection becomes the second intersection
    Last_intersection_clicked.first = Last_intersection_clicked.second;
    //second intersection gets the intersection that temp holds
    Last_intersection_clicked.second = temp; 
    //Goes to function to see if the user would like to see the directions to get from one intersection to the second intersection
    directionDecisions(Last_intersection_clicked);
    application->refresh_drawing();
}
/************************************* End Clicked Intersections ******************************************/

/************************************* Directions ******************************************/
//Function to create directions and put them in a string for displaying
std::string get_directions(std::vector <StreetSegmentIdx> streetseg_vector, int starting_intersectionIdx, int end_intersectionIdx){
    //initialize
    double dx = 0;
    double dy = 0;
    double dx2 = 0;
    double dy2 = 0;
    
    //Standard strings to describe directions for the user
    std::string Distance_string = " and travel for ";
    std::string Distance_units = " Meters\n";
    std::string basic_turn_instruction = "turn onto ";
    std::string next_line = "\n";
    std::string finalstring = "";
    std::string drive_along = "drive_along";
    std::string Go_straight_intr = "Continue heading straight and go onto ";
    std::string Turn_right_intr = "Make a right on ";
    std::string Turn_left_intr = "Make a left on  ";
    std::string Keep_right_intr = "Stay on the right and prepare to turn on ";
    std::string Keep_left_intr = "Stay on the left and prepare to turn on ";
    std::string Sharp_right_intr = "Prepare to make a sharp right and turn on ";
    std::string Sharp_left_intr = "Prepare to make a sharp left and turn on ";
    std::string Uturn = "Prepare to make a uturn and turn on ";
    std::string arrived_instr = "You have arrived at your destination";

    double street_segment_length;
    int street_segment_length_as_int;
    int same_street_offeset = 0;
    int totalLength = 0;
    std::string current_street;
    std::string next_street;
    std::string tempString;
    std::string len_of_street_str;
    //Finds the location of the first intersection
    ezgl::point2d previous_point = intersections[starting_intersectionIdx].xandy_coor_intersection;
    double final_distance_len = 0;
    int final_distance_len_int;
    int kilo;
    int meters;
    std::string kilo_str, meter_str;
    
    double path_time;
    int path_time_int;
    int minutes, hour, remainingSeconds, remainingTime;
    std::string second_str, minute_str, hour_str;
    
    //resizes the vector based on the new path
    Directions_to_2_from.resize(streetseg_vector.size());
    Directions_NESW.resize(streetseg_vector.size());
    
    //sets up global vector containing the path of street segments
    street_seg_path.clear();
    for(int i = 0; i < streetseg_vector.size(); i++){
        street_seg_path.push_back(streetseg_vector[i]);
    }

    //loops though all the street segments
    for(int number_of_streets = 0; number_of_streets < streetseg_vector.size(); number_of_streets++){
        //checks if the to point is the same as the previous point.
        if(Street_segments_INFO.at(streetseg_vector[number_of_streets]).to == previous_point){
            //if true, this means the direction is to -> from
            Directions_to_2_from.at(number_of_streets) = "to2from";
            previous_point = Street_segments_INFO.at(streetseg_vector[number_of_streets]).from;
            //if there are no curve points...
            if(Street_segments_INFO.at(streetseg_vector[number_of_streets]).num_curve_points == 0){
                //change in x and y for the streets. (dy and dx) and (dx2 and dy2) are the same here since there are no curves. 
                //dx and dy is for the begining of the road for curved streets, and the latter for the end of a curved street
                dx = Street_segments_INFO.at(streetseg_vector[number_of_streets]).from.x - Street_segments_INFO.at(streetseg_vector[number_of_streets]).to.x;
                dy = Street_segments_INFO.at(streetseg_vector[number_of_streets]).from.y - Street_segments_INFO.at(streetseg_vector[number_of_streets]).to.y;
                dx2 = Street_segments_INFO.at(streetseg_vector[number_of_streets]).from.x - Street_segments_INFO.at(streetseg_vector[number_of_streets]).to.x;
                dy2 = Street_segments_INFO.at(streetseg_vector[number_of_streets]).from.y - Street_segments_INFO.at(streetseg_vector[number_of_streets]).to.y;
            }
            //if it is a curved street
            else{
                //calculate dx and dy for the last curved point and to
                dx = Street_segments_INFO.at(streetseg_vector[number_of_streets]).Street_seg_points_xy.back().x - Street_segments_INFO.at(streetseg_vector[number_of_streets]).to.x;
                dy = Street_segments_INFO.at(streetseg_vector[number_of_streets]).Street_seg_points_xy.back().y - Street_segments_INFO.at(streetseg_vector[number_of_streets]).to.y;
                //calculate dx2 and dy2 for the first curved point and from
                dx2 = Street_segments_INFO.at(streetseg_vector[number_of_streets]).from.x - Street_segments_INFO.at(streetseg_vector[number_of_streets]).Street_seg_points_xy.front().x;
                dy2 = Street_segments_INFO.at(streetseg_vector[number_of_streets]).from.y - Street_segments_INFO.at(streetseg_vector[number_of_streets]).Street_seg_points_xy.front().y;
            }
        }
        //checks if he from point is the same as the to point
        else if(Street_segments_INFO.at(streetseg_vector[number_of_streets]).from == previous_point){
            //if true, this means the direction is from -> to
            Directions_to_2_from.at(number_of_streets) = "from2to";
            //update previous point
            previous_point = Street_segments_INFO.at(streetseg_vector[number_of_streets]).to;
            //if straight road
            if(Street_segments_INFO.at(streetseg_vector[number_of_streets]).num_curve_points == 0){
                //change in x and y for the streets. (dy and dx) and (dx2 and dy2) are the same here since there are no curves. 
                //dx and dy is for the begining of the road for curved streets, and the latter for the end of a curved street
                dx = Street_segments_INFO.at(streetseg_vector[number_of_streets]).to.x - Street_segments_INFO.at(streetseg_vector[number_of_streets]).from.x;
                dy = Street_segments_INFO.at(streetseg_vector[number_of_streets]).to.y - Street_segments_INFO.at(streetseg_vector[number_of_streets]).from.y;
                dx2 = Street_segments_INFO.at(streetseg_vector[number_of_streets]).to.x - Street_segments_INFO.at(streetseg_vector[number_of_streets]).from.x;
                dy2 = Street_segments_INFO.at(streetseg_vector[number_of_streets]).to.y - Street_segments_INFO.at(streetseg_vector[number_of_streets]).from.y;
            }
            //if its a curved street
            else{
                //calculate dx and dy for the last curved point and to
                dx = Street_segments_INFO.at(streetseg_vector[number_of_streets]).Street_seg_points_xy.front().x - Street_segments_INFO.at(streetseg_vector[number_of_streets]).from.x;
                dy = Street_segments_INFO.at(streetseg_vector[number_of_streets]).Street_seg_points_xy.front().y - Street_segments_INFO.at(streetseg_vector[number_of_streets]).from.y;
                //calculate dx2 and dy2 for the first curved point and from
                dx2 = Street_segments_INFO.at(streetseg_vector[number_of_streets]).to.x - Street_segments_INFO.at(streetseg_vector[number_of_streets]).Street_seg_points_xy.back().x;
                dy2 = Street_segments_INFO.at(streetseg_vector[number_of_streets]).to.y - Street_segments_INFO.at(streetseg_vector[number_of_streets]).Street_seg_points_xy.back().y;
            }
        }
        
        //Checks if the slope dy/dx is in the first quadrant. Then determines which cardinal direction its facing toward
        std::string Direction_one;
        if(dx > 0 && dy > 0){
            if(dy/dx > 5){
                Direction_one = "N";
            }
            else if(dy/dx < 0.2){
                Direction_one = "E";
            }
            else{
                Direction_one = "NE";
            }
        }
        //Checks if the slope dy/dx is in the second quadrant. Then determines which cardinal direction its facing toward
        else if(dx < 0 && dy > 0){
            if(dy/dx < -5){
                Direction_one = "N";
            }
            else if(dy/dx > -0.2){
                Direction_one = "W";
            }
            else{
                Direction_one = "NW";
            }
        }
        //Checks if the slope dy/dx is in the third quadrant. Then determines which cardinal direction its facing toward
        else if(dx < 0 && dy < 0){
            if(dy/dx > 5){
                Direction_one = "S";
            }
            else if(dy/dx < 0.2){
                Direction_one = "W";
            }
            else{
                Direction_one = "SW";
            }
        }
        //Checks if the slope dy/dx is in the fourth quadrant. Then determines which cardinal direction its facing toward
        else if(dx > 0 && dy < 0){
            if(dy/dx < -5){
                Direction_one = "S";
                
            }
            else if(dy/dx > -0.2){
                
                Direction_one = "E";
            }
            else{
                Direction_one = "SE";
            }
        }
        //checks if the line is vertical and then assigns cardinal direction
        else if(dx == 0){
            if(dy < 0){
                Direction_one = "S";
            }
            else{
                Direction_one = "N";
            }
        }
        //checks if the line is horizontal. then assigns a cardinal direction
        else if(dy == 0){
            if(dx < 0){
                Direction_one = "W";
            }
            else{
                Direction_one = "E";
            }
        }
        
        std::string Direction_two;
        //Checks if the slope dy2/dx2 is in the first quadrant. Then determines which cardinal direction its facing toward
        if(dx2 > 0 && dy2 > 0){
            if(dy2/dx2 > 5){
                Direction_two = "N";
            }
            else if(dy2/dx2 < 0.2){
                Direction_two = "E";
            }
            else{
                Direction_two = "NE";
            }
        }
        //Checks if the slope dy2/dx2 is in the second quadrant. Then determines which cardinal direction its facing toward
        else if(dx2 < 0 && dy2 > 0){
            if(dy2/dx2 < -5){
                Direction_two = "N";
            }
            else if(dy2/dx2 > -0.2){
                Direction_two = "W";
            }
            else{
                Direction_two = "NW";
            }
        }
        //Checks if the slope dy2/dx2 is in the third quadrant. Then determines which cardinal direction its facing toward
        else if(dx2 < 0 && dy2 < 0){
            if(dy2/dx2 > 5){
                Direction_two = "S";
            }
            else if(dy2/dx2 < 0.2){
                Direction_two = "W";
            }
            else{
                Direction_two = "SW";
            }
        }
        //Checks if the slope dy2/dx2 is in the fourth quadrant. Then determines which cardinal direction its facing toward
        else if(dx2 > 0 && dy2 < 0){
            if(dy2/dx2 < -5){
                Direction_two = "S";
                
            }
            else if(dy2/dx2 > -0.2){
                
                Direction_two = "E";
            }
            else{
                Direction_two = "SE";
            }
        }
        //checks if the line is vertical and then assigns cardinal direction
        else if(dx2 == 0){
            if(dy2 < 0){
                Direction_two = "S";
            }
            else{
                Direction_two = "N";
            }
        }
        //checks if the line is horizontal and then assigns cardinal direction
        else if(dy2 == 0){
            if(dx2 < 0){
                Direction_two = "W";
            }
            else{
                Direction_two = "E";
            }
        }
        //places directions into a pair into the appropriate vector
        Directions_NESW.at(number_of_streets) = std::make_pair(Direction_one,Direction_two);
    }

    
    //adds an offset in order to properly calculate the Left/Right turns
    Directions_NESW.insert(Directions_NESW.begin(), std::make_pair("",""));

    //if the vector is empty...
    if(streetseg_vector.size() == 0){
        return "You are already at your destination";
    }
    //if the vector size is 1, outputs this statement...
    else if(streetseg_vector.size() == 1){
        return "Travel " + Directions_NESW[1].second + " and arrive at your destination in " + std::__cxx11::to_string(rounding_int(findStreetSegmentLength((int)streetseg_vector.front()))) + " Metres";
    }

    //Displays the two intersections that the user has searched/clicked
    finalstring.append("Path between:\n" + getIntersectionName(starting_intersectionIdx)+ "  and  " + getIntersectionName(end_intersectionIdx) + next_line + next_line );
    //Displays the instructions for the first street
    finalstring.append("Travel " + Directions_NESW[1].second + " on ");
    //Loops through the vector...
    for(int number_of_streets = 0; number_of_streets < streetseg_vector.size(); number_of_streets++){
        //if the next street is not equal to the size of the vector...
        if(number_of_streets +1 != streetseg_vector.size()){
            //next street gets the next street name, and current gets the current street name
            next_street = getStreetName(getStreetSegmentInfo(streetseg_vector.at(number_of_streets+1)).streetID);
            current_street = getStreetName(getStreetSegmentInfo(streetseg_vector.at(number_of_streets)).streetID);
            
            //Find the length of the street_segment
            street_segment_length = findStreetSegmentLength(streetseg_vector.at(number_of_streets)); 
            //change from double -> int
            street_segment_length_as_int = street_segment_length;

            //if the current street name is different from the next street name, based on the direction of the next street, outputs a certain string to correlate it.
            if(current_street != next_street){
                if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "KR"){
                    finalstring.append(Keep_right_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "R"){
                    finalstring.append(Turn_right_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "SHR"){
                    finalstring.append(Sharp_right_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "U"){
                    finalstring.append(Uturn);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "SHL"){
                    finalstring.append(Sharp_left_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "L"){
                    finalstring.append(Turn_left_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "KL"){
                    finalstring.append(Keep_left_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "Straight"){
                    finalstring.append(Go_straight_intr);
                }
                
                //adds the length of a street with the same street name...
                len_of_street_str = std::__cxx11::to_string(rounding_int(totalLength+street_segment_length_as_int));
                //adds the information together into a string
                finalstring.append(current_street);
                finalstring.append(Distance_string);
                finalstring.append(len_of_street_str);
                finalstring.append(Distance_units);
                //resets the total length and same street offset
                totalLength = 0;
                same_street_offeset = 0;
            }
            else{
                //if the current and next street are the same
                //add the street segment length to the total length
                totalLength = totalLength + street_segment_length_as_int;
                //offset increases
                same_street_offeset = same_street_offeset +1;
            }
        }
        //if it is the final street segment
        else{
            //if the last street segment has the same name as the previous street segment..
            if(getStreetName(getStreetSegmentInfo(streetseg_vector.at(number_of_streets)).streetID) == getStreetName(getStreetSegmentInfo(streetseg_vector.at(number_of_streets-1)).streetID)){
                //if the current street name is different from the next street name, based on the direction of the next street, outputs a certain string to correlate it.
                if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "KR"){
                    finalstring.append(Keep_right_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "R"){
                    finalstring.append(Turn_right_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "SHR"){
                    finalstring.append(Sharp_right_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "U"){
                    finalstring.append(Uturn);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "SHL"){
                    finalstring.append(Sharp_left_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "L"){
                    finalstring.append(Turn_left_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "KL"){
                    finalstring.append(Keep_left_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets-same_street_offeset].second, Directions_NESW[number_of_streets+1-same_street_offeset].first)] == "Straight"){
                    finalstring.append(Go_straight_intr);
                }
                
                
                //Find the length of the street_segment and rounds to nearest 50
                street_segment_length = findStreetSegmentLength(streetseg_vector.at(number_of_streets)); 
                //change from double -> int
                street_segment_length_as_int = street_segment_length;
                //Finds the total length
                totalLength = totalLength + street_segment_length_as_int;
                //change the int value of the street into a string
                len_of_street_str = std::__cxx11::to_string(rounding_int(totalLength));
                //adds it to the final string of directions
                finalstring.append(getStreetName(getStreetSegmentInfo(streetseg_vector.at(number_of_streets)).streetID));
                finalstring.append(Distance_string);
                finalstring.append(len_of_street_str);
                finalstring.append(Distance_units);
                //resets the total length
                totalLength = 0;
            }
            //if the last street segment has a different name as the previous street segment...
            else{
                //if the current street name is different from the next street name, based on the direction of the next street, outputs a certain string to correlate it.
                if(direction_umap[std::make_pair(Directions_NESW[number_of_streets].second, Directions_NESW[number_of_streets+1].first)] == "KR"){
                    finalstring.append(Keep_right_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets].second, Directions_NESW[number_of_streets+1].first)] == "R"){
                    finalstring.append(Turn_right_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets].second, Directions_NESW[number_of_streets+1].first)] == "SHR"){
                    finalstring.append(Sharp_right_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets].second, Directions_NESW[number_of_streets+1].first)] == "U"){
                    finalstring.append(Uturn);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets].second, Directions_NESW[number_of_streets+1].first)] == "SHL"){
                    finalstring.append(Sharp_left_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets].second, Directions_NESW[number_of_streets+1].first)] == "L"){
                    finalstring.append(Turn_left_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets].second, Directions_NESW[number_of_streets+1].first)] == "KL"){
                    finalstring.append(Keep_left_intr);
                }
                else if(direction_umap[std::make_pair(Directions_NESW[number_of_streets].second, Directions_NESW[number_of_streets+1].first)] == "Straight"){
                    finalstring.append(Go_straight_intr);
                }
                //Find the length of the street_seg and rounds to nearst 50
                street_segment_length = findStreetSegmentLength(streetseg_vector.at(number_of_streets)); 
                //change from double -> int
                street_segment_length_as_int = street_segment_length;
                
                //converts the int value into a string
                len_of_street_str = std::__cxx11::to_string(rounding_int(street_segment_length_as_int));
                //adds it to the final string to be displayed to the user
                finalstring.append(getStreetName(getStreetSegmentInfo(streetseg_vector.at(number_of_streets)).streetID));
                finalstring.append(Distance_string);
                finalstring.append(len_of_street_str);
                finalstring.append(Distance_units);
            }
        }
    }
    //adds the final line of the string to be displayed to the user
    finalstring.append(arrived_instr + next_line + next_line);

    //For loop to find the total distance from the chosen intersections
    for(int street_num = 0; street_num < streetseg_vector.size(); street_num++ ){
        final_distance_len = final_distance_len + findStreetSegmentLength(streetseg_vector[street_num]);
    }

    final_distance_len_int = final_distance_len;

    //checks if the final distance length is greater than 1000...
    if (final_distance_len_int > 999){
        //if it is, calculates the kilometers and meter values
        kilo = final_distance_len_int/1000;
        meters = final_distance_len_int%1000;
        //converts the int values into strings
        kilo_str = std::__cxx11::to_string(rounding_int(kilo));
        meter_str = std::__cxx11::to_string(rounding_int(meters));
        //adds it to the final string
        finalstring.append("Total Distance: " + kilo_str + "km and " + meter_str + "m\n");
    } // if the final distance length is not greater... 
    else {
        //converts int value into string
        meter_str = std::__cxx11::to_string(rounding_int(final_distance_len_int));
        //adds it to the final string
        finalstring.append("Total Distance: " + meter_str + "m\n");
    }
    
    path_time = computePathTravelTime(streetseg_vector,30);
    path_time_int = path_time;
    //checks if the path time is greater than 3600 seconds (1 hour)
    if (path_time_int >= 3599){
        //calculates the time into hours, minutes and seconds
        hour = path_time_int/3600;
        remainingSeconds = path_time_int%3600;
        if (remainingSeconds >= 60){
            minutes = remainingSeconds/60;
            remainingTime = remainingSeconds%60;
            //converts the hours, minutes and seconds from int values to strings
            second_str = std::__cxx11::to_string((remainingTime));
            minute_str = std::__cxx11::to_string((minutes));
            hour_str = std::__cxx11::to_string((hour));
            //adds it to the final string to be displayed to the user
            finalstring.append("Estimated Time: " + hour_str + "hour(s), " + minute_str + " minute(s) and " + second_str + " second(s)");
        }
    //checks if the path time is greater than 60 seconds (1 minute)
    } else if (path_time_int > 60){
        //calculates the time into minutes and seconds
        minutes = path_time_int/60;
        remainingTime = path_time_int%60;
        //converts the minutes and seconds into strings
        second_str = std::__cxx11::to_string((remainingTime));
        minute_str = std::__cxx11::to_string((minutes));
        //adds it to the final string to be displayed to the user
        finalstring.append("Estimated Time: " + minute_str + " minute(s) and " + second_str + " second(s)");
    } else {
        //if less than 60 seconds, converts the time into a string
        second_str = std::__cxx11::to_string((path_time_int));
        //adds it to the final string to be displayed to the user
        finalstring.append("Estimated Time: " + second_str + " second(s)" + next_line);
    }
    return finalstring;
}

//Function to display the directions
void directionDecisions(std::pair<IntersectionIdx, IntersectionIdx> intersection_pair){
    //Creates the dialog box to display the directions information
    std::string directionQuestion, directionInfo;
    GtkWidget *dialog = gtk_dialog_new();
    GtkWidget *directionAns;
    GtkWidget *direction;
    GtkWidget *container;
    GtkWidget *directionContainer;
    GtkWidget *directionsPage = gtk_dialog_new();
    gboolean response;
    
    //sets the default for the vindow of the dialog box
    gtk_window_set_title(GTK_WINDOW(dialog), "Directions");
    gtk_widget_set_size_request(dialog, 200, 100);
    
    //adds buttons to the dialog box
    gtk_dialog_add_button(GTK_DIALOG(dialog), "Yes", 1);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "No", 2);
    //outputs this question to the user
    directionQuestion = "Would you like directions to get to " + getIntersectionName(intersection_pair.first) + 
            " from " + getIntersectionName(intersection_pair.second) + "?"; 
    directionAns = gtk_label_new(directionQuestion.c_str());
    container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(container), directionAns);
    gtk_widget_show_all(dialog);

    //response is retrieved by the buttons
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    //if the response is ...
    switch (response){
        case 1: //Yes
            {
                //highlighting of the searched path is turned on
                *searched_streets = 1; 
                //destroys the dialog box with the question
                gtk_widget_destroy(dialog);
                //sets the window of new dialog box
                gtk_window_set_title(GTK_WINDOW(directionsPage), "Directions");
                gtk_widget_set_size_request(directionsPage, 300, 200);
                //finds the path between intersections
                std::vector <StreetSegmentIdx> directions_vector = findPathBetweenIntersections(intersection_pair, 30 );
                
                //gets the string of directions for the path and displays the directions to the user
                directionInfo = get_directions(directions_vector, intersection_pair.first, intersection_pair.second);
                direction = gtk_label_new(directionInfo.c_str());
                directionContainer = gtk_dialog_get_content_area(GTK_DIALOG(directionsPage));
                gtk_container_add(GTK_CONTAINER(directionContainer), direction);
                gtk_widget_show_all(directionsPage);
                break;
            }
        case 2: //No
            {
                //destroys the dialog box
                gtk_widget_destroy(dialog);
                break;
            }
        default: break;
    }
}
/************************************* End Directions ******************************************/