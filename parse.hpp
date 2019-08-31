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

    template<class T>
    bool string(const std::string& s, T& ret)
    {
        std::stringstream sin(s);
        if (sin >> ret) { return true; }
        else { return false; }
    }

    std::unique_ptr<intersection::Region> region(
        const std::vector<std::string>& tokens,
        std::string targetAges,
        std::array<double, intersection::TIMESLOTS> activeFactors,
        const intersection::Box& routesBox)
    {
        static const std::array<double, intersection::TIMESLOTS> slotLengths {2, 8, 4};

        std::unique_ptr<intersection::Region> region = std::make_unique<intersection::Region>();
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            auto& token = tokens[i];
            auto& nextToken = tokens[i + 1];

            if (token == "MESH_ID") { region->meshId = nextToken; }

            else if (token.size() >= 5 && token[0] == 'G' && token[3] == 'T' && token[4] == 'Z')
            {
                char age = token[1];
                auto last = targetAges.end();
                if (std::find(targetAges.begin(), last, age) == last) { continue; }

                int time = token[5] - '0' - 2;
                if (time < 0) { continue; } // ignore the first always-zero time slot
                assertion(time >= 0 && time < intersection::TIMESLOTS, "Invalid time slot at mesh id " + region->meshId);

                int targets;
                parse::string(nextToken, targets);

                region->targets[time] += targets * activeFactors[time] * slotLengths[time];

                // std::clog << "In region with mesh " << region->meshId << " at slot " << time << ", of age group " << age << " there are "
                //     << targets << " targets\n";
                // std::clog << "The activity probability in this slot is " << activeFactors[time] << "\n";
                // std::clog << "The length of this slot is " << slotLengths[time] << "\n";
                // std::clog << "Number of targets, summed over ages, is " << region->targets[time] << "\n";
            }

            else if (token == "coordinates")
            {
                double x, y;
                while (i + 2 < tokens.size())
                {
                    auto success = parse::string(tokens[i + 1], x);
                    assertion(success, "Wrong coordinates");
                    success = parse::string(tokens[i + 2], y);
                    assertion(success, "Wrong coordinates");
                    i += 2;
                    region->polygon.push_back(intersection::Point{x, y});

                    if (x < region->box[0][0]) { region->box[0][0] = x; }
                    if (y < region->box[0][1]) { region->box[0][1] = y; }
                    if (x > region->box[1][0]) { region->box[1][0] = x; }
                    if (y > region->box[1][1]) { region->box[1][1] = y; }
                }

                bool may = intersection::mayIntersect(region->box, routesBox);
                if (!may)
                {
                    // this region does not intersect with any route and is worthless for optimization
                    return nullptr;
                }
            }
        }
        assertion(region->meshId != "", "Missing MESH_ID");
        assertion(region->polygon[0] == region->polygon.back(), "Open Polygon! " + region->meshId);
        return region;
    }

    std::vector<std::string> geojsonLine(std::string& nextLine)
    {
        for (int i = 0; nextLine[i]; ++i)
        {
            if (!(isalpha(nextLine[i]) || isdigit(nextLine[i]) || nextLine[i] == '.' || nextLine[i] == '_'))
            {
                nextLine[i] = ' ';
            }
        }
        std::stringstream stream(nextLine);
        std::vector<std::string> tokens;
        for (std::string token; stream >> token; )
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    void allRegions(
        std::vector<std::unique_ptr<intersection::Region>>& regions,
        std::string filename,
        std::string targetAges,
        std::array<double, intersection::TIMESLOTS> activeFactors,
        intersection::Box& routesBox)
    {
        std::ifstream stream {filename};
        assertion(stream.is_open(), "Regions geojson file missing!");

        std::string line;
        line.reserve(10000);

        double lineTime = 0.;
        double parseTime = 0.;
        double cleanTime = 0.;
        size_t lines = 0;
        clock_t lineStart = clock();
        while (std::getline(stream, line))
        {
            lineTime += since(lineStart);
            ++lines;

            clock_t cleanStart = clock();
            auto tokens = geojsonLine(line);
            cleanTime += since(cleanStart);
            if (tokens.size() >= 2 && tokens[0] == "type" && tokens[1] == "Feature")
            {
                // this is a route
                clock_t parseStart = clock();
                auto region = parse::region(tokens, targetAges, activeFactors, routesBox);
                parseTime += since(parseStart);

                if (region != nullptr)
                {
                    regions.push_back(std::move(region));
                }
            }
            lineStart = clock();
        }
        std::clog << "I found " << regions.size() << " regions." << std::endl;
        std::clog << "Reading all " << lines << " lines from " << filename << " took me "
            << lineTime << "ms" << std::endl;
        std::clog << "Cleaning all lines took " << cleanTime << "ms" << std::endl;
        std::clog << "Just interpreting the regions took " << parseTime << "ms" << std::endl;
    }

    std::unique_ptr<intersection::Route> route(const std::vector<std::string>& tokens, double& minCost, double& costGcd)
    {
        std::unique_ptr<intersection::Route> route = std::make_unique<intersection::Route>();

        for (size_t i = 0; i < tokens.size(); ++i)
        {
            auto& token = tokens[i];
            if (token == "RouteID")
            {
                assertion(i + 1 < tokens.size(), "Missing route outputId!");
                route->outputId = tokens[i+1];
            }
            else if (token == "Cost")
            {
                assertion(i + 1 < tokens.size(), "Missing route outputId!");
                parse::string(tokens[i+1], route->cost);

                if (route->cost < minCost)
                {
                    minCost = route->cost;
                }

                costGcd = static_cast<double>(knapsack::computeGcd(static_cast<int>(costGcd), static_cast<int>(route->cost)));
            }
            else if (token == "TZ2_Max")
            {
                assertion(i + 1 < tokens.size(), "Missing TZ2_Max!");
                parse::string(tokens[i+1], route->buses[0]);
                assertion(route->buses[0] >= 0, "Negative number of buses per time slot!");
            }
            else if (token == "TZ3_Max")
            {
                assertion(i + 1 < tokens.size(), "Missing TZ3_Max!");
                parse::string(tokens[i+1], route->buses[1]);
                assertion(route->buses[1] >= 0, "Negative number of buses per time slot!");
            }
            else if (token == "TZ4_Max")
            {
                assertion(i + 1 < tokens.size(), "Missing TZ4_Max!");
                parse::string(tokens[i+1], route->buses[2]);
                assertion(route->buses[2] >= 0, "Negative number of buses per time slot!");
            }
            else if (token == "coordinates")
            {
                double x, y;
                while (i + 2 < tokens.size())
                {
                    auto success = parse::string(tokens[i + 1], x);
                    assertion(success, "Wrong coordinates");
                    success = parse::string(tokens[i + 2], y);
                    assertion(success, "Wrong coordinates");
                    i += 2;
                    route->polyline.push_back(intersection::Point{x, y});

                    if (x < route->box[0][0]) { route->box[0][0] = x; }
                    if (y < route->box[0][1]) { route->box[0][1] = y; }
                    if (x > route->box[1][0]) { route->box[1][0] = x; }
                    if (y > route->box[1][1]) { route->box[1][1] = y; }
                }
            }
        }
        assertion(route->outputId != "", "Missing route outputId!");
        return route;
    }

    void allRoutes(
        std::vector<std::unique_ptr<intersection::Route>>& routes,
        intersection::Box& routesBox,
        double& minCost,
        double& costGcd,
        std::string filename)
    {
        std::ifstream stream {filename};
        assertion(stream.is_open(), "Route geojson file missing!");

        std::string line;
        line.reserve(10000);
        size_t lines = 0;
        double lineTime = 0.;
        double cleanTime = 0.;
        double parseTime = 0.;
        clock_t lineStart = clock();
        while (std::getline(stream, line))
        {
            ++lines;
            lineTime += since(lineStart);

            clock_t cleanStart = clock();
            std::vector<std::string> tokens = geojsonLine(line);
            cleanTime += since(cleanStart);
            if (tokens.size() >= 2 && tokens[0] == "type" && tokens[1] == "Feature")
            {
                // this is a route
                clock_t parseStart = clock();
                routes.push_back(parse::route(tokens, minCost, costGcd));
                parseTime += since(parseStart);
                auto& route = routes.back();

                if (route->box[0][0] < routesBox[0][0]) { routesBox[0][0] = route->box[0][0]; }
                if (route->box[0][1] < routesBox[0][1]) { routesBox[0][1] = route->box[0][1]; }
                if (route->box[1][0] > routesBox[1][0]) { routesBox[1][0] = route->box[1][0]; }
                if (route->box[1][1] > routesBox[1][1]) { routesBox[1][1] = route->box[1][1]; }
            }
            lineStart = clock();
        }
        std::clog << "I found " << routes.size() << " routes." << std::endl;
        std::clog << "Reading all " << lines << " lines from " << filename << " took me "
            << lineTime << "ms" << std::endl;
        std::clog << "Cleaning all lines took " << cleanTime << "ms" << std::endl;
        std::clog << "Just interpreting the routes took " << parseTime << "ms" << std::endl;
    }

    std::array<double, intersection::TIMESLOTS> activeFactors(std::string filename)
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

    std::string targetAges(std::string ageString)
    {
        std::string targetAges;
        std::stringstream stream(ageString);
        std::string group;
        while(getline(stream, group, ','))
        {
            // remove heading and trailing whitespace
            group.erase(std::remove_if(group.begin(), group.end(), ::isspace), group.end());

            assertion(group.size() == 1, "Age group consists of more than a single character");
            targetAges.push_back(group[0]);
        }

        return targetAges;
    }

    void input(
        std::vector<std::unique_ptr<intersection::Region>>& regions,
        std::vector<std::unique_ptr<intersection::Route>>& routes,
        double& budget,
        double& minCost,
        double& costGcd,
        intersection::Box& routesBox)
    {
        clock_t start = clock();

        // Line 1: target ages
        std::string ageString;
        std::getline(std::cin, ageString);
        std::string targetAges = parse::targetAges(ageString);
        std::clog << "The target age groups are " << targetAges << std::endl;

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

        // Line 3: Population JSON
        std::string populationPath;
        std::getline(std::cin, populationPath);
        while (isspace(populationPath.back())) { populationPath.pop_back(); }

        // Line 4: intersection::Route JSON
        std::string routePath;
        std::getline(std::cin, routePath);
        while (isspace(routePath.back())) { routePath.pop_back(); }

        // Line 5: Active CSV
        std::string activeCsv;
        std::getline(std::cin, activeCsv);
        while (isspace(activeCsv.back())) { activeCsv.pop_back(); }

        start = clock();
        std::array<double, intersection::TIMESLOTS> activeFactors = parse::activeFactors(activeCsv);
        std::clog << "The activity probabilities for the time slots are " << activeFactors[0] << ", " << activeFactors[1] << ", " << activeFactors[2] << std::endl;

        start = clock();
        parse::allRoutes(routes, routesBox, minCost, costGcd, routePath);
        std::clog << "Parsing all the routes took " << since(start) << "ms" << std::endl;

        start = clock();
        parse::allRegions(regions, populationPath, targetAges, activeFactors, routesBox);
        std::clog << "Parsing all the regions took " << since(start) << "ms" << std::endl;
    }
}
