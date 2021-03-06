#include <curl/curl.h>
#include <json/json.h>
#include <filesystem>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <vector>

#define DEFAULT_FCOUNT 500
#define FEEDLY_URI "https://cloud.feedly.com/v3/"

#ifndef _PROVIDER_H_
#define _PROVIDER_H_

using CurlString = std::unique_ptr<char, decltype(&curl_free)>;

struct UserData{
        std::map<std::string, std::string> categories;
        std::string id;
        std::string code;
        std::string authToken;
        std::string refreshToken;
        std::string galx;
};

struct PostData{
        std::string content;
        std::string title;
        std::string id;
        std::string originURL;
        std::string originTitle;
};

class FeedlyProvider{
        public:
                FeedlyProvider(const std::filesystem::path& tmpDir);
                void authenticateUser();
                void markPostsRead(const std::vector<std::string>& ids);
                void markPostsSaved(const std::vector<std::string>& ids);
                void markPostsUnsaved(const std::vector<std::string>& ids);
                void markCategoriesRead(const std::string& id, const std::string& lastReadEntryId);
                void markPostsUnread(const std::vector<std::string>& ids);
                void addSubscription(bool newCategory, const std::string& feed, std::vector<std::string> categories, const std::string& title = "");
                const std::vector<PostData>& giveStreamPosts(const std::string& category, bool whichRank = 0);
                const std::map<std::string, std::string>& getLabels();
                const std::string getUserId();
                PostData& getSinglePostData(int index);
                void setVerbose(bool value);
                void setChangeTokensFlag(bool value);
                void curl_cleanup();
        private:
                CURL *curl;
                CURLcode curl_res;
                std::ofstream log_stream;
                std::string feedly_url;
                std::string userAuthCode;
                std::string TOKEN_PATH, COOKIE_PATH, rtrv_count;
                const std::filesystem::path tempPath;
                std::filesystem::path logPath;
                std::filesystem::path configPath;
                UserData user_data;
                bool verboseFlag{}, changeTokens{};
                std::vector<PostData> feeds;
                void getCookies();
                void enableVerbose();
                Json::Value curl_retrieve(const std::string& uri, const Json::Value& jsonCont = Json::Value::nullSingleton());
                void extract_galx_value();
                void echo(bool on);
                void openLogStream();
                CurlString escapeCurlString(const std::string& s);
};

#endif
