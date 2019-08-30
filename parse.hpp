#pragma once

namespace parse
{
    void assertion(bool flg, std::string msg)
    {
        if (!flg)
        {
            fprintf(stderr, "[Error] %s\n", msg.c_str());
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
        std::array<double, intersection::TIMESLOTS> activeFactors)
    {
        static const std::array<double, intersection::TIMESLOTS> slotLengths {2, 8, 4};

        std::unique_ptr<intersection::Region> region(new intersection::Region());
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

                region->numTargets[time] += targets * activeFactors[time] * slotLengths[time];

                // std::cerr << "In region with mesh " << region->meshId << " at slot " << time << ", of age group " << age << " there are "
                //     << targets << " targets\n";
                // std::cerr << "The activity probability in this slot is " << activeFactors[time] << "\n";
                // std::cerr << "The length of this slot is " << slotLengths[time] << "\n";
                // std::cerr << "Number of targets, summed over ages, is " << region->numTargets[time] << "\n";
            }

            else if (token == "coordinates")
            {
                double x, y;
                while (i + 2 < tokens.size())
                {
                    assertion(parse::string(tokens[i + 1], x), "Wrong coordinates");
                    assertion(parse::string(tokens[i + 2], y), "Wrong coordinates");
                    i += 2;
                    region->polygon.push_back(std::array<double, 2>{x, y});

                    if (x < region->extremeX[0]) { region->extremeX[0] = x; }
                    if (x > region->extremeX[1]) { region->extremeX[1] = x; }
                    if (y < region->extremeY[0]) { region->extremeY[0] = y; }
                    if (y > region->extremeY[1]) { region->extremeY[1] = y; }
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
        std::array<double, intersection::TIMESLOTS> activeFactors)
    {
        std::ifstream stream {filename};
        assertion(stream.is_open(), "Regions geojson file missing!");

        std::string line;
        line.reserve(10000);
        while (std::getline(stream, line))
        {
            auto tokens = geojsonLine(line);
            if (tokens.size() >= 2 && tokens[0] == "type" && tokens[1] == "Feature")
            {
                // this is a route
                regions.push_back(parse::region(tokens, targetAges, activeFactors));
            }
        }
        std::cerr << boost::format("I found %d regions.") % regions.size() << std::endl;
    }

    std::unique_ptr<intersection::Route> route(const std::vector<std::string>& tokens)
    {
        std::unique_ptr<intersection::Route> route(new intersection::Route());
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            auto& token = tokens[i];
            auto& nextToken = tokens[i+1];
            if (token == "RouteID")
            {
                assertion(i + 1 < tokens.size(), "Missing route outputId!");
                route->outputId = nextToken;
            }
            else if (token == "Cost")
            {
                assertion(i + 1 < tokens.size(), "Missing route outputId!");
                parse::string(nextToken, route->cost);
            }
            else if (token == "TZ2_Max")
            {
                assertion(i + 1 < tokens.size(), "Missing TZ2_Max!");
                parse::string(nextToken, route->slotBuses[0]);
            }
            else if (token == "TZ3_Max")
            {
                assertion(i + 1 < tokens.size(), "Missing TZ3_Max!");
                parse::string(nextToken, route->slotBuses[1]);
            }
            else if (token == "TZ4_Max")
            {
                assertion(i + 1 < tokens.size(), "Missing TZ4_Max!");
                parse::string(nextToken, route->slotBuses[2]);
            }
            else if (token == "coordinates")
            {
                double x, y;
                while (i + 2 < tokens.size())
                {
                    assertion(parse::string(nextToken, x), "Wrong coordinates");
                    assertion(parse::string(tokens[i + 2], y), "Wrong coordinates");
                    i += 2;
                    route->polyline.push_back(std::array<double, 2>{x, y});
                }
            }
        }
        for (int i = 0; i < intersection::MAX_BUSES; ++i)
        {
            route->benefits[i] = 0.;
        }
        assertion(route->outputId != "", "Missing route outputId!");
        return route;
    }

    void allRoutes(std::vector<std::unique_ptr<intersection::Route>>& routes, std::string filename)
    {
        std::ifstream stream {filename};
        assertion(stream.is_open(), "Route geojson file missing!");

        std::string line;
        line.reserve(10000);
        while (std::getline(stream, line))
        {
            std::vector<std::string> tokens = geojsonLine(line);
            if (tokens.size() >= 2 && tokens[0] == "type" && tokens[1] == "Feature")
            {
                // this is a route
                routes.push_back(parse::route(tokens));
            }
        }
        std::cerr << boost::format("I found %d routes.") % routes.size() << std::endl;
    }

    std::array<double, intersection::TIMESLOTS> activeFactors(std::string filename)
    {
        std::ifstream factorsStream (filename);
        if (!factorsStream.is_open())
        {
            std::cerr << "Route geojson file missing!" << std::endl;
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
                std::cerr << "Active factor " << facString << " could not be parsed: " << ex.what() << std::endl;
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
            assertion(group.size() == 1, "Age group consists of more than a single character");
            targetAges.push_back(group[0]);
        }

        return targetAges;
    }

    void input(
        std::vector<std::unique_ptr<intersection::Region>>& regions,
        std::vector<std::unique_ptr<intersection::Route>>& routes,
        double& budget)
    {
        clock_t start = clock();

        std::string ageString;
        std::getline(std::cin, ageString);
        std::string targetAges = parse::targetAges(ageString);
        std::cerr << "The target age groups are " << targetAges << std::endl;

        // Line 2: budget
        std::string budgetString;
        std::getline(std::cin, budgetString);
        try
        {
            budget = std::stof(budgetString);
        }
        catch (const std::logic_error& ex) {
            std::cerr << "Budget: " << budgetString << " could not be parsed: " << ex.what() << std::endl;
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

        std::cerr << "Parsing all the paths took " << 1000.*double(clock() - start) / CLOCKS_PER_SEC << "ms" << std::endl;

        start = clock();
        std::array<double, intersection::TIMESLOTS> activeFactors = parse::activeFactors(activeCsv);
        std::cerr << "The activity probabilities for the time slots are " << activeFactors[0] << ", " << activeFactors[1] << ", " << activeFactors[2] << std::endl;
        std::cerr << "Parsing the activity probabilities took " << 1000.*double(clock() - start) / CLOCKS_PER_SEC << "ms" << std::endl;

        start = clock();
        parse::allRegions(regions, populationPath, targetAges, activeFactors);
        std::cerr << "Parsing all the regions took " << 1000.*double(clock() - start) / CLOCKS_PER_SEC << "ms" << std::endl;

        start = clock();
        parse::allRoutes(routes, routePath);
        std::cerr << "Parsing all the routes took " << 1000.*double(clock() - start) / CLOCKS_PER_SEC << "ms" << std::endl;
    }
}
