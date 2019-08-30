#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cctype>
#include <ctime>
#include <algorithm>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include <iostream>
#include <memory>
#include <limits>
#include <stdexcept>
#include <boost/format.hpp>

namespace intersection
{
    // constants
    const int TIMESLOTS = 3;
    const int MAX_BUSES = 4;

    struct Region
    {
        std::string meshId = "NONE";
        std::array<double, TIMESLOTS> numTargets {0., 0., 0.};
        std::vector<std::array<double, 2>> polygon;
        std::array<double, 2> extremeX {std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()};
        std::array<double, 2> extremeY {std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()};

        Region() { }
    };

    struct Route
    {
        std::string outputId;
        double cost;
        double benefits[MAX_BUSES];

        int slotBuses[TIMESLOTS];
        std::vector<std::array<double, 2>> polyline;

        Route() { }

        Route(std::string outputId, double cost, double benefits[MAX_BUSES])
        {
            this->outputId = outputId;
            this->cost = cost;
            for (int i = 0; i < MAX_BUSES; ++i)
            {
                this->benefits[i] = benefits[i];
            }
        }
    };
}

#include "parse.hpp"

int main()
{
    std::vector<std::unique_ptr<intersection::Region>> regions;
    std::vector<std::unique_ptr<intersection::Route>> routes;
    double budget;

    clock_t start = clock();
    parse::input(regions, routes, budget);
    std::cerr << "Total runtime is " << 1000.*double(clock() - start) / CLOCKS_PER_SEC << "ms" << std::endl;

    /*
    for (size_t i = 0; i < 6; ++i)
    {
        auto& region = *regions[i];
        std::cerr << "----------------- Region -----------------\n";
        std::cerr << "Id: " << i << "\n";
        std::cerr << "Mesh id: " << region.meshId << "\n";
        std::cerr << "Number of targets in time slots:"
            << " " << region.numTargets[0]
            << " " << region.numTargets[1]
            << " " << region.numTargets[2]
            << "\n";
        std::cerr << "Number of polygon points: " << region.polygon.size() << "\n";
        std::cerr << "Minimum point of the polygon: (" << region.extremeX[0] << ", " << region.extremeY[0] << ")\n"
            << "Maximum point of the polygon: (" << region.extremeX[1] << ", " << region.extremeY[1] << ")\n";
    }
    */

    // Output our solution
    std::map<std::string, int> allocations;
    allocations["1"] = 1;
    for (const auto& iter : allocations)
    {
        printf("%s,%d\n", iter.first.c_str(), iter.second);
    }

    return 0;
}
