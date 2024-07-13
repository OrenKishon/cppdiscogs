#include <iostream>
#include <nlohmann/json.hpp>
#include <getopt.h>

#include "collection.h"

using std::string;
using namespace std::literals;

static void usage()
{
	auto criteria_list = std::accumulate(next(Collection::sort_keys.begin()),
			Collection::sort_keys.end(),
			*Collection::sort_keys.begin(), [](auto a, auto b) { return a + ", " + b; });
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
			if (!Collection::sort_keys.contains(criteria))
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

	auto client = Collection(token, user, criteria);

	if (!client.init())
	{
		std::cout << "Failed to initialize client\n";
		return 1;
	}

	if (dump_collection)
	{
		std::cout << "Parsed JSON: " << std::setw(4) << client.getJson() << "\n";
		return 0;
	}

	do
	{
		printf("\nType `Enter` for next page (%d) or `q` to quit: ", client.getCurrentPage());
		std::string line;
		std::getline(std::cin, line);
		if (line == "q")
			break;

		for (auto title : client.listReleases())
			std::cout << title << std::endl;
	}
	while (client.downloadNextPage());

	return 0;
}
