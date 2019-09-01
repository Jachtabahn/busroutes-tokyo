#pragma once

namespace parse
{
    void assertion(bool check, std::string msg)
    {
        if (!check)
        {
            std::clog << msg << std::endl;
            exit(-1);
        }
    }

    // true if there is a next [ with at least one more character after pos
    // false if there is a ] before any [, or the input max_pos
    bool next(std::string::const_iterator& pos, const std::string::const_iterator& max_pos)
    {
        for (; pos != max_pos; ++pos)
        {
            switch (*pos)
            {
            case '[': // new inner array
                ++pos;
                return true;
            case ']': // old outer array
                ++pos;
                return false;
            }
        }
        return false;
    }

    // true if char found and skipped
    bool skip(char skip, std::string::const_iterator& pos, const std::string::const_iterator& max_pos)
    {
        for (; pos != max_pos; ++pos)
        {
            if (*pos == skip)
            {
                ++pos;
                return true;
            }
        }
        return false;
    }

    bool parse_int(int& i, std::string::const_iterator& pos, const std::string::const_iterator& max_pos)
    {
        for (; pos != max_pos; ++pos)
        {
            if (*pos != ' ' && *pos != ':') { break; }
        }

        std::string s{pos, max_pos};
        try
        {
            i = std::stoi(s);
            return true;
        }
        catch (const std::logic_error& ex)
        {
            std::clog << "Expect an integer, instead got " << s << std::endl;
            return false;
        }
    }

    bool parse_double(double& d, std::string::const_iterator& pos, const std::string::const_iterator& max_pos)
    {
        for (; pos != max_pos; ++pos)
        {
            if (*pos != ' ' && *pos != ':') { break; }
        }

        std::string s{pos, max_pos};
        try
        {
            d = std::stof(s);
            return true;
        }
        catch (const std::logic_error& ex)
        {
            std::clog << "Expect a double, instead got " << s << std::endl;
            return false;
        }
    }

    bool parse_polylines(
        std::vector<intersection::Point>& polyline,
        intersection::Box& box,
        std::string::const_iterator& pos,
        const std::string::const_iterator& max_pos)
    {
        if (!skip('[', pos, max_pos)) { return false; }

        while (next(pos, max_pos))
        {
            // polylines.emplace_back(std::vector<intersection::Point>{});
            // std::vector<intersection::Point>& polyline = polylines.back();

            while (next(pos, max_pos))
            {
                polyline.emplace_back(intersection::Point{});
                intersection::Point& point = polyline.back();

                parse_double(point[0], pos, max_pos);
                if (!skip(',', pos, max_pos)) { return false; }
                parse_double(point[1], pos, max_pos);
                if (!skip(']', pos, max_pos)) { return false; }

                if (point[0] < box[0][0]) { box[0][0] = point[0]; }
                if (point[1] < box[0][1]) { box[0][1] = point[1]; }
                if (point[0] > box[1][0]) { box[1][0] = point[0]; }
                if (point[1] > box[1][1]) { box[1][1] = point[1]; }
            }
        }
        return true;
    }

    bool parse_polygon(
        std::vector<intersection::Point>& polygon,
        intersection::Box& box,
        std::string::const_iterator& pos,
        const std::string::const_iterator& max_pos)
    {
        if (!skip('[', pos, max_pos)) { return false; }
        if (!skip('[', pos, max_pos)) { return false; }

        while (next(pos, max_pos))
        {
            while (next(pos, max_pos))
            {
                polygon.emplace_back(intersection::Point{});
                intersection::Point& point = polygon.back();

                parse_double(point[0], pos, max_pos);
                if (!skip(',', pos, max_pos)) { return false; }
                parse_double(point[1], pos, max_pos);
                if (!skip(']', pos, max_pos)) { return false; }

                if (point[0] < box[0][0]) { box[0][0] = point[0]; }
                if (point[1] < box[0][1]) { box[0][1] = point[1]; }
                if (point[0] > box[1][0]) { box[1][0] = point[0]; }
                if (point[1] > box[1][1]) { box[1][1] = point[1]; }

            }
        }
        if (!skip(']', pos, max_pos)) { return false; }
        return true;
    }

    std::unique_ptr<intersection::Region> region(
        const std::string& line,
        std::array<double, intersection::TIMESLOTS> active_factors,
        const std::string& target_ages)
    {
        static const std::array<double, intersection::TIMESLOTS> slot_length {2, 8, 4};

        std::unique_ptr<intersection::Region> region;
        auto max_pos = line.cend();
        std::string::const_iterator first = line.cbegin();
        skip('"', first, max_pos);
        std::string::const_iterator second = first;
        while (skip('"', second, max_pos))
        {
            std::string json_string {first, second-1};
            if (json_string == "Feature")
            {
                region = std::make_unique<intersection::Region>();
            }
            else if (json_string == "MESH_ID")
            {
                if (!parse_int(region->meshId, second, max_pos)) { return region; }
            }
            else if (second-1-first == 6 && json_string[0] == 'G'
                && json_string[2] == '_' && json_string[3] == 'T' && json_string[4] == 'Z')
            {
                char& age = json_string[1];
                int time = json_string[5] - '2';

                auto end = target_ages.end();
                if (std::find(target_ages.begin(), end, age) != end && time >= 0)
                {
                    double more_targets;
                    if (!parse_double(more_targets, second, max_pos)) { return region; }
                    region->targets[time] += more_targets * active_factors[time] * slot_length[time];
                }
            }
            else if (json_string == "coordinates")
            {
                if (!parse_polygon(region->polygon, region->box, second, max_pos)) { return region; }
            }

            skip('"', second, max_pos);
            first = second;
        }
        return region;
    }

    std::unique_ptr<intersection::Route> route(const std::string& line)
    {
        std::unique_ptr<intersection::Route> route;
        auto max_pos = line.cend();
        std::string::const_iterator first = line.cbegin();
        skip('"', first, max_pos);
        std::string::const_iterator second = first;
        while (skip('"', second, max_pos))
        {
            std::string json_string {first, second-1};
            // std::clog << "Found json string: " << json_string << std::endl;
            if (json_string == "Feature")
            {
                route = std::make_unique<intersection::Route>();
            }
            else if (json_string == "RouteID")
            {
                if (!parse_int(route->outputId, second, max_pos)) { return route; }
            }
            else if (json_string == "Cost")
            {
                if (!parse_double(route->cost, second, max_pos)) { return route; }
            }
            else if (json_string == "TZ2_Max")
            {
                if (!parse_int(route->buses[0], second, max_pos)) { return route; }
            }
            else if (json_string == "TZ3_Max")
            {
                if (!parse_int(route->buses[1], second, max_pos)) { return route; }
            }
            else if (json_string == "TZ4_Max")
            {
                if (!parse_int(route->buses[2], second, max_pos)) { return route; }
            }
            else if (json_string == "coordinates")
            {
                if (!parse_polylines(route->polyline, route->box, second, max_pos)) { return route; }
            }

            skip('"', second, max_pos);
            first = second;
        }
        return route;
    }

    void all_regions(
        std::vector<std::unique_ptr<intersection::Region>>& regions,
        std::string filename,
        std::string target_ages,
        std::array<double, intersection::TIMESLOTS> active_factors)
    {
        std::ifstream stream {filename};
        if (!stream.is_open())
        {
            std::clog << "Could not find the regions geojson file " << filename << std::endl;
            exit(-1);
        }

        std::string line;
        line.reserve(10000);

        size_t lines = 0;
        double readTime = 0.;
        double parseTime = 0.;
        clock_t read_start = clock();
        while (std::getline(stream, line))
        {
            readTime += since(read_start);
            ++lines;

            clock_t parseStart = clock();
            std::unique_ptr<intersection::Region> region = parse::region(line, active_factors, target_ages);
            parseTime += since(parseStart);

            if (region)
            {
                regions.push_back(std::move(region));
            }
            read_start = clock();
        }
        std::clog << "I found " << regions.size() << " regions." << std::endl;
        std::clog << "Reading all " << lines << " lines from " << filename << " took me "
            << readTime << "ms" << std::endl;
        std::clog << "Just interpreting the regions took " << parseTime << "ms" << std::endl;
    }

    void all_routes(
        std::vector<std::unique_ptr<intersection::Route>>& routes,
        double& minCost,
        double& costGcd,
        std::string filename)
    {
        std::ifstream stream {filename};
        if (!stream.is_open())
        {
            std::clog << "Could not find the routes geojson file " << filename << std::endl;
            exit(-1);
        }

        std::string line;
        line.reserve(10000);
        size_t lines = 0;
        double readTime = 0.;
        double parseTime = 0.;
        clock_t read_start = clock();
        while (std::getline(stream, line))
        {
            ++lines;
            readTime += since(read_start);

            // this is a route
            clock_t parseStart = clock();
            std::unique_ptr<intersection::Route> new_route = parse::route(line);
            parseTime += since(parseStart);
            if (!new_route) { continue; }
            routes.push_back(std::move(new_route));

            auto& route = routes.back();
            costGcd = static_cast<double>(knapsack::computeGcd(static_cast<int>(costGcd), static_cast<int>(route->cost)));

            if (route->cost < minCost) { minCost = route->cost; }

            read_start = clock();
        }
        std::clog << "I found " << routes.size() << " routes." << std::endl;
        std::clog << "Reading all " << lines << " lines from " << filename << " took me "
            << readTime << "ms" << std::endl;
        std::clog << "Just interpreting the routes took " << parseTime << "ms" << std::endl;
    }

    std::array<double, intersection::TIMESLOTS> active_factors(std::string filename)
    {
        std::ifstream factorsStream (filename);
        if (!factorsStream.is_open())
        {
            std::clog << "Route geojson file missing!" << std::endl;
            exit(1);
        }

        std::string facString;
        facString.reserve(100);
        factorsStream >> facString;
        std::getline(factorsStream, facString, ',');
        std::array<double, intersection::TIMESLOTS> actives;
        for (int f = 0; std::getline(factorsStream, facString, ','); ++f)
        {
            try { actives[f] = std::stof(facString); }
            catch (const std::logic_error& ex) {
                std::clog << "Active factor " << facString << " could not be parsed: " << ex.what() << std::endl;
                exit(1);
            }
        }
        return actives;
    }

    std::string target_ages(std::string ageString)
    {
        std::string target_ages;
        std::stringstream stream(ageString);
        std::string group;
        while(getline(stream, group, ','))
        {
            // remove heading and trailing whitespace
            group.erase(std::remove_if(group.begin(), group.end(), ::isspace), group.end());

            assertion(group.size() == 1, "Age group consists of more than a single character");
            target_ages.push_back(group[0]);
        }

        return target_ages;
    }

    void input(
        std::vector<std::unique_ptr<intersection::Region>>& regions,
        std::vector<std::unique_ptr<intersection::Route>>& routes,
        double& budget,
        double& minCost,
        double& costGcd)
    {
        clock_t start = clock();

        // Line 1: target ages
        std::string ageString;
        std::getline(std::cin, ageString);
        std::string target_ages = parse::target_ages(ageString);
        std::clog << "The target age groups are " << target_ages << std::endl;

        // Line 2: budget
        std::string budgetString;
        std::getline(std::cin, budgetString);
        try
        {
            budget = std::stof(budgetString);
        }
        catch (const std::logic_error& ex) {
            std::clog << "Budget: " << budgetString << " could not be parsed: " << ex.what() << std::endl;
            exit(1);
        }
        std::clog << "The total budget is " << budget << std::endl;

        // Line 3: path to regions geojson
        std::string populationPath;
        std::getline(std::cin, populationPath);
        while (isspace(populationPath.back())) { populationPath.pop_back(); }

        // Line 4: path to routes geojson
        std::string routePath;
        std::getline(std::cin, routePath);
        while (isspace(routePath.back())) { routePath.pop_back(); }

        // Line 5: path to activity probabilities csv
        std::string activeCsv;
        std::getline(std::cin, activeCsv);
        while (isspace(activeCsv.back())) { activeCsv.pop_back(); }

        start = clock();
        std::array<double, intersection::TIMESLOTS> active_factors = parse::active_factors(activeCsv);
        std::clog << "The activity probabilities for the time slots are " << active_factors[0] << ", " << active_factors[1] << ", " << active_factors[2] << std::endl;

        start = clock();
        parse::all_routes(routes, minCost, costGcd, routePath);
        std::clog << "Parsing all the routes took " << since(start) << "ms" << std::endl;

        start = clock();
        parse::all_regions(regions, populationPath, target_ages, active_factors);
        std::clog << "Parsing all the regions took " << since(start) << "ms" << std::endl;
    }
}
