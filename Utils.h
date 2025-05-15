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

class QDialog;

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

      private:
        /** \brief Helper method that returns the QSettings object to use.
         */
        QSettings applicationSettings() const;
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
} // namespace Utils

#endif // UTILS_H_