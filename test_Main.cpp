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

void run(
    const std::string& age_string,
    const std::string& budget_string,
    const std::string& regions_path,
    const std::string& routes_path,
    const std::string& active_path,
    const double& correct_value
    )
{
    std::clog << "INPUT:\n";
    std::clog << age_string << "\n";
    std::clog << budget_string << "\n";
    std::clog << regions_path << "\n";
    std::clog << routes_path << "\n";
    std::clog << active_path << "\n" << std::endl;

    std::vector<std::unique_ptr<intersection::Region>> regions;
    std::vector<std::unique_ptr<intersection::Route>> routes;
    double budget;
    double cost_gcd;
    double min_cost {std::numeric_limits<double>::infinity()};
    std::map<int, int> allocation;

    parse::input(regions, routes, budget, min_cost, cost_gcd,
        age_string, budget_string, regions_path, routes_path, active_path);

    intersection::all(regions, routes);

    double value = knapsack::optimize(routes, budget, min_cost, cost_gcd, allocation);

    std::clog << "OUTPUT:\n";
    // output our allocation of routes
    for (const auto& iter : allocation)
    {
        std::clog << iter.first << "," << iter.second << "\n";
    }
    std::clog << "(The allocation's value is " << value << ")\n";

    const double EPSILON = 1e-2;
    if (value >= correct_value - EPSILON and value <= correct_value + EPSILON)
    {
        std::clog << "PASSED!\n";
        std::clog << "***************************************************\n";
        std::clog << std::endl;
    }
    else
    {
        std::clog << "FAILED! Value should be " << correct_value << ", but is " << value;
        std::clog << std::endl;
        exit(-1);
    }
}

int main()
{
    clock_t total_start = clock();
    std::clog.precision(std::numeric_limits<double>::max_digits10);

    std::string age_string = "1,2,5";
    std::string budget_string = "10000000";
    std::string regions_path = "./data/Population_1.geojson";
    std::string routes_path = "./data/Route.geojson";
    std::string active_path = "./data/active.csv";
    double correct_value = 4537200.7236800026;
    run(age_string, budget_string, regions_path, routes_path, active_path, correct_value);

    age_string = "1,2,6";
    budget_string = "10000000";
    regions_path = "./data/Population_1.geojson";
    routes_path = "./data/Route.geojson";
    active_path = "./data/active.csv";
    correct_value = 4537907.0137599995;
    run(age_string, budget_string, regions_path, routes_path, active_path, correct_value);

    age_string = "                             1, 3, 5";
    budget_string = "10000000";
    regions_path = "./data/Population_1.geojson";
    routes_path = "./data/Route.geojson";
    active_path = "./data/active.csv";
    correct_value = 4917404.1240000101;
    run(age_string, budget_string, regions_path, routes_path, active_path, correct_value);

    age_string = "1, 2,  3, 4, 5, 6";
    budget_string = "10000000";
    regions_path = "./data/Population_1.geojson";
    routes_path = "./data/Route.geojson";
    active_path = "./data/active.csv";
    correct_value = 745095798538.8537597656;
    run(age_string, budget_string, regions_path, routes_path, active_path, correct_value);

    age_string = "1, 2, 3, 4, 5, 6";
    budget_string = "30000000";
    regions_path = "./data/Population_1.geojson";
    routes_path = "./data/Route.geojson";
    active_path = "./data/active.csv";
    correct_value = 1682337152420.7558593750;
    run(age_string, budget_string, regions_path, routes_path, active_path, correct_value);

    age_string = "1, 2, 3,  6";
    budget_string = "1200000";
    regions_path = "./data/Population_1.geojson";
    routes_path = "./data/Route.geojson";
    active_path = "./data/active.csv";
    correct_value = 702989.8705599997;
    run(age_string, budget_string, regions_path, routes_path, active_path, correct_value);

    age_string = "1, 2, 3,  6";
    budget_string = "100";
    regions_path = "./data/Population_1.geojson";
    routes_path = "./data/Route.geojson";
    active_path = "./data/active.csv";
    correct_value = 0;
    run(age_string, budget_string, regions_path, routes_path, active_path, correct_value);

    std::clog << "Total runtime of all tests is " << since(total_start) << "ms" << std::endl;
    return 0;
}
