#pragma once

namespace intersection
{
    typedef std::array<double, 2> Point;
    typedef std::array<Point, 2> Box;
}

namespace intersection
{
    // constants
    const int TIMESLOTS = 3;
    const double EPSILON = 1e-12;

    // boundary box constants
    const Point infimum {-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()};
    const Point supremum {std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
    const int MIN = 0;
    const int MAX = 1;
    const int X = 0;
    const int Y = 1;

    int sign(double x)
    {
        if (x < -EPSILON) { return -1; }
        return x > EPSILON ? 1 : 0;
    }

    /**
        Compute the difference between the two points, that is, the direction looking at a from b.

        @param a first point, where we look at
        @param b second point, where we stand at
        @return the direction from b to a
    */
    Point operator-(const Point& a, const Point& b)
    {
        return Point{a[0] - b[0], a[1] - b[1]};
    }

    /**
        Computes the determinant of the matrix whose columns are given by a and b.

        @param a left column
        @param b right column
        @return the determinant of the two-dimensional square matrix
    */
    double determinant(const Point& a, const Point& b)
    {
        return a[0]*b[1] - a[1]*b[0];
    }

    /**
        Tests whether the segment (a, b) intersects the segment (c, d). This function
        is not always correct if both segments lie in the same line. I still use it here
        because the challenge's scoring function uses it.

        @param a first point defining first segment
        @param b second point defining first segment
        @param c first point defining second segment
        @param d second point defining second segment
        @return true if first segment intersects second segment (except if both lie in the same line)
    */
    bool must(const Point& a, const Point& b, const Point& c, const Point& d)
    {
        if (std::min(a[0], b[0]) > std::max(c[0], d[0])) { return false; }
        if (std::min(a[1], b[1]) > std::max(c[1], d[1])) { return false; }
        if (std::max(a[0], b[0]) < std::min(c[0], d[0])) { return false; }
        if (std::max(a[1], b[1]) < std::min(c[1], d[1])) { return false; }

        Point ab = b-a;
        Point cd = d-c;
        return sign(determinant(c-b, ab)) * sign(determinant(d-b,ab)) <= 0
            and sign(determinant(a-d,cd)) * sign(determinant(b-d,cd)) <= 0;
    }

    bool must(const Point& a, const Point& b, const std::vector<Point>& polygon)
    {
        for (size_t i = 0, size = polygon.size()-1; i < size; ++i)
        {
            if (must(a, b, polygon[i], polygon[i + 1])) { return true; }
        }
        return false;
    }

    bool must(const std::vector<Point>& polyline, const std::vector<Point>& polygon)
    {
        for (size_t i = 0, size = polyline.size()-1; i < size; ++i)
        {
            if (must(polyline[i], polyline[i + 1], polygon)) { return true; }
        }
        return false;
    }

    bool must(const std::vector<std::vector<Point>>& polylines, const std::vector<Point>& polygon)
    {
        for (auto it = polylines.begin(), end = polylines.end(); it != end; ++it)
        {
            if (must(*it, polygon)) { return true; }
        }
        return false;
    }

    /**
        Represents a region read from a GeoJSON file. The targets array contains target numbers that have
        already been multiplied with time slot lengths, activity probabilities and filtered through
        age groups. The box consists of two points, the lower left corner and upper right corner of a box
        surrounding the region's polygon.
    */
    struct Region
    {
        int meshId = -1;
        std::array<double, TIMESLOTS> targets {0., 0., 0.};

        std::vector<Point> polygon;
        Box box {supremum, infimum};
    };

    /**
        Represents a route read from a GeoJSON file. The buses array contains the numbers of wrapping
        buses available at different time slots. The t-th entry in the benefits
        vector is the benefit of buying t+1 wrapping buses on this route, that is, the number of targets
        those buses together will hit. So the maximal length of the benefits array is the maximum
        number of wrapping buses available on this route over all time slots.
    */
    struct Route
    {
        int outputId = -1;
        double cost = -1.0;
        std::array<int, TIMESLOTS> buses {0, 0, 0};
        std::vector<double> benefits;

        std::vector<std::vector<Point>> polylines;
        Box box {supremum, infimum};
    };

    /**
        Checks whether the two given boundary boxes intersect.

        @param first first boundary box
        @param second second boundary box
        @return true if they intersect in at least one point
    */
    bool may(const intersection::Box& first, const intersection::Box& second)
    {
        if (first[MIN][X] > second[MAX][X]) { return false; }
        if (first[MIN][Y] > second[MAX][Y]) { return false; }
        if (first[MAX][X] < second[MIN][X]) { return false; }
        if (first[MAX][Y] < second[MIN][Y]) { return false; }
        return true;
    }

    /**
        Computes all the routes' benefits, which is done by computing
        all the intersections between routes and regions.

        The main trick here is to use the routes' and regions' precomputed boundary boxes: For a given
        route and region, we first check whether their corresponding boundary boxes intersect. If they do,
        only then we iterate through all segments of both objects to precisely look for a real intersection.
        But very often, even the boundary boxes don't intersect and so we don't need to do the expensive computation.

        This cuts down the running from 2600 milliseconds to 100 milliseconds
        on my system, while identifying the same intersections.

        @param regions all the regions
        @param routes all the routes which we want to evaluate
    */
    void all(
        std::vector<std::unique_ptr<intersection::Region>>& regions,
        std::vector<std::unique_ptr<intersection::Route>>& routes)
    {
        for (auto routeIt = routes.begin(), routeEnd = routes.end(); routeIt != routeEnd; ++routeIt)
        {
            auto& route = *routeIt;
            auto maxBuses = std::max({route->buses[0], route->buses[1], route->buses[2]});
            route->benefits.resize(maxBuses);
            std::fill(route->benefits.begin(), route->benefits.end(), 0.0);

            for (auto regionIt = regions.begin(), regionsEnd = regions.end(); regionIt != regionsEnd; ++regionIt)
            {
                auto& region = *regionIt;

                bool maybe = intersection::may(region->box, route->box);
                if (!maybe) { continue; }

                bool intersects = intersection::must(route->polylines, region->polygon);
                if (!intersects) { continue; }

                for (int s = 0; s < TIMESLOTS; ++s)
                {
                    if (route->buses[s] == 0) { continue; }
                    for (int b = 0; b < maxBuses; ++b)
                    {
                        auto actualCount = std::min(b+1, route->buses[s]);
                        route->benefits[b] += actualCount*region->targets[s];
                    }
                }
            }
        }
    }
}

/**
    Overloads the output operator for Point.

    @param stream the output stream
    @param point point to output
    @return stream the modified output stream
*/
std::ostream& operator<< (std::ostream& stream, const intersection::Point& point)
{
    return stream << "(" << point[0] << ", " << point[1] << ")";
}

/**
    Overloads the output operator for Box.

    @param stream the output stream
    @param box box to output
    @return stream the modified output stream
*/
std::ostream& operator<< (std::ostream& stream, const intersection::Box& box)
{
    return stream << "{" << box[0] << ", " << box[1] << "}";
}

/**
    Overloads the output operator for a vector of double numbers.

    @param stream the output stream
    @param vec the vector to output
    @return stream the modified output stream
*/
std::ostream& operator<< (std::ostream& stream, const std::vector<double>& vec)
{
    stream << "[";
    if (vec.size() == 0) { return stream << "]"; }

    auto end = vec.end()-1;
    for (auto it = vec.begin(); it != end; ++it)
    {
        stream << *it << ", ";
    }
    return stream << *end << "]";
}

/**
    Overloads the output operator for a vector of points.

    @param stream the output stream
    @param vec the vector to output
    @return stream the modified output stream
*/
std::ostream& operator<< (std::ostream& stream, const std::vector<intersection::Point>& vec)
{
    stream << "[";
    if (vec.size() == 0) { return stream << "]"; }

    auto end = vec.end()-1;
    for (auto it = vec.begin(); it != end; ++it)
    {
        stream << *it << ", ";
    }
    return stream << *end << "]";
}

/**
    Overloads the output operator for a vector of vectors of points.

    @param stream the output stream
    @param vec the vector to output
    @return stream the modified output stream
*/
std::ostream& operator<< (std::ostream& stream, const std::vector<std::vector<intersection::Point>>& vec)
{
    stream << "[";
    if (vec.size() == 0) { return stream << "]"; }

    auto end = vec.end()-1;
    for (auto it = vec.begin(); it != end; ++it)
    {
        stream << *it << ",\n";
    }
    return stream << *end << "]";
}

/**
    Overloads the output operator for a route.

    @param stream the output stream
    @param route the route to output
    @return stream the modified output stream
*/
std::ostream& operator<< (std::ostream& stream, const intersection::Route& route)
{
    std::cerr << "----------------- Route -----------------\n";
    stream << "Output id: " << route.outputId << "\n";
    stream << "Cost per bus: " << route.cost << "\n";
    stream << "Available buses: ["
        << route.buses[0] << ", "
        << route.buses[1] << ", "
        << route.buses[2] << "]\n";
    stream << "Numbers of targets hit (depending on #buses): " << route.benefits << "\n";
    stream << "Number of polylines: " << route.polylines.size() << "\n";
    stream << "The polylines are the following:" << "\n";
    for (auto it = route.polylines.begin(), end = route.polylines.end(); it != end; ++it)
    {
        stream << *it << "\n";
    }
    stream << "Bounding box: " << route.box << "\n";

    return stream;
}

/**
    Overloads the output operator for a region.

    @param stream the output stream
    @param region the region to output
    @return stream the modified output stream
*/
std::ostream& operator<< (std::ostream& stream, const intersection::Region& region)
{
    stream << "----------------- Region -----------------\n";
    stream << "Mesh id: " << region.meshId << "\n";
    stream << "Number of targets in time slots:"
        << " " << region.targets[0]
        << " " << region.targets[1]
        << " " << region.targets[2]
        << "\n";
    stream << "Number of polygon points: " << region.polygon.size() << "\n";
    stream << "Bounding box: " << region.box << "\n";

    return stream;
}
