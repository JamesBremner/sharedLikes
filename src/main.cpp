#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "cRunWatch.h"
#include "cSharedLikesDB.h"

struct sharedLikes
{
    std::vector<std::string> userName;
    std::vector<std::pair<std::string, double>> interest; // name, weight
    std::vector<std::pair<int, int>> like;                // user, interest
};

void populateFromdatabase()
{
    // ... this is a stub
}

void populateFromTest1(sharedLikes &data)
{
    data.userName = {"Alice", "Bob", "Carol", "David "};
    data.interest = {
        {"football", 1},
        {"tennis", 2},
        {"golf", 3}};
    data.like = {
        {0, 0}, // Alice likes football
        {0, 1},
        {1, 1},
        {1, 2},
        {2, 2},
        {2, 0},
        {3, 2},
        {3, 0}};
}

void populateRandom(int userCount, sharedLikes &data)
{
    for (int k = 0; k < userCount; k++)
        data.userName.push_back("user" + std::to_string(k));
    for (int k = 0; k < 100; k++)
        data.interest.push_back(std::make_pair(
            "interest " + std::to_string(k), k));
    for (int k = 0; k < userCount; k++)
        for (int l = 0; l < 3; l++)
            data.like.push_back(std::make_pair(
                k, rand() % 100));
}

/// @brief Cluster users by shared interests
/// @param userID to build a cluster around
/// @param data the shared like database extract
/// @return vector of scores indexed by users. Score > 0 indicates user in cluster

std::vector<double> cluster(int userID, sharedLikes &data)
{
    raven::set::cRunWatch aWatcher("cluster");

    std::vector<double> sharedScore(data.userName.size(), 0);

    // loop over user's interests
    for (auto &pui : data.like)
    {
        if (pui.first != userID)
            continue;

        // loop over other users
        for (int other = 0; other < data.userName.size(); other++)
        {
            if (other == userID)
                continue;

            double other_score = 0;

            // loop over other user's interests
            for (auto &poui : data.like)
            {
                if (poui.first != other)
                    continue;
                if (poui.second != pui.second)
                    continue;

                // found a shared interest
                // std::cout << userName[pui.first] << " and " << userName[poui.first]
                //           << " share " << interest[pui.second].first << " score " << interest[pui.second].second << "\n";

                sharedScore[other] += data.interest[poui.second].second;
            }
        }
    }

    return sharedScore;
}

void displayCluster(
    const std::vector<std::string> &userName,
    int owner,
    const std::vector<double> &cluster)
{
    // sort into descending order
    struct classcomp
    {
        bool operator()(const char &lhs, const char &rhs) const
        {
            return lhs > rhs;
        }
    };
    std::multimap<double, std::string, classcomp> scoreMap;
    for (int k = 0; k < userName.size(); k++)
    {
        if (cluster[k] > 0)
            scoreMap.insert(std::make_pair(cluster[k], userName[k]));
    }

    // display the cluster
    std::cout << "\nCluster around " << userName[owner] << "\n";
    for (auto it : scoreMap)
    {
        std::cout << it.second << "\t" << it.first << "\n";
    }
}
main(int argc, char *argv[])
{
    raven::set::cRunWatch::Start();

    if (argc != 2)
    {
        std::cout << "Usage: likes mem | db\n";
        exit(1);
    }
    if (argv[1] == "mem")
    {
        sharedLikes data;

        populateFromTest1(data);
        // populateRandom(100000, data);

        displayCluster(data.userName, 0, cluster(0, data));
        displayCluster(data.userName, 1, cluster(1, data));
        displayCluster(data.userName, 2, cluster(2, data));
        displayCluster(data.userName, 3, cluster(3, data));
    }
    else
    {
        cSharedLikesDB DB;
        //DB.populateFromTest1();
        DB.populateRandom( 100000 );
        auto userName = DB.userName();
        displayCluster(userName, 1, DB.cluster(0));
        displayCluster(userName, 2, DB.cluster(1));
        displayCluster(userName, 3, DB.cluster(2));
        displayCluster(userName, 4, DB.cluster(3));
    }

    raven::set::cRunWatch::Report();
    return 0;
}
