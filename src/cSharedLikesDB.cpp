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
            throw 5;
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

void cSharedLikesDB::populateRandom(int userCount)
{
    clear();
    std::string q;
    int rc;
    rc = sqlite3_exec(db,
                      "BEGIN TRANSACTION", 0, 0, &dbErrMsg);
    for (int k = 0; k < userCount; k++)
    {
        q = "INSERT INTO user VALUES ( 'user" + std::to_string(k) + "' );";
        rc = sqlite3_exec(db, q.c_str(), 0, 0, &dbErrMsg);
    }
    rc = sqlite3_exec(db,
                      "END TRANSACTION", 0, 0, &dbErrMsg);

    rc = sqlite3_exec(db,
                      "BEGIN TRANSACTION", 0, 0, &dbErrMsg);
    for (int k = 0; k < 100; k++)
    {
        q = "INSERT INTO interest VALUES (" + std::to_string(k) + "," + std::to_string(k) + ");";
        rc = sqlite3_exec(db, q.c_str(), 0, 0, &dbErrMsg);
    }
    rc = sqlite3_exec(db,
                      "END TRANSACTION", 0, 0, &dbErrMsg);

    rc = sqlite3_exec(db,
                      "BEGIN TRANSACTION", 0, 0, &dbErrMsg);
    for (int k = 0; k < userCount; k++)
        for (int l = 0; l < 3; l++)
        {
            q = "INSERT INTO like VALUES (" + std::to_string(k) + "," + std::to_string(rand() % 100) + ");";
            rc = sqlite3_exec(db, q.c_str(), 0, 0, &dbErrMsg);
        }
    rc = sqlite3_exec(db,
                      "END TRANSACTION", 0, 0, &dbErrMsg);
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
    // SQLite uses 1-based indexing
    owner++;

    // track time
    raven::set::cRunWatch aWatcher("DBcluster");

    // determing number of users, to size the output

    int userCount;
    while (sqlite3_step(countUsers) == SQLITE_ROW)
    {
        userCount = sqlite3_column_int(countUsers, 0);
    }
    sqlite3_reset(countUsers);

    std::vector<double> sharedScore(userCount + 1, 0);

    // find interests of owners

    std::string ownerInterests;
    int rc = sqlite3_bind_int(userInterests, 1, owner);
    while ((rc = sqlite3_step(userInterests)) == SQLITE_ROW)
    {
        if (!ownerInterests.empty())
            ownerInterests += ",";
        ownerInterests += std::string((char *)sqlite3_column_text(userInterests, 0));
    }
    sqlite3_reset(userInterests);

    // find users with matching interests
    
    std::string query = "SELECT userid,likeid "
                        "FROM like "
                        "WHERE userid != " +
                        std::to_string(owner) +
                        " AND likeid IN ( " + ownerInterests + " );";
    sqlite3_stmt *match;
    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &match, 0);
    while ((rc = sqlite3_step(match)) == SQLITE_ROW)
    {
        // found a match
        // std::cout << owner
        //           << " and " << sqlite3_column_text(match, 0)
        //           << " like " << sqlite3_column_text(match, 1) << "\n";

        // find weight of shared interest
        int other = sqlite3_column_int(match, 0);
        rc = sqlite3_bind_int(weight, 1, sqlite3_column_int(match, 1));
        rc = sqlite3_step(weight);
        sharedScore[other] += sqlite3_column_double(weight, 0);
        sqlite3_reset(weight);
    }
    sqlite3_reset(match);

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