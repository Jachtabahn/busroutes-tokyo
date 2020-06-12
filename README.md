# Quick Start

On Linux, do
```bash
git clone git@github.com:Jachtabahn/busroutes-tokyo.git
cd busroutes-tokyo/
./run.sh
```

There is also [a detailed explanation](https://github.com/Jachtabahn/busroutes-tokyo/blob/master/explain/explain-algorithm.md) of the employed
optimization algorithm.

# Bus routes problem

There is the city of Tokyo. This city is divided into city regions. Each city region sees a certain number of people of a certain age group in a certain time period of any day.

Then there are companies. A company offers some service to people of certain age groups across all city regions of Tokyo. This company wants to make itself known and decides to create an image advertisement.

But where will this advertising image be placed? On a bus! This bus will travel through many city regions of Tokyo and show the image to many people.

A bus has a bus route. A bus route goes through a certain set of city regions. On each bus route there is a fixed number of buses available in certain time periods of any day. A bus on a bus route can be *bought for a cost*. But then this bus will go off on that bus route and make itself seen by all the people in those city regions.

Now, we are that company. We are interested in people of certain age groups. We have a budget, which is a money sum. Which bus routes at what time slots are we going to allocate? And how many buses on that bus route in that time slot?

# Problem parameters

This program receives input over the standard input stream. This stream should start with lines with the following contents.
1. **TARGET_AGES** is a comma-separated string of characters '1' through '6' where each may be surrounded by spaces,
2. **BUDGET** is an integer string,
3. **POPULATION_GEOJSON** is a path to the GeoJSON file describing region features,
4. **ROUTE_GEOJSON** is a path to a GeoJSON file describing route features,
5. **ACTIVE_CSV** is a path to a CSV file describing activity probabilities.

This program will output lines to the standard output stream where each line is a comma-separated string of
1. **ROUTE_ID** is the id of a route from **ROUTE_GEOJSON**,
2. **COUNT** is the number of wrapping buses to buy on this route.
