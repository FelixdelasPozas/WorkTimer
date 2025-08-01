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
#include <QAudioOutput>
#include <QMediaDevices>
#include <QAudioDevice>

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
    m_remainMS{0},
    m_newTaskTime{0},
    m_audioDevices{this}
{
    m_timer.setSingleShot(true);
    m_progressTimer.setSingleShot(false);
    m_progressTimer.setInterval(1000);

    connect(&m_progressTimer, SIGNAL(timeout()), this, SLOT(updateProgress()), Qt::QueuedConnection);

    m_tictac.setSource(QUrl::fromLocalFile(":/WorkTimer/sounds/tictac.wav"));
    m_crank.setSource(QUrl::fromLocalFile(":/WorkTimer/sounds/crank.wav"));
    m_ring.setSource(QUrl::fromLocalFile(":/WorkTimer/sounds/deskbell.wav"));
    m_click.setSource(QUrl::fromLocalFile(":/WorkTimer/sounds/click.wav"));
    m_finish.setSource(QUrl::fromLocalFile(":/WorkTimer/sounds/finish.wav"));
    m_shortBreak.setSource(QUrl::fromLocalFile(":/WorkTimer/sounds/ShortBreak.wav"));
    m_longBreak.setSource(QUrl::fromLocalFile(":/WorkTimer/sounds/LongBreak.wav"));
    m_workUnit.setSource(QUrl::fromLocalFile(":/WorkTimer/sounds/WorkUnit.wav"));    

    connect(&m_audioDevices, SIGNAL(audioOutputsChanged()), this, SLOT(onAudioDeviceChanged()));
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
    m_useSounds = value;

    if(!value)
    {
        m_playList.clear();

        if(m_continuousTicTac && m_tictac.isPlaying())
            m_tictac.stop();
    }
}

//-----------------------------------------------------------------
void WorkTimer::startTimers()
{
    m_timer.start();
    m_progressTimer.start();
    m_progress = 0;
    m_remainMS = 0;

    queueSound(Sound::CRANK);

    if(m_useVoice)
    {
        switch(m_status)
        {
            case Status::ShortBreak:
                queueSound(Sound::SHORTBREAK);
                break;
            case Status::LongBreak:
                queueSound(Sound::LONGBREAK);
                break;
            case Status::Work:
                queueSound(Sound::WORKUNIT);
                break;
            case Status::Paused:
            case Status::Stopped:
                break;
        }
    }

    queueSound(Sound::TICTAC);    
}

//-----------------------------------------------------------------
QTime WorkTimer::getWorkDuration() const
{
    return QTime{0, 0, 0, 0}.addMSecs(m_workUnitTime);
}

//-----------------------------------------------------------------
QTime WorkTimer::getShortBreakDuration() const
{
    return QTime{0, 0, 0, 0}.addMSecs(m_shortBreakTime);;
}

//-----------------------------------------------------------------
QTime WorkTimer::getLongBreakDuration() const
{
    return QTime{0, 0, 0, 0}.addMSecs(m_longBreakTime);
}

//-----------------------------------------------------------------
void WorkTimer::stopTimers()
{
    // disconnect respective signals in the caller of this one.
    m_timer.stop();
    m_progressTimer.stop();
    m_remainMS = 0;

    const bool noSounds = (m_status == Status::Stopped || m_status == Status::Paused);
    if (!noSounds) {
        if (m_continuousTicTac) {
            queueSound(Sound::NONE);
        }

        queueSound(Sound::RING);
    }
}

//-----------------------------------------------------------------
bool WorkTimer::checkEndOfSession()
{
    if (m_numWorkUnits == m_sessionWorkUnits) {
        m_status = Status::Stopped;
        queueSound(Sound::FINISH);
        emit sessionEnded();
        return true;
    }

    return false;
}

//-----------------------------------------------------------------
void WorkTimer::startWorkUnit()
{
    m_timer.setInterval(m_workUnitTime);

    m_status = Status::Work;
    startTimers();
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(endWorkUnit()), Qt::QueuedConnection);
    m_newTaskTime = 0;

    emit beginWorkUnit();
}

//-----------------------------------------------------------------
void WorkTimer::startShortBreak()
{
    if (checkEndOfSession()) {
        return;
    }

    m_timer.setInterval(m_shortBreakTime);

    m_status = Status::ShortBreak;
    startTimers();
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(endShortBreak()), Qt::QueuedConnection);
    m_newTaskTime = 0;

    emit beginShortBreak();
}

//-----------------------------------------------------------------
void WorkTimer::startLongBreak()
{
    if (checkEndOfSession()) {
        return;
    }

    m_timer.setInterval(m_longBreakTime);

    m_status = Status::LongBreak;
    startTimers();
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(endLongBreak()), Qt::QueuedConnection);
    m_newTaskTime = 0;

    emit beginLongBreak();
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
        m_remainMS = m_timer.remainingTime();
        oldStatus = m_status;
        m_status = Status::Paused;
        m_timer.stop();
        m_progressTimer.stop();
        if (m_continuousTicTac) {
            queueSound(Sound::NONE);
        }
        queueSound(Sound::CLICK);
    } else {
        // resume
        m_status = oldStatus;
        m_timer.setInterval(m_remainMS);
        m_timer.start();
        m_remainMS = 0;

        m_progressTimer.start();
        queueSound(Sound::CLICK);
        if (m_continuousTicTac) {
            queueSound(Sound::TICTAC);
        }
    }
}

//-----------------------------------------------------------------
unsigned int WorkTimer::elapsed() const
{
    const auto unitTime = m_status == Status::Work ? getWorkDuration() : (m_status == Status::ShortBreak ? getShortBreakDuration() : getLongBreakDuration());
    if (Status::Paused != m_status) {
        return QTime{0,0,0}.msecsTo(unitTime) - m_timer.remainingTime() - m_newTaskTime;
    }

    return QTime{0,0,0}.msecsTo(unitTime) - m_remainMS - m_newTaskTime;
}

//-----------------------------------------------------------------
void WorkTimer::stop()
{
    if (Status::Stopped == m_status) {
        return;
    }

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
        default:
        case Status::Paused:
            break;
    }

    stopTimers();
    m_status = Status::Stopped;
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
    m_newTaskTime += elapsed();
}

//-----------------------------------------------------------------
void WorkTimer::updateProgress()
{
    const auto unitTime = m_status == Status::Work ? getWorkDuration() : (m_status == Status::ShortBreak ? getShortBreakDuration() : getLongBreakDuration());
    const auto unitMs = QTime{0,0,0}.msecsTo(unitTime);
    const unsigned int currentProgress = (std::abs(m_timer.remainingTime() - unitMs) * 100.f) / unitMs;

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

    disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(endWorkUnit()));
    stopTimers();

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

    disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(endShortBreak()));
    stopTimers();

    emit shortBreakEnded();
    startWorkUnit();
}

//-----------------------------------------------------------------
void WorkTimer::endLongBreak()
{
    ++m_numLongBreaks;

    disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(endLongBreak()));
    stopTimers();

    emit longBreakEnded();
    startWorkUnit();
}

//-----------------------------------------------------------------
void WorkTimer::onAudioDeviceChanged()
{
    const auto defaultOutput = QMediaDevices::defaultAudioOutput();

    m_tictac.setAudioDevice(defaultOutput);
    m_crank.setAudioDevice(defaultOutput);
    m_ring.setAudioDevice(defaultOutput);
    m_click.setAudioDevice(defaultOutput);
    m_finish.setAudioDevice(defaultOutput);
    m_shortBreak.setAudioDevice(defaultOutput);
    m_longBreak.setAudioDevice(defaultOutput);
    m_workUnit.setAudioDevice(defaultOutput);
}

//-----------------------------------------------------------------
void WorkTimer::setContinuousTicTac(bool value)
{
    m_continuousTicTac = value;
    m_tictac.setLoopCount((value ? QSoundEffect::Infinite : 1));
    if (m_status == Status::Work) {
        if(value)
            queueSound(Sound::TICTAC);
        else
            queueSound(Sound::NONE);
    }

}

//-----------------------------------------------------------------
void WorkTimer::setSessionWorkUnits(unsigned int value)
{
    if (m_status == Status::Stopped) {
        m_sessionWorkUnits = value;
    }
}

//-----------------------------------------------------------------
void WorkTimer::setWorkUnitsBeforeLongBreak(unsigned int value)
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
        returnTime = returnTime.addMSecs(m_remainMS);
    } else {
        if (Status::Stopped != m_status) {
            returnTime = returnTime.addMSecs(m_timer.remainingTime());
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
    if (!m_useSounds) {
        return;
    }
    
    m_playList << sound;

    if (m_playList.size() == 1) {
        playNextSound();
    }
}

//-----------------------------------------------------------------
void WorkTimer::playNextSound()
{
    if(m_playList.empty()) return;

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
        case Sound::CLICK:
            startTimer(LENGTH_CLICK);
            m_click.play();
            break;
        case Sound::FINISH:
            startTimer(LENGTH_FINISH);
            m_finish.play();
            break;
        case Sound::SHORTBREAK:
            startTimer(LENGTH_SHORT_BREAK);
            m_shortBreak.play();
            break;
        case Sound::LONGBREAK:
            startTimer(LENGTH_LONG_BREAK);
            m_longBreak.play();
            break;
        case Sound::WORKUNIT:
            startTimer(LENGTH_WORK_UNIT);
            m_workUnit.play();
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

    switch (m_playList.first()) {
        case Sound::CRANK:
            m_crank.stop();
            break;
        case Sound::RING:
            m_ring.stop();
            break;
        case Sound::TICTAC:
            m_tictac.stop();
            break;
        case Sound::CLICK:
            m_click.stop();
            break;
        case Sound::FINISH:
            m_finish.stop();
            break;
        case Sound::SHORTBREAK:
            m_shortBreak.stop();
            break;
        case Sound::LONGBREAK:
            m_longBreak.stop();
            break;
        case Sound::WORKUNIT:
            m_workUnit.stop();
            break;
        case Sound::NONE:
        default:
            break;
    }

    m_playList.removeFirst();

    if (!m_playList.empty()) {
        playNextSound();
    }
}
