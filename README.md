Feednix
=======
[![Build Status](https://travis-ci.org/anders-dc/Feednix.svg)](https://travis-ci.org/anders-dc/Feednix)

An ncurses based client for [Feedly](http://feedly.com/).

## Install

### Distribution Packages

[Arch Linux](https://aur.archlinux.org/packages/feednix/)

### From Source

First run autogen.sh script.

Then run your standard make commands. Here is a one liner:

`./configure && make && sudo make install`

Thank you @chrisjohnston for mentioning the following dependencies for Ubuntu:

`sudo apt-get install dh-autoreconf libjsoncpp-dev libcurl4-gnutls-dev libncurses5-dev`

For macOS with [Homebrew](https://brew.sh), install the prerequisites with:

`brew install jsoncpp`

Install to `/usr/local/bin` with the following:

`./configure --prefix=/usr/local && make && make install`

## Clarification on Sign In Method (PLEASE READ)

Due to the fact that this is open source, the administrators at Feedly have
asked me to use their developer tokens instead of giving me a client secret.
To explain, if I were to use a client secret I would need to distribute it
along with the source code. This is an obvious security risk and I am going
to keep this project open source, hence this new method.

**Unfortunatley, as of now, the Developer tokens have an expiration of**
**three months. This means that you must create a new one every three months.**

**I have modified Feednix to help you with this. IT CANNOT DO THIS**
**AUTOMATICALLY since developer tokens involve sending you an email**
**to retrieve them.**

## Usage

### Global Options

* q : Exit
* Vim Key mappings for navigation (j,k)

### Post List Options

* Enter : open post preview
* o (lower case) : Open post link in console (currently using w3m)
* O (upwer case) : Open post link in Browser (user default)
* r : Mark post read
* u : Mark post unread
* A : mark all posts read
* R : Refresh category
* = : Change sort type

### Category List Options

* Enter : Fetch Stream (Retrive post by category)
* A : Mark category read
* R : Refersh highlighted category (Retrive post by category)

## Contributing

Please visit this [page](https://feednix-jarkore.rhcloud.com) for details.

##Changelog

**The follwoing only lists major updates. For everything in between please see the ChangeLog**

###v0.9
Once again many thanks to [lejenome](https://github.com/lejenome) for the following:

* fix panels array length
* add travis-ci config file
* use global TMPDIR var instead of class depend tmpdir var … 
* move tmpdir creation to FeedluProvider and use it for temp.txt file too
* make update_statusline char\* params const
* minimaze updating info bar code into a common function
* replace tabs with 8 spaces
* add .gitignore file 

###v0.8

Many thanks to [lejenome](https://github.com/lejenome) for the following: 

* use $TMPDIR (mkdtemp) for preview.html file to avoid unneeded disk access
* add a new panel to shown a quick preview for current selected item (html formated by w3m) instead of the need to open w3m for previews
* add 3 new config options to add some interface tweaks (ctg_win_width: to change ctgWin width, view_win_height: to set viewWin height, view_win_height_per: to set viewWin height percent that can be used instead of the previous option and overwirte it)
* fix unreading unreaded post when handling "u"
* fix mark as read aleardy marked as read posts on "O" handling

