#include "sqlite3.h"

class cSharedLikesDB
{
    public:

    cSharedLikesDB();
    ~cSharedLikesDB();

    void populateFromTest1();

    /// @brief Find cluster around user of shared likes
    /// @param owner owner 1-based index
    /// @return vector of scores indexed ( 1-based ) by users. Score > 0 indicates user in cluster
    std::vector<double> cluster( int owner );

    /// @brief user names
    /// @return vector of user names 1-based index
    std::vector<std::string> userName();

    private:
    sqlite3 *db;
    char *dbErrMsg;
    sqlite3_stmt* selectAllOtherUsers;
    sqlite3_stmt* countUsers;
    sqlite3_stmt* userInterests;
    sqlite3_stmt* weight;

    void clear();
    void prepare();
};