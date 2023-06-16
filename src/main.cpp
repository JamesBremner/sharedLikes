#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "cRunWatch.h"

std::vector<std::string> userName;
std::vector<std::pair<std::string, double>> interest; // name, weight
std::vector<std::pair<int, int>> like;                // user, interest

void populateFromdatabase()
{
    // ... this is a stub
}

void populateFromTest1()
{
    userName = {"Alice", "Bob", "Carol", "David "};
    interest = {
        {"football", 1},
        {"tennis", 2},
        {"golf", 3}};
    like = {
        {0, 0}, // Alice likes football
        {0, 1},
        {1, 1},
        {1, 2},
        {2, 2},
        {2, 0},
        {3, 2},
        {3, 0}};
}

void populateRandom( int userCount )
{
    for (int k = 0; k < userCount; k++)
        userName.push_back("user" + std::to_string(k));
    for (int k = 0; k < 100; k++)
        interest.push_back(std::make_pair(
            "interest " + std::to_string(k), k));
    for (int k = 0; k < userCount; k++)
        for (int l = 0; l < 3; l++)
            like.push_back(std::make_pair(
                k, rand() % 100));
}

    void cluster(int userID)
    {
        raven::set::cRunWatch aWatcher("cluster");
        std::map<int, double> sharedScoreMap;

        // loop over user's interests
        for (auto &pui : like)
        {
            if (pui.first != userID)
                continue;

            // loop over other users
            for (int other = 0; other < userName.size(); other++)
            {
                if (other == userID)
                    continue;

                double other_score = 0;

                // loop over other user's interests
                for (auto &poui : like)
                {
                    if (poui.first != other)
                        continue;
                    if (poui.second != pui.second)
                        continue;

                    // found a shared interest
                    // std::cout << userName[pui.first] << " and " << userName[poui.first]
                    //           << " share " << interest[pui.second].first << " score " << interest[pui.second].second << "\n";

                    other_score += interest[poui.second].second;
                }
                if (other_score)
                    sharedScoreMap[other] += other_score;
            }
        }

        // display the cluster 
        // std::cout << "\n"
        //           << userName[userID] << "'s cluster\n";
        // for (auto it : sharedScoreMap)
        // {
        //     std::cout << userName[it.first] << "\t" << it.second << "\n";
        // }
    }
    main()
    {
        raven::set::cRunWatch::Start();

        // populateFromTest1();
        populateRandom(100000);

        cluster(0);
        cluster(10);
        cluster(20);
        cluster(30);

        raven::set::cRunWatch::Report();
        return 0;
    }
