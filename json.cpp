

/*
{ "type": "Feature",
"properties": { "RouteID": 89,
"Cost": 800000,
"TZ1_Max": 0,
"TZ2_Max": 1,
"TZ3_Max": 1,
"TZ4_Max": 1,
"course_Name": "AL01",
"length": 1507 },
"geometry": { "type": "MultiLineString",
"coordinates": [
[
[ 139.84984, 35.69470417 ],
[ 139.84999417, 35.69461861 ],
[ 139.85028111, 35.69460139 ],
[ 139.85080778, 35.694597 ],
[ 139.85153306, 35.6946425 ],
[ 139.85152709, 35.69506059 ],
[ 139.85152639, 35.69534556 ],
[ 139.85150583, 35.69580917 ],
[ 139.8514925, 35.69590806 ],
[ 139.85110111, 35.6965025 ],
[ 139.850898, 35.69680375 ],
[ 139.850545579999988, 35.69726027 ],
[ 139.85037028, 35.69715028 ],
[ 139.84961977, 35.69639936 ],
[ 139.84899083, 35.69618083 ],
[ 139.84865567, 35.69594045 ],
[ 139.84893347, 35.69561981 ],
[ 139.84928639, 35.69525972 ],
[ 139.84984, 35.69470417 ] ],

[
[ 139.84856549, 35.68965997 ],
[ 139.848328329999987, 35.68977611 ],
[ 139.84835389, 35.69027 ],
[ 139.84845152, 35.69089166 ],
[ 139.84847801, 35.6910603 ],
[ 139.84848806, 35.6919425 ],
[ 139.84846651, 35.69225391 ],
[ 139.848424169999987, 35.69286583 ],
[ 139.84841981, 35.69291944 ],
[ 139.84836139, 35.69363778 ],
[ 139.848432519999989, 35.69367635 ],
[ 139.84984, 35.69470417 ] ] ] } }
*/

#include <cmath>
#include <algorithm>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <memory>

typedef std::array<double, 2> Point;

struct Route
{
    int outputId = -1;
    double cost = -1.0;
    std::array<int, 3> buses {0, 0, 0};

    std::vector<std::vector<Point>> polylines;
};

std::ostream& operator<< (std::ostream& stream, const Route& route)
{
    std::cerr << "----------------- Route -----------------\n";
    stream << "Output id: " << route.outputId << "\n";
    stream << "Cost per bus: " << route.cost << "\n";
    stream << "Available buses: ["
        << route.buses[0] << ", "
        << route.buses[1] << ", "
        << route.buses[2] << "]\n";
    return stream;
}

enum Context
{
    BEFORE_KEY,
    INSIDE_MULTILINESTRING,
    INSIDE_LINESTRING,
    INSIDE_POINT,
    INSIDE_KEY,
    VALUE
};

std::unique_ptr<Route> parseRoute(const std::string& line)
{
    auto route = std::make_unique<Route>();
    double x, y;
    std::string token;
    std::string lastKey;
    Context context {Context::BEFORE_KEY};
    for (auto charIt = line.begin(), lineEnd = line.end(); charIt != lineEnd; ++charIt)
    {
        auto& c = *charIt;

        if (isalpha(c) || isdigit(c) || c == '.' || c == '_')
        {
            token.push_back(c);
            continue;
        }

        switch(c)
        {
        case '"':
            if (context == Context::BEFORE_KEY)
            {
                context = Context::INSIDE_KEY;
            }
            else if (context == Context::INSIDE_KEY)
            {
                std::swap(lastKey, token);
                token.clear();
                context = Context::VALUE;
            }
            break;
        case ',':
        case '}':
            if (lastKey == "RouteID")
            {
                try { route->outputId = std::stoi(token); }
                catch (const std::logic_error& ex)
                {
                    std::clog << "Expect an output id as an integer, instead got " << token << std::endl;
                    exit(-1);
                }
            }
            else if (lastKey == "Cost")
            {
                route->cost = std::stof(token);
            }
            else if (lastKey == "TZ2_Max")
            {
                route->buses[0] = std::stoi(token);
            }
            else if (lastKey == "TZ3_Max")
            {
                route->buses[1] = std::stoi(token);
            }
            else if (lastKey == "TZ4_Max")
            {
                route->buses[2] = std::stoi(token);
            }
            else if (lastKey == "coordinates")
            {
                x = std::stof(token);
            }
            else
            {
                std::clog << "Ignoring key " << lastKey << std::endl;
            }
            token.clear();
            lastKey.clear();
            context = Context::BEFORE_KEY;
            break;
        case '{':
            context = Context::BEFORE_KEY;
            break;
        case ' ':
        case ':':
            break;
        // case '[':
        //     switch (context)
        //     {
        //     case Context::OUTSIDE_COORDINATES:
        //         context = Context::INSIDE_MULTILINESTRING;
        //     case Context::INSIDE_MULTILINESTRING:
        //         context = Context::INSIDE_LINESTRING;
        //     case Context::INSIDE_LINESTRING:
        //         context = Context::INSIDE_POINT;
        //     case Context::INSIDE_POINT:
        //         std::clog << "Unexpected control character inside a point: [" << std::endl;
        //     }
        //     context = Context::INSIDE_MULTILINESTRING;
        // case ']':
        //     switch (context)
        //     {
        //     case Context::OUTSIDE_COORDINATES:
        //         std::clog << "Unexpected control character outside of the coordinates array: [" << std::endl;
        //     case Context::INSIDE_MULTILINESTRING:
        //         context = Context::OUTSIDE_COORDINATES;
        //     case Context::INSIDE_LINESTRING:
        //         context = Context::INSIDE_MULTILINESTRING;
        //     case Context::INSIDE_POINT:
        //         y = std::stoi(token);
        //         route->polylines.back().push_back(Point{x, y});

        //         context = Context::INSIDE_LINESTRING;
        //     }
        default:
            std::clog << "Unknown control character: " << c << std::endl;
            // exit(-1);
        }
    }
    return route;
}

int main()
{

    auto filename = "data/test.json";
    std::ifstream stream {filename};
    if (!stream.is_open())
    {
        std::clog << "Could not open file " << filename << std::endl;
        exit(-1);
    }

    std::string line;
    line.reserve(10000);

    while (std::getline(stream, line))
    {
        auto route = parseRoute(line);
        std::clog << *route;
    }
}



/*
enum Type
{
    KEY,
    STRING,
    DOUBLE,
    INTEGER
};

enum State
{
    DONE_TOKEN,
    READING_TOKEN,
};

enum Context
{
    Context::INSIDE_KEY
    TARGETS
    BUSES
    COST
    X
    Y
    OUTPUT_ID
    VALUE
}

struct Automaton
{
    int key, d, i;
    Point point;
    Context context;
    State state;
    Type tokenType;
}

parseRegion(std::string line)
{
    auto region = std::make_unique<Region>();
    while (c = line.getchar())
    {
        switch (c)
        {
        case ',':
                automaton->state = DONE_TOKEN;
                switch (automaton->tokenType)
                {
                case DOUBLE:
                        automaton->d = std::atof(automaton->token);
                case INTEGER:
                        automaton->i = std::atoi(automaton->token);
                }
                switch (automaton->context)
                {
                case OUTPUT_ID:
                    route->outputId = automaton->i;
                case COST:
                    route->cost = automaton->i;
                case TARGETS:
                    region->targets[automaton->key] += automaton->d;
                case BUSES:
                    route->buses[automaton->key] += automaton->i;
                case X:
                    automaton->point[0] = automaton->d;
                case Y:
                    automaton->point[1] = automaton->d;
                    region->polygon.push_back(automaton->point);
                }
                automaton->context = VALUE;
        case ']':
                automaton->state = DONE_TOKEN;
        case '[':
                automaton->state = DONE_TOKEN;
        case '"':
            switch (automaton->context)
            {
            case Context::INSIDE_KEY:
                automaton->state = DONE_TOKEN;
                automaton->tokenType = KEY;
                switch(automaton->token)
                {
                case "Cost":
                    automaton->context = COST;
                }
            default:
                automaton->state = READING_TOKEN;
                automaton->context = Context::INSIDE_KEY;
            }
        case '{':
                automaton->state = DONE_TOKEN;
        case ':':
        case ' ':
                break;
        case 'a':
                automaton->state = READING_TOKEN;
                token.append(c);
        }
    }
    return region;
}










*/
