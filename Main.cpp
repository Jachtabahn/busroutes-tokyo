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

#include <iostream>
#include <limits>
#include <boost/format.hpp>

// constants
const int NUM_TIMESLOTS = 3;
const int MAX_BUSES = 4;
const std::array<double, NUM_TIMESLOTS> slot_length {2, 8, 4};


void assertion(bool flg, std::string msg) {
    if (!flg) {
        fprintf(stderr, "[Error] %s\n", msg.c_str());
        exit(-1);
    }
}

template<class T>
bool parseString(const std::string& s, T& ret) {
    std::stringstream sin(s);
    if (sin >> ret) {
        return true;
    } else {
        return false;
    }
}







struct Region {
    std::string meshId;
    double numTargets[NUM_TIMESLOTS];
    std::vector<std::array<double, 2>> polygon;
    std::array<double, 2> extremeX {-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
    std::array<double, 2> extremeY {-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};

    Region(const std::vector<std::string>& tokens,
        std::vector<int> targetAges,
        std::array<double, NUM_TIMESLOTS> activeFactors) {
        for (size_t i = 0; i < tokens.size(); ++i) {
            auto& token = tokens[i];
            auto& nextToken = tokens[i + 1];
            if (token == "MESH_ID") {
                meshId = nextToken;
            } else if (token.size() >= 5 && token[0] == 'G' && token[3] == 'T' && token[4] == 'Z') {
                int age = token[1] - '0';
                auto last = targetAges.end();
                if (std::find(targetAges.begin(), last, age) != last) {
                    continue;
                }
                int num;
                parseString(nextToken, num);
                int time = token[5] - '0' - 2;
                assertion(time >= 0 && time < NUM_TIMESLOTS, "Invalid time slot at mesh id " + meshId);
                numTargets[time] += num * activeFactors[time] * slot_length[time];
            } else if (token == "coordinates") {
                double x, y;
                while (i + 2 < tokens.size()) {
                    assertion(parseString(tokens[i + 1], x), "Wrong coordinates");
                    assertion(parseString(tokens[i + 2], y), "Wrong coordinates");
                    i += 2;
                    polygon.push_back(std::array<double, 2>{x, y});

                    if (x < extremeX[0]) {
                        extremeX[0] = x;
                    }
                    if (x > extremeY[1]) {
                        extremeX[1] = x;
                    }
                    if (y < extremeY[0]) {
                        extremeY[0] = y;
                    }
                    if (y > extremeY[1]) {
                        extremeY[1] = y;
                    }
                }
            }
        }
        assertion(meshId != "", "Missing MESH_ID");
        assertion(polygon[0] == polygon.back(), "Open Polygon! " + meshId);
    }
};



std::vector<std::string> cleanGeojsonLine(std::string& nextLine) {
    for (int i = 0; nextLine[i]; ++i) {
        if (!(isalpha(nextLine[i]) || isdigit(nextLine[i]) || nextLine[i] == '.' || nextLine[i] == '_')) {
            nextLine[i] = ' ';
        }
    }
    std::stringstream stream(nextLine);
    std::vector<std::string> tokens;
    for (std::string token; stream >> token; ) {
        tokens.push_back(token);
    }
    return tokens;
}


std::vector<Region> loadRegions(
    std::string filename,
    std::vector<int> targetAges,
    std::array<double, NUM_TIMESLOTS> activeFactors) {
    std::ifstream stream {filename};
    assertion(stream.is_open(), "Regions geojson file missing!");

    std::vector<Region> regions;
    std::string line;
    line.reserve(10000);
    while (std::getline(stream, line)) {
        auto tokens = cleanGeojsonLine(line);
        if (tokens.size() >= 2 && tokens[0] == "type" && tokens[1] == "Feature") {
            // this is a route
            regions.push_back(Region(tokens, targetAges, activeFactors));
        }
    }
    for (size_t i = 0; i < regions.size(); ++i) {
        for (size_t j = 0; j < i; ++j) {
            assertion(regions[i].meshId != regions[j].meshId, "Duplicate MESH_ID: " + regions[i].meshId);
        }
    }
    std::cerr << boost::format("[INFO] There are %d regions in total.") % regions.size() << std::endl;
    return regions;
}










struct Route {
    std::string outputId;
    double cost;
    double benefits[MAX_BUSES];

    int TZ_Max[4 + 1];
    std::vector<std::array<double, 2>> polyline;

    Route(const std::vector<std::string>& tokens) {
        for (size_t i = 0; i < tokens.size(); ++i) {
            auto& token = tokens[i];
            auto& nextToken = tokens[i+1];
            if (token == "RouteID") {
                assertion(i + 1 < tokens.size(), "Missing route outputId!");
                outputId = nextToken;
            } else if (token == "Cost") {
                assertion(i + 1 < tokens.size(), "Missing route outputId!");
                parseString(nextToken, cost);
            } else if (token == "TZ1_Max") {
                assertion(i + 1 < tokens.size(), "Missing TZ1_Max!");
                parseString(nextToken, TZ_Max[1]);
            } else if (token == "TZ2_Max") {
                assertion(i + 1 < tokens.size(), "Missing TZ2_Max!");
                parseString(nextToken, TZ_Max[2]);
            } else if (token == "TZ3_Max") {
                assertion(i + 1 < tokens.size(), "Missing TZ3_Max!");
                parseString(nextToken, TZ_Max[3]);
            } else if (token == "TZ4_Max") {
                assertion(i + 1 < tokens.size(), "Missing TZ4_Max!");
                parseString(nextToken, TZ_Max[4]);
            } else if (token == "coordinates") {
                double x, y;
                while (i + 2 < tokens.size()) {
                    assertion(parseString(nextToken, x), "Wrong coordinates");
                    assertion(parseString(tokens[i + 2], y), "Wrong coordinates");
                    i += 2;
                    polyline.push_back(std::array<double, 2>{x, y});
                }
            }
        }
        for (int i = 0; i < MAX_BUSES; ++i) {
            benefits[i] = 0.;
        }
        assertion(outputId != "", "Missing route outputId!");
    }

    Route(std::string outputId, double cost, double benefits[MAX_BUSES]) {
        this->outputId = outputId;
        this->cost = cost;
        for (int i = 0; i < MAX_BUSES; ++i) {
            this->benefits[i] = benefits[i];
        }
    }
};

std::vector<Route*> loadRoutes(std::string filename) {
    std::ifstream stream {filename};
    assertion(stream.is_open(), "Route geojson file missing!");

    std::vector<Route*> routes;
    std::string line;
    line.reserve(10000);
    while (std::getline(stream, line)) {
        std::vector<std::string> tokens = cleanGeojsonLine(line);
        if (tokens.size() >= 2 && tokens[0] == "type" && tokens[1] == "Feature") {
            // this is a route
            auto route = new Route(tokens);
            routes.push_back(route);
        }
    }
    for (size_t i = 0; i < routes.size(); ++i) {
        for (size_t j = 0; j < i; ++j) {
            assertion(routes[i]->outputId != routes[j]->outputId, "Duplicate route outputId: " + routes[i]->outputId);
        }
    }
    std::cerr << boost::format("I found %d routes.") % routes.size() << std::endl;
    return routes;
}

std::array<int, NUM_TIMESLOTS> loadActive(std::string filename) {
    std::ifstream factorsStream (filename);
    if (!factorsStream.is_open()) {
        std::cerr << "Route geojson file missing!" << std::endl;
        exit(1);
    }

    int activeFactors[NUM_TIMESLOTS];

    std::string line;
    line.reserve(100);

    factorsStream >> line;
    std::getline(factorsStream, line, ',');
    for (int f = 0; std::getline(factorsStream, line, ','); ++f) {
        std::stringstream s(line);
        s >> activeFactors[f];
        std::cerr << line;
        std::cerr << activeFactors[f];
    }

    return std::array<int, NUM_TIMESLOTS> {};
}

std::set<int> loadTargetAgeGroup(std::string str) {
    std::set<int> target_age_groups;
    std::stringstream ss(str);
    std::string group;
    while(getline(ss, group, ','))
        target_age_groups.insert(atoi(group.c_str()));

    return target_age_groups;
}


int main() {
    // ---- READ INPUT ----
    std::string line;
    line.reserve(100);
    // Line 1: age group
    std::getline(std::cin, line);
    std::set<int> target_age_groups = loadTargetAgeGroup(line);
    // Line 2: budget
    std::getline(std::cin, line);
    double budget;
    parseString(line, budget);
    // Line 3: Population JSON
    std::getline(std::cin, line);
    std::string POPULATION_JSON = line;
    while (isspace(POPULATION_JSON.back())) {
        POPULATION_JSON.pop_back();
    }
    // Line 4: Route JSON
    std::getline(std::cin, line);
    std::string ROUTE_JSON = line;
    while (isspace(ROUTE_JSON.back())) {
        ROUTE_JSON.pop_back();
    }
    // Line 5: Active CSV
    std::getline(std::cin, line);
    std::string ACTIVE_CSV = line;
    while (isspace(ACTIVE_CSV.back())) {
        ACTIVE_CSV.pop_back();
    }

    std::array<int, NUM_TIMESLOTS> activeFactors = loadActive(ACTIVE_CSV);
    std::cerr << "Here are the active factors:" << activeFactors[0] << std::endl;
    std::cerr << "Here are the active factors:" << activeFactors[1] << std::endl;
    std::cerr << "Here are the active factors:" << activeFactors[2] << std::endl;

    // std::vector<Route*> routes = loadRoutes(ROUTE_JSON);
    // std::vector<Region> regions = loadRegions(POPULATION_JSON);
    // ---- READ INPUT END ----





    // int tz_length[5];
    // tz_length[1] = 10;
    // tz_length[2] = 2;
    // tz_length[3] = 8;
    // tz_length[4] = 4;



    // Output our solution
    std::map<std::string, int> allocations;
    for (const auto& iter : allocations) {
        printf("%s,%d\n", iter.first.c_str(), iter.second);
    }



    return 0;
}
