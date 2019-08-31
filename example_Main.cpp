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

void assertion(bool flg, std::string msg) {
    if (!flg) {
        fprintf(stderr, "[Error] %s\n", msg.c_str());
        exit(-1);
    }
}

const int MAX_LEN = 10000000;
const double EPS = 1e-8;

int sign(double x) {
    if (x < -EPS) {
        return -1;
    }
    return x > EPS;
}

struct Point {
    double x, y;
    Point() {}
    Point(double x, double y) : x(x), y(y) {}
};

Point operator + (const Point& a, const Point& b) {
    return Point(a.x + b.x, a.y + b.y);
}

Point operator - (const Point& a, const Point& b) {
    return Point(a.x - b.x, a.y - b.y);
}

Point operator * (const Point& a, const double& b) {
    return Point(a.x * b, a.y * b);
}

Point operator / (const Point& a, const double& b) {
    return Point(a.x / b, a.y / b);
}

double det(const Point& a, const Point& b) {
    return a.x * b.y - a.y * b.x;
}

double dot(const Point& a, const Point& b) {
    return a.x * b.x + a.y * b.y;
}

bool operator == (const Point& a, const Point& b) {
    return fabs(a.x - b.x) < EPS && fabs(a.y - b.y) < EPS;
}









struct Region {
    std::string MESH_ID;
    double G_TZ[6 + 1][4 + 1];
    std::vector<Point> polygon;

    Region(const std::vector<std::string>& tokens) {
        // "G1_TZ1": 68.6211, "G1_TZ2": 68.6211, "G1_TZ3": 0.0, "G1_TZ4": 0.0, "G2_TZ1": 0.0, "G2_TZ2": 0.0, "G2_TZ3": 0.0, "G2_TZ4": 4.9962, "G3_TZ1": 1.2687, "G3_TZ2": 8.9979, "G3_TZ3": 7.5105, "G3_TZ4": 14.2905, "G4_TZ1": 16.2124, "G4_TZ2": 4.3919, "G4_TZ3": 4.0807, "G4_TZ4": 4.4664
        // coordinates
        for (int i = 0; i < tokens.size(); ++i) {
            if (tokens[i] == "MESH_ID") {
                assertion(i + 1 < tokens.size(), "Missing MESH_ID!");
                MESH_ID = tokens[i + 1];
            } else if (tokens[i].size() >= 5 && tokens[i][0] == 'G' && tokens[i][3] == 'T' && tokens[i][4] == 'Z') {
                assertion(i + 1 < tokens.size(), "Missing G_TZ!");
                int g = tokens[i][1] - '0';
                int time = tokens[i][5] - '0';
                fromString(tokens[i + 1], G_TZ[g][time]);
            } else if (tokens[i] == "coordinates") {
                double x, y;
                while (i + 2 < tokens.size()) {
                    // if (MESH_ID == "533954264") {
                    //     fprintf(stderr, "[Debug] %s %s\n", tokens[i + 1].c_str(), tokens[i + 2].c_str());
                    // }
                    assertion(fromString(tokens[i + 1], x), "Wrong coordinates");
                    assertion(fromString(tokens[i + 2], y), "Wrong coordinates");
                    i += 2;
                    polygon.push_back(Point(x, y));
                }
            }
        }
        assertion(MESH_ID != "", "Missing MESH_ID");
        assertion(polygon[0] == polygon.back(), "Open Polygon! " + MESH_ID);
    }
};

std::vector<Region> loadRegions(std::string filename) {
    FILE* in = fopen(filename.c_str(), "r");
    assertion(in != NULL, "Route geojson file missing!");

    std::vector<Region> regions;
    while (fgets(line, MAX_LEN, in)) {
        std::vector<std::string> tokens = clean(line);
        if (tokens.size() >= 2 && tokens[0] == "type" && tokens[1] == "Feature") {
            // this is a route
            regions.push_back(Region(tokens));
        }
    }
    for (int i = 0; i < regions.size(); ++i) {
        for (int j = 0; j < i; ++j) {
            assertion(regions[i].MESH_ID != regions[j].MESH_ID, "Duplicate MESH_ID: " + regions[i].MESH_ID);
        }
    }
    fprintf(stderr, "[INFO] Total Grids = %d\n", regions.size());
    return regions;
}












char line[MAX_LEN + 1];


template<class T>
bool fromString(const std::string& s, T& ret) {
    std::stringstream sin(s);
    if (sin >> ret) {
        return true;
    } else {
        return false;
    }
}


const int MAX_BUSES = 4;

struct Route {
    std::string name;
    double cost;
    double benefits[MAX_BUSES];
    int TZ_Max[4 + 1];
    std::vector<Point> polyline;

    Route(const std::vector<std::string>& tokens) {
        for (int i = 0; i < tokens.size(); ++i) {
            if (tokens[i] == "RouteID") {
                assertion(i + 1 < tokens.size(), "Missing route name!");
                name = tokens[i + 1];
            } else if (tokens[i] == "Cost") {
                assertion(i + 1 < tokens.size(), "Missing route name!");
                fromString(tokens[i + 1], cost);
            } else if (tokens[i] == "TZ1_Max") {
                assertion(i + 1 < tokens.size(), "Missing TZ1_Max!");
                fromString(tokens[i + 1], TZ_Max[1]);
            } else if (tokens[i] == "TZ2_Max") {
                assertion(i + 1 < tokens.size(), "Missing TZ2_Max!");
                fromString(tokens[i + 1], TZ_Max[2]);
            } else if (tokens[i] == "TZ3_Max") {
                assertion(i + 1 < tokens.size(), "Missing TZ3_Max!");
                fromString(tokens[i + 1], TZ_Max[3]);
            } else if (tokens[i] == "TZ4_Max") {
                assertion(i + 1 < tokens.size(), "Missing TZ4_Max!");
                fromString(tokens[i + 1], TZ_Max[4]);
            } else if (tokens[i] == "coordinates") {
                double x, y;
                while (i + 2 < tokens.size()) {
                    assertion(fromString(tokens[i + 1], x), "Wrong coordinates");
                    assertion(fromString(tokens[i + 2], y), "Wrong coordinates");
                    i += 2;
                    polyline.push_back(Point(x, y));
                }
            }
        }
        for (int i = 0; i < MAX_BUSES; ++i) {
            benefits[i] = 0.;
        }
        assertion(name != "", "Missing route name!");
    }

    Route(std::string name, double cost, double benefits[MAX_BUSES]) {
        this->name = name;
        this->cost = cost;
        for (int i = 0; i < MAX_BUSES; ++i) {
            this->benefits[i] = benefits[i];
        }
    }
};

std::vector<std::string> clean(char line[]) {
    for (int i = 0; line[i]; ++i) {
        if (isalpha(line[i]) || isdigit(line[i]) || line[i] == '.' || line[i] == '_') {
            // keep it as it is
        } else {
            line[i] = ' ';
        }
    }
    std::stringstream sin(line);
    std::vector<std::string> tokens;
    for (std::string token; sin >> token;) {
        tokens.push_back(token);
    }
    return tokens;
}

std::vector<Route*> loadRoutes(std::string filename) {
    FILE* in = fopen(filename.c_str(), "r");
    assertion(in != NULL, "Route geojson file missing!");

    std::vector<Route*> routes;
    while (fgets(line, MAX_LEN, in)) {
        std::vector<std::string> tokens = clean(line);
        if (tokens.size() >= 2 && tokens[0] == "type" && tokens[1] == "Feature") {
            // this is a route
            auto route = new Route(tokens);
            routes.push_back(route);
        }
    }
    for (int i = 0; i < routes.size(); ++i) {
        for (int j = 0; j < i; ++j) {
            assertion(routes[i]->name != routes[j]->name, "Duplicate route name: " + routes[i]->name);
        }
    }
    fprintf(stderr, "[INFO] Total Routes = %d\n", routes.size());
    return routes;
}


double tz_multiplier[4 + 1];

void loadActive(std::string filename) {
    FILE* in = fopen(filename.c_str(), "r");
    assertion(in != NULL, "Route geojson file missing!");
    fgets(line, MAX_LEN, in);
    fscanf(in, "%lf,%lf,%lf,%lf", &tz_multiplier[1], &tz_multiplier[2], &tz_multiplier[3], &tz_multiplier[4]);

    fprintf(stderr, "[INFO] tz_multipliers = %.2f, %.2f, %.2f, %.2f\n", tz_multiplier[1], tz_multiplier[2], tz_multiplier[3], tz_multiplier[4]);
}

std::set<int> loadTargetAgeGroup(std::string str) {
    std::set<int> target_age_groups;
    std::stringstream ss(str);
    std::string group;
    while(getline(ss, group, ','))
        target_age_groups.insert(atoi(group.c_str()));

    return target_age_groups;
}





bool hasIntersection(const Point& a, const Point& b, const Point& c, const Point& d) {
    return sign(det(c - a, b - a)) * sign(det(d - a, b - a)) <= 0 && sign(det(a - c, d - c)) * sign(det(b - c, d - c)) <= 0;
}

bool hasIntersection(const Point& a, const Point& b, const std::vector<Point>& polygon) {
    for (int i = 0; i + 1 < polygon.size(); ++i) {
        if (hasIntersection(a, b, polygon[i], polygon[i + 1])) {
            return true;
        }
    }
    return false;
}

bool hasIntersection(const std::vector<Point>& polyline, const std::vector<Point>& polygon) {
    for (int i = 0; i + 1 < polyline.size(); ++i) {
        if (hasIntersection(polyline[i], polyline[i + 1], polygon)) {
            return true;
        }
    }
    return false;
}

void computeBenefits(
    const std::vector<Route*>& routes,
    const std::vector<Region>& regions,
    const std::set<int>& target_age_groups,
    const std::vector<std::vector<bool>>& intersection,
    const int tz_length[]) {
    for (int routeId = 0; routeId < routes.size(); ++routeId) {
        // compute this route's benefit
        for (int age_group = 1; age_group <= 6; ++age_group) {
            if (!target_age_groups.count(age_group)) {
                continue;
            }

            for (int regionId = 0; regionId < regions.size(); ++regionId) {
                bool intersects = hasIntersection(routes[routeId]->polyline, regions[regionId].polygon);
                if (!intersects) {
                    continue;
                }

                for (int time = 2; time <= 4; ++time) {
                    auto& route = routes[routeId];
                    for (int buses = 1; buses <= route->TZ_Max[time]; ++buses) {
                        route->benefits[buses-1] +=
                            regions[regionId].G_TZ[age_group][time] *
                            tz_multiplier[time] *
                            tz_length[time];
                    }
                }
            }
        }
    }
}





int main() {
    std::setbuf(stderr, NULL);



    // ---- READ INPUT ----
    // Line 1: age group
    fgets(line, MAX_LEN, stdin);
    std::set<int> target_age_groups = loadTargetAgeGroup(line);
    // Line 2: budget
    fgets(line, MAX_LEN, stdin);
    double budget;
    fromString(line, budget);
    // Line 3: Population JSON
    fgets(line, MAX_LEN, stdin);
    std::string POPULATION_JSON = line;
    while (isspace(POPULATION_JSON.back())) {
        POPULATION_JSON.pop_back();
    }
    // Line 4: Route JSON
    fgets(line, MAX_LEN, stdin);
    std::string ROUTE_JSON = line;
    while (isspace(ROUTE_JSON.back())) {
        ROUTE_JSON.pop_back();
    }
    // Line 5: Active CSV
    fgets(line, MAX_LEN, stdin);
    std::string ACTIVE_CSV = line;
    while (isspace(ACTIVE_CSV.back())) {
        ACTIVE_CSV.pop_back();
    }

    std::vector<Route*> routes = loadRoutes(ROUTE_JSON);
    std::vector<Region> regions = loadRegions(POPULATION_JSON);
    loadActive(ACTIVE_CSV);
    // ---- READ INPUT END ----





    int tz_length[5];
    tz_length[1] = 10;
    tz_length[2] = 2;
    tz_length[3] = 8;
    tz_length[4] = 4;

    computeBenefits(routes, regions, target_age_groups, intersection, tz_length);

    // sort the routes according to their utility benefits[0]/cost
    struct {
        bool operator()(const Route* a, const Route* b) const {
            return a->cost/a->benefits[0] < b->cost/b->benefits[0];
        }
    } routeComparator;
    std::sort(routes.begin(), routes.end(), routeComparator);

    /*
    for (const auto& iter : routes) {
        fprintf(stderr, "Route %s: Cost=%f\n", iter->name.c_str(), iter->cost);
        for (int b = 0; b < MAX_BUSES; ++b) {
            fprintf(stderr, "\tTo buy bus #%d: Disutility=%f, Benefit=%f\n", b+1, iter->cost/iter->benefits[b], iter->benefits[b]);
        }
    }
    */

    double budget_left = budget;
    std::map<std::string, int> allocations;
    for (int r = 0; r < routes.size() && budget_left > 0; ++r) {
        auto route = routes[r];
        if (budget_left < route->cost) {
            continue;
        }
        allocations[route->name] += 1;
        budget_left -= route->cost;
        fprintf(stderr, "selected %s, total_cost = %.2f / %.2f\n", route->name.c_str(), budget-budget_left, budget);

        if (route->benefits[1] < EPS) {
            continue;
        }

        double ben[MAX_BUSES];
        for (int b = 0; b < MAX_BUSES-1; ++b) {
            ben[b] = route->benefits[b+1];
        }
        auto new_route = new Route(route->name, route->cost, ben);
        auto new_index = std::upper_bound(routes.begin(), routes.end(), route, routeComparator);
        fprintf(stderr, "Just selected route #%d, now inserting this route back to index #%d \n", r, new_index);
        routes.insert(new_index, new_route);
    }






    // Output our solution
    for (const auto& iter : allocations) {
        printf("%s,%d\n", iter.first.c_str(), iter.second);
    }



    return 0;
}
