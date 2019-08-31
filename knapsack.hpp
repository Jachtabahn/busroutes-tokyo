namespace knapsack
{
    int computeGcd(int x, int y)
    {
        while (y > 0)
        {
            auto temp = x % y;
            x = y;
            y = temp;
        }
        return x;
    }

    void values(
        const std::vector<std::unique_ptr<intersection::Route>>& routes,
        const double& totalBudget,
        const double& minCost,
        const double& costGcd,
        std::map<std::string, int>& allocation)
    {
        // route.benefits[t] is the benefit of buying the first t+1 buses

        double curBudget;
        std::map<double, std::vector<double>> solution;
        // solution[totalBudget][first] is the maximum achievable value considering only items up to the 'first' item and using up to 'totalBudget'
        for (size_t first = 0, numRoutes = routes.size(); first < numRoutes; ++first)
        {
            auto& route = routes[first];
            for (curBudget = minCost; curBudget <= totalBudget; curBudget += costGcd)
            {
                // this is the value we get when taking none of this item type
                auto maxValue = first > 0 ? solution[curBudget][first-1] : 0.0;

                // we want to figure out how many of this item type we need to maximize the value
                auto takeBudget {curBudget};
                for (size_t takeIndex = 0; takeIndex < route->benefits.size() && takeBudget >= route->cost; ++takeIndex)
                {
                    takeBudget -= route->cost;
                    auto countValue = (first > 0 && takeBudget >= minCost) ? solution[takeBudget][first-1] : 0.0;
                    countValue += route->benefits[takeIndex];
                    if (countValue > maxValue) { maxValue = countValue; }
                }

                if (solution[curBudget].size() == 0)
                {
                    solution[curBudget].resize(numRoutes, 0.0);
                }
                solution[curBudget][first] = maxValue;
            }
        }

        // now we allocate the items to achieve the value solution[totalBudget][numRoutes]
        curBudget -= costGcd;
        std::cerr << "Allocating routes to reach " << solution[curBudget][routes.size()-1] << " targets." << std::endl;
        // at the moment, curBudget is the biggest key of solution
        for (int last = routes.size()-1; last >= 0 && curBudget >= minCost; --last)
        {
            // this is the value we want to reach
            auto maxValue = solution[curBudget][last];

            // this is the value we get when taking none of this item type
            auto countValue = last > 0 ? solution[curBudget][last-1] : 0.0;
            if (maxValue <= countValue) { continue; }

            // now we want to figure out how many of this item type we need to reach the maximum value
            auto& route = routes[last];
            int takeCount = 0;
            while (maxValue > countValue)
            {
                curBudget -= route->cost; // this must not make curBudget negative because maxValue is reachable
                countValue = (last > 0 && curBudget >= minCost) ? solution[curBudget][last-1] : 0.0;
                countValue += route->benefits[takeCount];
                ++takeCount;
            }
            allocation[route->outputId] = takeCount;
        }
    }
}
