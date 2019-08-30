#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cctype>
#include <map>
#include <set>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

void outputScore(double score)
{
    fprintf(stderr, "[INFO] Your final score is %.6f\n", score);
}

const int MAX_LEN = 10000000;
const double EPS = 1e-8;

double sqr(double x)
{
    return x * x;
}

int sign(double x)
{
    if (x < -EPS) {
        return -1;
    }
    return x > EPS;
}

char line[MAX_LEN + 1];

void myAssert(bool flg, string msg)
{
    if (!flg) {
        fprintf(stderr, "[Error] %s\n", msg.c_str());
        outputScore(-1.0);
        exit(-1);
    }
}

struct Point
{
    double x, y;
    Point() {}
    Point(double x, double y) : x(x), y(y) {}
};

Point operator + (const Point& a, const Point& b)
{
    return Point(a.x + b.x, a.y + b.y);
}

Point operator - (const Point& a, const Point& b)
{
    return Point(a.x - b.x, a.y - b.y);
}

Point operator * (const Point& a, const double& b)
{
    return Point(a.x * b, a.y * b);
}

Point operator / (const Point& a, const double& b)
{
    return Point(a.x / b, a.y / b);
}

double det(const Point& a, const Point& b)
{
    return a.x * b.y - a.y * b.x;
}

double dot(const Point& a, const Point& b)
{
    return a.x * b.x + a.y * b.y;
}

bool operator == (const Point& a, const Point& b)
{
    return fabs(a.x - b.x) < EPS && fabs(a.y - b.y) < EPS;
}

template<class T>
bool fromString(const string& s, T& ret)
{
    stringstream sin(s);
    if (sin >> ret) {
        return true;
    } else {
        return false;
    }
}

vector<string> clean(char line[])
{
    for (int i = 0; line[i]; ++ i) {
        if (isalpha(line[i]) || isdigit(line[i]) || line[i] == '.' || line[i] == '_') {
            // keep it as it is
        } else {
            line[i] = ' ';
        }
    }
    stringstream sin(line);
    vector<string> tokens;
    for (string token; sin >> token;) {
        tokens.push_back(token);
    }
    return tokens;
}

struct Route
{
    string name;
    double cost;
    int TZ_Max[4 + 1];
    vector<Point> polyline;

    Route(const vector<string>& tokens) {
        for (int i = 0; i < tokens.size(); ++ i) {
            if (tokens[i] == "RouteID") {
                myAssert(i + 1 < tokens.size(), "Missing route name!");
                name = tokens[i + 1];
            } else if (tokens[i] == "Cost") {
                myAssert(i + 1 < tokens.size(), "Missing route name!");
                fromString(tokens[i + 1], cost);
            } else if (tokens[i] == "TZ1_Max") {
                myAssert(i + 1 < tokens.size(), "Missing TZ1_Max!");
                fromString(tokens[i + 1], TZ_Max[1]);
            } else if (tokens[i] == "TZ2_Max") {
                myAssert(i + 1 < tokens.size(), "Missing TZ2_Max!");
                fromString(tokens[i + 1], TZ_Max[2]);
            } else if (tokens[i] == "TZ3_Max") {
                myAssert(i + 1 < tokens.size(), "Missing TZ3_Max!");
                fromString(tokens[i + 1], TZ_Max[3]);
            } else if (tokens[i] == "TZ4_Max") {
                myAssert(i + 1 < tokens.size(), "Missing TZ4_Max!");
                fromString(tokens[i + 1], TZ_Max[4]);
            } else if (tokens[i] == "coordinates") {
                double x, y;
                while (i + 2 < tokens.size()) {
                    myAssert(fromString(tokens[i + 1], x), "Wrong coordinates");
                    myAssert(fromString(tokens[i + 2], y), "Wrong coordinates");
                    i += 2;
                    polyline.push_back(Point(x, y));
                }
            }
        }
        myAssert(name != "", "Missing route name!");
    }
};

vector<Route> loadRoutes(string filename)
{
    FILE* in = fopen(filename.c_str(), "r");
    myAssert(in != NULL, "Route geojson file missing!");

    vector<Route> routes;
    while (fgets(line, MAX_LEN, in)) {
        vector<string> tokens = clean(line);
        if (tokens.size() >= 2 && tokens[0] == "type" && tokens[1] == "Feature") {
            // this is a route
            routes.push_back(Route(tokens));
        }
    }
    for (int i = 0; i < routes.size(); ++ i) {
        for (int j = 0; j < i; ++ j) {
            myAssert(routes[i].name != routes[j].name, "Duplicate route name: " + routes[i].name);
        }
    }
    fprintf(stderr, "[INFO] Total Routes = %d\n", routes.size());
    return routes;
}

struct Grid
{
    string MESH_ID;
    double G_TZ[6 + 1][4 + 1];
    vector<Point> polygon;

    Grid(const vector<string>& tokens) {
        // "G1_TZ1": 68.6211, "G1_TZ2": 68.6211, "G1_TZ3": 0.0, "G1_TZ4": 0.0, "G2_TZ1": 0.0, "G2_TZ2": 0.0, "G2_TZ3": 0.0, "G2_TZ4": 4.9962, "G3_TZ1": 1.2687, "G3_TZ2": 8.9979, "G3_TZ3": 7.5105, "G3_TZ4": 14.2905, "G4_TZ1": 16.2124, "G4_TZ2": 4.3919, "G4_TZ3": 4.0807, "G4_TZ4": 4.4664
        // coordinates
        for (int i = 0; i < tokens.size(); ++ i) {
            if (tokens[i] == "MESH_ID") {
                myAssert(i + 1 < tokens.size(), "Missing MESH_ID!");
                MESH_ID = tokens[i + 1];
            } else if (tokens[i].size() >= 5 && tokens[i][0] == 'G' && tokens[i][3] == 'T' && tokens[i][4] == 'Z') {
                myAssert(i + 1 < tokens.size(), "Missing G_TZ!");
                int g = tokens[i][1] - '0';
                int tz = tokens[i][5] - '0';
                fromString(tokens[i + 1], G_TZ[g][tz]);
            } else if (tokens[i] == "coordinates") {
                double x, y;
                while (i + 2 < tokens.size()) {
                    // if (MESH_ID == "533954264") {
                    //     fprintf(stderr, "[Debug] %s %s\n", tokens[i + 1].c_str(), tokens[i + 2].c_str());
                    // }
                    myAssert(fromString(tokens[i + 1], x), "Wrong coordinates");
                    myAssert(fromString(tokens[i + 2], y), "Wrong coordinates");
                    i += 2;
                    polygon.push_back(Point(x, y));
                }
            }
        }
        myAssert(MESH_ID != "", "Missing MESH_ID");
        myAssert(polygon[0] == polygon.back(), "Open Polygon! " + MESH_ID);
    }
};

vector<Grid> loadPopulations(string filename)
{
    FILE* in = fopen(filename.c_str(), "r");
    myAssert(in != NULL, "Route geojson file missing!");

    vector<Grid> grids;
    while (fgets(line, MAX_LEN, in)) {
        vector<string> tokens = clean(line);
        if (tokens.size() >= 2 && tokens[0] == "type" && tokens[1] == "Feature") {
            // this is a route
            grids.push_back(Grid(tokens));
        }
    }
    for (int i = 0; i < grids.size(); ++ i) {
        for (int j = 0; j < i; ++ j) {
            myAssert(grids[i].MESH_ID != grids[j].MESH_ID, "Duplicate MESH_ID: " + grids[i].MESH_ID);
        }
    }
    fprintf(stderr, "[INFO] Total Grids = %d\n", grids.size());
    return grids;
}

double tz_multiplier[4 + 1];

void loadActive(string filename)
{
    FILE* in = fopen(filename.c_str(), "r");
    myAssert(in != NULL, "Route geojson file missing!");
    fgets(line, MAX_LEN, in);
    fscanf(in, "%lf,%lf,%lf,%lf", &tz_multiplier[1], &tz_multiplier[2], &tz_multiplier[3], &tz_multiplier[4]);

    fprintf(stderr, "[INFO] tz_multipliers = %.2f, %.2f, %.2f, %.2f\n", tz_multiplier[1], tz_multiplier[2], tz_multiplier[3], tz_multiplier[4]);
}

bool onSegment(const Point& x, const Point& a, const Point& b)
{
    return sign(det(x - a, b - a)) == 0 && sign(dot(a - x, b - x)) < 0;
}

bool isInside(const Point& a, const vector<Point>& polygon)
{
    int cnt = 0;
    for (int i = 0; i + 1 < polygon.size(); ++ i) {
        if (onSegment(a, polygon[i], polygon[i + 1])) {
            return true;
        }
        int k = sign(det(polygon[i + 1] - polygon[i], a - polygon[i]));
        int d1 = sign(polygon[i].y - a.y);
        int d2 = sign(polygon[i + 1].y - a.y);
        if (k > 0 && d1 <= 0 && d2 > 0) {
            ++ cnt;
        }
        if (k < 0 && d2 <= 0 && d1 > 0) {
            -- cnt;
        }
    }
    return cnt != 0;
}

bool hasIntersection(const Point& a, const Point& b, const Point& c, const Point& d)
{
    return sign(det(c - a, b - a)) * sign(det(d - a, b - a)) <= 0 && sign(det(a - c, d - c)) * sign(det(b - c, d - c)) <= 0;
}

bool hasIntersection(const Point& a, const Point& b, const vector<Point>& polygon)
{
    for (int i = 0; i + 1 < polygon.size(); ++ i) {
        if (hasIntersection(a, b, polygon[i], polygon[i + 1])) {
            return true;
        }
    }
    return false;
}

bool hasIntersection(const vector<Point>& polyline, const vector<Point>& polygon)
{
    for (int i = 0; i + 1 < polyline.size(); ++ i) {
        if (hasIntersection(polyline[i], polyline[i + 1], polygon)) {
            return true;
        }
    }
    return false;
}

map<string, int> loadBusPlan(string filename)
{
    FILE* in = fopen(filename.c_str(), "r");
    myAssert(in != NULL, "Bus Plan file missing!");

    map<string, int> busCnt;
    while (fgets(line, MAX_LEN, in)) {
        vector<string> tokens = clean(line);
        myAssert(tokens.size() == 2, "Wrong Bus Plan format!");
        if (tokens[0] == "RouteID") {
            // skip the header
            continue;
        }
        string routeID = tokens[0];
        int cnt;
        fromString(tokens[1], cnt);
        busCnt[routeID] = cnt;
    }
    fprintf(stderr, "[INFO] # of Distinct Purchased RouteIDs = %d\n", busCnt.size());
    return busCnt;
}

set<int> loadTargetAgeGroup(string str)
{
    set<int> target_age_groups;
    std::stringstream ss(str);
    string group;
    while(getline(ss, group, ','))
        target_age_groups.insert(atoi(group.c_str()));

    return target_age_groups;
}

int main(int argc, char* argv[])
{
    // string ROUTE_JSON = "data_latest0810/Route.geojson";
    string ROUTE_JSON = argv[1];
    // string POPULATION_JSON = "data_latest0810/Population.geojson";
    string POPULATION_JSON = argv[2];
    // string ACTIVE_CSV = "data_latest0810/active.csv";
    string ACTIVE_CSV = argv[3];
    // string SUBMISSION_FILE = "example_submission.csv";
    string SUBMISSION_FILE = argv[4];

    // set<int> target_age_groups = set<int>({1, 2, 3, 4, 5, 6}); // full set now
    set<int> target_age_groups = loadTargetAgeGroup(argv[5]);
    fprintf(stderr, "[INFO] Age groups = \"%s\"\n", argv[5]);
    double budget = std::stod(argv[6]);
    fprintf(stderr, "[INFO] Budget = %.0lf\n", budget);
    double calc_time_ms = std::stod(argv[7]);
    fprintf(stderr, "[INFO] calc_time_ms = %.0lf\n", calc_time_ms);

    vector<Route> routes = loadRoutes(ROUTE_JSON);
    vector<Grid> grids = loadPopulations(POPULATION_JSON);
    loadActive(ACTIVE_CSV);
    map<string, int> busCnt = loadBusPlan(SUBMISSION_FILE);
    vector<vector<bool>> intersection(routes.size(), vector<bool>(grids.size(), false));
    int cnt = 0;
    for (int i = 0; i < routes.size(); ++ i) {
        for (int j = 0; j < grids.size(); ++ j) {
            intersection[i][j] = hasIntersection(routes[i].polyline, grids[j].polygon);
            cnt += intersection[i][j];
        }
    }
    fprintf(stderr, "[INFO] Total intersections = %d\n", cnt);

    double total_cost = 0, number_of_population_reached = 0, MAX_POPULATION = 0;
    int tz_length[5];
    tz_length[1] = 10;
    tz_length[2] = 2;
    tz_length[3] = 8;
    tz_length[4] = 4;
    for (int i = 0; i < routes.size(); ++ i) {
        int cnt = busCnt[routes[i].name]; // default value will be 0
        total_cost += cnt * routes[i].cost;
        myAssert(total_cost < budget + EPS, "Budget Exceeded!");
        for (int age_group = 1; age_group <= 6; ++ age_group) {
            if (target_age_groups.count(age_group)) {
                for (int tz = 1; tz <= 4; ++ tz) {
                    int actual_cnt = min(cnt, routes[i].TZ_Max[tz]);
                    int max_cnt = routes[i].TZ_Max[tz];
                    for (int j = 0; j < grids.size(); ++ j) {
                        if (intersection[i][j]) {
                            double outside_population = grids[j].G_TZ[age_group][tz] * tz_multiplier[tz];
                            number_of_population_reached += outside_population * actual_cnt * tz_length[tz];
                            if (outside_population * actual_cnt * tz_length[tz] != 0) {
                                // fprintf(stderr, "[INFO] grids[%d].G_TZ[%d][%d] = %.2f\n", j, age_group, tz, grids[j].G_TZ[age_group][tz]);
                                // fprintf(stderr, "[INFO] outside_population = %.2f\n", outside_population);
                                // fprintf(stderr, "[INFO] actual_cnt = %.2f\n", actual_cnt);
                                // fprintf(stderr, "[INFO] tz_multiplier[%d] = %.2f\n", tz, tz_length[tz]);
                                // fprintf(stderr, "[INFO] number_of_population_reached = %.2f\n", number_of_population_reached);
                            }
                            MAX_POPULATION += outside_population * max_cnt * tz_length[tz];
                        }
                    }
                }
            }
        }
    }
    fprintf(stderr, "[INFO] number_of_population_reached = %.2f\n", number_of_population_reached);
    fprintf(stderr, "[INFO] MAX_POPULATION = %.2f\n", MAX_POPULATION);
    double raw_score = number_of_population_reached / MAX_POPULATION;
    fprintf(stderr, "[INFO] raw_score = %.10f\n", raw_score);
    // TBD
    double score = raw_score / sqr(max(1.0, calc_time_ms / 100.0)) * 1000;
    fprintf(stderr, "[INFO] score = %.10f\n", score);

    outputScore(score);
    return 0;
}