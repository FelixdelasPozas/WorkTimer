/*
    File: WorkTimer.h
    Created on: 22/04/2015
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

#ifndef WORKTIMER_H_
#define WORKTIMER_H_

// Qt
#include <QTimer>
#include <QDateTime>
#include <QStringList>
#include <QObject>
#include <QString>
#include <QIcon>
#include <QThread>
#include <QMap>
#include <QSoundEffect>

class QSoundEffect;
class QTemporaryFile;
class QTimerEvent;

// C++11
#include <cstdint>

/** \class WorkTimer
 *  \brief Implements the timer technique using Qt timers
 *
 */
class WorkTimer
: public QObject
{
	Q_OBJECT
	public:
    /** \brief WorkTimer class constructor.
     *
     */
		WorkTimer();

    /** \brief WorkTimer class virtual destructor.
     *
     */
		virtual ~WorkTimer();

    /** \brief Clears the internal values and starts the session.
     *
     */
		void start();

    /** \brief Pauses the current unit.
     *
     */
		void pause();

    /** \brief Stops the session.
     *
     */
		void stop();

    /** \brief Returns the elapsed time of the unit in milliseconds.
     *
     */
		unsigned int elapsed() const;

    /** \brief Sets the duration of the work unit.
     * \param[in] time QTime of the duration.
     *
     */
		void setWorkDuration(QTime time);

    /** \brief Sets the duration of the short break unit.
     * \param[in] time QTime of the duration.
     *
     */
		void setShortBreakDuration(QTime time);

    /** \brief Sets the duration of the long break unit.
     * \param[in] time QTime of the duration.
     *
     */
		void setLongBreakDuration(QTime time);

    /** \brief Sets the name of the task for the current unit.
     * \param[in] title task description.
     *
     */
		void setTaskTitle(QString title);

    /** \brief Enables/disables the continuous tic-tac sound while the work unit is running.
     * \param[in] enabled boolean value
     *
     */
		void setContinuousTicTac(bool enabled);

    /** \brief Sets the number of work units to complete per session.
     * \param[in] value
     *
     */
		void setSessionWorkUnits(unsigned int value);

    /** \brief Sets the number of work units to be completed before a long break.
     * \param[in] value
     *
     */
		void setWorkUnitsBeforeBreak(unsigned int value);

    /** \brief Enables/disables the sounds.
     * \param[in] enabled boolean value.
     *
     */
		void setUseSounds(bool enabled);

    /** \brief Returns the duration of a work unit.
     *
     */
		QTime getWorkDuration() const;

		/** \brief Returns the duration of a short break unit.
     *
     */
		QTime getShortBreakDuration() const;

    /** \brief Returns the duration of a long break unit.
     *
     */
		QTime getLongBreakDuration() const;

    /** \brief Returns the number of work units to be completed before a long break.
     *
     */
		unsigned long getWorkUnitsBeforeLongBreak() const
		{ return m_numBeforeBreak; }

    /** \brief Returns the number of work units of a session.
     *
     */
		unsigned int getWorkUnitsInSession() const
		{ return m_sessionWorkUnits; };

    /** \brief Returns the current task.
     *
     */
		QString getTaskTitle() const
		{ return m_task; };

    /** \brief Returns the completed work units and their tasks.
     *
     */
		QMap<int, QString> getCompletedTasks() const
		{ return m_completedTasks; };

    /** \brief Returns the number of completed work units in the session.
     *
     */
		unsigned int completedWorkUnits() const
		{ return m_numWorkUnits; };

    /** \brief Returns the number of completed short breaks in the session.
     *
     */
		unsigned int completedShortBreaks() const
		{ return m_numShortBreaks; };

    /** \brief Returns the number of completed long breaks in the session.
     *
     */
		unsigned int completedLongBreaks() const
		{	return m_numLongBreaks;	}

    /** \brief Returns the total time of completed units in the session.
     *
     */
		QTime completedSessionTime() const;

		/** \brief Returns the total time of the session;
		 *
		 */
		QTime sessionTime() const;

    /** \brief Returns the time elapsed since the beginning of the current unit.
     *
     */
		QTime elapsedTime() const;

    /** \brief Resets the session.
     *
     */
		void clear();

    /** \brief Timer states.
     */
    enum class Status : char
    {
      Work = 1,
      ShortBreak,
      LongBreak,
      Stopped,
      Paused
    };

    /** \brief Returns the current status of the session (Stopped/Paused/Current unit).
     *
     */
	  Status status() const
	  { return m_status; };

    /** \brief Returns a description of the current status (Stopped/Paused/Current unit).
     *
     */
	  QString statusMessage();

	signals:
		void beginWorkUnit();
		void workUnitEnded();
		void beginShortBreak();
		void shortBreakEnded();
		void beginLongBreak();
		void longBreakEnded();
		void progress(unsigned int);
		void sessionEnded();

	protected:
		virtual void timerEvent(QTimerEvent *e) override;

	private slots:
    /** \brief Updates the icon for the completed unit intervals.
     *
     */
	  void updateProgress();

    /** \brief Ends the work unit and starts a short break or a long break depending in the number of work units per long break.
     *
     */
	  void endWorkUnit();

    /** \brief Ends a short break and starts a work unit.
     *
     */
	  void endShortBreak();

    /** \brief Ends a long break and starts a work unit.
     *
     */
	  void endLongBreak();

	private:
    /** \brief Types of sounds
     */
    enum class Sound
    {
      CRANK,
      TICTAC,
      RING,
      CLICK,
      NONE
    };

    static const int LENGTH_CRANK = 530;  /** length in milliseconds of the crack sound. */
    static const int LENGTH_TICTAC = 450; /** length in milliseconds of the tic-tac sound. */
    static const int LENGTH_RING = 1300;  /** length in milliseconds of the ring sound. */
    static const int LENGTH_CLICK = 440;  /** length in milliseconds of the ring sound. */

    /** \brief Queues the sound to play.
     * \param[in] sound.
     *
     */
    void queueSound(Sound sound);

    /** \brief Plays a queued sound.
     *
     */
    void playNextSound();

    /** \brief Starts a work unit.
     *
     */
    void startWorkUnit();

    /** \brief Starts a short break.
     *
     */
    void startShortBreak();

    /** \brief Starts a long break.
     *
     */
    void startLongBreak();

    /** \brief Starts the timers for the current unit.
     *
     */
	  void startTimers();

	  /** \brief Stops the timers for the current unit.
     *
     */
	  void stopTimers();

    unsigned long m_workUnitTime;        /** Duration of a work unit.                                       */
    unsigned long m_shortBreakTime;      /** Duration of a short break.                                     */
    unsigned long m_longBreakTime;       /** Duration of a long break.                                      */
    unsigned long m_numBeforeBreak;      /** Number of work units before a long break.                      */
    unsigned long m_numWorkUnits;        /** Number of work units completed in the current session.         */
    unsigned long m_numShortBreaks;      /** Number of short breaks in the session.                         */
    unsigned int  m_numLongBreaks;       /** Number of long breaks in the session.                          */
    QString       m_task;                /** Title of the task.                                             */
    QTimer        m_timer;               /** Timer.                                                         */
    QTimer        m_progressTimer;       /** Timer used for progress notifications in 1/8 of the unit time. */
    unsigned int  m_progress;            /** Progress counter.                                              */
    Status        m_status;              /** Actual status of the timer.                                    */
    bool          m_continuousTicTac;    /** Continuous tic-tac sound.                                      */
    unsigned int  m_sessionWorkUnits;    /** Number of work units in a session.                             */
    bool          m_useSounds;           /** Use sounds.                                                    */
    unsigned int  m_remainMS;         /** Elapsed seconds since the last time m_timer started.           */
    QMap<int, QString> m_completedTasks; /** Task names of completed work units.                            */

    QSoundEffect m_crank;  /** Crank sound      */
    QSoundEffect m_tictac; /** Tic-tac sound    */
    QSoundEffect m_ring;   /** Alarm ring sound */
    QSoundEffect m_click;  /** Click sound      */

    QList<Sound> m_playList; /** list of sounds to play. */
};


#endif /* WORKTIMER_H_ */
