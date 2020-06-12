# Bus challenge

## Parsing of input and formatting output
This includes programs that parse the inputs such as
* region data,
* route data,
* activity data,
* target age groups,
* and budget.

And also programs to output
* route allocations.

## Simplify problem
Initially, we are given
* a set of time slots,
* a set of regions,
* a set of routes,
* for each route, the cost per bus,
* for each route, several polylines,
* for each region, a polygon,
* for each route, the number of available buses per time slot,
* a set of target age groups,
* the number of people of each age group per region per time slot,
* for each time slot, the number of hours inside it (one bus goes per one hour),
* the budget

Write programs to transform them to a route problem as defined below.

## Evaluate all routes
For each route, compute the number of target people it reaches per bus count of that route, that is, the route's benefits. For this, you need to determine which routes intersect which regions, that is, which route lines intersect which region lines.

## Knapsack problem
The knapsack problem consists of
* a set of item types,
* for each item type, a set of items of this type,
* for each item type, the cost of all items of this type,
* for each item, its benefit,
* and the budget.
I'm given a budget and items of different types, and I have to collect a set of items such that the total cost fits within my budget, but the total benefit is as high as possible.

## Clean up
Run your code through every linter you can find on the Internet.

***

## Meaning
My program will be used
> to determine a good set of wrapping buses to buy for advertisement in Tokyo. A good set, while fitting the budget, maximizes the number of reached targets.

My solution to this problem has three main components:
1. Input parsing,
2. Computing intersections,
3. Dynamic programming.

# Input parser
The hardest-to-parse input are the routes and regions. They are in their own files respectively. Those files are in the GeoJSON format plus the addition that each *feature* is on its own line.

Each route feature contains, among other things, the following valueable information:
* the route ID,
* for each of three time slots, the maximum number of wrapping buses available on this route at those times,
* a list of polylines that describe the route on Earth.

Each region feature contains, among other things, the following valueable information:
* the region ID,
* for each target age group and each time slot, the expected number of people of this age in this region at this time,
* a polygon that describes the region on Earth.

I don't parse the lines as JSONs, but instead I parse a more general format that JSON also fulfills:
```python
LINE ::= E "K" W V LINE | $
K ::= <anything except ">
W ::= : | <space>
V ::= <2d array of points P> | <3d array of points P> | <number>
P ::= [ W <number> W , W <number> W ]
```

This way, I read each line character by character and extract that valueable information on the way. This speeds up input parsing from about 330ms down to 100ms ~~on my system~~.

Computing intersections
===============================
Two line segments (a, b) and (c, d) intersect if
* the triplets (a, b, c), (a, b, d) have different orientations, and
* the triplets (c, d, a), (c, d, b) have different orientations.

Given a triplet (a, b, c), its orientation is the signum of the determinant of the two-dimensional matrix (c-b | b-a). This can have values -1, 0, 1, and distinguishes the three cases of a negative, zero, positive angle between the first and second (or second and first?) columns of the matrix. So, in order to determine whether two line segments have an intersection, compute the four orientations and check them as above. In this section, the challenger assumes that each route goes through at least two regions (because otherwise we wouldn't detect that a route, which is completely inside one region, reaches this region).

The computation of the four orientations involves multiplications and subtractions of 64-bit floating point numbers. This is a major operation, and not always necessary since we can often exclude the possibility of an intersection through a prior, simpler check: Before computing the four orientations, we compute the boundary boxes of the compared line segments and check whether, for example, the maximum horizontal coordinate of one is strictly smaller than the minimum horizontal coordinate of the other, which, if the case, convinces us that there can be no intersection between the line segments.

We can do even better: We can compute boundary boxes for entire regions and routes. Then for a given route, when enumerating all regions to find all intersected regions, we skip any region whose boundary box does not intersect with the route's boundary box. That speeds up computation from ca. 2600 milliseconds down to 100 milliseconds.

# Dynamic programming
When computing intersections, we also evaluate each route enriching them with numbers of hit targets with a certain bus count. After the intersections are out of the way, the problem is a Knapsack problem where each
* a route is an item type,
* a route together with a bus count (usually between 1 and 3, but not explicitly upper-bounded) is one item,
* a route's number of maximum buses over all time slots is the count of items of this type,
* the number of hit targets at a certain bus count of a specific route is this item's value,
* the route's cost is the cost of all items of this type.

This is an [unbounded Knapsack problem](https://en.wikipedia.org/wiki/Knapsack_problem) where of each item type there can more than only one item available and each of those items have different costs and benefits. I solve it with a **dynamic programming** algorithm similar to the one described on that page. The main difference is that I pre-compute the greatest common divisor of all costs viewed as integers. Then I compute the maximum value of considering the first so-and-so many items, having so-and-so much budget, where the budget is always the minimum cost plus a multiple of the greatest common divisor, such that this sum is still below the total given budget. There are two iterations of this algorithm. First, I compute these values starting from the bottom. Second, I, going backwards in that values list, recover the multi-set of routes to achieve that maximum total value.

This whole computation takes about 1ms. My program currently needs 180ms on my system and that time breaks down among these components approximately as follows:
* 66% of the time is used for parsing the input,
* 33% of the time is used for computing the intersections and assigning benefits to routes,
* 1% of the time is used for optimization.

For relaxation
--------------------
<a href="http://www.youtube.com/watch?feature=player_embedded&v=3ipEq9m1vgo
" target="_blank"><img src="http://img.youtube.com/vi/3ipEq9m1vgo/0.jpg"
alt="JFla could not be shown" width="480" height="360" border="0" /></a>
