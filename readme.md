
WorkTimer
=========

# Summary
- [Description](#description)
- [Compilation](#compilation-requirements)
- [Install](#install)
- [Screenshots](#screenshots)
- [Repository information](#repository-information)

# Description
WorkTimer is a simple application for controlling procrastination and getting statistics on the time spent at work. It uses a system similar to the popular "Pomodoro" technique to intersperse breaks between work tasks.

## Options
The timer can be configured for the work, short and long breaks. By default it uses 25 minutes for a work unit and 5 and 15 minutes for the short and long breaks. Once a session has started those options cannot be configured until the session has finished or stopped.
If miminized the application can be configured to show a tray message informing the start of a new unit or break, with a motivational quote.
Sounds can be enabled and disabled. When using it with headphones I find it usefull to enabled the "voice unit announcements" when working away from the computer. 
A desktop widget can be configured to show the current task name and progress in the desktop. If minimized the same progress is show in the tray icon, but showing just the remaining minutes in the unit.
Statistics about the time invested can be seen in the two charts and exported to CSV or Excel. 


# Compilation requirements
## To build the tool:
* cross-platform build system: [CMake](http://www.cmake.org/cmake/resources/software.html).
* compiler: [Mingw64](http://sourceforge.net/projects/mingw-w64/) on Windows.

## External dependencies
The following libraries are required:
* [Qt 6](http://www.qt.io/).
* [SQLite](https://www.sqlite.org/).
* [Libxlsxwriter](https://github.com/jmcnamara/libxlsxwriter/).

# Install
Download the latest ![release](https://github.com/FelixdelasPozas/WorkTimer/releases/) installer.

Supported versions are from Windows 8 onwards. Neither the application or the installer are digitally signed so the system will ask for approval before running it the first time.

# Screenshots

The main window with the list of completed units. There are two tables, one with the work/procastination times and other (the lower one) with the task names and completed time and units. 

![main]()

When you have completed tasks, you can get statistics in the pie and histogram charts tabs. Putting the mouse in each of the pie slice or histogram bar it will give you the total time invested in that task.

![pie_graph]()
![hist_graph]()

When you minimize the application the tray icon will show the clock (if the session has not started) or the remaining time of the task or break.

![icon]()

The tray icon and context menu. 

![icon_menu]()

The configuration dialog lets you change the duration of each of the units and assign its colors. Also the configuration options for a session can be modified here (work units before a long break, number of units in a session). 

![config]()

Optionally a desktop widget can be shown for the duration of the session with the name of the unit and its current progression.

![widget]()

# Repository information

**Version**: 1.0.0

**Status**: finished.

**cloc statistics**

| Language                     |files          |blank      |comment       |code      |
|:-----------------------------|--------------:|----------:|-------------:|---------:|
| C++                          |  12           |  551      |   400        | 2619     |
| C/C++ Header                 |  12           |  302      |   828        |  624     |
| CMake                        |   1           |   20      |    10        |   84     |
| **Total**                    | **25**        | **873**   | **1238**     | **3327** |
