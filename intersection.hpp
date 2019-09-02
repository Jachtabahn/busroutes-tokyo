#pragma once

namespace intersection
{
    typedef std::array<double, 2> Point;
    typedef std::array<Point, 2> Box;
}

std::ostream& operator<< (std::ostream& stream, const intersection::Point& point)
{
    return stream << "(" << point[0] << ", " << point[1] << ")";
}

std::ostream& operator<< (std::ostream& stream, const intersection::Box& box)
{
    return stream << "{" << box[0] << ", " << box[1] << "}";
}

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

namespace intersection
{
    // constants
    const int TIMESLOTS = 3;
    const int MAX_BUSES = 4;
    const double EPSILON = 1e-12;
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

    Point operator-(const Point& a, const Point& b)
    {
        return Point{a[0] - b[0], a[1] - b[1]};
    }

    double determinant(const Point& a, const Point& b)
    {
        return a[0]*b[1] - a[1]*b[0];
    }

    bool has(const Point& a, const Point& b, const Point& c, const Point& d)
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

    bool has(const Point& a, const Point& b, const std::vector<Point>& polygon)
    {
        for (size_t i = 0, size = polygon.size()-1; i < size; ++i)
        {
            if (has(a, b, polygon[i], polygon[i + 1])) { return true; }
        }
        return false;
    }

    bool has(const std::vector<Point>& polyline, const std::vector<Point>& polygon)
    {
        for (size_t i = 0, size = polyline.size()-1; i < size; ++i)
        {
            if (has(polyline[i], polyline[i + 1], polygon)) { return true; }
        }
        return false;
    }

    bool has(const std::vector<std::vector<Point>>& polylines, const std::vector<Point>& polygon)
    {
        for (auto it = polylines.begin(), end = polylines.end(); it != end; ++it)
        {
            if (has(*it, polygon)) { return true; }
        }
        return false;
    }

    struct Region
    {
        int meshId = -1;
        std::array<double, TIMESLOTS> targets {0., 0., 0.};

        std::vector<Point> polygon;
        Box box {supremum, infimum};
    };

    struct Route
    {
        int outputId = -1;
        double cost = -1.0;
        std::array<int, TIMESLOTS> buses {0, 0, 0};
        std::vector<double> benefits;

        std::vector<std::vector<Point>> polylines;
        Box box {supremum, infimum};
    };

    bool mayIntersect(const intersection::Box& first, const intersection::Box& second)
    {
        if (first[MIN][X] > second[MAX][X]) { return false; }
        if (first[MIN][Y] > second[MAX][Y]) { return false; }
        if (first[MAX][X] < second[MIN][X]) { return false; }
        if (first[MAX][Y] < second[MIN][Y]) { return false; }
        return true;
    }

    void all(
        std::vector<std::unique_ptr<intersection::Region>>& regions,
        std::vector<std::unique_ptr<intersection::Route>>& routes)
    {
        std::string intersectFile = "intersect.csv";
        std::ofstream intersectStream {intersectFile};

        for (auto routeIt = routes.begin(), routeEnd = routes.end(); routeIt != routeEnd; ++routeIt)
        {
            auto& route = *routeIt;
            auto maxBuses = std::max({route->buses[0], route->buses[1], route->buses[2]});
            route->benefits.resize(maxBuses);
            std::fill(route->benefits.begin(), route->benefits.end(), 0.0);

            intersectStream << route->outputId;

            for (auto regionIt = regions.begin(), regionsEnd = regions.end(); regionIt != regionsEnd; ++regionIt)
            {
                auto& region = *regionIt;

                bool maybe = mayIntersect(region->box, route->box);
                if (!maybe) { continue; }

                // int my_mesh_id = 533935682;
                // if (route->outputId == 60 and region->meshId == my_mesh_id)
                // {
                //     std::clog << "Polygon of region with mesh id " << my_mesh_id << std::endl;
                //     std::clog << region->polygon << std::endl;
                // }

                bool intersects = intersection::has(route->polylines, region->polygon);
                if (!intersects) { continue; }

                intersectStream << "," << region->meshId;

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
            intersectStream << std::endl;

            // int my_route_id = 60;
            // if (route->outputId == my_route_id)
            // {
            //     std::clog << "Polylines of route " << my_route_id << std::endl;
            //     std::clog << route->polylines << std::endl;
            // }
        }
    }
}

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
