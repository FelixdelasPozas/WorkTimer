/*
    File: WorkUnit.cpp
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

// Project
#include <WorkTimer.h>

// Qt
#include <QString>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QTimerEvent>
#include <QWidget>

//-----------------------------------------------------------------
WorkTimer::WorkTimer() :
    QObject{},
    m_workUnitTime{25 * 60 * 1000},
    m_shortBreakTime{5 * 60 * 1000},
    m_longBreakTime{15 * 60 * 1000},
    m_numBeforeBreak{4},
    m_numWorkUnits{0},
    m_numShortBreaks{0},
    m_numLongBreaks{0},
    m_task{"Undefined task"},
    m_progress{0},
    m_status{Status::Stopped},
    m_continuousTicTac{false},
    m_sessionWorkUnits{12},
    m_useSounds{true},
    m_elapsedMSeconds{0}
{
    m_timer.setSingleShot(true);
    m_progressTimer.setSingleShot(false);
    m_progressTimer.setInterval(1000);

    connect(&m_progressTimer, SIGNAL(timeout()), this, SLOT(updateProgress()), Qt::QueuedConnection);

    m_tictac.setSource(QUrl::fromLocalFile(":/WorkTimer/sounds/tictac.wav"));
    m_crank.setSource(QUrl::fromLocalFile(":/WorkTimer/sounds/crank.wav"));
    m_ring.setSource(QUrl::fromLocalFile(":/WorkTimer/sounds/deskbell.wav"));
}

//-----------------------------------------------------------------
WorkTimer::~WorkTimer()
{
    if (m_status != Status::Stopped) {
        stop();
    }
}

//-----------------------------------------------------------------
void WorkTimer::setUseSounds(bool value)
{
    if (m_status == Status::Stopped) {
        m_useSounds = value;
    }
}

//-----------------------------------------------------------------
void WorkTimer::startTimers()
{
    m_timer.start();
    m_progressTimer.start();
    m_startTime = QDateTime::currentDateTime();
    m_progress = 0;
    m_elapsedMSeconds = 0;
    updateProgress();

    if (m_useSounds) {
        queueSound(Sound::CRANK);
        queueSound(Sound::TICTAC);
    }
}

//-----------------------------------------------------------------
QTime WorkTimer::getWorkDuration() const
{
    QTime time{0, 0, 0, 0};
    time = time.addMSecs(m_workUnitTime);
    return time;
}

//-----------------------------------------------------------------
QTime WorkTimer::getShortBreakDuration() const
{
    QTime time{0, 0, 0, 0};
    time = time.addMSecs(m_shortBreakTime);
    return time;
}

//-----------------------------------------------------------------
QTime WorkTimer::getLongBreakDuration() const
{
    QTime time{0, 0, 0, 0};
    time = time.addMSecs(m_longBreakTime);
    return time;
}

//-----------------------------------------------------------------
void WorkTimer::stopTimers()
{
    // disconnect respective signals in the caller of this one.
    m_timer.stop();
    m_progressTimer.stop();

    bool noSounds = (m_status == Status::Stopped || m_status == Status::Paused);
    if (m_useSounds && !noSounds) {
        if (m_continuousTicTac) {
            queueSound(Sound::NONE);
        }

        queueSound(Sound::RING);
    }
}

//-----------------------------------------------------------------
void WorkTimer::startWorkUnit()
{
    m_timer.setInterval(m_workUnitTime);

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(endWorkUnit()), Qt::QueuedConnection);
    m_status = Status::Work;

    emit beginWorkUnit();

    startTimers();
}

//-----------------------------------------------------------------
void WorkTimer::startShortBreak()
{
    m_timer.setInterval(m_shortBreakTime);

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(endShortBreak()), Qt::QueuedConnection);
    m_status = Status::ShortBreak;

    emit beginShortBreak();

    startTimers();
}

//-----------------------------------------------------------------
void WorkTimer::startLongBreak()
{
    m_timer.setInterval(m_longBreakTime);

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(endLongBreak()), Qt::QueuedConnection);
    m_status = Status::LongBreak;

    emit beginLongBreak();

    startTimers();
}

//-----------------------------------------------------------------
void WorkTimer::start()
{
    Q_ASSERT(m_status == Status::Stopped);

    m_numLongBreaks = 0;
    m_numShortBreaks = 0;
    m_numWorkUnits = 0;
    m_progress = 0;
    m_completedTasks.clear();

    startWorkUnit();
}

//-----------------------------------------------------------------
void WorkTimer::pause()
{
    if (Status::Stopped == m_status) {
        return;
    }

    static Status oldStatus;

    if (Status::Paused != m_status) {
        // pause timers
        m_elapsedMSeconds += m_startTime.msecsTo(QDateTime::currentDateTime());
        oldStatus = m_status;
        m_status = Status::Paused;
        m_timer.stop();
        m_progressTimer.stop();
        if (m_continuousTicTac) {
            queueSound(Sound::NONE);
        }
    } else {
        // resume
        QTime time{0, 0, 0, 0};
        time = time.addMSecs(m_elapsedMSeconds);
        unsigned long mSeconds = 0;

        switch (oldStatus) {
            case Status::Work:
                mSeconds = time.msecsTo(getWorkDuration());
                break;
            case Status::ShortBreak:
                mSeconds = time.msecsTo(getShortBreakDuration());
                break;
            case Status::LongBreak:
                mSeconds = time.msecsTo(getLongBreakDuration());
                break;
            default:
                Q_ASSERT(false);
        }

        m_status = oldStatus;

        m_startTime = QDateTime::currentDateTime();
        m_timer.setInterval(mSeconds);
        m_timer.start();

        m_progressTimer.start();

        if (m_continuousTicTac) {
            queueSound(Sound::TICTAC);
        }
    }
}

//-----------------------------------------------------------------
unsigned int WorkTimer::elapsed() const
{
    if (Status::Paused == m_status) {
        return m_elapsedMSeconds;
    }

    return m_elapsedMSeconds + m_startTime.msecsTo(QDateTime::currentDateTime());
}

//-----------------------------------------------------------------
void WorkTimer::stop()
{
    if (Status::Stopped == m_status) {
        return;
    }

    stopTimers();

    switch (m_status) {
        case Status::Work:
            m_timer.disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(endWorkUnit()));
            break;
        case Status::ShortBreak:
            m_timer.disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(endShortBreak()));
            break;
        case Status::LongBreak:
            m_timer.disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(endLongBreak()));
            break;
        case Status::Paused:
            pause();
            stop();
            break;
        default:
            Q_ASSERT(false);
            break;
    }

    m_status = Status::Stopped;
}

//-----------------------------------------------------------------
void WorkTimer::invalidateCurrent()
{
    if (Status::Stopped == m_status) {
        return;
    }

    if (Status::Paused != m_status) {
        stopTimers();
    }

    switch (m_status) {
        case Status::Work:
            m_timer.disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(endWorkUnit()));
            startWorkUnit();
            break;
        case Status::ShortBreak:
            m_timer.disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(endShortBreak()));
            startShortBreak();
            break;
        case Status::LongBreak:
            m_timer.disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(endLongBreak()));
            startLongBreak();
            break;
        case Status::Paused:
            pause();
            invalidateCurrent();
            break;
        default:
            break;
    }
}

//-----------------------------------------------------------------
void WorkTimer::setWorkDuration(QTime duration)
{
    m_workUnitTime = QTime{0, 0, 0, 0}.msecsTo(duration);
}

//-----------------------------------------------------------------
void WorkTimer::setShortBreakDuration(QTime duration)
{
    m_shortBreakTime = QTime{0, 0, 0, 0}.msecsTo(duration);
}

//-----------------------------------------------------------------
void WorkTimer::setLongBreakDuration(QTime duration)
{
    m_longBreakTime = QTime{0, 0, 0, 0}.msecsTo(duration);
}

//-----------------------------------------------------------------
void WorkTimer::setTaskTitle(QString taskTitle)
{
    m_task = taskTitle;
}

//-----------------------------------------------------------------
void WorkTimer::updateProgress()
{
    const unsigned int currentProgress = (std::abs(m_timer.remainingTime() - m_timer.interval()) * 100.f) / m_timer.interval();

    if(m_progress != currentProgress)
    {
        m_progress = currentProgress;
        emit progress(m_progress);
    }
}

//-----------------------------------------------------------------
void WorkTimer::endWorkUnit()
{
    m_completedTasks[m_numWorkUnits] = m_task;

    ++m_numWorkUnits;

    if (m_numWorkUnits == m_sessionWorkUnits) {
        stop();
        emit sessionEnded();
        return;
    }

    stopTimers();
    disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(endWorkUnit()));

    emit workUnitEnded();

    if ((m_numWorkUnits % m_numBeforeBreak) == 0) {
        startLongBreak();
    } else {
        startShortBreak();
    }
}

//-----------------------------------------------------------------
void WorkTimer::endShortBreak()
{
    ++m_numShortBreaks;

    stopTimers();
    disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(endShortBreak()));

    emit shortBreakEnded();
    startWorkUnit();
}

//-----------------------------------------------------------------
void WorkTimer::endLongBreak()
{
    ++m_numLongBreaks;

    stopTimers();
    disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(endLongBreak()));

    emit longBreakEnded();
    startWorkUnit();
}

//-----------------------------------------------------------------
void WorkTimer::setContinuousTicTac(bool value)
{
    if (m_status == Status::Stopped) {
        m_continuousTicTac = value;
        m_tictac.setLoopCount((value ? QSoundEffect::Infinite : 1));
    }
}

//-----------------------------------------------------------------
QIcon WorkTimer::icon() const
{
    return m_icon;
}

//-----------------------------------------------------------------
void WorkTimer::setSessionWorkUnits(unsigned int value)
{
    if (m_status == Status::Stopped) {
        m_sessionWorkUnits = value;
    }
}

//-----------------------------------------------------------------
void WorkTimer::setWorkUnitsBeforeBreak(unsigned int value)
{
    if (m_status == Status::Stopped) {
        m_numBeforeBreak = value;
    }
}

//-----------------------------------------------------------------
QTime WorkTimer::completedSessionTime() const
{
    QTime elapsedTime{0, 0, 0, 0};
    unsigned long seconds = m_numWorkUnits * m_workUnitTime / 1000;
    seconds += m_numShortBreaks * m_shortBreakTime / 1000;
    seconds += m_numLongBreaks * m_longBreakTime / 1000;

    return elapsedTime.addSecs(seconds);
}

//-----------------------------------------------------------------
QTime WorkTimer::sessionTime() const
{
    QTime sessionTime{0, 0, 0, 0};
    unsigned long seconds = m_sessionWorkUnits * m_workUnitTime / 1000;
    const auto numLongBreaks =
        (m_sessionWorkUnits / m_numBeforeBreak) - ((m_sessionWorkUnits % m_numBeforeBreak == 0) ? 1 : 0);
    seconds += numLongBreaks * m_longBreakTime / 1000;
    seconds += (m_sessionWorkUnits - numLongBreaks - 1) * m_shortBreakTime / 1000;

    sessionTime = sessionTime.addSecs(seconds);

    return sessionTime;
}

//-----------------------------------------------------------------
QTime WorkTimer::elapsedTime() const
{
    QTime returnTime{0, 0, 0, 0};
    if (Status::Paused == m_status) {
        returnTime = returnTime.addMSecs(m_elapsedMSeconds);
    } else {
        if (Status::Stopped != m_status) {
            returnTime = returnTime.addMSecs(m_elapsedMSeconds + m_startTime.msecsTo(QDateTime::currentDateTime()));
        }
    }

    return returnTime;
}

//-----------------------------------------------------------------
void WorkTimer::clear()
{
    m_numWorkUnits = 0;
    m_numShortBreaks = 0;
    m_numLongBreaks = 0;
}

//-----------------------------------------------------------------
QString WorkTimer::statusMessage()
{
    QString returnVal;

    switch (m_status) {
        case Status::Stopped:
            returnVal = QString("Stopped.");
            break;
        case Status::Work:
            returnVal = QString("In a work unit.");
            break;
        case Status::ShortBreak:
            returnVal = QString("In a short break.");
            break;
        case Status::LongBreak:
            returnVal = QString("In a long break.");
            break;
        case Status::Paused:
            returnVal = QString("Paused.");
            break;
        default:
            Q_ASSERT(false);
            break;
    }

    return returnVal;
}

//-----------------------------------------------------------------
void WorkTimer::queueSound(Sound sound)
{
    m_playList << sound;

    if (m_playList.size() == 1) {
        playNextSound();
    }
}

//-----------------------------------------------------------------
void WorkTimer::playNextSound()
{
    switch (m_playList.first()) {
        case Sound::CRANK:
            startTimer(LENGTH_CRANK);
            m_crank.play();
            break;
        case Sound::RING:
            startTimer(LENGTH_RING);
            m_ring.play();
            break;
        case Sound::TICTAC:
            startTimer(LENGTH_TICTAC);
            m_tictac.play();
            break;
        case Sound::NONE:
            if (m_tictac.loopCount() == QSoundEffect::Infinite) {
                startTimer(LENGTH_TICTAC);
                m_tictac.stop();
            }
            break;
        default:
            break;
    }
}

//-----------------------------------------------------------------
void WorkTimer::timerEvent(QTimerEvent* e)
{
    killTimer(e->timerId());
    m_playList.removeFirst();

    if (!m_playList.empty()) {
        playNextSound();
    }
}
