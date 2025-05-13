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

// Qt
#include <QPainter>
#include <QPainterPath>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDialog>
#include <QWidget>
#include <QSvgRenderer>

// C++
#include <iostream>

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
    m_workColor = QColor::fromString(settings.value(WORK_COLOR, "#00FF00").toString());
    m_shortBreakColor = QColor::fromString(settings.value(SMALLBREAK_COLOR, "#FFFF00").toString());
    m_longBreakColor = QColor::fromString(settings.value(LARGEBREAK_COLOR, "#0000FF").toString());
    m_useWidget = settings.value(DESKTOP_WIDGET, true).toBool();
    m_widgetOpacity = settings.value(DESKTOP_OPACITY, 75).toInt();
    m_widgetPosition = settings.value(DESKTOP_POSITION, QPoint{0, 0}).toPoint();
    m_useSound = settings.value(SOUND_USE, true).toBool();
    m_continuousTicTac = settings.value(SOUND_TIC_TAC, false).toBool();
}

//-----------------------------------------------------------------
void Utils::Configuration::save()
{
    QSettings settings = applicationSettings();
    settings.setValue(WORKUNIT_TIME, m_workUnitTime);
    settings.setValue(SMALLBREAK_TIME, m_shortBreakTime);
    settings.setValue(LARGEBREAK_TIME, m_longBreakTime);
    settings.setValue(UNITS_IN_SESSION, m_unitsPerSession);
    settings.setValue(WORK_COLOR, m_workColor.name());
    settings.setValue(SMALLBREAK_COLOR, m_shortBreakColor.name());
    settings.setValue(LARGEBREAK_COLOR, m_longBreakColor.name());
    settings.setValue(DESKTOP_WIDGET, m_useWidget);
    settings.setValue(DESKTOP_OPACITY, m_widgetOpacity);
    settings.setValue(DESKTOP_POSITION, m_widgetPosition);
    settings.setValue(SOUND_USE, m_useSound);
    settings.setValue(SOUND_TIC_TAC, m_continuousTicTac);

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
