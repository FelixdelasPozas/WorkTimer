/*
 File: AboutDialog.cpp
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
#include <AboutDialog.h>
#include <Utils.h>
#include <version.h>

// Qt
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QtGlobal>

// SQLite
extern "C" {
#include <sqlite3/sqlite3.h>
}

// libxlsxwriter
#include <xlsxwriter.h>


//-----------------------------------------------------------------
AboutDialog::AboutDialog(QWidget* parent, Qt::WindowFlags flags) :
    QDialog(parent, flags)
{
    setupUi(this);

    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint) & ~(Qt::WindowMaximizeButtonHint) &
                   ~(Qt::WindowMinimizeButtonHint));

    const auto compilation_date = QString(__DATE__);
    const auto compilation_time = QString(" (") + QString(__TIME__) + QString(")");

    m_compilationDate->setText(tr("Compiled on ") + compilation_date + compilation_time);
    m_version->setText(WorkTimer_Version);
    m_xlsxversion->setText(tr("version %1").arg(QString::fromLocal8Bit(lxw_version())));
    m_qtVersion->setText(tr("version %1").arg(qVersion()));
    m_sqliteVersion->setText(QString("version %1").arg(QString::fromLatin1(sqlite3_version)));    
    m_copy->setText(
        tr("Copyright (c) %1 Félix de las Pozas Álvarez").arg(QDateTime::currentDateTime().date().year()));

    QObject::connect(m_kofiLabel, &Utils::ClickableHoverLabel::clicked,
                     []() { QDesktopServices::openUrl(QUrl{"https://ko-fi.com/felixdelaspozas"}); });
}

//-----------------------------------------------------------------
void AboutDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    Utils::scaleDialog(this);
    Utils::centerDialog(this);
}
