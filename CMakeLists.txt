cmake_minimum_required (VERSION 3.10.0)

project(WorkTimer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

string(TIMESTAMP CURRENT_YEAR "%Y")

# Version Number
set (WORKTIMER_VERSION_MAJOR 1)
set (WORKTIMER_VERSION_MINOR 0)
set (WORKTIMER_VERSION_PATCH 0)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt6 COMPONENTS Widgets Multimedia Svg Charts)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt6Widgets_EXECUTABLE_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt6Multimedia_EXECUTABLE_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -m64 -Wall -mwindows -Wno-deprecated")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -m64 -Wall -mconsole -Wno-deprecated")

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CORE_EXTERNAL_LIBS ${CORE_EXTERNAL_LIBS} ${QT_QTTEST_LIBRARY})
endif (CMAKE_BUILD_TYPE MATCHES Debug)

if(DEFINED MINGW)
  configure_file("${PROJECT_SOURCE_DIR}/resources.rc.in" "${PROJECT_BINARY_DIR}/resources.rc")
  configure_file("${PROJECT_SOURCE_DIR}/version.h.in" "${PROJECT_BINARY_DIR}/version.h")
  configure_file("${PROJECT_SOURCE_DIR}/installer/script.iss.in" "${PROJECT_BINARY_DIR}/script.iss")
  set(CORE_SOURCES ${CORE_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/resources.rc)
  set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
  enable_language(RC)
endif(DEFINED MINGW)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/external # External code, not from the project.  
  ${PROJECT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}          # Generated .h files
  ${CMAKE_CURRENT_BINARY_DIR}  # For wrap/ui files
  )

# External sqlite code
set (SQLITE_FILES
  external/sqlite3/sqlite3.c
)

set (TASKBARBUTTON_FILES
  external/QTaskBarButton/QTaskBarButton.cpp
)

# Add Qt Resource files
qt6_add_resources(RESOURCES
  rsc/resources.qrc
)

qt6_wrap_ui(CORE_UI
  # .ui for Qt
  AboutDialog.ui
  MainWindow.ui
  ConfigurationDialog.ui
  RangeSelectorWidget.ui
)
	
set (CORE_SOURCES
  # project files
  ${CORE_SOURCES}
  ${RESOURCES}
  ${CORE_MOC}
  ${CORE_UI}
  Main.cpp
  AboutDialog.cpp
  Alarm.cpp
  DesktopWidget.cpp
  WorkTimer.cpp
  Utils.cpp
  MainWindow.cpp
  ProgressWidget.cpp
  ConfigurationDialog.cpp
  PieChart.cpp
  ChartsTooltip.cpp
  RangeSelectorWidget.cpp
  ${SQLITE_FILES}
  ${TASKBARBUTTON_FILES}
)

set(CORE_EXTERNAL_LIBS
  Qt6::Widgets
  Qt6::Multimedia
  Qt6::Svg
  Qt6::Charts
)
  
add_executable(WorkTimer ${CORE_SOURCES})
target_link_libraries (WorkTimer ${CORE_EXTERNAL_LIBS})
