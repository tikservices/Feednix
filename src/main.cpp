#include <filesystem>
#include <iostream>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "CursesProvider.h"

namespace fs = std::filesystem;

#define HOME_PATH getenv("HOME")
static fs::path TMPDIR;

void atExitFunction(){
        auto errorCode = std::error_code{};

        // Remove $TMPDIR/feednix.XXXXXX.
        if(!TMPDIR.empty()){
                fs::remove_all(TMPDIR, errorCode);
        }

        // Remove all under $HOME/.config/feednix except config.json and log.txt.
        const auto home_path = fs::path{HOME_PATH};
        const auto config_dir = home_path / ".config" / "feednix";
        for(const auto& entry : fs::directory_iterator(config_dir)){
                const auto& path = entry.path();
                const auto& filename = path.filename();
                if((filename != "config.json") && (filename != "log.txt")){
                        fs::remove_all(path, errorCode);
                }
        }
}

void sighandler(int signum){
        atExitFunction();
        signal(signum, SIG_DFL);
        kill(getpid(), signum);
}

void printUsage();

int main(int argc, char **argv){
        signal(SIGINT, sighandler);
        signal(SIGTERM, sighandler);
        signal(SIGSEGV, sighandler);
        atexit(atExitFunction);

        bool verboseEnabled = false;
        bool changeTokens = false;

        // Create ~/.config/feednix/config.json if it doesn't exist.
        const auto home_path = fs::path{HOME_PATH};
        const auto config_dir = home_path / ".config" / "feednix";
        const auto config_path = config_dir / "config.json";
        fs::create_directories(config_dir);
        if(!fs::exists(fs::status(config_path))){
                fs::copy_file("/etc/xdg/feednix/config.json", config_path);
                fs::permissions(config_path, fs::perms::owner_read | fs::perms::owner_write);
        }

        const auto pathTemp = fs::temp_directory_path() / "feednix.XXXXXX";
        const auto& pathTempString = pathTemp.native();
        auto pathTempBuffer = std::vector(pathTempString.begin(), pathTempString.end());
        pathTempBuffer.push_back('\0');
        TMPDIR = fs::path(mkdtemp(pathTempBuffer.data()));

        if(argc >= 2){
                for(int i = 1; i < argc; ++i){
                        if(argv[i][0] == '-' && argv[i][1] == 'h' && strlen(argv[1]) <= 2){
                                printUsage();
                                exit(EXIT_SUCCESS);
                        }
                        else{
                                if(argv[i][0] == '-' && argv[i][1] == 'v' && strlen(argv[1]) <= 2)
                                        verboseEnabled = true;
                                if(argv[i][0] == '-' && argv[i][1] == 'c' && strlen(argv[1]) <= 2)
                                        changeTokens = true;
                                if(argv[i][0] != '-' || !(argv[i][1] == 'v' || argv[i][1] == 'c' || argv[i][1] == 'h' || strlen(argv[1]) >= 2)){
                                        printUsage();
                                        std::cerr << "ERROR: Invalid option " << "\'" << argv[i] << "\'" << std::endl;
                                        exit(EXIT_FAILURE);
                                }
                        }
                }
        }

        auto curses = CursesProvider(TMPDIR, verboseEnabled, changeTokens);
        curses.init();
        curses.control();
        return 0;
}

void printUsage(){
        std::cout << "Usage: feednix [OPTIONS]" << std::endl;
        std::cout << "  An ncurses-based console client for Feedly written in C++" << std::endl;
        std::cout << "\n Options:\n  -h        Display this help and exit\n  -v        Set curl to output in verbose mode during login" << std::endl;
        std::cout << "\n Config:\n   Feednix uses a config file to set colors and\n   and the amount of posts to be retrived per\n   request." << std::endl;
        std::cout << "\n   This file can be found and must be placed in:\n     $HOME/.config/feednix\n   A sample config can be found in /etc/feednix" << std::endl;
        std::cout << "\n Author:\n   Copyright Jorge Martinez Hernandez <jorgemartinezhernandez@gmail.com>\n   Licensing information can be found in the source code" << std::endl;
        std::cout << "\n Bugs:\n   Please report any bugs on Github <https://github.com/Jarkore/Feednix>" << std::endl;
}
