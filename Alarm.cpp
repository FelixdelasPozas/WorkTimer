/*
		File: Alarm.cpp
    Created on: 04/07/2015
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
#include <Alarm.h>

Alarm::AlarmTime::AlarmTime(int d, int h, int m, int s)
{
  seconds = s % 60;
  minutes = (m % 60) + (s / 60);
  hours = (h % 24) + (minutes / 60);
  days = d + (hours / 24);
}

//-----------------------------------------------------------------
QString Alarm::AlarmTime::text() const
{
    QString text;

    if (days != 0) {
        text += QString("%1 Day%2 ").arg(days).arg((days > 1 ? "s" : ""));
    }

    if (hours > 0) {
        text += QTime{hours, minutes, seconds}.toString("hh:mm:ss");
    } else if (minutes > 0) {
        text += QTime{hours, minutes, seconds}.toString("mm:ss");
    } else {
        text += QTime{hours, minutes, seconds}.toString("ss");
    }

    return text;
}

//-----------------------------------------------------------------
Alarm::Alarm(Alarm::AlarmTime time, bool loop) :
    m_time{time},
    m_remainingTime{time},
    m_loop{loop},
    m_progress{0},
    m_dProgress{0}
{
    m_timer.setInterval(1000);
    m_timer.setSingleShot(false);

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(second()));
}

//-----------------------------------------------------------------
void Alarm::start()
{
    if (!m_timer.isActive()) {
        m_timer.start();
    }
}

//-----------------------------------------------------------------
void Alarm::stop()
{
    if (m_timer.isActive()) {
        m_timer.stop();

        m_progress = 0;
        m_dProgress = 0;
        m_remainingTime = m_time;
    }
}

//-----------------------------------------------------------------
void Alarm::pause(bool paused)
{
    if (paused == m_timer.isActive()) {
        paused ? m_timer.stop() : m_timer.start();
    }
}

//-----------------------------------------------------------------
bool Alarm::isRunning() const
{
    return m_timer.isActive();
}

//-----------------------------------------------------------------
unsigned int Alarm::progress() const
{
    return m_progress;
}

//-----------------------------------------------------------------
double Alarm::precisionProgress() const
{
    return m_dProgress;
}

//-----------------------------------------------------------------
const Alarm::AlarmTime Alarm::remainingTime() const
{
    return m_remainingTime;
}

//-----------------------------------------------------------------
const Alarm::AlarmTime Alarm::time() const
{
    return m_time;
}

//-----------------------------------------------------------------
void Alarm::second()
{
    auto beforeProgress = m_progress;

    --m_remainingTime.seconds;

    if ((m_remainingTime.seconds == 0) && (m_remainingTime.minutes == 0) && (m_remainingTime.hours == 0) &&
        (m_remainingTime.days == 0)) {
        emit timeout();

        if (!m_loop) {
            m_timer.stop();
        } else {
            m_remainingTime = m_time;
        }
    } else {
        if (m_remainingTime.seconds == -1) {
            --m_remainingTime.minutes;
            m_remainingTime.seconds = 59;

            if (m_remainingTime.minutes == -1) {
                --m_remainingTime.hours;
                m_remainingTime.minutes = 59;

                if (m_remainingTime.hours == -1) {
                    --m_remainingTime.days;
                    m_remainingTime.hours = 23;
                }
            }
        }
    }

    computeProgressValues();

    emit tic();

    if (beforeProgress != m_progress) {
        emit progress(m_progress);
    }
}

//-----------------------------------------------------------------
QString Alarm::remainingTimeText() const
{
    return m_remainingTime.text();
}

//-----------------------------------------------------------------
void Alarm::computeProgressValues()
{
    constexpr unsigned long long secondsInDay = 24 * 60 * 60;
    constexpr unsigned long long secondsInHour = 60 * 60;
    unsigned long long totalTime =
        (m_time.days * secondsInDay) + (m_time.hours * secondsInHour) + (m_time.minutes * 60) + m_time.seconds;
    unsigned long long remaining = (m_remainingTime.days * secondsInDay) + (m_remainingTime.hours * secondsInHour) +
                                   (m_remainingTime.minutes * 60) + m_remainingTime.seconds;

    m_progress = 100 - static_cast<int>((100 * remaining) / static_cast<double>(totalTime));
    m_dProgress = 100.0 - (100 * remaining) / static_cast<double>(totalTime);
}
