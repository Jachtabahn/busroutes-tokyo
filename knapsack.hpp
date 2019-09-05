#pragma once

namespace knapsack
{
    /**
        Computes the greatest common divisor of x and y using the algorithm of Euclid. One of the numbers
        should be non-zero.

        @param x first number to divide
        @param y second number to divide
        @return the greatest common divisor of x and y
    */
    int compute_gcd(int x, int y)
    {
        while (y > 0)
        {
            auto temp = x % y;
            x = y;
            y = temp;
        }
        return x;
    }

    /**
        Finds an optimal allocation of wrapping buses through dynamic programming.

        The problem is understood as a Knapsack problem where we want to select affordable items so
        as to maximize their usefulness.

        We go through pairs (budget, item) where 'budget' is some number at most the given total budget
        and 'item' is the index of some route in the order in which we read them from the file. Then
        for each such pair, we compute the maximum number of targets reachable with that money and only
        using items that are to the left of 'item' in that ordering. For low 'budget' and 'item', these
        maxima are easily computed, and then we bootstrap from there to compute the maxima for greater
        values, until we finally have the maximum number of targets reachable using our full given
        budget and using all available items.

        Bootstrapping works, because once we know the maxima for all budgets smaller than some budget B,
        and for all items smaller than some item I, then the value of not taking item I is the maximum
        value corresponding to B and I-1. The value of taking item I once is the value corresponding
        to B-c(I) and I-1 plus b(I), where c(I) and b(I) are the cost and benefit of taking that item
        once respectively. The value of taking item I twice is the value corresponding to
        B-c_2(I) and I-1 plus b_2(I), where c_2(I) and b_2(I) are the cost and benefit of taking that
        item twice, and so for the maximum number of available instances of that item. Then we just compare
        all these values and their maximum is the maximum value of having budget B considerung all items up to I.

        The main trick here to reduce running time is to use the greatest common divisor of all
        costs of routes.

        @param routes the vector with the routes, our items
        @param total_budget the total given budget
        @param min_cost the minimum cost across all routes
        @param cost_gcd the greatest common divisor of all route costs
        @param allocation our optimal route allocation
    */
    void optimize(
        const std::vector<std::unique_ptr<intersection::Route>>& routes,
        const double& total_budget,
        const double& min_cost,
        const double& cost_gcd,
        std::map<int, int>& allocation)
    {
        // route.benefits[t] is the benefit of buying the first t+1 buses

        double curBudget;
        std::map<double, std::vector<double>> solution;
        // solution[total_budget][first] is the maximum achievable value considering only items up to the 'first' item and using up to 'total_budget'
        for (size_t first = 0, numRoutes = routes.size(); first < numRoutes; ++first)
        {
            auto& route = routes[first];
            for (curBudget = min_cost; curBudget <= total_budget; curBudget += cost_gcd)
            {
                // this is the value we get when taking none of this item type
                auto maxValue = first > 0 ? solution[curBudget][first-1] : 0.0;

                // we want to figure out how many of this item type we need to maximize the value
                auto takeBudget {curBudget};
                for (size_t takeIndex = 0; takeIndex < route->benefits.size() && takeBudget >= route->cost; ++takeIndex)
                {
                    takeBudget -= route->cost;
                    auto countValue = (first > 0 && takeBudget >= min_cost) ? solution[takeBudget][first-1] : 0.0;
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

        // now we allocate the items to achieve the value solution[total_budget][numRoutes]
        curBudget -= cost_gcd;
        // std::cerr << "Allocating routes to reach " << solution[curBudget][routes.size()-1] << " targets." << std::endl;
        // at the moment, curBudget is the biggest key of solution
        for (int last = routes.size()-1; last >= 0 && curBudget >= min_cost; --last)
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
                countValue = (last > 0 && curBudget >= min_cost) ? solution[curBudget][last-1] : 0.0;
                countValue += route->benefits[takeCount];
                ++takeCount;
            }
            allocation[route->outputId] = takeCount;
        }
    }
}
