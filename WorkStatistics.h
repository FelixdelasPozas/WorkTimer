/*
    File: WorkStatistics.h
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

#ifndef WORK_STATISTICS_H_
#define WORK_STATISTICS_H_

// Project
#include <WorkTimer.h>
#include "ui_WorkStatistics.h"

// Qt
#include <QDialog>
#include <QTimeLine>
#include <QTimer>

// C++
#include <memory>

class QObject;
class QEvent;

/** \class WorkStatistics
 *  \brief Work statistics and control dialog.
 *
 */
class WorkStatistics : public QDialog, public Ui_WorkStatistics
{
    Q_OBJECT
  public:
    /** \brief WorkStatistics class constructor.
	   * \param[in] worktimer work timer smart pointer.
	   * \param[in] parent raw pointer of the parent of this object.
	   * \param[in] f Window flags.
	   *
	   */
    explicit WorkStatistics(std::shared_ptr<WorkTimer> worktimer, QWidget* parent = nullptr,
                            Qt::WindowFlags f = Qt::WindowFlags());

    /** \brief WorkStatistics class virtual destructor.
		 *
		 */
    virtual ~WorkStatistics();

    enum class Result : char
    {
        None = 1,
        Stop = 2,
        Continue = 3
    };

    /** \brief Returns the result of the dialog after closing.
		 *
		 */
    Result getResult() const;

  private slots:
    /** \brief Updates the dialog with the Work values.
     *
     */
    void updateGUI();

    /** \brief Invalidates current Work unit.
     *
     */
    void invalidate();

    /** \brief Stops the Work and quits the dialog.
		 *
		 */
    void stop();

    /** \brief Closes the dialog.
		 *
		 */
    void resume();

    /** \brief Pauses the Work.
		 *
		 */
    void pause();

    /** \brief Updates the elapsed time in the dialog.
		 *
		 */
    void updateElapsedTime();

    /** \brief Updates the Work's task.
		 *
		 */
    void updateTask();

  private:
    /** \brief Updates the progress bar.
		 *
		 */
    void updateProgress();

    std::shared_ptr<WorkTimer> m_workTimer; /** Work object.                             */
    QTimer m_timer;                         /** update timer.                                */
    bool m_paused;                          /** true if Work is paused, false otherwise. */
    Result m_result;                        /** Current state */
};

#endif // WORK_STATISTICS_H_
