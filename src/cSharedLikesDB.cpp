#include <iostream>
#include "cRunWatch.h"
#include "cSharedLikesDB.h"

cSharedLikesDB::cSharedLikesDB()
    : dbErrMsg(0)
{
    int rc = sqlite3_open("test.dat", &db);
    if (rc)
    {
        std::cout << "cannot open database\n";
        exit(2);
    }

    rc = sqlite3_exec(db,
                      "CREATE TABLE IF NOT EXISTS user ( name );",
                      0, 0, &dbErrMsg);
    rc = sqlite3_exec(db,
                      "CREATE TABLE IF NOT EXISTS interest (name, weight);",
                      0, 0, &dbErrMsg);
    rc = sqlite3_exec(db,
                      "CREATE TABLE IF NOT EXISTS like ( userid, likeid );",
                      0, 0, &dbErrMsg);

    prepare();
    int fb = rc;
}
void cSharedLikesDB::prepare()
{
    try
    {
        int rc = sqlite3_prepare_v2(db,
                                    "SELECT count(*) FROM user;",
                                    -1, &countUsers, 0);
        if (rc)
            throw 1;
        rc = sqlite3_prepare_v2(db,
                                "SELECT rowid FROM user WHERE rowid != ?1;",
                                -1, &selectAllOtherUsers, 0);
        if (rc)
            throw 2;
        rc = sqlite3_prepare_v2(db,
                                "SELECT likeid FROM like WHERE userid = ?1;",
                                -1, &userInterests, 0);
        if (rc)
            throw 3;
        rc = sqlite3_prepare_v2(db,
                                "SELECT weight FROM interest WHERE rowid = ?1;",
                                -1, &weight, 0);
        if (rc)
            throw 4;
    }
    catch (int p)
    {
        std::cout << "Bad prepare " << p << "\n";
        exit(3);
    }
}
cSharedLikesDB::~cSharedLikesDB()
{

    sqlite3_close(db);
}
void cSharedLikesDB::populateFromTest1()
{
    clear();
    int rc = sqlite3_exec(db,
                          "INSERT INTO user VALUES "
                          "( 'Alice' ),('Bob'),('Carol'),('David');",
                          0, 0, &dbErrMsg);
    rc = sqlite3_exec(db,
                      "INSERT INTO interest VALUES "
                      "( 'football', 1 ),('tennis',2),('golf',3);",
                      0, 0, &dbErrMsg);
    rc = sqlite3_exec(db,
                      "INSERT INTO like VALUES "
                      "(1,1),(1,2),"
                      "(2,2),(2,3),"
                      "(3,3),(3,1),"
                      "(4,3),(4,1);",
                      0, 0, &dbErrMsg);
}

std::vector<std::string> cSharedLikesDB::userName()
{
    std::vector<std::string> ret;
    ret.push_back("zero-unused");
    sqlite3_stmt *pq;
    int rc = sqlite3_prepare_v2(db,
                                "SELECT name FROM user;",
                                -1, &pq, 0);
    while (sqlite3_step(pq) == SQLITE_ROW)
        ret.push_back(std::string((char *)sqlite3_column_text(pq, 0)));
    sqlite3_finalize(pq);
    return ret;
}

std::vector<double> cSharedLikesDB::cluster(int owner)
{
    owner++;

    raven::set::cRunWatch aWatcher("DBcluster");

    int userCount;
    while (sqlite3_step(countUsers) == SQLITE_ROW)
    {
        userCount = sqlite3_column_int(countUsers, 0);
    }
    sqlite3_reset(countUsers);

    std::vector<double> sharedScore(userCount + 1, 0);

    std::vector<int> ownerInterests;
    int rc = sqlite3_bind_int(userInterests, 1, owner);
    while (sqlite3_step(userInterests) == SQLITE_ROW)
    {
        ownerInterests.push_back(sqlite3_column_int(userInterests, 0));
    }
    sqlite3_reset(userInterests);

    // loop over other users
    rc = sqlite3_bind_int(selectAllOtherUsers, 1, owner);
    while (sqlite3_step(selectAllOtherUsers) == SQLITE_ROW)
    {
        // loop over interests of other user
        int other = sqlite3_column_int(selectAllOtherUsers, 0);
        sqlite3_bind_int(userInterests, 1, other);
        while (sqlite3_step(userInterests) == SQLITE_ROW)
        {
            int interest = sqlite3_column_int(userInterests, 0);
            if (std::find(
                    ownerInterests.begin(), ownerInterests.end(),
                    interest) != ownerInterests.end())
            {
                // found a match
                rc = sqlite3_bind_int(weight, 1, interest);
                rc = sqlite3_step(weight);
                sharedScore[other] += sqlite3_column_double(weight, 0);
                sqlite3_reset(weight);

                //std::cout << owner << " and " << other << " like " << interest << "\n";
            }
        }
        sqlite3_reset(userInterests);
    }
    sqlite3_reset(selectAllOtherUsers);

    return sharedScore;
}
void cSharedLikesDB::clear()
{
    int rc = sqlite3_exec(db,
                      "DELETE FROM user;",
                      0, 0, &dbErrMsg);
    rc = sqlite3_exec(db,
                      "DELETE FROM interest;",
                      0, 0, &dbErrMsg);
    rc = sqlite3_exec(db,
                      "DELETE FROM like;",
                      0, 0, &dbErrMsg);
}