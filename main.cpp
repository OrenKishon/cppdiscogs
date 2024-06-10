#include <iostream>
#include <curl/curl.h>
#include <string>
#include <nlohmann/json.hpp>
#include <getopt.h>
#include <set>

using std::string;
using namespace std::literals;
using json = nlohmann::json;

static size_t writeCallback(void *contents, size_t size, size_t nmemb, string *output) {
	size_t total_size = size * nmemb;
	output->append((char*)contents, total_size);
	return total_size;
}

static const std::set<string> sort_keys = {
	"label", "artist", "title", "catno", "format", "rating", "added", "year"
};

class Client
{
public:
	Client(const string &token, const string &user) :
		mToken(token),
		mUser(user)
	{}
	~Client();

	bool init(string criteria);
	bool downloadNextPage();
	json getCollection() const { return mJson; }
	std::vector<string> list() const;
	int getCurrentPage() const { return mCurrentPage; }

private:
	bool getNext();
	string mToken;
	string mUser;
	CURL *mCurl = nullptr;
	json mJson;
	int mCurrentPage = 0;
};

Client::~Client()
{
	std::cout << "\nClosing connection, ciao!...\n";
	if (mCurl)
		curl_easy_cleanup(mCurl);
	curl_global_cleanup();
}

bool Client::init(string criteria)
{
	curl_global_init(CURL_GLOBAL_DEFAULT);

	if (!mCurl)
		mCurl = curl_easy_init();

	if (!mCurl)
		return false;

	/* ID 0, the `All` folder, which cannot have releases added to it, and
	   ID 1, the `Uncategorized` folder which requires a token to access.
	 */
	auto folder = mToken.empty() ? "0" : "1";

	auto url = "https://api.discogs.com/users/"+mUser+"/collection/folders/"+folder+"/releases?sort="+criteria;

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, ("Authorization: Discogs token="+mToken).c_str());
	curl_easy_setopt(mCurl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(mCurl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(mCurl, CURLOPT_USERAGENT, "cppdiscogs/1.0");
	curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, writeCallback);

	if (!getNext())
		return false;

	std::cout << "Collection of " << mUser << " listed by " << criteria << ":\n";
	std::cout << "\titems = " << mJson["pagination"]["items"] << std::endl;
	std::cout << "\tpages = " << mJson["pagination"]["pages"] << std::endl;
	std::cout << "\tper_page = " << mJson["pagination"]["per_page"] << std::endl;
	std::cout << "\tnext = " << mJson["pagination"]["urls"]["next"] << std::endl;	

	return true;
}

bool Client::downloadNextPage()
{
	auto next = mJson["pagination"]["urls"]["next"];
	if (next.is_null())
		return false;

	curl_easy_setopt(mCurl, CURLOPT_URL, next.get<string>().c_str());
	return getNext();
}

bool Client::getNext()
{
	string response;
	curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, &response);

	auto res = curl_easy_perform(mCurl);

	if (res != CURLE_OK)
	{
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		return false;
	}

	/* Parse JSON as descibed in: 
	 * www.discogs.com/developers#page:user-collection,header:user-collection-collection-items-by-folder
	 */
	mJson = json::parse(response);
	mCurrentPage = mJson["pagination"]["page"];

	return true;
}

std::vector<string> Client::list() const
{
	std::vector<string> titles;
	for (auto &item : mJson["releases"])
	{
		string artist = item["basic_information"]["artists"][0]["name"];
		string title = item["basic_information"]["title"];
		string label = item["basic_information"]["labels"][0]["name"];
		int year = item["basic_information"]["year"];
		titles.push_back(artist + " - " + title + " (" + label + ", " + std::to_string(year) + ")");
	}
	return titles;
}

static void usage()
{
	auto criteria_list = std::accumulate(next(sort_keys.begin()), sort_keys.end(), *sort_keys.begin(), [](auto a, auto b) { return a + ", " + b; });
	std::cout << "Usage: cppdiscogs [options]\n"
		"Options:\n"
		"  -t, --token <token>				Discogs API token\n"
		"  -u, --user <user>				Discogs user name\n"
		"  -l, --list-by <"+criteria_list+">		List criteria\n"
		"  -d, --dump-collection			Dump collection\n";
}

int main(int argc, char *argv[])
{
	// Parse command line arguments with getpot
	option long_options[] = {
		{"token", required_argument, 0, 't'},
		{"user", required_argument, 0, 'u'},
		// List collection by: titles, artists, years
		{"list-by", required_argument, 0, 'l'},
		// Dump collection
		{"dump-collection", no_argument, 0, 'd'},
		{0, 0, 0, 0}
	};

	auto token = "";
	auto user = ""s;
	auto criteria = ""s;
	auto dump_collection = false;

	while (1) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "t:u:l:d", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 't':
			token = optarg;
			break;
		case 'u':
			user = optarg;
			break;
		case 'l':
			criteria = optarg;
			if (!sort_keys.contains(criteria))
			{
				std::cout << "Invalid criteria: " << criteria << "\n";
				usage();
				return 1;
			}
			break;
		case 'd':
			dump_collection = true;
			break;
		default:
			usage();
			return 1;
		}
	}

	if (user.empty())
	{
		usage();
		return 1;
	}

	auto client = Client(token, user);

	if (!client.init(criteria))
	{
		std::cout << "Failed to initialize client\n";
		return 1;
	}

	if (dump_collection)
	{
		std::cout << "Parsed JSON: " << std::setw(4) << client.getCollection() << "\n";
		return 0;
	}

	do
	{
		printf("\nType `Enter` for next page (%d) or `q` to quit: ", client.getCurrentPage());
		std::string line;
		std::getline(std::cin, line);
		if (line == "q")
			break;

		for (auto title : client.list())
			std::cout << title << std::endl;
	}
	while (client.downloadNextPage());

	return 0;
}
