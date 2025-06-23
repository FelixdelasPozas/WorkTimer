/*
 File: DesktopWidget.cpp
 Created on: 25/11/2015
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

// project
#include <DesktopWidget.h>
#include <Utils.h>

// Qt
#include <QPainter>
#include <QPointF>
#include <QMouseEvent>
#include <QScreen>
#include <QApplication>
#include <QPixmap>
#include <QSvgRenderer>
#include <QPainterPath>

const int DesktopWidget::WIDGET_SIZE = 100;

//-----------------------------------------------------------------
DesktopWidget::DesktopWidget(bool dragEnable, QWidget* parent) :
    QWidget{parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint |
                        Qt::WindowTransparentForInput | Qt::Tool},
    m_progress{0},
    m_color{Qt::black},
    m_name{""},
    m_buttonDown{false}
{
    // NOTE 1: attribute Qt::WA_TransparentForMouseEvents is useless, use Qt::WindowTransparentForInput instead.
    // NOTE 2: Qt::WindowTransparentForInput collides with WindowOkButtonHint so once set/unset there is no
    //         way to enable it again, thus, enabled or disabled on constructor.
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setAttribute(Qt::WA_TranslucentBackground);

    if (dragEnable) {
        setWindowFlags(windowFlags() & ~Qt::WindowTransparentForInput);
    }

    const auto desktopRect = QApplication::screens().at(0)->availableVirtualGeometry();
    m_limitX = desktopRect.width() - WIDGET_SIZE;
    m_limitY = desktopRect.height() - WIDGET_SIZE;

    setGeometry(QRect(0, 0, WIDGET_SIZE, WIDGET_SIZE));
    setWindowOpacity(0.60);

    move(0, 0);
}

//-----------------------------------------------------------------
void DesktopWidget::setProgress(double value)
{
    if ((m_progress != value) && (value >= 0) && (value <= 100)) {
        m_progress = value;

        if (isVisible()) {
            setAttribute(Qt::WA_AlwaysStackOnTop);
            update();
            repaint();
        }
    }
}

//-----------------------------------------------------------------
void DesktopWidget::mousePressEvent(QMouseEvent* e)
{
    if (underMouse() && e->button() == Qt::MouseButton::LeftButton) {
        m_point = e->globalPosition().toPoint();

        m_buttonDown = true;
        setOpacity(80);
        emit beingDragged();
    } else {
        QWidget::mousePressEvent(e);
    }
}

//-----------------------------------------------------------------
void DesktopWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (underMouse() && e->button() == Qt::MouseButton::LeftButton) {
        m_point -= e->globalPosition().toPoint();
        setPosition(pos() - m_point);
        m_point = e->globalPosition().toPoint();

        m_buttonDown = false;
        setOpacity(60);
    } else {
        QWidget::mouseReleaseEvent(e);
    }
}

//-----------------------------------------------------------------
void DesktopWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (m_buttonDown) {
        m_point -= e->globalPosition().toPoint();
        setPosition(pos() - m_point);
        m_point = e->globalPosition().toPoint();
    } else {
        QWidget::mouseMoveEvent(e);
    }
}

//-----------------------------------------------------------------
void DesktopWidget::setPosition(const QPoint& position)
{
    if (position != pos()) {
        auto newPosition = position;
        if (newPosition.x() < 0) {
            newPosition.setX(0);
        }
        if (newPosition.y() < 0) {
            newPosition.setY(0);
        }

        if (newPosition.x() > m_limitX) {
            newPosition.setX(m_limitX);
        }
        if (newPosition.y() > m_limitY) {
            newPosition.setY(m_limitY);
        }

        move(newPosition);
    }
}

//-----------------------------------------------------------------
void DesktopWidget::setOpacity(const int opacity)
{
    if (opacity != windowOpacity()) {
        setWindowOpacity(static_cast<double>(opacity) / 100.0);

        if (isVisible()) {
            repaint();
        }
    }
}

//-----------------------------------------------------------------
void DesktopWidget::setColor(const QColor& color)
{
    m_color = color;

    const auto blackDistance = color.red() + color.green() + color.blue();
    const auto whiteDistance = (254 * 3) - blackDistance;

    m_contrastColor = (blackDistance < whiteDistance ? "white" : "black");

    if (isVisible()) {
        repaint();
    }
}

//-----------------------------------------------------------------
void DesktopWidget::setTitle(const QString& name)
{
    if (m_name != name) {
        m_name = name;

        if (isVisible()) {
            repaint();
        }
    }
}

//-----------------------------------------------------------------
QIcon DesktopWidget::asIcon(unsigned int minutes)
{
    QPixmap image{QSize{128,128}};
    image.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&image);
    paintWidget(painter, image.rect());

    auto font = QFont("Arial", 65);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    // need to invert whatever color the user choosed to see the minutes.
    painter.setCompositionMode(QPainter::CompositionMode_Exclusion);
    painter.drawText(image.rect(), Qt::AlignCenter, QString::number(minutes));
    painter.end();

    return QIcon(image);
}

//-----------------------------------------------------------------
void DesktopWidget::paintEvent(QPaintEvent* e)
{
    if(!isVisible()) return;
    
    QBrush brush(m_contrastColor, Qt::SolidPattern);
    const auto windowRect = rect();

    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);
    painter.setRenderHint(QPainter::RenderHint::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::RenderHint::TextAntialiasing, true);
    paintWidget(painter, rect());

    if(!m_icon.isEmpty())
    {
        auto svgRect = QRect{windowRect.x() + 15, windowRect.y() + 15, windowRect.width() - 30, windowRect.height() - 30};
        auto pixmap = Utils::svgPixmap(m_icon, m_color.darker());
        painter.setOpacity(0.5);
        painter.drawPixmap(svgRect, pixmap, pixmap.rect());
    }

    painter.setOpacity(1.);

    auto font = QFont("Arial", 15);
    font.setBold(true);
    QFontMetrics fm{font};
    auto height = fm.height();

    const auto color = (m_contrastColor == Qt::black ? Qt::white : Qt::black);
    painter.setPen(color);
    painter.setBrush(m_contrastColor);

    QString displayText;
    auto parts = m_name.split(" ");
    int totalHeight = parts.size() * height;
    while(totalHeight > windowRect.height())
    {
        font.setPointSize(font.pointSize()-1);
        fm = QFontMetrics{font};
        height = fm.height();
        totalHeight = parts.size() * height;
    }

    int maxWidth = windowRect.width() + 1;
    std::function<int(QFontMetrics &)> computeMaxWidth = [&maxWidth, &parts](QFontMetrics &f) {
        int maxValue = 0;
        for (const auto& part : parts) {
            maxValue = std::max(maxValue, f.boundingRect(part).width());
        }
        return maxValue;
    };

    bool started = false;
    while(maxWidth > windowRect.width())
    {
        if(started)
        {
            font.setPointSize(font.pointSize() - 1);
            fm = QFontMetrics{font};
        }
        maxWidth = computeMaxWidth(fm);
        height = fm.height();
        totalHeight = parts.size() * height;
        started = true;
    }

    painter.setFont(font);
    auto pen = painter.pen();
    pen.setWidth(1);
    painter.setPen(pen);
    const float initialY = ((windowRect.height() - totalHeight) - (height/2)) / 2;
    for (int i = 0; i < parts.size(); ++i) {
        const auto part = parts.at(i);
        const auto width = fm.boundingRect(part).width();
        const float xPos = (windowRect.width() - width) / 2;

        QPainterPath ppath;
        ppath.addText({xPos, initialY + ((1+i) * height)}, font, part);
        painter.drawPath(ppath);
    }
    painter.end();
}

//-----------------------------------------------------------------
void DesktopWidget::paintWidget(QPainter &painter, const QRect &windowRect)
{
    QBrush brush(m_contrastColor, Qt::SolidPattern);

    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(brush);
    painter.setPen(m_contrastColor);
    painter.drawRoundedRect(windowRect, 30, 30);

    brush.setColor(m_color);
    brush.setStyle(Qt::SolidPattern);
    painter.setPen(m_color);
    painter.setBrush(brush);

    auto smallRect = QRect{windowRect.x() + 5, windowRect.y() + 5, windowRect.width() - 10, windowRect.height() - 10};
    const auto progressValue = (360.0 - 360.0 * (m_progress / 100.0)) * 16;
    painter.drawPie(smallRect, 90 * 16, progressValue);
}
