namespace knapsack
{
    typedef std::array<size_t, double> Index;

    void values(
        const std::vector<std::unique_ptr<intersection::Route>>& routes,
        const double& budget,
        std::map<std::string, int>& allocations)
    {
        std::vector<double> subBudgets;

        std::vector<Index> indices;
        std::vector<double> values;
        auto& numRoutes = routes.size();
        for (int r = 0; r < numRoutes; ++r)
        {
            auto& route = routes[r];
            for (auto curBudgetIt = subBudgets.begin(), end = subBudgets.end(); curBudgetIt != end; ++curBudgetIt)
            {
                auto& curBudget = *curBudgetIt;
                // auto maxValue =
            }
        }


        // solution = dict()
        // # solution[(first, budget)] is the maximum achievable value considering only items up to the 'first' item, using up to 'budget'
        // for first in range(num_items):
        //     item = items[first]
        //     for cur_budget in sub_budgets:
        //         # this is the value we get when taking none of this item type
        //         max_value = solution[(first-1, cur_budget)] if first > 0 else .0

        //         # we want to figure out how many of this item type we need to maximize the value
        //         for take_count in range(1, len(item.benefits)+1):
        //             if cur_budget >= take_count*item.cost:
        //                 take_budget = cur_budget-take_count*item.cost
        //                 count_value = solution[(first-1, take_budget)] if first > 0 and take_budget >= min_cost else .0
        //                 count_value += sum(item.benefits[:take_count])
        //                 if count_value > max_value:
        //                     max_value = count_value
        //         solution[(first, cur_budget)] = max_value
    }
}
