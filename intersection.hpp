namespace intersection
{
    typedef std::array<double, 2> Point;
    typedef std::array<Point, 2> Box;

    // constants
    const int TIMESLOTS = 3;
    const int MAX_BUSES = 4;
    const double EPSILON = 1e-8;
    const Point infimum = Point{-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()};
    const Point supremum = Point{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
    const int MIN = 0;
    const int MAX = 1;
    const int X = 0;
    const int Y = 1;

    int sign(double x) {
        if (x < -EPSILON) {
            return -1;
        }
        return x > EPSILON;
    }


    Point operator + (const Point& a, const Point& b) {
        return Point{a[0] + b[0], a[1] + b[1]};
    }

    Point operator - (const Point& a, const Point& b) {
        return Point{a[0] - b[0], a[1] - b[1]};
    }

    Point operator * (const Point& a, const double& b) {
        return Point{a[0] * b, a[1] * b};
    }

    Point operator / (const Point& a, const double& b) {
        return Point{a[0] / b, a[1] / b};
    }

    double det(const Point& a, const Point& b) {
        return a[0] * b[1] - a[1] * b[0];
    }

    double dot(const Point& a, const Point& b) {
        return a[0] * b[0] + a[1] * b[1];
    }

    bool operator == (const Point& a, const Point& b) {
        return std::fabs(a[0] - b[0]) < EPSILON && std::fabs(a[1] - b[1]) < EPSILON;
    }


    bool has(const Point& a, const Point& b, const Point& c, const Point& d) {
        return sign(det(c - a, b - a)) * sign(det(d - a, b - a)) <= 0 && sign(det(a - c, d - c)) * sign(det(b - c, d - c)) <= 0;
    }

    bool has(const Point& a, const Point& b, const std::vector<Point>& polygon) {
        for (size_t i = 0; i + 1 < polygon.size(); ++i) {
            if (has(a, b, polygon[i], polygon[i + 1])) {
                return true;
            }
        }
        return false;
    }

    bool has(const std::vector<Point>& polyline, const std::vector<Point>& polygon) {
        for (size_t i = 0; i + 1 < polyline.size(); ++i) {
            if (has(polyline[i], polyline[i + 1], polygon)) {
                return true;
            }
        }
        return false;
    }

    struct Region
    {
        std::string meshId = "NONE";
        std::array<double, TIMESLOTS> numTargets {0., 0., 0.};

        std::vector<std::array<double, 2>> polygon;
        Box box {supremum, infimum};
    };

    struct Route
    {
        std::string outputId = "NONE";
        double cost = -1.0;
        std::array<int, TIMESLOTS> busesPerSlot {0, 0, 0};
        std::vector<double> benefits;

        std::vector<std::array<double, 2>> polyline;
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
        for (auto routeIt = routes.begin(), routeEnd = routes.end(); routeIt != routeEnd; ++routeIt)
        {
            auto& route = *routeIt;
            auto maxBuses = std::max({route->busesPerSlot[0], route->busesPerSlot[1], route->busesPerSlot[2]});
            route->benefits.resize(maxBuses);
            std::fill(route->benefits.begin(), route->benefits.end(), 0.0);

            for (auto regionIt = regions.begin(), regionsEnd = regions.end(); regionIt != regionsEnd; ++regionIt)
            {
                auto& region = *regionIt;

                bool maybe = mayIntersect(region->box, route->box);
                if (!maybe) { continue; }

                bool intersects = intersection::has(route->polyline, region->polygon);
                if (!intersects) { continue; }

                for (int s = 0; s < TIMESLOTS; ++s)
                {
                    for (int b = 0; b < route->busesPerSlot[s]; ++b)
                    {
                        route->benefits[b] += region->numTargets[s];
                    }
                }
            }
        }
    }
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
    if (vec.size() > 1)
    {
        for (auto it = vec.begin(), end = vec.end()-1; it != end; ++it)
        {
            stream << *it << ", ";
        }
    }
    stream << vec[vec.size()-1] << "]";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, const intersection::Route& route)
{
    // std::cerr << "----------------- Route -----------------\n";
    stream << route.outputId << "\n";
    // stream << "Output id: " << route.outputId << "\n";
    // stream << "Cost per bus: " << route.cost << "\n";
    // stream << "Available buses: ["
    //     << route.busesPerSlot[0] << ", "
    //     << route.busesPerSlot[1] << ", "
    //     << route.busesPerSlot[2] << "]\n";
    // stream << "Numbers of targets hit (depending on #buses): " << route.benefits << "\n";
    // stream << "Number of polyline points: " << route.polyline.size() << "\n";
    // stream << "Bounding box: " << route.box << "\n";
    stream << route.benefits << "\n";

    return stream;
}

std::ostream& operator<< (std::ostream& stream, const intersection::Region& region)
{
    stream << "----------------- Region -----------------\n";
    stream << "Mesh id: " << region.meshId << "\n";
    stream << "Number of targets in time slots:"
        << " " << region.numTargets[0]
        << " " << region.numTargets[1]
        << " " << region.numTargets[2]
        << "\n";
    stream << "Number of polygon points: " << region.polygon.size() << "\n";
    stream << "Minimum point of the polygon: " << region.box[0] << "\n"
        << "Maximum point of the polygon: " << region.box[1] <<"\n";

    return stream;
}
