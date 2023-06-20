#include "sqliteClass.h"

class cSharedLikesDBClass
{
    public:

    cSharedLikesDBClass();
    ~cSharedLikesDBClass();

    void populateFromTest1();
    void populateRandom(int userCount);

    /// @brief Find cluster around user of shared likes
    /// @param owner owner 1-based index
    /// @return vector of scores indexed ( 1-based ) by users. Score > 0 indicates user in cluster
    std::vector<double> cluster( int owner );

    /// @brief user names
    /// @return vector of user names 1-based index
    std::vector<std::string> userName();

    private:

    raven::sqliteClass DB;
    raven::sqliteClassStmt* selectAllOtherUsers;
    raven::sqliteClassStmt* countUsers;
    raven::sqliteClassStmt* userInterests;
    raven::sqliteClassStmt* weight;

    void clear();
    void prepare();
};