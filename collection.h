#include "client.h"
#include <vector>
#include <set>
#include <nlohmann/json.hpp>

class Collection
{
public:
	static const std::set<std::string> sort_keys;

	Collection(const std::string &token, const std::string &user, std::string criteria) :
		mToken(token), mUser(user), mCriteria(criteria)
	{}
	bool init();
	bool downloadNextPage();
	nlohmann::json getJson() const { return mJson; }
	std::vector<std::string> listReleases() const;
	int getCurrentPage() const { return mCurrentPage; }

private:
	bool getNextJson(const std::string &url);
	std::string mToken;
	std::string mUser;
	std::string mCriteria;
	Client mClient;
	nlohmann::json mJson;
	int mCurrentPage = 0;
};
