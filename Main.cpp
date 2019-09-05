#include <algorithm>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <memory>

/**
    Returns the number of milliseconds since the given timestamp.

    @param start given timestamp
    @return time since given timestamp in milliseconds
*/
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

    std::vector<std::unique_ptr<intersection::Region>> regions;
    std::vector<std::unique_ptr<intersection::Route>> routes;
    double budget;
    double cost_gcd;
    double min_cost {std::numeric_limits<double>::infinity()};
    std::map<int, int> allocation;

    parse::input(regions, routes, budget, min_cost, cost_gcd);

    intersection::all(regions, routes);

    knapsack::optimize(routes, budget, min_cost, cost_gcd, allocation);

    // output our allocation of routes
    for (const auto& iter : allocation)
    {
        std::cout << iter.first << "," << iter.second << "\n";
    }

    std::clog << "Total runtime is " << since(total_start) << "ms" << std::endl;
    return 0;
}
