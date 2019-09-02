#include <cmath>
#include <algorithm>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <memory>

double since(clock_t start)
{
    return 1000.*double(clock() - start) / CLOCKS_PER_SEC;
}

#include "intersection.hpp"

#include "knapsack.hpp"

#include "parse.hpp"

int main()
{
    clock_t total_start = clock();

    std::clog.precision(std::numeric_limits<double>::max_digits10);

    // solution variables
    std::vector<std::unique_ptr<intersection::Region>> regions;
    std::vector<std::unique_ptr<intersection::Route>> routes;
    double budget;
    double costGcd;
    double minCost {std::numeric_limits<double>::infinity()};
    std::map<int, int> allocation;

    clock_t start = clock();
    parse::input(regions, routes, budget, minCost, costGcd);
    std::clog << "Parsing of all input took " << since(start) << "ms" << std::endl;
    std::clog << "The minimum cost is " << minCost << std::endl;
    std::clog << "The greatest common divisor of all costs is " << costGcd << std::endl;

    start = clock();
    intersection::all(regions, routes);
    std::clog << "Evaluating all routes took " << since(start) << "ms" << std::endl;

    start = clock();
    knapsack::values(routes, budget, minCost, costGcd, allocation);
    std::clog << "Optimizing my allocation took " << since(start) << "ms" << std::endl;

    /*
    std::clog << "\n" << std::endl;
    for (size_t i = 0; i < 6; ++i)
    {
        auto& region = *regions[i];
        std::clog << region;
    }
    std::clog << "\n" << std::endl;
    for (size_t i = 0, size = 6; i < size; ++i)
    {
        auto& route = *routes[i];
        std::clog << route;
    }
    */

    // Output our solution
    // std::clog << "\nMy allocation is" << std::endl;
    for (const auto& iter : allocation)
    {
        std::cout << iter.first << "," << iter.second << "\n";
        std::clog << iter.first << "," << iter.second << std::endl;
    }

    std::clog << "----------------------------------\n";
    std::clog << "Total runtime is " << since(total_start) << "ms" << std::endl;

    return 0;
}
