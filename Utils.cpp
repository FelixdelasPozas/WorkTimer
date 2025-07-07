/*
 File: Utils.cpp
 Created on: 20/04/2025
 Author: Felix de las Pozas Alvarez

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Project
#include <Utils.h>

// libxlsxwriter
#include <xlsxwriter.h>

// Qt
#include <QPainter>
#include <QPainterPath>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDialog>
#include <QWidget>
#include <QSvgRenderer>
#include <QStandardPaths>
#include <QFileInfo>
#include <QFile>
#include <QStyle>

// C++
#include <iostream>
#include <functional>
#include <string>
#include <stringapiset.h>

// SQLite
extern "C"
{
#include <sqlite3/sqlite3.h>
}

const QString INI_FILENAME = "WorkTimer.ini";
const QString WORKUNIT_TIME = "Work unit time";
const QString SMALLBREAK_TIME = "Small break time";
const QString LARGEBREAK_TIME = "Large break time";
const QString UNITS_IN_SESSION = "Units in session";
const QString WORK_COLOR = "Work unit color";
const QString SMALLBREAK_COLOR = "Small break color";
const QString LARGEBREAK_COLOR = "Large break color";
const QString DESKTOP_WIDGET = "Use desktop widget";
const QString DESKTOP_OPACITY = "Desktop widget opacity";
const QString DESKTOP_POSITION = "Desktop widget position";
const QString SOUND_USE = "Use sounds";
const QString SOUND_TIC_TAC = "Use continuous tic-tac";
const QString ICON_MESSAGES = "Show tray icon messages";
const QString DATA_DIRECTORY = "Data directory";
const QString EXPORT_UNIXDATE = "Export unix date";
const QString VOICE_ANNOUNCEMENTS = "Voice announcements";
const QString UNITS_PER_BREAK = "Number of work units before a long break";
const QString GEOMETRY = "Application geometry";
const QString STATE = "Application state";

constexpr int DEFAULT_LOGICAL_DPI = 96;

//-----------------------------------------------------------------
Utils::ClickableHoverLabel::ClickableHoverLabel(QWidget* parent, Qt::WindowFlags f) :
    QLabel(parent, f)
{
}

//-----------------------------------------------------------------
Utils::ClickableHoverLabel::ClickableHoverLabel(const QString& text, QWidget* parent, Qt::WindowFlags f) :
    QLabel(text, parent, f)
{
}

//-----------------------------------------------------------------
Utils::ClickableHoverLabel::~ClickableHoverLabel()
{
}

//-----------------------------------------------------------------
void Utils::ClickableHoverLabel::mousePressEvent(QMouseEvent* e)
{
    emit clicked();
    QLabel::mousePressEvent(e);
}

//-----------------------------------------------------------------
void Utils::ClickableHoverLabel::enterEvent(QEnterEvent* event)
{
    setCursor(Qt::PointingHandCursor);
    QLabel::enterEvent(event);
}

//-----------------------------------------------------------------
void Utils::ClickableHoverLabel::leaveEvent(QEvent* event)
{
    setCursor(Qt::ArrowCursor);
    QLabel::leaveEvent(event);
}

//-----------------------------------------------------------------
void Utils::Configuration::load()
{
    QSettings settings = applicationSettings();
    m_workUnitTime = settings.value(WORKUNIT_TIME, 25).toInt();
    m_shortBreakTime = settings.value(SMALLBREAK_TIME, 5).toInt();
    m_longBreakTime = settings.value(LARGEBREAK_TIME, 15).toInt();
    m_unitsPerSession = settings.value(UNITS_IN_SESSION, 14).toInt();
    m_workUnitsBeforeBreak = settings.value(UNITS_PER_BREAK, 4).toInt();
    m_workColor = QColor::fromString(settings.value(WORK_COLOR, "#00FF00").toString());
    m_shortBreakColor = QColor::fromString(settings.value(SMALLBREAK_COLOR, "#FFFF00").toString());
    m_longBreakColor = QColor::fromString(settings.value(LARGEBREAK_COLOR, "#0000FF").toString());
    m_useWidget = settings.value(DESKTOP_WIDGET, true).toBool();
    m_widgetOpacity = settings.value(DESKTOP_OPACITY, 75).toInt();
    m_widgetPosition = settings.value(DESKTOP_POSITION, QPoint{0, 0}).toPoint();
    m_useSound = settings.value(SOUND_USE, true).toBool();
    m_useVoice = settings.value(VOICE_ANNOUNCEMENTS, false).toBool();
    m_continuousTicTac = settings.value(SOUND_TIC_TAC, false).toBool();
    m_iconMessages = settings.value(ICON_MESSAGES, true).toBool();
    m_exportMs = settings.value(EXPORT_UNIXDATE, false).toBool();
    m_geometry = settings.value(GEOMETRY, QByteArray()).toByteArray();
    m_state = settings.value(STATE, QByteArray()).toByteArray();

    m_dataDir = settings.value(DATA_DIRECTORY, "").toString();

    if (m_dataDir.isEmpty() || !QDir{QDir::fromNativeSeparators(m_dataDir)}.exists()) {
        m_dataDir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();
        QDir().mkdir(m_dataDir);
    }

    openDatabase();
}

//-----------------------------------------------------------------
int Utils::Configuration::minutesInSession() const
{
    int minutes = m_unitsPerSession * m_workUnitTime;
    minutes += (m_unitsPerSession / m_workUnitsBeforeBreak) * ((m_workUnitsBeforeBreak-1) * m_shortBreakTime + m_longBreakTime) -
               (m_unitsPerSession % m_workUnitsBeforeBreak == 0 ? m_longBreakTime : 0);
    if ((m_unitsPerSession % m_workUnitsBeforeBreak) != 0) {
        minutes += ((m_unitsPerSession % m_workUnitsBeforeBreak) - 1) * m_shortBreakTime;
    }

    return minutes;
}

//-----------------------------------------------------------------
void Utils::Configuration::save() const
{
    QSettings settings = applicationSettings();
    settings.setValue(WORKUNIT_TIME, m_workUnitTime);
    settings.setValue(SMALLBREAK_TIME, m_shortBreakTime);
    settings.setValue(LARGEBREAK_TIME, m_longBreakTime);
    settings.setValue(UNITS_IN_SESSION, m_unitsPerSession);
    settings.setValue(UNITS_PER_BREAK, m_workUnitsBeforeBreak);
    settings.setValue(WORK_COLOR, m_workColor.name());
    settings.setValue(SMALLBREAK_COLOR, m_shortBreakColor.name());
    settings.setValue(LARGEBREAK_COLOR, m_longBreakColor.name());
    settings.setValue(DESKTOP_WIDGET, m_useWidget);
    settings.setValue(DESKTOP_OPACITY, m_widgetOpacity);
    settings.setValue(DESKTOP_POSITION, m_widgetPosition);
    settings.setValue(SOUND_USE, m_useSound);
    settings.setValue(VOICE_ANNOUNCEMENTS, m_useVoice);
    settings.setValue(SOUND_TIC_TAC, m_continuousTicTac);
    settings.setValue(ICON_MESSAGES, m_iconMessages);
    settings.setValue(EXPORT_UNIXDATE, m_exportMs);
    settings.setValue(GEOMETRY, m_geometry);
    settings.setValue(STATE, m_state);

    settings.sync();
}

//-----------------------------------------------------------------
QSettings Utils::Configuration::applicationSettings() const
{
    QDir applicationDir{QCoreApplication::applicationDirPath()};
    if (applicationDir.exists(INI_FILENAME)) {
        const auto fInfo = QFileInfo(applicationDir.absoluteFilePath(INI_FILENAME));
        qEnableNtfsPermissionChecks();
        const auto isWritable = fInfo.isWritable();
        qDisableNtfsPermissionChecks();

        if (isWritable) {
            return QSettings(applicationDir.absoluteFilePath(INI_FILENAME), QSettings::IniFormat);
        }
    }

    return QSettings("Felix de las Pozas Alvarez", "WorkTimer");
}

//-----------------------------------------------------------------
void Utils::Configuration::openDatabase()
{
    int retValue = 0;
    if (SQLITE_OK != (retValue = sqlite3_initialize())) {
        const std::string message =
            std::string("Unable to initialize SQLite engine! Error: ") + std::to_string(retValue);
        throw std::runtime_error(message.c_str());
    }

    // Sets sqlite3 temporal directory, needed for windows apparently.
    const auto tempPathWS = QDir::tempPath().toStdWString();
    char zPathBuf[MAX_PATH + 1];
    memset(zPathBuf, 0, sizeof(zPathBuf));
    WideCharToMultiByte(CP_UTF8, 0, tempPathWS.c_str(), -1, zPathBuf, sizeof(zPathBuf), NULL, NULL);
    sqlite3_temp_directory = sqlite3_mprintf("%s", zPathBuf);

    const auto dataDir = QDir{m_dataDir};
    const auto dbFilename = QDir{m_dataDir}.absoluteFilePath("worktimer.db");

    if (SQLITE_OK != (retValue = sqlite3_open_v2(dbFilename.toStdString().c_str(), &m_database,
                                                 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr))) {
        const std::string message = std::string("Unable to open SQLite database! Error: ") + std::to_string(retValue);
        throw std::runtime_error(message.c_str());
    }

    // sqlite can't store 64 bit integers, so we will use text and atoi later, but it will be slower.
    std::string createQuery =
        "CREATE TABLE IF NOT EXISTS TASKS(TTIME TEXT PRIMARY KEY, TNAME TEXT NOT NULL, TDURATION TEXT NOT NULL);";
    sqlite3_stmt* createStmt;
    sqlite3_prepare(m_database, createQuery.c_str(), createQuery.size(), &createStmt, NULL);
    if (SQLITE_DONE != (retValue = sqlite3_step(createStmt))) {
        const std::string message = std::string("Unable to create task_time table! Error: ") + std::to_string(retValue);
        throw std::runtime_error(message.c_str());
    }
    sqlite3_finalize(createStmt);
}

//-----------------------------------------------------------------
void Utils::insertUnitIntoDatabase(Configuration& config, const Utils::TaskTableEntry &entry)
{
    std::string insertQuery = "INSERT OR REPLACE INTO TASKS(TTIME, TNAME, TDURATION) VALUES ('" + std::to_string(entry.taskTime) + "','" +
                              entry.name + "','" + std::to_string(entry.durationMs) + "');";
    sqlite3_stmt* insertStmt;
    sqlite3_prepare(config.m_database, insertQuery.c_str(), insertQuery.size(), &insertStmt, NULL);
    int retValue = 0;
    if (SQLITE_DONE != (retValue = sqlite3_step(insertStmt))) {
        const std::string message = std::string("Unable to insert data! Error: ") + std::to_string(retValue);
        throw std::runtime_error(message.c_str());
    }
    sqlite3_finalize(insertStmt);
}

//-----------------------------------------------------------------
int Utils::tasksTableCallback(void* p_data, int num_fields, char** p_fields, char** p_col_names)
{
    auto tableContents = static_cast<Utils::TaskTableEntries*>(p_data);
    try {
        const unsigned long long taskTime = std::stoull(p_fields[0]);
        const auto taskName = p_fields[1];
        const unsigned long long taskDuration = std::stoull(p_fields[2]);
        tableContents->emplace_back(taskName, taskTime, taskDuration);
    } catch (...) {
        // abort select on failure, don't let exception propogate thru sqlite3 call-stack
        return 1;
    }
    return 0;
}

//-----------------------------------------------------------------
void Utils::insertUnitIntoDatabase(Configuration& config, const unsigned long long startTime, const std::string name,
                                   const unsigned long long duration)
{
    insertUnitIntoDatabase(config, TaskTableEntry{name, startTime, duration});
}

//-----------------------------------------------------------------
Utils::TaskTableEntries Utils::tasksQuery(const std::string &stmt, Utils::Configuration &config)
{
    Utils::TaskTableEntries contents;

    char* errmsg;
    int ret = sqlite3_exec(config.m_database, stmt.c_str(), &Utils::tasksTableCallback, &contents, &errmsg);
    if (ret != SQLITE_OK) {
        std::cerr << "Error in select statement " << stmt << "[" << errmsg << "]\n";
    } else {
        // std::cerr << contents.size() << " records returned.\n";
    }

    return contents;
}

//-----------------------------------------------------------------
Utils::TaskTableEntries Utils::tasksList(Utils::Configuration &config, const QDateTime &from, const QDateTime &to)
{
    std::string stmt;
    if (from == QDateTime() && to == QDateTime()) {
        stmt = "SELECT * FROM TASKS;";
    } else {
        auto beginning = from;
        beginning.setTime(QTime{0, 0, 0});
        auto ending = to;
        ending.setTime(QTime{23, 59, 59});

        stmt = "SELECT * FROM TASKS WHERE TTIME >= " + std::to_string(beginning.toMSecsSinceEpoch()) + " AND TTIME < " +
               std::to_string(ending.toMSecsSinceEpoch()) + ";";
    }

    return tasksQuery(stmt, config);
}

//-----------------------------------------------------------------
QString Utils::toCamelCase(const QString& s)
{
    QStringList parts = s.split(' ', Qt::SkipEmptyParts);
    for (int i = 0; i < parts.size(); ++i) {
        parts[i].replace(0, 1, parts[i][0].toUpper());
    }

    return parts.join(" ");
}

//-----------------------------------------------------------------
bool Utils::exportDataCSV(const QString& filename, const TaskTableEntries& entries, bool useMilliseconds) 
{
    QFile file{filename};
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
        return false;

    const QString header("Date,Task name,Duration\n");
    file.write(header.toUtf8());
    for(const auto &task: entries)
    {
        if (!useMilliseconds) {
            const auto startTime = QDateTime::fromMSecsSinceEpoch(task.taskTime);
            const auto duration = QTime{0, 0, 0}.addMSecs(task.durationMs);
            const auto line = QString("%1,\"%2\",%3\n")
                                  .arg(startTime.toString())
                                  .arg(QString::fromStdString(task.name))
                                  .arg(duration.toString("hh::mm::ss"));
            file.write(line.toUtf8());
        } else {
            const auto line = QString("%1,\"%2\",%3\n")
                                  .arg(task.taskTime)
                                  .arg(QString::fromStdString(task.name))
                                  .arg(task.durationMs);
            file.write(line.toUtf8());
        }
    }

    file.flush();
    file.close();

    return true;
}

//-----------------------------------------------------------------
bool Utils::exportDataExcel(const QString& filename, const TaskTableEntries& entries, bool useMilliseconds) 
{
    lxw_workbook  *workbook  = workbook_new(filename.toStdString().c_str());
    if(!workbook)
        return false;

    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, nullptr);
    if(!worksheet)
        return false;

    /* Change the columns width for clarity. */
    worksheet_set_column(worksheet, 0, 0, 40, nullptr);
    worksheet_set_column(worksheet, 1, 1, 40, nullptr);
    worksheet_set_column(worksheet, 2, 2, 12, nullptr);

    const auto start = QDateTime::fromMSecsSinceEpoch(entries.front().taskTime);
    const auto ending = QDateTime::fromMSecsSinceEpoch(entries.back().taskTime);
    const std::string header = start.toString().toStdString() + " to " + ending.toString().toStdString();
    worksheet_set_header(worksheet, header.c_str());

    int i = 0; 
    for(const auto &task: entries)
    {
        if (!useMilliseconds) {
            auto taskTime = QDateTime::fromMSecsSinceEpoch(task.taskTime);
            worksheet_write_string(worksheet, i, 0, taskTime.toString().toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, i, 1, task.name.c_str(), nullptr);
            auto taskDuration = QTime{0, 0, 0}.addMSecs(task.durationMs);
            worksheet_write_string(worksheet, i++, 2, taskDuration.toString("hh:mm:ss").toStdString().c_str(), nullptr);
        } else {
            worksheet_write_number(worksheet, i, 0, task.taskTime, nullptr);
            worksheet_write_string(worksheet, i, 1, task.name.c_str(), nullptr);
            worksheet_write_number(worksheet, i++, 2, task.durationMs, nullptr);
        }
    }

    workbook_close(workbook);

    return true;
}

//-----------------------------------------------------------------
void Utils::clearDatabase(sqlite3* db, const std::string& tableName)
{
    if(!db) return;

    const std::string statement1 = "DELETE FROM " + tableName + ";";
    const std::string statement2 = "VACUUM";
    char* errmsg;

    for(const auto &stmt: {statement1, statement2})
    {
        int ret = sqlite3_exec(db, stmt.c_str(), nullptr, nullptr, &errmsg);
        if (ret != SQLITE_OK) {
            std::cerr << "Error clearing table " << tableName << "[" << errmsg << "]\n";
            std::cerr << "Statement: " << stmt << std::endl;
            return;
        } 
    }
}

//-----------------------------------------------------------------
int countCallback(void* count, int argc, char** argv, char** p_col_names)
{
    int *c = reinterpret_cast<int *>(count);
    *c = atoi(argv[0]);
    return 0;
}

//-----------------------------------------------------------------
int Utils::numberOfEntries(sqlite3* db, const std::string& tableName)
{
    if(!db) return -1;

    const std::string stmt = "SELECT COUNT(*) FROM " + tableName + ";";
    char *errmsg = nullptr;
    int count = 0;

    int ret = sqlite3_exec(db, stmt.c_str(), countCallback, &count, &errmsg);
    if (ret != SQLITE_OK) {
            std::cerr << "Error counting table entries " << tableName << "[" << errmsg << "]\n";
            std::cerr << "Statement: " << stmt << std::endl;
    } 

    return count;
}

//-----------------------------------------------------------------
Utils::TaskTableEntries Utils::generateTestData(const QDateTime& from, const QDateTime& to, const QStringList& tasks)
{
    TaskTableEntries tasksList;
    srand(time(nullptr));

    const Configuration config;

    auto fromDate = from;
    while(fromDate < to.addDays(1))
    {
        fromDate.setTime(QTime(9, rand() % 50, 0));
        int i = 0;
        for(; i < config.m_unitsPerSession - rand() % 3; ++i)
        {
            const auto task = tasks.at(rand() % tasks.size());
            tasksList.emplace_back(task.toStdString(), fromDate.toMSecsSinceEpoch(), config.m_workUnitTime * 60 * 1000);
            fromDate = QDateTime::fromMSecsSinceEpoch(fromDate.toMSecsSinceEpoch() + (config.m_workUnitTime * 60 * 1000));
        }

        const auto breaks = (i-1) / 2;
        const auto longBreaks = breaks / config.m_workUnitsBeforeBreak;
        for(i = 0; i <= longBreaks; ++i)
        {
            tasksList.emplace_back("Long break", fromDate.toMSecsSinceEpoch(), config.m_longBreakTime * 60 * 1000);
            fromDate = QDateTime::fromMSecsSinceEpoch(fromDate.toMSecsSinceEpoch() + (config.m_longBreakTime * 60 * 1000));
        }
        const auto shortBreaks = breaks - longBreaks;
        for(i = 0; i <= shortBreaks; ++i)
        {
            tasksList.emplace_back("Short break", fromDate.toMSecsSinceEpoch(), config.m_shortBreakTime * 60 * 1000);
            fromDate = QDateTime::fromMSecsSinceEpoch(fromDate.toMSecsSinceEpoch() + (config.m_shortBreakTime * 60 * 1000));
        }

        fromDate = fromDate.addDays(1);
    }

    return tasksList;
};

//-----------------------------------------------------------------
Utils::TaskDurationList Utils::taskNamesAndTimes(Utils::Configuration& config)
{
    std::map<QString, QTime> taskMap;
    for(auto &task: tasksList(config))
    {
        const auto taskName = QString::fromStdString(task.name);
        taskMap[taskName] = taskMap[taskName].addMSecs(task.durationMs);
    }

    Utils::TaskDurationList tasks;

    for(auto &task: taskMap)
    {
        tasks.emplace_back(task.first, task.second);
    }

    std::sort(tasks.begin(), tasks.end(), [](Utils::TaskDuration &lhs, Utils::TaskDuration &rhs){ return lhs.name < rhs.name;});

    return tasks;
}

//-----------------------------------------------------------------
Utils::TaskHistogram Utils::taskHistogram(const QDateTime& from, const QDateTime& to, Utils::Configuration &config)
{
    auto beginning = from;
    beginning.setTime(QTime{0,0,0});
    auto ending = to;
    ending.setTime(QTime{23,59,59});

    const std::string stmt = "SELECT * FROM TASKS WHERE TTIME >= " + std::to_string(beginning.toMSecsSinceEpoch()) + " AND TTIME < " + std::to_string(ending.toMSecsSinceEpoch()) + ";";

    TaskHistogram result;
    const auto tasksQueried = tasksQuery(stmt, config);
    if(tasksQueried.empty())
        return result;

    TaskDurationList tasks;
    std::map<QString, QTime> taskMap;
    for(unsigned long long i = beginning.toMSecsSinceEpoch(); i < static_cast<unsigned long long>(ending.toMSecsSinceEpoch());)
    {
        beginning = beginning.addDays(1);
        taskMap.clear();
        tasks.clear();

        for(auto &task: tasksQueried)
        {
            if(task.taskTime < i || task.taskTime > static_cast<unsigned long long>(beginning.toMSecsSinceEpoch())) continue;

            const auto taskName = QString::fromStdString(task.name);
            if(taskMap.find(taskName) == taskMap.cend())
                taskMap[taskName] = QTime{0,0,0};

            taskMap[taskName] = taskMap[taskName].addMSecs(task.durationMs);
        }

        for (auto& task : taskMap) {
            tasks.emplace_back(task.first, task.second);
        }

        std::sort(tasks.begin(), tasks.end(),
                  [](Utils::TaskDuration& lhs, Utils::TaskDuration& rhs) { return lhs.name < rhs.name; });

        result[i] = tasks;
        i = beginning.toMSecsSinceEpoch();
    }

    return result;
}

//-----------------------------------------------------------------
void Utils::scaleDialog(QDialog* window)
{
    if (window) {
        const auto scale = (window->logicalDpiX() == DEFAULT_LOGICAL_DPI) ? 1. : 1.25;

        window->setMaximumSize(QSize{QWIDGETSIZE_MAX, QWIDGETSIZE_MAX});
        window->setMinimumSize(QSize{0, 0});

        window->adjustSize();
        window->setFixedSize(window->size() * scale);
    }
}

//-----------------------------------------------------------------
void Utils::centerDialog(QDialog* dialog)
{
    if(dialog)
    {
        const auto parentW = dialog->parentWidget();
        if(parentW)
        {
            QRect parentRect(parentW->mapToGlobal(QPoint(0, 0)), parentW->size());
            dialog->move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, dialog->size(), parentRect).topLeft());
        }

    }
}

//-----------------------------------------------------------------
QPixmap Utils::svgPixmap(const QString& name, const QColor color)
{
    // open svg resource load contents to qbytearray
    QFile file(name);
    file.open(QIODevice::ReadOnly);
    QByteArray baData = file.readAll();
    baData.replace("#000000", 7, color.name().toStdString().c_str(), 7);

    // create svg renderer with edited contents
    QSvgRenderer svgRenderer(baData);
    // create pixmap target (could be a QImage)
    QPixmap pix(svgRenderer.defaultSize());
    pix.fill(Qt::transparent);
    // create painter to act over pixmap
    QPainter pixPainter(&pix);
    // use renderer to render over painter which paints on pixmap
    svgRenderer.setAspectRatioMode(Qt::AspectRatioMode::KeepAspectRatio);
    svgRenderer.render(&pixPainter);

    return pix;
}
