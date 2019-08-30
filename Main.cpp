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

#include "parse.hpp"

struct Solution
{
    std::vector<std::unique_ptr<intersection::Region>> regions;
    std::vector<std::unique_ptr<intersection::Route>> routes;
    double budget;

    intersection::Box routesBox {intersection::supremum, intersection::infimum};

    std::map<std::string, int> allocations;
} solution;

int main()
{
    std::cerr.precision(std::numeric_limits<double>::max_digits10);

    clock_t total_start = clock();
    clock_t start = clock();
    parse::input(solution.regions, solution.routes, solution.budget, solution.routesBox);
    std::cerr << "Parsing of all input took " << since(start) << "ms" << std::endl;

    start = clock();
    intersection::all(solution.regions, solution.routes);
    std::cerr << "Evaluating all routes took " << since(start) << "ms" << std::endl;

    start = clock();
    knapsack::values(solution.regions, solution.routes, solution.budget, solution.allocations);
    std::cerr << "Evaluating all routes took " << since(start) << "ms" << std::endl;

    std::cerr << "\n" << std::endl;
    for (size_t i = 0; i < 6; ++i)
    {
        auto& region = *solution.regions[i];
        std::cerr << region;
    }
    std::cerr << "\n" << std::endl;
    for (size_t i = 0, size = solution.routes.size(); i < size; ++i)
    {
        auto& route = *solution.routes[i];
        std::cerr << route;
    }
    std::cerr << "\nThe box enclosing all routes is " << solution.routesBox << std::endl;

    // Output our solution
    solution.allocations["1"] = 1;
    for (const auto& iter : solution.allocations)
    {
        printf("%s,%d\n", iter.first.c_str(), iter.second);
    }

    std::cerr << "----------------------------------\n";
    std::cerr << "Total runtime is " << since(total_start) << "ms" << std::endl;

    return 0;
}
