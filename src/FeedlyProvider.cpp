#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

#include <json/json.h>
#include <json/writer.h>

#include <iterator>
#include <istream>
#include <termios.h>
#include <unistd.h>
#include <ctime>

#include "FeedlyProvider.h"

#define HOME_PATH getenv("HOME")
extern std::string TMPDIR;

using namespace std::literals::string_literals;

FeedlyProvider::FeedlyProvider(){
        curl_global_init(CURL_GLOBAL_DEFAULT);
        verboseFlag = false;

        TEMP_PATH = TMPDIR + "/temp.txt";

        Json::Value root;
        Json::Reader reader;

        std::ifstream tokenFile(std::string(std::string(HOME_PATH) + "/.config/feednix/config.json").c_str(), std::ifstream::binary);
        if(reader.parse(tokenFile, root)){
                rtrv_count = root["posts_retrive_count"].asString();
        }
        tokenFile.close();
}
void FeedlyProvider::authenticateUser(){
        Json::Value root;
        Json::Reader reader;

        std::string configPath = std::string(std::string(HOME_PATH) + "/.config/feednix/config.json");

        std::ifstream initialConfig(configPath.c_str(), std::ifstream::binary);
        bool parsingSuccesful = reader.parse(initialConfig, root);

        if(!parsingSuccesful){
                if(!isLogStreamOpen) openLogStream();
                log_stream << "ERROR: Log In Failed - Unable to read from config file" << std::endl;
                log_stream << reader.getFormattedErrorMessages() << std::endl;
                exit(EXIT_FAILURE);
        }
        if(root["developer_token"] == Json::nullValue || changeTokens){
                std::cout << "You will now be redirected to Feedly's Developer Log In page..." << std::endl;
                std::cout << "Please sign in, copy your user id and retrive the token from your email and copy it onto here." << std::endl;

#ifdef __APPLE__
                system(std::string("open \"https://feedly.com/v3/auth/dev\" &> /dev/null &").c_str());
#else
                const auto xdgOpenExitCode = system("xdg-open \"https://feedly.com/v3/auth/dev\" > /dev/null");
                if(xdgOpenExitCode == 0){
                        // xdg-open succeeded, but the browser might have failed to launch or show the page.
                        // For example, xdg-open may launch www-browser (w3m) in a non-desktop session,
                        // but the browser will terminate immediately because the output is redirected.
                        // Ask the user to open the authentication page manually as a fallback method.
                        std::cout << "If the browser doesn't launch, please open ";
                        std::cout << "\"https://feedly.com/v3/auth/dev\" by yourself." << std::endl << std::endl;
                }
                else{
                        std::cerr << "Feednix failed to launch the browser with an exit code " << std::to_string(xdgOpenExitCode) << "." << std::endl;
                        std::cerr << "Please open \"https://feedly.com/v3/auth/dev\" by yourself." << std::endl << std::endl;
                }
#endif

                sleep(3);

                std::string devToken, userID;

                std::cout << "[Enter User ID] >> ";
                std::cin >> userID;

                std::cout << "[Enter token] >> ";
                std::cin >> devToken;

                root["developer_token"] = devToken;
                root["userID"] = userID;

                Json::StyledWriter writer;

                std::ofstream newConfig(configPath.c_str());
                newConfig << root;

                newConfig.close();
        }
        initialConfig.close();

        user_data.authToken = (root["developer_token"]).asString();
        user_data.id = (root["userID"]).asString(); 
}
const std::map<std::string, std::string>* FeedlyProvider::getLabels(){
        user_data.categories.clear();
        user_data.categories["All"] = "user/" + user_data.id + "/category/global.all";
        user_data.categories["Saved"] = "user/" + user_data.id + "/tag/global.saved";
        user_data.categories["Uncategorized"] = "user/" + user_data.id + "/category/global.uncategorized";

        try{
                const auto root{ curl_retrieve("categories") };
                for(const auto& item : root){
                    user_data.categories[item["label"].asString()] = item["id"].asString();
                }
        }
        catch(const std::exception& e){
                if(!isLogStreamOpen) openLogStream();
                log_stream << "Could not get labels" << std::endl;
                log_stream << e.what() << std::endl;
                throw;
        }

        return &(user_data.categories);
}
const std::vector<PostData>* FeedlyProvider::giveStreamPosts(const std::string& category, bool whichRank){
        feeds.clear();

        std::string rank = "newest";
        if(whichRank)
                rank = "oldest";

        Json::Value root;
        try{
                if(category == "All")
                        root = curl_retrieve("streams/contents?ranked=" + rank + "&count=" + rtrv_count + "&unreadOnly=true&streamId=" + std::string(curl_easy_escape(curl, (user_data.categories["All"]).c_str(), 0)));
                else if(category == "Uncategorized")
                        root = curl_retrieve("streams/contents?ranked=" + rank + "&count=" + rtrv_count + "&unreadOnly=true&streamId=" + std::string(curl_easy_escape(curl, (user_data.categories["Uncategorized"]).c_str(), 0)));
                else if(category == "Saved")
                        root = curl_retrieve("streams/contents?ranked=" + rank + "&count=" + rtrv_count + "&unreadOnly=true&streamId=" + std::string(curl_easy_escape(curl, (user_data.categories["Saved"]).c_str(), 0)));
                else
                        root = curl_retrieve("streams/" + std::string(curl_easy_escape(curl, user_data.categories[category].c_str(), 0)) + "/contents?unreadOnly=true&ranked=newest&count=" + rtrv_count);
        }
        catch(const std::exception& e){
                if(!isLogStreamOpen) openLogStream();
                log_stream << "Could not get posts" << std::endl;
                log_stream << e.what() << std::endl;
                throw;
        }

        if(root["items"].size() == 0){
                return NULL;
        }

        for(const auto& item : root["items"]){
                auto url = std::string{};
                for(const auto& alternate : item["alternate"]){
                        if(alternate["type"].asString() == "text/html"){
                                url = alternate["href"].asString();
                                break;
                        }
                }

                feeds.push_back({
                    item["summary"]["content"].asString(),
                    item["title"].asString(),
                    item["id"].asString(),
                    url,
                    item["origin"]["title"].asString()});
        }

        return &(feeds);
}
void FeedlyProvider::markPostsRead(const std::vector<std::string>* ids){
        Json::Value jsonCont;
        Json::Value array;

        jsonCont["type"] = "entries";

        for(std::vector<std::string>::const_iterator it = ids->begin(); it != ids->end(); ++it)
                array.append("entryIds") = *it;

        jsonCont["entryIds"] = array;
        jsonCont["action"] = "markAsRead";

        try{
                curl_retrieve("markers", jsonCont);
        }
        catch(const std::exception& e){
                if(!isLogStreamOpen) openLogStream();
                log_stream << "Could not mark post(s) as read" << std::endl;
                log_stream << e.what() << std::endl;
                throw;
        }
}
void FeedlyProvider::markPostsUnread(const std::vector<std::string>* ids){
        Json::Value jsonCont;
        Json::Value array;

        jsonCont["type"] = "entries";

        for(std::vector<std::string>::const_iterator it = ids->begin(); it != ids->end(); ++it)
                array.append("entryIds") = *it;

        jsonCont["entryIds"] = array;
        jsonCont["action"] = "keepUnread";

        try{
                curl_retrieve("markers", jsonCont);
        }
        catch(const std::exception& e){
                if(!isLogStreamOpen) openLogStream();
                log_stream << "Could not mark post(s) as unread" << std::endl;
                log_stream << e.what() << std::endl;
                throw;
        }
}
void FeedlyProvider::markPostsSaved(const std::vector<std::string>* ids){
        Json::Value jsonCont;
        Json::Value array;

        jsonCont["type"] = "entries";

        for(std::vector<std::string>::const_iterator it = ids->begin(); it != ids->end(); ++it)
                array.append("entryIds") = *it;

        jsonCont["entryIds"] = array;
        jsonCont["action"] = "markAsSaved";

        try{
                curl_retrieve("markers", jsonCont);
        }
        catch(const std::exception& e){
                if(!isLogStreamOpen) openLogStream();
                log_stream << "Could not mark post(s) as saved" << std::endl;
                log_stream << e.what() << std::endl;
                throw;
        }
}
void FeedlyProvider::markPostsUnsaved(const std::vector<std::string>* ids){
        Json::Value jsonCont;
        Json::Value array;

        jsonCont["type"] = "entries";

        for(std::vector<std::string>::const_iterator it = ids->begin(); it != ids->end(); ++it)
                array.append("entryIds") = *it;

        jsonCont["entryIds"] = array;
        jsonCont["action"] = "markAsUnsaved";

        try{
                curl_retrieve("markers", jsonCont);
        }
        catch(const std::exception& e){
                if(!isLogStreamOpen) openLogStream();
                log_stream << "Could not mark post(s) as unsaved" << std::endl;
                log_stream << e.what() << std::endl;
                throw;
        }
}
void FeedlyProvider::markCategoriesRead(const std::string& id, const std::string& lastReadEntryId){
        Json::Value jsonCont;
        Json::Value array;

        jsonCont["type"] = "categories";

        array.append("categoryIds") = id;

        jsonCont["lastReadEntryId"] = lastReadEntryId;
        jsonCont["categoryIds"] = array;
        jsonCont["action"] = "markAsRead";

        try{
                curl_retrieve("markers", jsonCont);
        }
        catch(const std::exception& e){
                if(!isLogStreamOpen) openLogStream();
                log_stream << "Could not mark category(ies) as read" << std::endl;
                log_stream << e.what() << std::endl;
                throw;
        }
}
void FeedlyProvider::addSubscription(bool newCategory, const std::string& feed, std::vector<std::string> categories, const std::string& title){
        int i = 0;

        Json::Value jsonCont;
        Json::Value array;

        jsonCont["id"] = "feed/" + feed;
        jsonCont["title"] = title;

        if(categories.size() > 0){
                for(auto it = categories.begin(); it != categories.end(); ++it){
                        jsonCont["categories"][i]["id"] = user_data.categories[*it];
                        jsonCont["categories"][i]["label"] = *it;
                        i++;
                }
        }
        else{
                jsonCont["categories"] = Json::Value(Json::arrayValue);
        }

        try{
                curl_retrieve("subscriptions", jsonCont);
        }
        catch(const std::exception& e){
                if(!isLogStreamOpen) openLogStream();
                log_stream << "Could not add subscription" << std::endl;
                log_stream << e.what() << std::endl;
                throw;
        }
}

PostData* FeedlyProvider::getSinglePostData(const int index){
        return &(feeds[index]);
}

const std::string FeedlyProvider::getUserId(){
        return user_data.id;
}

void FeedlyProvider::enableVerbose(){
        if(verboseFlag)
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
}
void FeedlyProvider::setVerbose(bool value){
        verboseFlag = value;
}
void FeedlyProvider::setChangeTokensFlag(bool value){
        changeTokens = value;
}
Json::Value FeedlyProvider::curl_retrieve(const std::string& uri, const Json::Value& jsonCont){
        struct curl_slist *chunk = NULL;

        FILE* data_holder = fopen(TEMP_PATH.c_str(), "wb");
        chunk = curl_slist_append(chunk, ("Authorization: OAuth " + user_data.authToken).c_str());

        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, (std::string(FEEDLY_URI) + uri).c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        const auto isPost{ !jsonCont.isNull() };
        if(isPost){
                Json::StyledWriter writer;
                std::string document = writer.write(jsonCont);
                curl_easy_setopt(curl, CURLOPT_POST, true);
                curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, document.c_str());
                chunk = curl_slist_append(chunk, "Content-Type: application/json");
        }

        enableVerbose();

        curl_res = curl_easy_perform(curl);
        if(curl_res != CURLE_OK){
                throw std::runtime_error("curl_easy_perform() failed: "s + curl_easy_strerror(curl_res));
        }

        fclose(data_holder);
        curl_easy_cleanup(curl);

        if(isPost){
                return Json::Value();
        }

        std::ifstream data(TEMP_PATH.c_str(), std::ifstream::binary);
        if(!data){
                throw std::runtime_error("Failed to open a temporary file: "s + TEMP_PATH);
        }

        Json::Reader reader;
        Json::Value root;
        if(!reader.parse(data, root)){
                throw std::runtime_error("Failed to parse tokens file: "s + reader.getFormattedErrorMessages());
        }

        if(root.isObject() && root.isMember("errorMessage") && root.isMember("errorId")){
                const auto message = "Feedly returned an error: "s
                    + root["errorMessage"].asString()
                    + " ("s + root["errorId"].asString() + ")"s;
                throw std::runtime_error(message.c_str());
        }

        return root;
}
void FeedlyProvider::openLogStream(){
        if(!log_stream.is_open()){
                const char* log_path = std::string(std::string(HOME_PATH) + "/.config/feednix/log.txt").c_str();
                log_stream.open(log_path, std::ofstream::out | std::ofstream::app);

                time_t current = time(NULL);
                char* dt = ctime(&current);

                log_stream << "======== " << std::string(dt) << "\n";

                isLogStreamOpen = true;
        }
}
void FeedlyProvider::echo(bool on = true){
        struct termios settings;
        tcgetattr( STDIN_FILENO, &settings );
        settings.c_lflag = on
                ? (settings.c_lflag |   ECHO )
                : (settings.c_lflag & ~(ECHO));
        tcsetattr( STDIN_FILENO, TCSANOW, &settings );
}
void FeedlyProvider::curl_cleanup(){
        curl_global_cleanup();
        if(log_stream.is_open())
                log_stream.close();
}
