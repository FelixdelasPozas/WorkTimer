/*
    File: ConfigurationDialog.cpp
    Created on: 02/05/2025
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
#include <ConfigurationDialog.h>
#include <Utils.h>
#include <DesktopWidget.h>

// Qt
#include <QColorDialog>
#include <QApplication>
#include <QScreen>

const QStringList DEFAULT_POSITIONS = {"Top Left",     "Top Center",  "Top Right",     "Center Left", "Center",
                                       "Center Right", "Bottom Left", "Bottom Center", "Bottom Right"};

//----------------------------------------------------------------------------
ConfigurationDialog::ConfigurationDialog(const Utils::Configuration& config, QWidget* parent, Qt::WindowFlags f) :
    QDialog{parent, f}
{
    setupUi(this);
    connectSignals();

    QPixmap icon(QSize(64, 64));
    icon.fill(config.m_workColor);
    workColorButton->setIcon(QIcon(icon));
    workColorButton->setProperty("iconColor", config.m_workColor.name());

    icon.fill(config.m_shortBreakColor);
    shortColorButton->setIcon(QIcon(icon));
    shortColorButton->setProperty("iconColor", config.m_shortBreakColor.name());

    icon.fill(config.m_longBreakColor);
    longColorButton->setIcon(QIcon(icon));
    longColorButton->setProperty("iconColor", config.m_longBreakColor.name());

    computeWidgetPositions();
    onWidgetCheckBoxChanged();
    onUseSoundCheckBoxChanged();
}

//----------------------------------------------------------------------------
void ConfigurationDialog::onPositionChanged()
{
    if (positionComboBox->currentIndex() == 0) {
        // TODO
    }
}

//----------------------------------------------------------------------------
void ConfigurationDialog::onWidgetCheckBoxChanged()
{
    const auto enabled = widgetCheckBox->isChecked();
    positionComboBox->setEnabled(enabled);
    positionLabel->setEnabled(enabled);
    opacitySlider->setEnabled(enabled);
    opacitySpinBox->setEnabled(enabled);
    opacityLabel->setEnabled(enabled);
}

//----------------------------------------------------------------------------
void ConfigurationDialog::onUseSoundCheckBoxChanged()
{
    const auto enabled = soundCheckBox->isChecked();
    ticTacCheckBox->setEnabled(enabled);
}

//----------------------------------------------------------------------------
void ConfigurationDialog::showEvent(QShowEvent* e)
{
    QDialog::showEvent(e);
    Utils::scaleDialog(this);
}

//----------------------------------------------------------------------------
void ConfigurationDialog::getConfiguration(Utils::Configuration& config)
{
    config.m_breakTime = workSpinBox->value();
    config.m_shortBreakColor = shortSpinBox->value();
    config.m_longBreakColor = longSpinBox->value();
    config.m_unitsPerSession = sessionSpinBox->value();
    config.m_workColor = QColor(workColorButton->property("iconColor").toString());
    config.m_shortBreakColor = QColor(shortColorButton->property("iconColor").toString());
    config.m_longBreakColor = QColor(longColorButton->property("iconColor").toString());
    config.m_useWidget = widgetCheckBox->isChecked();
    config.m_widgetOpacity = opacitySpinBox->value();
    config.m_useSound = soundCheckBox->isChecked();
    config.m_continuousTicTac = ticTacCheckBox->isChecked();

    // TODO
    const auto posIdx = positionComboBox->currentIndex();
    const auto position = posIdx == 0 ? QPoint{0, 0} : m_widgetPositions.at(posIdx);
}

//----------------------------------------------------------------------------
void ConfigurationDialog::connectSignals()
{
    connect(workColorButton, SIGNAL(pressed()), this, SLOT(onColorButtonClicked()));
    connect(shortColorButton, SIGNAL(pressed()), this, SLOT(onColorButtonClicked()));
    connect(longColorButton, SIGNAL(pressed()), this, SLOT(onColorButtonClicked()));
    connect(widgetCheckBox, SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(onWidgetCheckBoxChanged()));
    connect(soundCheckBox, SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(onUseSoundCheckBoxChanged()));
}

//----------------------------------------------------------------------------
void ConfigurationDialog::computeWidgetPositions()
{
    QStringList positionNames;

    positionNames << tr("Custom (drag to position)");
    m_widgetPositions << QPoint{0, 0};

    const auto screens = QApplication::screens();
    computePositions(QRect{screens.at(0)->availableVirtualGeometry()}, "Global ", positionNames);

    for (int i = 0; i < screens.size(); ++i) {
        computePositions(screens.at(i)->availableGeometry(), QString("Monitor %1 ").arg(i), positionNames);
    }

    positionComboBox->insertItems(0, positionNames);
}

//----------------------------------------------------------------------------
void ConfigurationDialog::computePositions(const QRect& rect, const QString& screenName, QStringList& positionNames)
{
    auto widgetSize = DesktopWidget::WIDGET_SIZE;

    for (int y : {rect.y(), rect.y() + (rect.height() - widgetSize) / 2, rect.y() + rect.height() - widgetSize}) {
        for (int x : {rect.x(), rect.x() + (rect.width() - widgetSize) / 2, rect.x() + rect.width() - widgetSize}) {
            m_widgetPositions << QPoint{x, y};
        }
    }

    for (auto position : DEFAULT_POSITIONS) {
        positionNames << screenName + position;
    }
}

//----------------------------------------------------------------------------
void ConfigurationDialog::onColorButtonClicked()
{
    auto button = qobject_cast<QToolButton*>(sender());
    auto color = QColor(button->property("iconColor").toString());

    QColorDialog dialog;
    dialog.setCurrentColor(color);
    dialog.setOption(QColorDialog::ColorDialogOption::ShowAlphaChannel, true);
    dialog.setWindowIcon(QIcon(":/WorkTimer/configuration.svg"));

    if (dialog.exec() != QColorDialog::Accepted) {
        return;
    }

    color = dialog.selectedColor();

    QPixmap icon(QSize(64, 64));
    icon.fill(color);
    button->setIcon(QIcon(icon));
    button->setProperty("iconColor", color.name(QColor::HexArgb));
}
