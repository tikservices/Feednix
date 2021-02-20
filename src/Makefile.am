bin_PROGRAMS = feednix

feednix_SOURCES = \
	CursesProvider.cpp \
	CursesProvider.h \
	FeedlyProvider.cpp \
	FeedlyProvider.h \
	main.cpp

feednix_CPPFLAGS = -DDEBUG -std=c++17 -Wall
AM_CFLAGS = -lcurl -ljsoncpp -lmenuw -lpanelw -lncursesw
AM_LIBS = curl jsoncpp menuw panelw ncursesw
