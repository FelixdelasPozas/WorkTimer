
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
The timer can be configured for the work, short and long breaks. By default it uses 25 minutes for a work unit and 5 and 15 minutes for the short and long breaks. 
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

![icon]()
![icon_menu]()
![config]()
![pie_graph]()
![hist_graph]()

# Repository information

**Version**: 1.0.0

**Status**: finished.

**cloc statistics**

| Language                     |files          |blank      |comment       |code      |
|:-----------------------------|--------------:|----------:|-------------:|---------:|
| C++                          |  12           |  523      |   395        | 2494     |
| C/C++ Header                 |  12           |  294      |   807        |  613     |
| CMake                        |   1           |   20      |    10        |   84     |
| **Total**                    | **25**        | **837**   | **1212**     | **3191** |
