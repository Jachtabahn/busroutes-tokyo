from json import load

with open('../data/Route.geojson') as f:
    routes = load(f)

total_budget = 100
costs = [feat['properties']['Cost'] for feat in routes['features']]
output_ids = [feat['properties']['RouteID'] for feat in routes['features']]
costs = [cost/1e5 for cost in costs]
num_items = len(costs)
min_cost = min(costs)

print("The total budget is {}".format(total_budget))
print("The minimum cost is {}".format(min_cost))

with open("../data/benefits.json") as benefits_file:
    benefits_dict = load(benefits_file)

class Item:
    def __init__(self, output_id, cost, benefits):
        self.output_id = output_id
        self.cost = cost

        # this is a list of non-increasing benefits indicating the benefit if the i-th item of this type
        # (only len(benefits) of these items are available)
        self.benefits = benefits
items = [Item(output_id, cost, benefits_dict[str(output_id)]) for output_id, cost in zip(output_ids, costs)]
print("The number of items is {}".format(num_items))
print("The items are {}".format(items))

def computeGcd(x, y):
    while (y):
        x, y = y, x % y
    return x

# determine the greatest common divisor of the costs
unique_costs = list(set(costs))
gcd = int(unique_costs[0])
for c in unique_costs[1:]:
    gcd = computeGcd(gcd, int(c))
print("The greatest common divisor is {}".format(gcd))

# determine all sums of costs, starting at the minimal cost, taking a few too many cost sums for simplification
sub_budgets = set(unique_costs)
r = min_cost + gcd
while r <= total_budget:
    sub_budgets.add(r)
    r += gcd
sub_budgets = list(set(unique_costs + [min_cost+r for r in range(gcd, int(total_budget-min_cost)+gcd, gcd) if min_cost+r <= total_budget]))
print("All the cost sums are {}".format(sub_budgets))
max_budget = max(sub_budgets)
print("The maximal sub budget is {}".format(max_budget))

solution = dict()
# solution[(first, budget)] is the maximum achievable value considering only items up to the 'first' item, using up to 'budget'
for first in range(num_items):
    item = items[first]
    for cur_budget in sub_budgets:
        # this is the value we get when taking none of this item type
        max_value = solution[(first-1, cur_budget)] if first > 0 else .0

        # we want to figure out how many of this item type we need to maximize the value
        for take_count in range(1, len(item.benefits)+1):
            if cur_budget >= take_count*item.cost:
                take_budget = cur_budget-take_count*item.cost
                count_value = solution[(first-1, take_budget)] if first > 0 and take_budget >= min_cost else .0
                count_value += sum(item.benefits[:take_count])
                if count_value > max_value:
                    max_value = count_value
        solution[(first, cur_budget)] = max_value
print("The solution of the Knapsack problem is", solution)

taking = dict()
# taking is the list of items we take to achieve the value solution[(num_items, total_budget)]
cur_budget = max_budget
for last in range(num_items)[::-1]:
    # this is the value we want to reach
    max_value = solution[(last, cur_budget)]

    # this is the value we get when taking none of this item type
    count_value = solution[(last-1, cur_budget)] if last > 0 else .0
    if max_value <= count_value:
        continue

    # now we want to figure out how many of this item type we need to reach the maximum value
    item = items[last]
    take_count = 0
    take_budget = cur_budget
    while max_value > count_value:
        take_count += 1
        take_budget -= item.cost
        count_value = solution[(last-1, take_budget)] if last > 0 and take_budget >= min_cost else .0
        count_value += sum(item.benefits[:take_count])
    taking[last] = take_count

    # now reduce our current budget by their cost
    cur_budget = take_budget
    if cur_budget < min_cost: break

print("I allocate the items as follows:", dict([(output_ids[t], count) for t, count in taking.items()]))

taken_cost = sum([items[t].cost*count for t, count in taking.items()])
taken_benefit = sum([sum(items[t].benefits[:count]) for t, count in taking.items()])
print("The benefit of the taken items is {}".format(taken_benefit))
print("The cost of the taken items is {}".format(taken_cost))
