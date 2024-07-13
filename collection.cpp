#include "collection.h"
#include <iostream>

using json = nlohmann::json;
using namespace std;

const std::set<std::string> Collection::sort_keys = {
	"label", "artist", "title", "catno", "format", "rating", "added", "year" };

bool Collection::init()
{
	/* ID 0, the `All` folder, which cannot have releases added to it, and
	   ID 1, the `Uncategorized` folder which requires a token to access.
	 */
	auto folder = mToken.empty() ? "0" : "1";

	if (!mClient.init(mToken))
		return false;

	auto url = "https://api.discogs.com/users/"+mUser+"/collection/folders/"+folder+"/releases?sort="+mCriteria;

	if (!getNextJson(url))
		return false;

	std::cout << "Collection of " << mUser << " listed by " << mCriteria << ":\n";

	/* Parse JSON as descibed in: 
	 * www.discogs.com/developers#page:user-collection,header:user-collection-collection-items-by-folder
	 */
	try
	{
		std::cout << "\titems = " << mJson["pagination"]["items"] << std::endl;
		std::cout << "\tpages = " << mJson["pagination"]["pages"] << std::endl;
		std::cout << "\tper_page = " << mJson["pagination"]["per_page"] << std::endl;
		std::cout << "\tnext = " << mJson["pagination"]["urls"]["next"] << std::endl;	
	}
	catch (json::exception &e)
	{
		std::cerr << __func__ <<": Error: " << e.what() << std::endl;
		return false;
	}

	return true;
}

bool Collection::getNextJson(const string &url)
{
	auto response = mClient.getNext(url);

	if (response.empty())
		return false;

	try
	{
		mJson = json::parse(response);
		return (!mJson.is_null());
	}
	catch (json::exception &e)
	{
		std::cerr << __func__ <<": Error: " << e.what() << std::endl;
		return false;
	}

	return false;
}

bool Collection::downloadNextPage()
{
	auto url = ""s;

	try
	{
		auto next = mJson["pagination"]["urls"]["next"];
		if (next.is_null())
			return false;

		url = next.get<string>();
	}
	catch (json::exception &e)
	{
		std::cerr << __func__ <<": Error: " << e.what() << std::endl;
		return false;
	}

	return getNextJson(url);
}

std::vector<string> Collection::listReleases() const
{
	std::vector<string> titles;

	try
	{
		for (auto &item : mJson["releases"])
		{
			string artist = item["basic_information"]["artists"][0]["name"];
			string title = item["basic_information"]["title"];
			string label = item["basic_information"]["labels"][0]["name"];
			int year = item["basic_information"]["year"];
			titles.push_back(artist + " - " + title + " (" + label + ", " + std::to_string(year) + ")");
		}
	}
	catch (json::exception &e)
	{
		std::cerr << __func__ <<": Error: " << e.what() << std::endl;
	}

	return titles;
}
