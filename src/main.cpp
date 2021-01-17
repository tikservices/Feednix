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
CursesProvider *curses;
std::string TMPDIR;

void atExitFunction(void){
        system(std::string("find " + std::string(HOME_PATH) + "/.config/feednix -type f -not -name \'config.json\' -and -not -name \'log.txt\' -delete 2> /dev/null").c_str());
        system(std::string("rm -R " + TMPDIR + " 2> /dev/null").c_str());
        curses->cleanup();
}

void sighandler(int signum){
        system(std::string("find " + std::string(HOME_PATH) + "/.config/feednix -type f -not -name \'config.json\' -and -not -name \'log.txt\' -delete 2> /dev/null").c_str());
        signal(signum, SIG_DFL);
        curses->cleanup();
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

        char *sys_tmpdir = getenv("TMPDIR");
        if(!sys_tmpdir)
                sys_tmpdir = (char *)"/tmp";

        char * pathTemp = (char *)malloc(sizeof(char) * (strlen(sys_tmpdir) + 16));
        strcpy(pathTemp, sys_tmpdir);
        strcat(pathTemp, "/feednix.XXXXXX");

        TMPDIR = std::string(mkdtemp(pathTemp));
        free(pathTemp);

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
                curses = new CursesProvider(verboseEnabled, changeTokens);
        }
        else{
                curses = new CursesProvider(false, false);
        }


        curses->init();
        curses->control();

        delete curses;
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
