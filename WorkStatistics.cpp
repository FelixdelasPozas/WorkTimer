/*
    File: WorkStatistics.cpp
    Created on: 22/04/2025
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
#include <WorkStatistics.h>

// Qt
#include <QKeyEvent>
#include <QObject>
#include <QEvent>
#include <QLineEdit>
#include <QInputDialog>
#include <QPainter>
#include <QFontMetrics>
#include <QStyleFactory>

//-----------------------------------------------------------------
WorkStatistics::WorkStatistics(std::shared_ptr<WorkTimer> worktimer, QWidget* parent, Qt::WindowFlags f)
: QDialog(parent, f)
, m_workTimer{worktimer}
, m_paused{worktimer->status() == WorkTimer::Status::Paused}
, m_result{Result::None}
{
	setupUi(this);
	setWindowFlags(Qt::Window|Qt::CustomizeWindowHint|Qt::WindowTitleHint|Qt::WindowSystemMenuHint);

	m_status->setTextFormat(Qt::RichText);
	m_progress->setStyle(QStyleFactory::create("windowsvista"));

  connect(m_workTimer.get(), SIGNAL(beginWork()),
          this,             SLOT(updateGUI()));

  connect(m_workTimer.get(), SIGNAL(beginShortBreak()),
          this,             SLOT(updateGUI()));

  connect(m_workTimer.get(), SIGNAL(beginLongBreak()),
          this,             SLOT(updateGUI()));

  connect(m_workTimer.get(), SIGNAL(sessionEnded()),
          this,             SLOT(updateGUI()));

	connect(m_invalidate, SIGNAL(pressed()),
	        this,         SLOT(invalidate()));

	connect(m_stop, SIGNAL(pressed()),
	        this,   SLOT(stop()));

	connect(m_continue, SIGNAL(pressed()),
	        this,       SLOT(resume()));

	connect(m_pause, SIGNAL(pressed()),
	        this,   SLOT(pause()));

	if(m_workTimer->status() != WorkTimer::Status::Stopped)
	{
		m_timer.setInterval(1000);
		m_timer.setSingleShot(false);

		connect(&m_timer, SIGNAL(timeout()),
		        this,     SLOT(updateElapsedTime()), Qt::DirectConnection);

		connect(m_taskButton, SIGNAL(pressed()),
		        this,         SLOT(updateTask()), Qt::QueuedConnection);

		m_timer.start();
	}

	updateGUI();
}

//-----------------------------------------------------------------
WorkStatistics::~WorkStatistics()
{
  disconnect(m_workTimer.get(), SIGNAL(beginWork()),
          this,                SLOT(updateGUI()));

  disconnect(m_workTimer.get(), SIGNAL(beginShortBreak()),
          this,                SLOT(updateGUI()));

  disconnect(m_workTimer.get(), SIGNAL(beginLongBreak()),
          this,                SLOT(updateGUI()));

  disconnect(m_workTimer.get(), SIGNAL(sessionEnded()),
          this,                SLOT(updateGUI()));

	disconnect(&m_timer, SIGNAL(timeout()),
	           this,     SLOT(updateElapsedTime()));

	m_timer.stop();
}

//-----------------------------------------------------------------
WorkStatistics::Result WorkStatistics::getResult() const
{
  return m_result;
}

//-----------------------------------------------------------------
void WorkStatistics::invalidate()
{
	m_workTimer->invalidateCurrent();

	updateGUI();
}

//-----------------------------------------------------------------
void WorkStatistics::stop()
{
	m_workTimer->stop();
	m_result = Result::Stop;
	accept();
}

//-----------------------------------------------------------------
void WorkStatistics::resume()
{
	m_result = Result::Continue;
	accept();
}

//-----------------------------------------------------------------
void WorkStatistics::pause()
{
	m_paused = !m_paused;
	m_workTimer->pause();

	updateGUI();
}

//-----------------------------------------------------------------
void WorkStatistics::updateElapsedTime()
{
	if (m_workTimer->status() == WorkTimer::Status::Stopped)
	{
		m_elapsedTime->setText("--:--:--");
	}
	else
	{
		auto elapsedTime = m_workTimer->elapsedTime();
		m_elapsedTime->setText(elapsedTime.toString(Qt::TextDate));

		updateProgress();
	}

	m_elapsedTime->repaint();
}

//-----------------------------------------------------------------
void WorkStatistics::updateProgress()
{
  auto time = QTime(0,0,0,0);

  auto totalSecs = time.secsTo(m_workTimer->getWorkDuration()) * m_workTimer->completedWorkUnits();
  totalSecs += time.secsTo(m_workTimer->getShortBreakDuration()) * m_workTimer->completedShortBreaks();
  totalSecs += time.secsTo(m_workTimer->getLongBreakDuration()) * m_workTimer->completedLongBreaks();

  const auto elapsedSecs = time.secsTo(m_workTimer->elapsedTime());
  const auto sessionSecs = time.secsTo(m_workTimer->sessionTime());

  m_progress->setValue(((totalSecs+elapsedSecs)*100)/sessionSecs);
}

//-----------------------------------------------------------------
void WorkStatistics::updateGUI()
{
  if (m_paused)
  {
    m_pause->setText("Resume");
  }
  else
  {
    m_pause->setText("Pause");
  }

  m_invalidate->setEnabled(!m_paused);
  m_continue  ->setEnabled(!m_paused);
  m_stop      ->setEnabled(!m_paused);

	unsigned long totalSecs = 0;
	auto WorkTime = QTime(0, 0, 0, 0);

	unsigned long seconds = WorkTime.secsTo(m_workTimer->getWorkDuration());
	seconds *= m_workTimer->completedWorkUnits();
	totalSecs += seconds;
	WorkTime = WorkTime.addSecs(seconds);

	m_completedWorkUnits->setText(QString("%1 (%2)").arg(m_workTimer->completedWorkUnits()).arg(WorkTime.toString(Qt::TextDate)));

	auto shortBreakTime = QTime(0, 0, 0, 0);
	seconds = shortBreakTime.secsTo(m_workTimer->getShortBreakDuration());
	seconds *= m_workTimer->completedShortBreaks();
	totalSecs += seconds;
	shortBreakTime = shortBreakTime.addSecs(seconds);

	m_completedShortBreaks->setText(QString("%1 (%2)").arg(m_workTimer->completedShortBreaks()).arg(shortBreakTime.toString(Qt::TextDate)));

	auto longBreakTime = QTime(0, 0, 0, 0);
	seconds = longBreakTime.secsTo(m_workTimer->getLongBreakDuration());
	seconds *= m_workTimer->completedLongBreaks();
	totalSecs += seconds;
	longBreakTime = longBreakTime.addSecs(seconds);

	m_completedLongBreaks->setText(QString("%1 (%2)").arg(m_workTimer->completedLongBreaks()).arg(longBreakTime.toString(Qt::TextDate)));

	auto totalTime = QTime(0, 0, 0, 0);
	totalTime = totalTime.addSecs(totalSecs);

	auto elapsedTime = m_workTimer->elapsedTime();
	m_elapsedTime->setText(elapsedTime.toString(Qt::TextDate));

	updateProgress();

	switch(m_workTimer->status())
	{
	  case WorkTimer::Status::LongBreak:
	    m_status->setText("<span style=\" color:#00ff00;\">In a long break</span>");
	    break;
	  case WorkTimer::Status::ShortBreak:
			m_status->setText("<span style=\" color:#0000ff;\">In a short break</span>");
	    break;
	  case WorkTimer::Status::Work:
	    m_status->setText("<span style=\" color:#ff0000;\">In a Work</span>");
	    break;
	  case WorkTimer::Status::Paused:
			m_status->setText("<span style=\" color:#000000;\">Paused</span>");
	    break;
	  case WorkTimer::Status::Stopped:
      m_status->setText("<span style=\" color:#000000;\">Session finished</span>");
      m_elapsedTime->setText("--:--:--");
      m_invalidate->setEnabled(false);
      m_pause->setEnabled(false);
	    break;
	  default:
	    Q_ASSERT(false);
	    break;
	}

	m_completedTime->setText(totalTime.toString(Qt::TextDate));
	m_taskName->setText(m_workTimer->getTaskTitle());

	repaint();
}

//-----------------------------------------------------------------
void WorkStatistics::updateTask()
{
	 bool ok;
	 const auto text = QInputDialog::getText(this,
			                                     tr("Enter task name"),
			                                     tr("Task name:"),
			                                     QLineEdit::Normal,
			                                     m_taskName->text(), &ok);
	 if (ok && !text.isEmpty())
	 {
		 m_taskName->setText(text);
		 m_workTimer->setTaskTitle(m_taskName->text());
	 }

	 m_taskButton->setDown(false);
}
