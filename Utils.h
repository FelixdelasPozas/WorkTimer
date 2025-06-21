/*
 File: Utils.h
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

#ifndef UTILS_H_
#define UTILS_H_

// Qt
#include <QLabel>
#include <QFrame>
#include <QColor>
#include <QSettings>
#include <QTime>
#include <QDateTime>

class QDialog;
struct sqlite3;

namespace Utils
{
    /** \class ClickableHoverLabel
    * \brief ClickableLabel subclass that changes the mouse cursor when hovered.
    *
    */
    class ClickableHoverLabel : public QLabel
    {
        Q_OBJECT
      public:
        /** \brief ClickableHoverLabel class constructor.
        * \param[in] parent Raw pointer of the widget parent of this one.
        * \f Widget flags.
        *
        */
        explicit ClickableHoverLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

        /** \brief ClickableHoverLabel class constructor.
        * \param[in] text Label text.
        * \param[in] parent Raw pointer of the widget parent of this one.
        * \f Widget flags.
        *
        */
        explicit ClickableHoverLabel(const QString& text, QWidget* parent = nullptr,
                                     Qt::WindowFlags f = Qt::WindowFlags());

        /** \brief ClickableHoverLabel class virtual destructor.
        *
        */
        virtual ~ClickableHoverLabel();

      signals:
        void clicked();

      protected:
        void mousePressEvent(QMouseEvent* e) override;
        virtual void enterEvent(QEnterEvent* event) override;
        virtual void leaveEvent(QEvent* event) override;
    };

    /** \struct TaskTableEntry
     * \brief Struct to contains the information of an entry into the database.
     */
    struct TaskTableEntry
    {
        std::string name;                  /** name of the task. */
        unsigned long long taskTime = 0;   /** starting time of the task unix format. */
        unsigned long long durationMs = 0; /** duration of the task in milliseconds. */

        /** \brief TaskTableEntry struct constructor. 
         * \param[in] name Name of the task.
         * \param[in] tTime Starting time of the task unix format. 
         * \param[in] duration duration of the task in milliseconds.
         */
        TaskTableEntry(const std::string name, const unsigned long long tTime, const unsigned long long duration) :
            name{name},
            taskTime{tTime},
            durationMs{duration} {};

        /** \brief TaskTableEntry struct empty constructor. 
         */
        TaskTableEntry() :
            name{"unknown"},
            taskTime{0},
            durationMs{0} {};
    };

    using TaskTableEntries = std::vector<Utils::TaskTableEntry>;

    /** \class Configuration
     * \brief Implements a progress bar that represents the progres in the session.
     *
     */
    class Configuration
    {
      public:
        /** \brief Load configuration of the application from an ini file or the registry.
         */
        void load();

        /** \brief Saves the configuration of the application to an ini file or the registry.
         */
        void save() const;

        /** \brief Returns the total minutes in a session for this configuration.
         *
         */
        int minutesInSession() const;

        int m_workUnitTime = 25;                      /** minutes of a work unit. */
        int m_shortBreakTime = 5;                     /** minutes of a short break. */
        int m_longBreakTime = 15;                     /** minutes of a long break. */
        int m_unitsPerSession = 14;                   /** number of work units per session. */
        QColor m_workColor = QColor(0, 1., 0);        /** color of work unit. */
        QColor m_shortBreakColor = QColor(1., 1., 0); /** color of short breaks. */
        QColor m_longBreakColor = QColor(0, 0, 1.);   /** color of long breaks. */
        bool m_useWidget = true;                      /** true to show the desktop widget and false otherwise. */
        int m_widgetOpacity = 75;                     /** desktop widget opacity in [10,100]. */
        QPoint m_widgetPosition = QPoint{0, 0};       /** position of the desktop widget. */
        bool m_useSound = true;                       /** true to use sounds and false otherwise. */
        bool m_continuousTicTac = false;              /** true to use the continuous tic-tac sound during work units. */
        bool m_iconMessages = true;                   /** true to show tray icon messages and false otherwise.  */
        QString m_dataDir;                            /** directory that contains the database. */
        sqlite3* m_database = nullptr;                /** sqlite database. */
        bool m_exportMs = false;                      /** true to use milliseconds time when exporting data, or dates and duration if false. */

      private:
        /** \brief Helper method that returns the QSettings object to use.
         */
        QSettings applicationSettings() const;

        /** \brief Opens the database file or creates it if it doesn't exists.
         *
         */
        void openDatabase();
    };

    /** \brief Helper method to scale the dialog and mininize its size.
     * \param[in] dialog Dialog to scale.
     *
     */
    void scaleDialog(QDialog* dialog);

    /** \brief Returns the given svg as a pixmap in the given color. 
     * \param[in] name SVG name in resources.
     * \param[in] color Color to use. 
     *
     */
    QPixmap svgPixmap(const QString &name, const QColor color);

    /** \brief Helper method to insert values into the database. 
     * \param[in] config Application configuration that contains the database handle. 
     * \param[in] entry TaskTableEntry struct reference.
     *
     */
    void insertUnitIntoDatabase(Configuration &config, const TaskTableEntry &entry);

    /** \brief Helper method to insert values into the database. 
     * \param[in] config Application configuration that contains the database handle. 
     * \param[in] startTime Task start time. 
     * \param[in] name Task name. 
     * \param[in] duration Task duration in ms.
     *
     */
    void insertUnitIntoDatabase(Configuration &config, const unsigned long long startTime, const std::string name, const unsigned long long duration);

    /** \brief Returns the result of a given query to the task table in the database.
     * \param[in] config Application configuration that contains the database handle.
     * 
     */
    TaskTableEntries tasksQuery(const std::string &stmt, Utils::Configuration &config);

    /** \brief Returns the contents of the task table in the database. 
     * \param[in] config Application configuration that contains the database handle.
     * \param[in] from From date.
     * \param[in] to To Date.
     *
     */
    TaskTableEntries tasksList(Utils::Configuration &config, const QDateTime &from = QDateTime(), const QDateTime &to = QDateTime());

    struct TaskDuration
    {
        QString name;   /** name of the task. */
        QTime duration; /** task duration. */

        /** \brief Struct TaskDuration empty constructor.
         * 
         */
        TaskDuration() :
            name{"Unknown"},
            duration{QTime{0, 0, 0}} {};

        /** \brief Struct TaskDuration constructor.
         * \param[in] taskName Task name.
         * \param[in] taskTimeMs Task duration time in milliseconds. 
         * 
         */
        TaskDuration(const QString& taskName, const unsigned long long& taskTimeMs) :
            name{taskName},
            duration{QTime{0,0,0}.addMSecs(taskTimeMs)} {};

        /** \brief Struct TaskDuration constructor.
        * \param[in] taskName Task name.
        * \param[in] taskTime Task duration time. 
        * 
        */
        TaskDuration(const QString& taskName, const QTime& taskTime) :
            name{taskName},
            duration{taskTime} {};

    };
    using TaskDurationList = std::vector<TaskDuration>;

    using TaskHistogram = std::map<unsigned long long, TaskDurationList>;

    /** \brief Returns the task names and times of the database. 
     * \param[in] config Application configuration that contains the database handle.
     * 
     */
    TaskDurationList taskNamesAndTimes(Utils::Configuration &config);

    /** \brief Returns the histogram of task for the given days interval.
     * \param[in] from Start date. 
     * \param[in] to End date.
     * \param[in] config Application configuration that contains the database handle.
     *
     */
    TaskHistogram taskHistogram(const QDateTime &from, const QDateTime &to, Utils::Configuration &config);

    /** \brief SQLite table contents callback.
     * \param[inout] p_data Data conteiner pointer to insert the data of the table.
     * \param[in] num_fields Number of fields
     * \param[in] p_fields Fields data pointer. 
     * \param[in] p_col_names table column names.
     *
     */
    int tasksTableCallback(void* p_data, int num_fields, char** p_fields, char** p_col_names);

    /** \brief Helper method to return the camel case version of a given string.
     * \param[in] s String to transform.
     *
     */
    QString toCamelCase(const QString& s);

    /** \brief Exports the given entries to a CSV file on disk with the given filename. Returns true on success.
     * \param[in] filename Filename of file on disk.
     * \param[in] entries Task entries list. 
     * \param[in] useMilliseconds True to export millisecond values and false to use text for times and dates. 
     *
     */
    bool exportDataCSV(const QString &filename, const TaskTableEntries &entries, bool useMilliseconds);

    /** \brief Exports the given entries to a Excel file on disk with the given filename. Returns true on success.
     * \param[in] filename Filename of file on disk.
     * \param entries Task entries list. 
     * \param[in] useMilliseconds True to export millisecond values and false to use text for times and dates. 
     *
     */
    bool exportDataExcel(const QString &filename, const TaskTableEntries &entries, bool useMilliseconds);
} // namespace Utils

#endif // UTILS_H_