#include <string>
#include <curl/curl.h>

class Client
{
public:
	virtual ~Client();
	bool init(const std::string &token);
	std::string getNext(const std::string &url);

private:
	CURL *mCurl = nullptr;
};
