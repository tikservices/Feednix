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
                CursesProvider(bool verbose, bool change);
                void init();
                void control();
                void cleanup();
        private:
                FeedlyProvider feedly;
                WINDOW *ctgWin, *postsWin, *viewWin;
                PANEL  *panels[2], *top;
                ITEM **ctgItems, **postsItems;
                MENU *ctgMenu, *postsMenu;
                std::string lastEntryRead, statusLine[3];
                int totalPosts = 0, numRead = 0, numUnread = 0;
                int viewWinHeightPer = VIEW_WIN_HEIGHT_PER, viewWinHeight = 0, ctgWinWidth = CTG_WIN_WIDTH;
                bool currentCategoryRead;
                void createCategoriesMenu();
                void createPostsMenu();
                void changeSelectedItem(MENU* curMenu, int req);
                void ctgMenuCallback(char* label);
                void postsMenuCallback(ITEM* item, bool preview);
                void markItemRead(ITEM* item);
                void win_show(WINDOW *win, char *label, int label_color, bool highlight);
                void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
                void print_in_center(WINDOW *win, int starty, int startx, int height, int width, char *string, chtype color);
                void clear_statusline();
                void update_statusline(const char* update, const char* post, bool showCounter);
                void update_infoline(const char* info);
};

#endif
