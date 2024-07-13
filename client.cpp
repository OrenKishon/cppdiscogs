#include "client.h"
#include <iostream>

using namespace std;

static auto writeCallback(void *contents, size_t size, size_t nmemb, string *output)
{
	size_t total_size = size * nmemb;
	output->append((char*)contents, total_size);
	return total_size;
}

bool Client::init(const string &token)
{
	curl_global_init(CURL_GLOBAL_DEFAULT);

	if (!mCurl)
		mCurl = curl_easy_init();

	if (!mCurl)
		return false;

	struct curl_slist *headers = NULL;
	if (!token.empty())
		headers = curl_slist_append(headers, ("Authorization: Discogs token="+token).c_str());
	curl_easy_setopt(mCurl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(mCurl, CURLOPT_USERAGENT, "cppdiscogs/1.0");
	curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, writeCallback);

	return true;
}

string Client::getNext(const string &url)
{
	curl_easy_setopt(mCurl, CURLOPT_URL, url.c_str());
	string response;
	curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, &response);

	auto res = curl_easy_perform(mCurl);

	if (res != CURLE_OK)
		return "";

	return response;
}

Client::~Client()
{
	std::cout << "\nClosing connection, ciao!...\n";
	if (mCurl)
		curl_easy_cleanup(mCurl);
	curl_global_cleanup();
}
