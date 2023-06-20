#include <iostream>
#include "cRunWatch.h"
#include "cSharedLikesDBClass.h"

cSharedLikesDBClass::cSharedLikesDBClass()
{
    DB.open("test.dat");

    DB.exec("CREATE TABLE IF NOT EXISTS user ( name );");
    DB.exec("CREATE TABLE IF NOT EXISTS interest (name, weight);");
    DB.exec("CREATE TABLE IF NOT EXISTS like ( userid, likeid );");

    prepare();
}
void cSharedLikesDBClass::prepare()
{
    countUsers = DB.prepare("SELECT count(*) FROM user;");
    countUsers = DB.prepare("SELECT rowid FROM user WHERE rowid != ?1;");
    countUsers = DB.prepare("SELECT likeid FROM like WHERE userid = ?1;");
    countUsers = DB.prepare("SELECT weight FROM interest WHERE rowid = ?1;");
}
cSharedLikesDBClass::~cSharedLikesDBClass()
{

    // sqlite3_close(db);
}
void cSharedLikesDBClass::populateFromTest1()
{
    clear();
    DB.exec(
        "INSERT INTO user VALUES "
        "( 'Alice' ),('Bob'),('Carol'),('David');");

    DB.exec(
        "INSERT INTO interest VALUES "
        "( 'football', 1 ),('tennis',2),('golf',3);");

    DB.exec(
        "INSERT INTO like VALUES "
        "(1,1),(1,2),"
        "(2,2),(2,3),"
        "(3,3),(3,1),"
        "(4,3),(4,1);");
}

void cSharedLikesDBClass::populateRandom(int userCount)
{
    clear();
    std::string q;
    int rc;
    rc = DB.exec(
        "BEGIN TRANSACTION");

    for (int k = 0; k < userCount; k++)
    {
        q = "INSERT INTO user VALUES ( 'user" + std::to_string(k) + "' );";
        rc = DB.exec(q);
    }
    rc = DB.exec(
        "END TRANSACTION");

    rc = DB.exec(
        "BEGIN TRANSACTION");
    for (int k = 0; k < 100; k++)
    {
        q = "INSERT INTO interest VALUES (" + std::to_string(k) + "," + std::to_string(k) + ");";
        rc = DB.exec(q);
    }
    rc = DB.exec(
        "END TRANSACTION");

    rc = DB.exec(
        "BEGIN TRANSACTION");
    for (int k = 0; k < userCount; k++)
        for (int l = 0; l < 3; l++)
        {
            q = "INSERT INTO like VALUES (" + std::to_string(k) + "," + std::to_string(rand() % 100) + ");";
            rc = DB.exec(q);
        }
    rc = DB.exec(
        "END TRANSACTION");
}

std::vector<std::string> cSharedLikesDBClass::userName()
{
    std::vector<std::string> ret;
    ret.push_back("zero-unused");

    DB.exec("SELECT name FROM user;",
            [&](raven::sqliteClassStmt &stmt) -> bool
            {
                ret.push_back(stmt.column_string(0));
                return true;
            });
    return ret;
}

std::vector<double> cSharedLikesDBClass::cluster(int owner)
{
    // SQLite uses 1-based indexing
    owner++;

    // track time
    raven::set::cRunWatch aWatcher("DBcluster");

    // determing number of users, to size the output

    int userCount;
    DB.exec(countUsers,
            [&](raven::sqliteClassStmt &stmt) -> bool
            {
                userCount = stmt.column_int(0);
                return true;
            });

    std::vector<double> sharedScore(userCount + 1, 0);

    // find interests of owner

    std::string ownerInterests;
    DB.bind(userInterests, 1, owner);
    DB.exec(userInterests,
            [&](raven::sqliteClassStmt &stmt) -> bool
            {
                if (!ownerInterests.empty())
                    ownerInterests += ",";
                ownerInterests += stmt.column_string(0);
                return true;
            });

    // find users with matching interests

    std::string query = "SELECT userid,likeid "
                        "FROM like "
                        "WHERE userid != " +
                        std::to_string(owner) +
                        " AND likeid IN ( " + ownerInterests + " );";
    DB.exec(query,
            [&](raven::sqliteClassStmt &stmt) -> bool
            {
                // found a match
                int other = stmt.column_int(0);
                DB.bind(weight, 1, stmt.column_int(1));
                DB.exec(weight,
                        [&](raven::sqliteClassStmt &stmt)
                            -> bool
                        {
                            sharedScore[other] += stmt.column_double(0);
                            return true;
                        });
                return true;
            });

    return sharedScore;
}

void cSharedLikesDBClass::clear()
{
    DB.exec("DELETE FROM user;");
    DB.exec("DELETE FROM interest;");
    DB.exec("DELETE FROM like;");
}