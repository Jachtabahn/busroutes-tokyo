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

struct Solution
{
    std::vector<std::unique_ptr<intersection::Region>> regions;
    std::vector<std::unique_ptr<intersection::Route>> routes;
    double budget;

    intersection::Box routesBox {intersection::supremum, intersection::infimum};
    double costGcd;
    double minCost {std::numeric_limits<double>::infinity()};

    std::map<std::string, int> allocation;
} solution;

int main()
{
    std::cerr.precision(std::numeric_limits<double>::max_digits10);

    clock_t total_start = clock();
    clock_t start = clock();
    parse::input(solution.regions, solution.routes, solution.budget, solution.minCost, solution.costGcd, solution.routesBox);
    std::cerr << "Parsing of all input took " << since(start) << "ms" << std::endl;
    std::cerr << "The box enclosing all routes is " << solution.routesBox << std::endl;
    std::cerr << "My total budget is " << solution.budget << std::endl;
    std::cerr << "The minimum cost is " << solution.minCost << std::endl;
    std::cerr << "The greatest common divisor of all costs is " << solution.costGcd << std::endl;

    start = clock();
    intersection::all(solution.regions, solution.routes);
    std::cerr << "Evaluating all routes took " << since(start) << "ms" << std::endl;

    start = clock();
    knapsack::values(solution.routes, solution.budget, solution.minCost, solution.costGcd, solution.allocation);
    std::cerr << "Optimizing my allocation took " << since(start) << "ms" << std::endl;

    /*
    std::cerr << "\n" << std::endl;
    for (size_t i = 0; i < 6; ++i)
    {
        auto& region = *solution.regions[i];
        std::cerr << region;
    }
    std::cerr << "\n" << std::endl;
    for (size_t i = 0, size = 6; i < size; ++i)
    {
        auto& route = *solution.routes[i];
        std::cerr << route;
    }
    */

    // Output our solution
    std::cerr << "\nMy allocation is" << std::endl;
    for (const auto& iter : solution.allocation)
    {
        std::cout << iter.first << "," << iter.second << std::endl;
        std::cerr << iter.first << "," << iter.second << std::endl;
    }

    std::cerr << "----------------------------------\n";
    std::cerr << "Total runtime is " << since(total_start) << "ms" << std::endl;

    return 0;
}
