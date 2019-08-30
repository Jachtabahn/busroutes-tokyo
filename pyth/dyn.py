
from json import load
from random import random, seed

with open('../data/Route.geojson') as f:
    routes = load(f)

budget = 100
costs = [feat['properties']['Cost'] for feat in routes['features']]
costs = [cost/1e5 for cost in costs]
min_cost = min(costs)

num_items = len(costs)
seed(3)
benefits = [random()*10 for _ in range(num_items)]

gcd = 2
cost_sums = list(set(costs +
    [min_cost+r for r in range(gcd, num_items, gcd) if min_cost+r <= budget]))

print("The number of items is {}".format(num_items))
print("The budget is {}".format(budget))
print("The costs are {}".format(costs))
print("The gcd is {}".format(gcd))
print("The minimum cost is {}".format(min_cost))
print("The benefits are {}".format(benefits))
print("All the cost sums are {}".format(cost_sums))

solution = dict()
for virtual_budget in cost_sums:
    cheap_items = [i for i in range(num_items) if costs[i] <= virtual_budget]
    best_value = 0
    best_item = None
    for i in cheap_items:
        reduced_virtual_budget = virtual_budget - costs[i]
        value = benefits[i]
        if reduced_virtual_budget >= min_cost:
            value += benefits[i] + solution[reduced_virtual_budget][1]
        if best_value < value:
            best_value = value
            best_item = i

    lower_budget = virtual_budget-gcd
    if lower_budget >= min_cost:
        below_pair = solution[lower_budget]
        if best_value > below_pair[1]:
            new_itemset = [best_item] + below_pair[0]
            solution[virtual_budget] = (new_itemset, best_value)
        else:
            solution[virtual_budget] = below_pair
    else:
        solution[virtual_budget] = ([best_item], best_value)

print("Values:")
print(solution)
