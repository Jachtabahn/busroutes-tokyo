# <span style="color:blue">Finding The Best Bus Routes for Advertisement in Tokyo v1.0</span>

## Overview

My solution can be broken down into
1. [input parsing](#input-parsing),
2. [computing intersections](#computing-intersections), and
3. [dynamic programming](#dynamic-programming).

## Input parsing

The most time consuming part of parsing the input is parsing the lines that describe routes and regions. I make this process efficient by reading those lines very carefully, character by character, and extracting valuable information along the way. For example, a number string is read digit by digit, where each digit continuously updates the parsed number.

## Computing intersections

To be able to identify the best routes, it is necessary to know their *benefits*, that is, the number of targets they will probably reach. For that, we need to compute the set of regions that a given route visits. Here, I assume that no route fully stays within one particular region, in which case the set of visited regions is the same as the set of intersected regions.

To verify that a route intersects a region, I verify that some line segment of the route intersects some line segment of the region. I use a variation of [this algorithm](https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/), which uses the notion of an *orientation between two vectors* to determine this. In my variation of the algorithm I assume that one of the four considered orientations is non-zero, that is, that no route segment lies in the same line as some region segment.

But to exclude the possibility of an intersection between a route and a region, I use three techniques as described in the following three paragraphs.

The computation of the four orientations involves multiplications and subtractions of 64-bit floating point numbers. This is a major operation, and not always necessary since we can often exclude the possibility of an intersection through a much simpler check: Before computing the four orientations, we look at the boundary boxes of the compared line segments, that is, the boxes that tightly surround their line segments on all four sides. Then we check whether the compared line segments' boxes intersect, by comparing, for example, the maximum X coordinate of the first boundary box with the minimum X coordinate of the second, and if the former is strictly smaller, then the line segments cannot intersect, and we can rule out an intersection without computing the four orientations.

We can do even better: We can compute boundary boxes for entire regions and routes. Then for a given route, when enumerating all regions to find the intersections, we skip every region whose boundary box does not intersect with the route's boundary box.

We can do even more better: After parsing a new region from the regions file, we do not save this region if it does not intersect with the boundary box of *all routes*. To be able to do this, we compute the lowest and highest X and Y coordinates across all routes when parsing them first.

Each route has several benefits: There is one benefit per bus count. The bus count goes from one to the maximum number of wrapping buses available on this route across all time slots of the day. For example, the benefit of having two buses on this route is the number of targets reached by two buses of that route. Note that the benefit of using three buses is not necessarily the benefit of using one bus times three, because it might be that only one out of three time slots has all three buses available.

We initialize all the benefits to 0. Once we have determined that a given route intersects a given region, we update all the benefits of this route with this region's information.

## Dynamic programming

Once we know the routes' benefits, we can forget that routes are routes and that there were any regions altogether. Now, all we have that is useful for us to optimally solve the task can be seen as a [Knapsack problem](https://en.wikipedia.org/wiki/Knapsack_problem):
1. There are *items* (counts of buses on a route).
2. Each item has an *item type* (a route).
3. Each item has a positive *item cost* (the route cost times the bus count).
4. Each item has a positive *benefit* (the bus count's benefit as computed above).
5. There is a positive, *total budget*.

I'm given a budget and limited numbers of items of different types, each with its own cost and benefit. My task is to collect items of distinct types such that their total cost fits within my total budget, whilst lifting their total benefit as high up as possible.

For purposes of explanation, let's assume that there is exactly one item per item type available.

The Knapsack problem can be solved step by step: First, I look at the first item, and take it or don't take it. Then I look at the second item, and again, take it or don't take it, up until the last item. When we are already at the last item and we know it, the decision is easy: Take it if you can. But if we are at the second-to-last item, the decision is already harder: We can't take this item just because we can, as then we might not be able to take the last item which could bring us much more benefit.

These considerations lead to the following idea: At that last step, we know exactly how much total benefit we can possibly get, if we take the item and continue (namely the item's benefit) and how much total benefit we can possibly get, if we don't take the item and continue (namely zero). This makes our decision easy: If we can afford it, we buy the item. It would be useful to have this information available at that second-to-last step as well, that is, on the one hand the total benefit we can possibly get, if we take the item and continue, and on the other hand the total benefit we can possibly get, if we don't take the item and continue.

Now, remember that actually we have item types and there might be more than one item of the same type. The problem can still be solved step by step as above, except that at every step, I look at an item type, and decide whether I want to buy some item of this type and, if so, which one. Now, for every item type, it is useful to know what the total benefit is that we can possibly get by not buying any item of this type and continue, and for each item of this type, the total benefit of buying it and continue. Once we have this information, we can easily decide on an alternative: Do whatever gets us the greatest, total benefit.

So, our algorithm can be broken down into these two phases:

1. Compute that *useful information* for all item types from the first to the last. Note that the greatest of the total benefits of the alternatives at the first step (namely not taking any items of the first item type, and for all items of this type, taking it) is the greatest, total benefit that can be achieved in this task altogether.

2. Use that information to extract the set of items of distinct types that achieves the greatest, achievable, total benefit. We have already seen how the second step works: Just look at all alternatives of a given item type, pick the one that promises the greatest, total benefit and do the corresponding action, that is, omit this item type or take that particular item of this type, and then continue up along the chain until after the last decision has been made.

The first phase is the hard part. Consider, for example, the case where we are at the second-to-last item. Buying or not buying the item results in a different budget at the last step before making the last decision. So, in order to know the two total benefits of buying or not buying the second-to-last item, we must have previously computed the total benefit of being at the last step, *whilst having each of those two different budgets*.

If you think about, the problem gets even more complicated. For example, we might have bought every second item before arriving at the last item. Now our budget is the difference between the total budget and all those items' costs, that is in general the difference between the total budget and some combination of all the possible costs. One way to solve the problem is to compute the total benefit of being at the last step for every possible, positive budget, that is at most the given total budget. Then, whatever the budgets turn out to be when we consider our alternatives at the second-to-last step, our *useful information* will contain total benefits for all those budgets.

But this is extremely inefficient. It's because most of those budgets will never be queried. Remember: The important budgets we are left with after making a choice are always a difference between the given total budget and some combination of all the costs. But we *can* know all combinations of all the costs! We could compute all the combined item costs, that are at most the given total budget and then subtract them from the given total budget. These are the budgets that matter.

But we can go on thinking: Assume that all costs are integers. Then, a combination of costs will always be divided by the greatest common divisor of all the involved costs. So, we will not miss the important budgets if we just consider all (small) multiples of the costs' greatest common divisor, and subtract these from the given total budget.

Furthermore, consider the case when some budget is smaller than the minimum cost across all items. Then, for every item type, every possible alternative will always have total benefit 0, so we don't need to consider these budgets.

Going one more step further, notice that we don't need to subtract those multiples from the given total budget, if the greatest common divisor of all costs also divides the given total budget. But even if it doesn't, it is ok to omit the subtractions, because all they do is shift the budgets by some neglible amount.

I don't explicitly compute all the combined costs of items, but instead I use the second approach. The *relevant budgets* are of the form: Minimum cost plus some multiple of the greatest common divisor of all costs, such that this sum stays at most the given total budget.

For every item type and every relevant budget, I compute the total benefits for all the possible decisions and then continuing along to the next item type. Hereby, the item types, meaning the routes, are ordered in the same way as they are read from disk. Thereafter, I recover the items to achieve the greatest, achievable, total benefit.
