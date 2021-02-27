#include <chrono>
#include <iostream>
#include <curses.h>
#include <menu.h>
#include <panel.h>

#ifndef _CURSES_H
#define _CURSES_H

#include "FeedlyProvider.h"

#define CTG_WIN_WIDTH 40
#define VIEW_WIN_HEIGHT_PER 50

class CursesProvider{
        public:
                CursesProvider(const std::filesystem::path& tmpPath, bool verbose, bool change);
                void init();
                void control();
                ~CursesProvider();
        private:
                FeedlyProvider feedly;
                WINDOW *ctgWin, *postsWin, *viewWin;
                PANEL  *panels[3], *top;
                std::vector<ITEM*> ctgItems{};
                std::vector<ITEM*> postsItems{};
                MENU *ctgMenu, *postsMenu;
                std::string lastEntryRead, statusLine[3];
                std::chrono::time_point<std::chrono::steady_clock> lastPostSelectionTime{std::chrono::time_point<std::chrono::steady_clock>::max()};
                std::chrono::seconds secondsToMarkAsRead;
                std::string textBrowser;
                const std::filesystem::path previewPath;
                bool currentRank = 0;
                int totalPosts = 0, numRead = 0, numUnread = 0;
                int viewWinHeightPer = VIEW_WIN_HEIGHT_PER, viewWinHeight = 0, ctgWinWidth = CTG_WIN_WIDTH;
                bool currentCategoryRead{};
                void clearCategoryItems();
                void clearPostItems();
                void createCategoriesMenu();
                void createPostsMenu();
                void changeSelectedItem(MENU* curMenu, int req);
                void ctgMenuCallback(const char* label);
                void postsMenuCallback(ITEM* item, bool preview);
                void markItemRead(ITEM* item);
                void markItemReadAutomatically(ITEM* item);
                void renderWindow(WINDOW *win, const char *label, int labelColor, bool highlight);
                void printInMiddle(WINDOW *win, int starty, int startx, int width, const char *string, chtype color);
                void printInCenter(WINDOW *win, int starty, int startx, int height, int width, const char *string, chtype color);
                void clear_statusline();
                void update_statusline(const char* update, const char* post, bool showCounter);
                void update_infoline(const char* info);
};

#endif
