/*
    File: RangeSelectorWidget.cpp
    Created on: 27/04/2025
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
#include <RangeSelectorWidget.h>

// Qt
#include <QDateTime>
#include <QMenu>
#include <QAction>

const QString CSV_FILE = "CSV file";
const QString XLS_FILE = "Excel file";

//----------------------------------------------------------------------------
RangeSelectorWidget::RangeSelectorWidget(QWidget* parent, Qt::WindowFlags f) :
    QWidget{parent, f}
{
    setupUi(this);
    connectSignals();

    m_weekButton->setChecked(true);
    m_fromDateEdit->setTimeZone(QTimeZone::LocalTime);
    m_toDateEdit->setTimeZone(QTimeZone::LocalTime);

    auto menu = new QMenu(m_export);
    auto csvAction = new QAction(QIcon(":/WorkTimer/csv.svg"), CSV_FILE, menu);
    auto excelAction = new QAction(QIcon(":/WorkTimer/excel.svg"), XLS_FILE, menu);
    menu->addAction(csvAction);
    menu->addAction(excelAction);
    m_export->setMenu(menu);

    connect(csvAction, SIGNAL(triggered(bool)), this, SLOT(onExportClicked()));
    connect(excelAction, SIGNAL(triggered(bool)), this, SLOT(onExportClicked()));
}

//----------------------------------------------------------------------------
void RangeSelectorWidget::setRange(const QDateTime& from, const QDateTime& to, const bool emitSignal)
{
    auto fromDate = from;
    auto toDate = to;
    fromDate.setTime(QTime{0,0,0});
    toDate.setTime(QTime{23,59,59});

    m_fromDateEdit->blockSignals(true);
    m_toDateEdit->blockSignals(true);

    m_fromDateEdit->setDateRange(QDate(), to.addDays(-1).date());
    m_toDateEdit->setDateRange(from.addDays(1).date(), QDateTime::currentDateTime().date());
    m_fromDateEdit->setDateTime(fromDate);
    m_toDateEdit->setDateTime(toDate);

    m_fromDateEdit->blockSignals(false);
    m_toDateEdit->blockSignals(false);

    if (emitSignal) {
        emit rangeChanged(m_fromDateEdit->dateTime(), m_toDateEdit->dateTime());
    }
}

//----------------------------------------------------------------------------
void RangeSelectorWidget::setButton(const Button button)
{
    m_weekButton->blockSignals(true);
    m_monthButton->blockSignals(true);
    m_yearButton->blockSignals(true);

    switch (button) {
        case Button::WEEK:
            m_weekButton->setChecked(true);
            break;
        case Button::MONTH:
            m_monthButton->setChecked(true);
            break;
        case Button::YEAR:
            m_yearButton->setChecked(true);
            break;
        default:
        case Button::CUSTOM:
            m_custom->setChecked(true);
            break;
    }

    m_weekButton->blockSignals(false);
    m_monthButton->blockSignals(false);
    m_yearButton->blockSignals(false);
}

//----------------------------------------------------------------------------
void RangeSelectorWidget::onButtonClicked()
{
    auto button = qobject_cast<QPushButton*>(sender());
    if (button && button->isChecked()) {
        QDateTime from;
        QDateTime to;
        auto currentDate = QDateTime::currentDateTime();
        currentDate.setTime(QTime{0, 0, 0});

        if (button == m_weekButton) {
            auto monday = currentDate.addDays(1 - currentDate.date().dayOfWeek());
            monday.setTime(QTime{0,0,0});
            auto weekEnd = monday.addDays(6);
            weekEnd.setTime(QTime{23,59,59});
            setRange(monday, weekEnd);
            return;
        }

        if (button == m_monthButton) {
            const auto first = QDateTime(QDate{currentDate.date().year(),currentDate.date().month(), 1}, QTime{0,0,0});
            const auto last = QDateTime(QDate{currentDate.date().year(),currentDate.date().month(), currentDate.date().daysInMonth()}, QTime{23,59,59});
            setRange(first, last);
            return;
        }

        if (button == m_yearButton) {
            const auto first = QDateTime(QDate{currentDate.date().year(),1, 1}, QTime{0,0,0}); 
            const auto last = QDateTime(QDate{currentDate.date().year(),12, 31}, QTime{23,59,59}); 
            setRange(first, last);
            return;
        }

        if (button == m_custom) {
        }
    }
}

//----------------------------------------------------------------------------
void RangeSelectorWidget::onDateChanged()
{
    auto dateEdit = qobject_cast<QDateEdit*>(sender());
    if (dateEdit && dateEdit->isEnabled()) {
        setRange(m_fromDateEdit->dateTime(), m_toDateEdit->dateTime());
    }
}

//----------------------------------------------------------------------------
void RangeSelectorWidget::onExportClicked()
{
    auto action = qobject_cast<QAction *>(sender());
    if(action)
    {
        if(action->text().compare(CSV_FILE) == 0)
        {
            emit exportDataCSV(m_fromDateEdit->dateTime(), m_toDateEdit->dateTime());
        }
        else
            emit exportDataExcel(m_fromDateEdit->dateTime(), m_toDateEdit->dateTime());
    }
}

//----------------------------------------------------------------------------
void RangeSelectorWidget::connectSignals()
{
    connect(m_weekButton, SIGNAL(toggled(bool)), this, SLOT(onButtonClicked()));
    connect(m_monthButton, SIGNAL(toggled(bool)), this, SLOT(onButtonClicked()));
    connect(m_yearButton, SIGNAL(toggled(bool)), this, SLOT(onButtonClicked()));
    connect(m_custom, SIGNAL(toggled(bool)), this, SLOT(onButtonClicked()));
    connect(m_fromDateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(onDateChanged()));
    connect(m_toDateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(onDateChanged()));
}

//----------------------------------------------------------------------------
RangeSelectorWidget::Button RangeSelectorWidget::button() const
{
    if(m_weekButton->isChecked())
        return Button::WEEK;

    if(m_monthButton->isChecked())
        return Button::MONTH;

    if(m_yearButton->isChecked())
        return Button::YEAR;

    return Button::CUSTOM;
}