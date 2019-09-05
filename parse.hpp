#pragma once

namespace parse
{
    /**
        Advances the string position until after the next occurrence of '[' or ']',
        whichever happens first. In the case of an opening bracket, return true,
        in the other case false.

        @param pos the string iterator to be advanced
        @param max_pos the end of the string range
        @return if encountered '[' before any ']' then true, otherwise false
    */
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

    /**
        Advances the string position until one after the next occurrence of the given character.

        @param skip given character
        @param pos the string iterator to be advanced
        @param max_pos the end of the string range
        @return false if the range ended before we found the character, otherwise true
    */
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

    /**
        Parses an integer from the string range [pos, max_pos). The integer may follow any sequence of
        character ':' and ' '. If the first character following that sequence is not a digit, the parsed
        integer is 0.
        Adapted from https://www.geeksforgeeks.org/fast-io-for-competitive-programming

        @param number the integer to store the result
        @param pos the beginning of the string range
        @param max_pos the end of the string range
        @return false if the range ended unexpectedly, otherwise true
    */
    bool int_number(int &number, std::string::const_iterator& pos, const std::string::const_iterator& max_pos)
    {
        // 1) content check
        // 2) iterate
        // 3) bounds check
        // 4) read

        if (pos == max_pos) { return false; }
        int character = *pos;

        for (; character == ' ' or character == ':'; character = *pos)
        {
            ++pos;
            if (pos == max_pos) { return false; }
        }

        bool negative = false;
        if (character == '-')
        {
            negative = true;

            ++pos;
            if (pos == max_pos) { return false; }
            character = *pos;
        }

        number = 0;
        for (; character >= '0' and character <= '9'; character = *pos)
        {
            number = number*10 + character-'0';

            ++pos;
            if (pos == max_pos) { break; }
        }

        if (negative) { number *= -1; }
        return true;
    }

    /**
        Parses a double from the string range [pos, max_pos). The double may follow any sequence of
        character ':' and ' '. If the first character following that sequence is not a digit or '.',
        the parsed double is 0.
        Adapted from https://www.geeksforgeeks.org/fast-io-for-competitive-programming

        @param number the double to store the result
        @param pos the beginning of the string range
        @param max_pos the end of the string range
        @return false if the range ended unexpectedly, otherwise true
    */
    bool double_number(
        double &number,
        std::string::const_iterator& pos,
        const std::string::const_iterator& max_pos)
    {
        // 1) content check
        // 2) iterate
        // 3) bounds check
        // 4) read

        if (pos == max_pos) { return false; }
        int character = *pos;

        for (; character == ' ' or character == ':'; character = *pos)
        {
            ++pos;
            if (pos == max_pos) { return false; }
        }

        bool negative = false;
        if (character == '-')
        {
            negative = true;

            ++pos;
            if (pos == max_pos) { return false; }
            character = *pos;
        }

        double after_comma = 0.0;
        number = 0;
        for (; (character >= '0' and character <= '9') or (character == '.' and after_comma == 0.0);
            character = *pos)
        {
            if (character == '.') { after_comma = 10; }
            else if (after_comma == 0.0) { number = number*10 + character-'0'; }
            else
            {
                number = number + (character-'0')/after_comma;
                after_comma *= 10;
            }

            ++pos;
            if (pos == max_pos) { break; }
        }

        if (negative) { number *= -1; }
        return true;
    }

    /**
        Parses a list of lists of pairs of points given in the JSON syntax.

        @param polylines the vector to store the result
        @param box this will store the boundary box of all parsed points
        @param pos the beginning of the string range
        @param max_pos the end of the string range
        @return false if the range ended unexpectedly, otherwise true
    */
    bool polylines_from_array(
        std::vector<std::vector<intersection::Point>>& polylines,
        intersection::Box& box,
        std::string::const_iterator& pos,
        const std::string::const_iterator& max_pos)
    {
        if (!skip('[', pos, max_pos)) { return false; }

        while (next(pos, max_pos))
        {
            polylines.emplace_back(std::vector<intersection::Point>{});
            std::vector<intersection::Point>& polyline = polylines.back();

            while (next(pos, max_pos))
            {
                polyline.emplace_back(intersection::Point{});
                intersection::Point& point = polyline.back();

                parse::double_number(point[0], pos, max_pos);
                if (!skip(',', pos, max_pos)) { return false; }
                parse::double_number(point[1], pos, max_pos);
                if (!skip(']', pos, max_pos)) { return false; }

                if (point[0] < box[0][0]) { box[0][0] = point[0]; }
                if (point[1] < box[0][1]) { box[0][1] = point[1]; }
                if (point[0] > box[1][0]) { box[1][0] = point[0]; }
                if (point[1] > box[1][1]) { box[1][1] = point[1]; }
            }
        }
        return true;
    }

    /**
        Parses a list of lists of lists of pairs of points given in the JSON syntax.

        @param polygon the vector to store the result
        @param box this will store the boundary box of all parsed points
        @param pos the beginning of the string range
        @param max_pos the end of the string range
        @return false if the range ended unexpectedly, otherwise true
    */
    bool polygon_from_array(
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

                parse::double_number(point[0], pos, max_pos);
                if (!skip(',', pos, max_pos)) { return false; }
                parse::double_number(point[1], pos, max_pos);
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

    /**
        Parses a region GeoJSON object. The parsed region will contain target numbers that have
        already been multiplied with time slot lengths, activity probabilities and filtered through
        age groups.

        @param line the line containing the GeoJSON string
        @param active_factors the activity probabilities of targets in different time slots
        @param target_ages the string of characters describing the age groups of our targets
        @return a smart pointer to a region
    */
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
                if (!parse::int_number(region->meshId, second, max_pos)) { return region; }
            }
            else if (second-1-first == 6 and json_string[0] == 'G'
                and json_string[2] == '_' and json_string[3] == 'T' and json_string[4] == 'Z')
            {
                char& age = json_string[1];
                int time = json_string[5] - '2';

                auto end = target_ages.end();
                if (std::find(target_ages.begin(), end, age) != end and time >= 0)
                {
                    double more_targets;
                    if (!parse::double_number(more_targets, second, max_pos)) { return region; }
                    region->targets[time] += more_targets * active_factors[time] * slot_length[time];
                }
            }
            else if (json_string == "coordinates")
            {
                if (!parse::polygon_from_array(region->polygon, region->box, second, max_pos)) { return region; }
            }

            skip('"', second, max_pos);
            first = second;
        }
        return region;
    }

    /**
        Parses a route GeoJSON object.

        @param line the line containing the GeoJSON string
        @return a smart pointer to a region
    */
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
            if (json_string == "Feature")
            {
                route = std::make_unique<intersection::Route>();
            }
            else if (json_string == "RouteID")
            {
                if (!parse::int_number(route->outputId, second, max_pos)) { return route; }
            }
            else if (json_string == "Cost")
            {
                if (!parse::double_number(route->cost, second, max_pos)) { return route; }
            }
            else if (json_string == "TZ2_Max")
            {
                if (!parse::int_number(route->buses[0], second, max_pos)) { return route; }
            }
            else if (json_string == "TZ3_Max")
            {
                if (!parse::int_number(route->buses[1], second, max_pos)) { return route; }
            }
            else if (json_string == "TZ4_Max")
            {
                if (!parse::int_number(route->buses[2], second, max_pos)) { return route; }
            }
            else if (json_string == "coordinates")
            {
                if (!parse::polylines_from_array(route->polylines, route->box, second, max_pos)) { return route; }
            }

            skip('"', second, max_pos);
            first = second;
        }
        return route;
    }

    /**
        Parses a GeoJSON file containing region data.

        @param regions the vector to store the smart pointers to all the parsed regions
        @param filename path to the GeoJSON file
        @param target_ages contains the target age groups
        @param active_factors activity probabilities, that is, expected ratio of people outside of buildings at different times
        @param routes_boundary the box containing all the route polylines
    */
    void all_regions(
        std::vector<std::unique_ptr<intersection::Region>>& regions,
        std::string filename,
        std::string target_ages,
        std::array<double, intersection::TIMESLOTS> active_factors,
        const intersection::Box& routes_boundary)
    {
        std::ifstream stream {filename};
        if (!stream.is_open())
        {
            std::clog << "Could not find the regions geojson file " << filename << std::endl;
            exit(-1);
        }

        std::string line;
        while (std::getline(stream, line))
        {
            std::unique_ptr<intersection::Region> region = parse::region(line, active_factors, target_ages);

            if (not region or not intersection::may(region->box, routes_boundary))
            {
                continue;
            }

            regions.push_back(std::move(region));
        }
    }

    /**
        Parses a GeoJSON file containing route data.

        @param routes the vector to store the smart pointers to all the parsed routes
        @param min_cost this is the minimum cost of any wrapping bus (needed for optimization)
        @param cost_gcd this is the greatest divisor of the costs of all wrapping buses (needed for optimization)
        @param routes_boundary the box containing all the route polylines
        @param filename path to the GeoJSON file
    */
    void all_routes(
        std::vector<std::unique_ptr<intersection::Route>>& routes,
        double& min_cost,
        double& cost_gcd,
        intersection::Box& routes_boundary,
        std::string filename)
    {
        std::ifstream stream {filename};
        if (!stream.is_open())
        {
            std::clog << "Could not find the routes geojson file " << filename << std::endl;
            exit(-1);
        }

        std::string line;
        while (std::getline(stream, line))
        {
            std::unique_ptr<intersection::Route> new_route = parse::route(line);
            if (!new_route) { continue; }
            routes.push_back(std::move(new_route));

            auto& route = routes.back();
            cost_gcd = static_cast<double>(knapsack::compute_gcd(static_cast<int>(cost_gcd), static_cast<int>(route->cost)));

            if (route->cost < min_cost) { min_cost = route->cost; }

            if (route->box[0][0] < routes_boundary[0][0]) { routes_boundary[0][0] = route->box[0][0]; }
            if (route->box[0][1] < routes_boundary[0][1]) { routes_boundary[0][1] = route->box[0][1]; }
            if (route->box[1][0] > routes_boundary[1][0]) { routes_boundary[1][0] = route->box[1][0]; }
            if (route->box[1][1] > routes_boundary[1][1]) { routes_boundary[1][1] = route->box[1][1]; }
        }
    }

    /**
        Parses activity factors, that is, the expected ratios of people outside of buildings at different times.

        @param filename path to the file with the activity factors in specific CSV format
        @return the activity factors
    */
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
            try { actives[f] = std::stod(facString); }
            catch (const std::logic_error& ex) {
                std::clog << "Active factor " << facString << " could not be parsed: " << ex.what() << std::endl;
                exit(1);
            }
        }
        return actives;
    }

    /**
        Parses target age groups. These are the age groups of people we target with advertisements.

        @param age_string a CSV string with age groups
        @return string of characters where each character represents an age group
    */
    std::string target_ages(std::string age_string)
    {
        std::string target_ages;
        std::stringstream stream(age_string);
        std::string group;
        while(getline(stream, group, ','))
        {
            // remove all the whitespace
            group.erase(std::remove_if(group.begin(), group.end(), ::isspace), group.end());

            if (group.size() != 1)
            {
                std::clog << "Age group \"" << group << "\" does not consist of a single character";
                exit(-1);
            }
            target_ages.push_back(group[0]);
        }

        return target_ages;
    }

    /**
        Parses the entire input, that is, the target age groups, the budget, the activity probabilities,
        the regions and the routes.

        @param regions vector to store the regions
        @param routes vector to store the routes
        @param budget total given budget
        @param min_cost this is the minimum cost of any wrapping bus (needed for optimization)
        @param cost_gcd this is the greatest divisor of the costs of all wrapping buses (needed for optimization)
    */
    void input(
        std::vector<std::unique_ptr<intersection::Region>>& regions,
        std::vector<std::unique_ptr<intersection::Route>>& routes,
        double& budget,
        double& min_cost,
        double& cost_gcd)
    {
        // clock_t start = clock();

        // Line 1: target ages
        std::string age_string;
        std::getline(std::cin, age_string);
        std::string target_ages = parse::target_ages(age_string);
        // std::clog << "The target age groups are " << target_ages << std::endl;

        // Line 2: budget
        std::string budgetString;
        std::getline(std::cin, budgetString);
        try { budget = std::stod(budgetString); }
        catch (const std::logic_error& ex) {
            std::clog << "Budget: " << budgetString << " could not be parsed: " << ex.what() << std::endl;
            exit(1);
        }
        // std::clog << "The total budget is " << budget << std::endl;

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

        // start = clock();
        std::array<double, intersection::TIMESLOTS> active_factors = parse::active_factors(activeCsv);
        // std::clog << "The activity probabilities for the time slots are " << active_factors[0] << ", " << active_factors[1] << ", " << active_factors[2] << std::endl;

        intersection::Box routes_boundary {intersection::supremum, intersection::infimum};

        // start = clock();
        parse::all_routes(routes, min_cost, cost_gcd, routes_boundary, routePath);
        // std::clog << "Parsing all the routes took " << since(start) << "ms" << std::endl;

        // start = clock();
        parse::all_regions(regions, populationPath, target_ages, active_factors, routes_boundary);
        // std::clog << "Parsing all the regions took " << since(start) << "ms" << std::endl;
    }
}
