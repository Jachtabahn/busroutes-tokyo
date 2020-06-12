# Optimization of bus route allocation

This program receives input over the standard input stream. This stream should start with lines with the following contents.
1. **TARGET_AGES** is a comma-separated string of characters '1' through '6' where each may be surrounded by spaces,
2. **BUDGET** is an integer string,
3. **POPULATION_GEOJSON** is a path to the GeoJSON file describing region features,
4. **ROUTE_GEOJSON** is a path to a GeoJSON file describing route features,
5. **ACTIVE_CSV** is a path to a CSV file describing activity probabilities.

This program will output lines to the standard output stream where each line is a comma-separated string of
1. **ROUTE_ID** is the id of a route from **ROUTE_GEOJSON**,
2. **COUNT** is the number of wrapping buses to buy on this route.

There is also [a detailed explanation](https://github.com/Jachtabahn/busroutes-tokyo/blob/master/explain/explain.md) of the employed optimization algorithm.
