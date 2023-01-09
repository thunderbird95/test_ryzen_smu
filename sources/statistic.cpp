#include "statistic.h"
#include "ui_statistic.h"

#include "definitions.h"
#include "ryzen_control.h"

#include <QScrollBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>

//-----------------------------------------------------------------------------------------------------------------------------------
Statistic::Statistic(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Statistic)
{
    ui->setupUi(this);

    //m_ryzenControl = static_cast<RyzenControl*>(this->parent());

    connect(ui->clearReferenceStatistic, &QPushButton::clicked, this, [ this ] () {
        m_referenceStatisticCounter = 0; m_referenceStatistic.fill({ .min = 0, .max = 0, .sum = 0 });
        displayStatistic(m_referenceStatistic, m_referenceStatisticCounter, ui->referenceTable, ui->referenceStatisticCounter);
    }   );

    connect(ui->clearCurrentStatistic, &QPushButton::clicked, this, [ this ] () {
        m_currentStatisticCounter = 0; m_currentStatistic.fill({ .min = 0, .max = 0, .sum = 0 });
        displayStatistic(m_currentStatistic, m_currentStatisticCounter, ui->currentTable, ui->currentStatisticCounter);
    }   );


    connect(ui->referenceTable->verticalScrollBar(), &QScrollBar::valueChanged, [this] (int position) { ui->currentTable->verticalScrollBar()->setValue(position); });
    connect(ui->currentTable->verticalScrollBar(), &QScrollBar::valueChanged, [this] (int position) { ui->referenceTable->verticalScrollBar()->setValue(position); });

    connect(ui->saveReference, &QPushButton::clicked, this, &Statistic::saveStatistic);
    connect(ui->loadReference, &QPushButton::clicked, this, &Statistic::loadStatistic);
    connect(ui->saveCurrent, &QPushButton::clicked, this, &Statistic::saveStatistic);
    connect(ui->loadCurrent, &QPushButton::clicked, this, &Statistic::loadStatistic);

//    m_testDisablingWidgets << ui->loadReference << ui->saveReference << ui->loadCurrent << ui->saveCurrent << ui->clearCurrentStatistic
//                           << ui->clearReferenceStatistic << ui->enableCollectCurrentStatistic << ui->enableCollectReferenceStatistic;
    m_testDisablingWidgets.append(ui->enableCollectReferenceStatistic);
    m_testDisablingWidgets.append(ui->loadReference);
    m_testDisablingWidgets.append(ui->saveReference);
    m_testDisablingWidgets.append(ui->clearReferenceStatistic);
    m_testDisablingWidgets.append(ui->enableCollectCurrentStatistic);
    m_testDisablingWidgets.append(ui->loadCurrent);
    m_testDisablingWidgets.append(ui->saveCurrent);
    m_testDisablingWidgets.append(ui->clearCurrentStatistic);

    connect(ui->startTest, &QPushButton::clicked, this, &Statistic::startTest);
    connect(ui->stopTest, &QPushButton::clicked, [ this ] () {
        foreach(QWidget* widget, m_testDisablingWidgets)    widget->setEnabled(true);
        if (!m_testState.isEnabled) return;
        m_testSettingsWidget.requestSettingsDescription();
        m_testState.isEnabled = false; testLog(QString("Test aborted by user")); } );

    connect(ui->viewTestSettings, &QPushButton::clicked, &m_testSettingsWidget, &TestSettings::show);

    //connect(&m_testSettingsWidget, &TestSettings::settingsAreRight, ui->startTest, &QPushButton::setEnabled);
    connect(&m_testSettingsWidget, &TestSettings::settingsChanged, [this] (bool rightFlag, QString description, QString extendedDescription) {
        ui->startTest->setEnabled(rightFlag); ui->labelTestSettingsDescription->setText(description); ui->labelTestSettingsDescription->setToolTip(extendedDescription); } );

    m_testSettingsWidget.requestSettingsDescription();
}

//-----------------------------------------------------------------------------------------------------------------------------------
Statistic::~Statistic()
{
    delete ui;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::closeEvent(QCloseEvent *event)
{
    m_testSettingsWidget.close();
    event->accept();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::setDescriptions(QVector<QString> descriptions)
{
    m_names = descriptions;
    m_referenceStatistic.resize(descriptions.length());
    m_referenceStatisticCounter = 0;
    m_referenceStatistic.fill({ .min = 0, .max = 0, .sum = 0 });
    m_currentStatistic.resize(descriptions.length());
    m_currentStatisticCounter = 0;
    m_currentStatistic.fill({ .min = 0, .max = 0, .sum = 0 });
    ui->referenceTable->setRowCount(descriptions.length());
    for (int i = 0; i < descriptions.length(); i++)
        ui->referenceTable->setVerticalHeaderItem(i, new QTableWidgetItem(QString("%1: %2").arg(i).arg(descriptions[i])));
    ui->currentTable->setRowCount(descriptions.length());
    for (int i = 0; i < descriptions.length(); i++)
        ui->currentTable->setVerticalHeaderItem(i, new QTableWidgetItem(QString("%1").arg(i)));
    displayStatistic(m_referenceStatistic, m_referenceStatisticCounter, ui->referenceTable, ui->referenceStatisticCounter);
    displayStatistic(m_currentStatistic, m_currentStatisticCounter, ui->currentTable, ui->currentStatisticCounter);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::setRyzenControlLink(RyzenControl *link)
{
    m_ryzenControl = link;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::saveStatistic()
{
    QPushButton* sender = qobject_cast<QPushButton*>(this->sender());
    QString saveFileName;
    if (sender == 0)
        saveFileName = QString("saved_statistic.txt");
    else
    {
        saveFileName = QFileDialog::getSaveFileName(this);
        if (saveFileName.isEmpty())
            return;
    }

    if (sender == ui->saveCurrent)
        saveStatisticToFile(m_currentStatistic, m_currentStatisticCounter, saveFileName);
    else
        saveStatisticToFile(m_referenceStatistic, m_referenceStatisticCounter, saveFileName);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::loadStatistic()
{
    QPushButton* sender = qobject_cast<QPushButton*>(this->sender());
    QString loadFileName;
    if (sender == 0)
        loadFileName = QString("saved_statistic.txt");
    else
    {
        loadFileName = QFileDialog::getOpenFileName(this);
        if (loadFileName.isEmpty())
            return;
    }

    bool isSuccess = false;
    if (sender == ui->loadCurrent)
        isSuccess = loadStatisticFromFile(m_currentStatistic, &m_currentStatisticCounter, loadFileName);
    else
        isSuccess = loadStatisticFromFile(m_referenceStatistic, &m_referenceStatisticCounter, loadFileName);

    if (isSuccess)
        displayStatistic(m_referenceStatistic, m_referenceStatisticCounter, ui->referenceTable, ui->referenceStatisticCounter);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::handleNewValues(QVector<float> newValues)
{
    QVector<StatisticData>* currentStatistic;
    int* counter;
    QTableWidget* table;
    QLabel* counterLabel;
    QVector<StatisticData>* referenceStatistic = 0;

    if (ui->enableCollectReferenceStatistic->isChecked())
    {
        currentStatistic = &m_referenceStatistic;
        counter = &m_referenceStatisticCounter;
        table = ui->referenceTable;
        counterLabel = ui->referenceStatisticCounter;
    }
    else if (ui->enableCollectCurrentStatistic->isChecked())
    {
        currentStatistic = &m_currentStatistic;
        counter = &m_currentStatisticCounter;
        table = ui->currentTable;
        counterLabel = ui->currentStatisticCounter;
        referenceStatistic = &m_referenceStatistic;
    }
    else
        return;

    if (newValues.length() != currentStatistic->length())
    {
        emit error(FATAL_ERROR, objectName(), QString("Uncorrect length of new values"));
        return;
    }

    if ((*counter) <= 0)
    {
        for (int i = 0; i < newValues.length(); i++)
        {
            (*currentStatistic)[i].min = newValues[i];
            (*currentStatistic)[i].max = newValues[i];
        }
    }
    for (int i = 0; i < newValues.length(); i++)
    {
        if ((*currentStatistic)[i].min > newValues[i])
            (*currentStatistic)[i].min = newValues[i];
        if ((*currentStatistic)[i].max < newValues[i])
            (*currentStatistic)[i].max = newValues[i];
        (*currentStatistic)[i].sum += newValues[i];
    }

    (*counter)++;

    if (referenceStatistic == 0)
        displayStatistic(*currentStatistic, *counter, table, counterLabel, newValues);
    else
        displayStatistic(*currentStatistic, *counter, table, counterLabel, newValues, *referenceStatistic);

    testProcess();
    return;

#if 0
    if (ui->enableCollectReferenceStatistic->isChecked())
    {
        if (newValues.length() != m_referenceStatistic.length())
        {
            emit error(FATAL_ERROR, objectName(), QString("Uncorrect length of new values"));
            return;
        }

        if (m_referenceStatisticCounter <= 0)
        {
            for (int i = 0; i < newValues.length(); i++)
            {
                m_referenceStatistic[i].min = newValues[i];
                m_referenceStatistic[i].max = newValues[i];
            }
        }
        for (int i = 0; i < newValues.length(); i++)
        {
            if (m_referenceStatistic[i].min > newValues[i])
                m_referenceStatistic[i].min = newValues[i];
            if (m_referenceStatistic[i].max < newValues[i])
                m_referenceStatistic[i].max = newValues[i];
            m_referenceStatistic[i].sum += newValues[i];
        }

        m_referenceStatisticCounter++;

        displayStatistic(m_referenceStatistic, m_referenceStatisticCounter, ui->referenceTable, ui->referenceStatisticCounter, newValues);
        //displayStatistic(m_referenceStatistic, ui->referenceTable, m_referenceStatisticCounter, newValues);
    }
#endif
}

//-----------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::displayStatistic(QVector<StatisticData> currentStatistic, int counter, QTableWidget* table, QLabel* counterLabel, QVector<float> currentValues, QVector<StatisticData> referenceStatistic)
{
    Q_UNUSED(referenceStatistic);

    if ((currentStatistic.length() != table->rowCount()) || (table->columnCount() != 4))
    {
        emit error(FATAL_ERROR, objectName(), QString("Uncorrect statistic size!"));
        return;
    }

    if (!currentValues.isEmpty())
    {
        if (currentValues.length() != table->rowCount())
        {
            emit error(FATAL_ERROR, objectName(), QString("Uncorrect current values size!"));
            return;
        }

        for (int i = 0; i < currentValues.length(); i++)
            table->setItem(i, 3, new QTableWidgetItem(QString::number(currentValues[i], 'f', 3)));
    }

    if (counter > 0)
    {
        for (int i = 0; i < currentStatistic.length(); i++)
        {
            QVector<QTableWidgetItem*> currentItems(3);
            currentItems[0] = new QTableWidgetItem(QString::number(currentStatistic[i].min, 'f', 3));
            currentItems[1] = new QTableWidgetItem(QString::number(currentStatistic[i].max, 'f', 3));
            currentItems[2] = new QTableWidgetItem(QString::number(currentStatistic[i].sum / counter, 'f', 3));

            if (!referenceStatistic.isEmpty())
            {
                if (currentStatistic[i].min < (referenceStatistic[i].min - MIN_QREAL_DIFF))
                    currentItems[0]->setBackground(QBrush(QColorConstants::Yellow));
                if (currentStatistic[i].max > (referenceStatistic[i].max + MIN_QREAL_DIFF))
                    currentItems[1]->setBackground(QBrush(QColorConstants::Yellow));
            }

            for (int j = 0; j < 3; j++)
                table->setItem(i, j, currentItems[j]);
            continue;

            table->setItem(i, 0, new QTableWidgetItem(QString::number(currentStatistic[i].min, 'f', 3)));
            table->setItem(i, 1, new QTableWidgetItem(QString::number(currentStatistic[i].max, 'f', 3)));
            table->setItem(i, 2, new QTableWidgetItem(QString::number(currentStatistic[i].sum / counter, 'f', 3)));
        }
    }
    else
    {
        for (int i = 0; i < currentStatistic.length(); i++)
        {
            for (int j = 0; j < 3; j++)
                table->setItem(i, j, new QTableWidgetItem(QString("-")));
        }
    }
    counterLabel->setText(QString("Counter: %1").arg(counter));
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::saveStatisticToFile(QVector<StatisticData> &currentStatistic, int counter, QString fileName)
{
    QStringList dataToSave;
    dataToSave.append(QString("Counter: %1").arg(counter));
    for (int i = 0; i < currentStatistic.length(); i++)
        dataToSave.append(QString("%1. %2: %3 - %4 - %5").arg(i).arg(m_names[i]).arg(QString::number(currentStatistic[i].min, 'f', 7)).
                          arg(QString::number(currentStatistic[i].sum / counter, 'f', 7)).arg(QString::number(currentStatistic[i].max, 'f', 7)));

    QFile fileForSave(fileName);
    if (!fileForSave.open(QFile::WriteOnly))
    {
        QMessageBox::warning(this, QString("Open file error"), QString("Cannot open file %1").arg(fileName));
        return;
    }
    fileForSave.write(dataToSave.join("\r\n").toLocal8Bit());
    fileForSave.close();
}

//-----------------------------------------------------------------------------------------------------------------------------------
bool Statistic::loadStatisticFromFile(QVector<StatisticData> &currentStatistic, int *counter, QString fileName)
{
    QVector<StatisticData> tempStatistic(currentStatistic.length());
    int tempStatisticCounter = 0;
    bool conversionSuccess = true;

    QFile loadedFile(fileName);
    if (!loadedFile.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, QString("Open file error"), QString("Cannot open file %1").arg(fileName));
        return false;
    }
    QString data = QString::fromLocal8Bit(loadedFile.readAll());
    loadedFile.close();

    QStringList dataLines = data.split("\n");
    if (dataLines.length() != (1 + currentStatistic.length()))
    {
        QMessageBox::warning(this, QString("File parsing error"), QString("File %1 has %2 lines, need %3 lines").arg(fileName).
                             arg(dataLines.length()).arg(1 + currentStatistic.length()));
        return false;
    }

    QStringList counterStringSplitted = dataLines[0].split(" ");
    if ((!dataLines[0].contains(QString("Counter:"))) || (counterStringSplitted.length() < 2))
    {
        QMessageBox::warning(this, QString("File parsing error"), QString("File %1 has first line (%2) in uncorrect format (correct - Counter: X, where X - statistic counter)").
                             arg(fileName).arg(dataLines[0]));
        return false;
    }

    tempStatisticCounter = counterStringSplitted[1].toInt(&conversionSuccess);
    if ((tempStatisticCounter <= 0) || (conversionSuccess == false))
    {
        QMessageBox::warning(this, QString("File parsing error"), QString("File %1 has uncorrect value (%2) of statistic counter (must be positive number)").
                             arg(fileName).arg(counterStringSplitted[1]));
        return false;
    }

    for (int i = 0; i < tempStatistic.length(); i++)
    {
        // ("%1. %2: %3 - %4 - %5")
        QStringList counterStringSplitted = dataLines[i + 1].split(" ");
        if (counterStringSplitted.length() < 7)
        {
            QMessageBox::warning(this, QString("File parsing error"), QString("File %1 has uncorrect line %2 (%3) - uncorrect format (correct - X1. X2: X3 - X4 - X5, where X1 - index, X2 - name, X3 - min, X4 - mid, X5 - max)").
                                 arg(fileName).arg(i + 1).arg(dataLines[i + 1]));
            return false;
        }
        tempStatistic[i].min = counterStringSplitted[2].toDouble(&conversionSuccess);
        if (!conversionSuccess)
        {
            QMessageBox::warning(this, QString("File parsing error"), QString("File %1 has uncorrect line %2 (%3) - min value (%4) not double").
                                 arg(fileName).arg(i + 1).arg(dataLines[i + 1]).arg(counterStringSplitted[2]));
            return false;
        }
        tempStatistic[i].sum = counterStringSplitted[4].toDouble(&conversionSuccess) * tempStatisticCounter;
        if (!conversionSuccess)
        {
            QMessageBox::warning(this, QString("File parsing error"), QString("File %1 has uncorrect line %2 (%3) - mid value (%4) not double").
                                 arg(fileName).arg(i + 1).arg(dataLines[i + 1]).arg(counterStringSplitted[4]));
            return false;
        }
        tempStatistic[i].max = counterStringSplitted[6].toDouble(&conversionSuccess);
        if (!conversionSuccess)
        {
            QMessageBox::warning(this, QString("File parsing error"), QString("File %1 has uncorrect line %2 (%3) - max value (%4) not double").
                                 arg(fileName).arg(i + 1).arg(dataLines[i + 1]).arg(counterStringSplitted[6]));
            return false;
        }
    }

    currentStatistic = tempStatistic;
    *counter = tempStatisticCounter;

    return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::startTest()
{
    if (!m_testSettingsWidget.getSettings(&m_testSettingsData/*m_testSettingsData.mp1smuAddresses, m_testSettingsData.psmuAddresses, m_testSettingsData.values0, m_testSettingsData.values1, &(m_testSettingsData.oneValuePeriod)*/))
    {
        QMessageBox::warning(this, QString("Start test error"), QString("Some errors in test settings detected (check firmware)"));
        return;
    }

    if (m_referenceStatisticCounter < 1)
    {
        QMessageBox::warning(this, QString("Start test error"), QString("No reference staistic!"));
        return;
    }

    m_currentStatisticCounter = 0;
    m_currentStatistic.fill({ .min = 0, .max = 0, .sum = 0 });
    displayStatistic(m_currentStatistic, m_currentStatisticCounter, ui->currentTable, ui->currentStatisticCounter);

    ui->enableCollectReferenceStatistic->setChecked(false);
    ui->enableCollectCurrentStatistic->setChecked(true);
    foreach(QWidget* widget, m_testDisablingWidgets)
        widget->setEnabled(false);

    m_testState.logFileName = QString("Test_%1.txt").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss"));
    QFile logFile(m_testState.logFileName);
    if (!logFile.open(QFile::WriteOnly))
    {
        ui->log->appendPlainText(QString("Cannot open log file %1\n").arg(m_testState.logFileName));
        return;
    }
    logFile.close();
    testLog(QString("Test started, description: %1").arg(ui->labelTestSettingsDescription->text()));
    testLog(QString("Extended description: %1").arg(ui->labelTestSettingsDescription->toolTip()));
    testLog(QString());
    m_testState.testsCounter = -1;
    m_testState.isEnabled = true;
    m_testState.lastCommandOk = false;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::testProcess()
{
    if (!m_testState.isEnabled)
        return;

    int addressesCount = m_testSettingsData.mp1smuAddresses.length() + m_testSettingsData.psmuAddresses.length();
    int allTestsCount = addressesCount * m_testSettingsData.values0.length() * m_testSettingsData.values1.length();

    if ((m_currentStatisticCounter >= m_testSettingsData.oneValuePeriod) || (!m_testState.lastCommandOk))
    {
        if (m_testState.lastCommandOk)
        {
            for (int i = 0; i < m_currentStatistic.length(); i++)
            {
                if ((m_currentStatistic[i].min < (m_referenceStatistic[i].min * (1.0 - m_testSettingsData.avaluableDiff / 100.0) - MIN_QREAL_DIFF)) ||
                        (m_currentStatistic[i].max > (m_referenceStatistic[i].max * (1.0 + m_testSettingsData.avaluableDiff / 100.0) + MIN_QREAL_DIFF)))
                {
                    QString referenceStatisticString = QString("%3 - %4 - %5").arg(QString::number(m_referenceStatistic[i].min, 'f', 3)).
                            arg(QString::number(m_referenceStatistic[i].sum / m_referenceStatisticCounter, 'f', 3)).arg(QString::number(m_referenceStatistic[i].max, 'f', 3));
                    QString currentStatisticString = QString("%3 - %4 - %5").arg(QString::number(m_currentStatistic[i].min, 'f', 3)).
                            arg(QString::number(m_currentStatistic[i].sum / m_currentStatisticCounter, 'f', 3)).arg(QString::number(m_currentStatistic[i].max, 'f', 3));
                    qreal minDiffPercent = (m_referenceStatistic[i].min - m_currentStatistic[i].min) * 100.0 / m_referenceStatistic[i].min;
                    qreal maxDiffPercent = (m_currentStatistic[i].max - m_referenceStatistic[i].max) * 100.0 / m_referenceStatistic[i].max;
                    if (qIsNaN(minDiffPercent))
                        minDiffPercent = 0.0;
                    if (qIsNaN(maxDiffPercent))
                        maxDiffPercent = 0.0;
                    qreal maxPercent = qMax(minDiffPercent, maxDiffPercent);
                    QString maxLabel = (minDiffPercent > maxDiffPercent) ? QString("MIN") : QString("MAX");
                    testLog(QString("%1. %2: %3 --> %4 (%5 %6 %)").arg(i).arg(m_names[i]).arg(referenceStatisticString).arg(currentStatisticString).arg(maxLabel).arg(maxPercent));
//                    testLog(QString("%1. %2: %3 - %4 - %5 --> %6 - %7 - %8").arg(i).arg(m_names[i]).
//                            arg(QString::number(m_referenceStatistic[i].min, 'f', 3)).arg(QString::number(m_referenceStatistic[i].sum / m_referenceStatisticCounter, 'f', 3)).
//                            arg(QString::number(m_referenceStatistic[i].max, 'f', 3)).arg(QString::number(m_currentStatistic[i].min, 'f', 3)).
//                            arg(QString::number(m_currentStatistic[i].sum / m_currentStatisticCounter, 'f', 3)).arg(QString::number(m_currentStatistic[i].max, 'f', 3)));
                }
            }
        }

        testLog(QString());

        m_currentStatisticCounter = 0;
        m_currentStatistic.fill({ .min = 0, .max = 0, .sum = 0 });

        m_testState.testsCounter++;
        if (m_testState.testsCounter >= allTestsCount)
        {
            // TEST ENDED
            foreach(QWidget* widget, m_testDisablingWidgets)
                widget->setEnabled(true);
            m_testState.isEnabled = false;
            testLog(QString("Test ended"));
            m_testSettingsWidget.requestSettingsDescription();
            return;
        }

        int temp = m_testState.testsCounter;
        int value1counter = temp % m_testSettingsData.values1.length();
        temp /= m_testSettingsData.values1.length();
        int value0counter = temp % m_testSettingsData.values0.length();
        temp /= m_testSettingsData.values0.length();
        if (temp >= addressesCount)
        {
            QMessageBox::critical(this, QString("Test process error"),
                                  QString("Test counter value (%1) uncorrect. Addresses count - %2, values0 count - %3, values1 count - %4. Check firmware").
                                  arg(m_testState.testsCounter).arg(addressesCount).arg(m_testSettingsData.values0.length()).arg(m_testSettingsData.values1.length()));
            return;
        }
        bool isPsmu = false;
        if (temp >= m_testSettingsData.mp1smuAddresses.length())
        {
            isPsmu = true;
            temp -= m_testSettingsData.mp1smuAddresses.length();
        }

        int address = isPsmu ? m_testSettingsData.psmuAddresses[temp] : m_testSettingsData.mp1smuAddresses[temp];
        QString commandResult = m_ryzenControl->setRegValue(isPsmu, address, m_testSettingsData.values0[value0counter], m_testSettingsData.values1[value1counter]);
        m_testState.lastCommandOk = commandResult.isEmpty();
        if (commandResult.isEmpty())
            testLog(QString("Command %1; addr = %2; value[0] = %3; value[1] = %4; OK").arg(isPsmu ? QString("PSMU") : QString("MP1_SMU")).arg(address).
                  arg(m_testSettingsData.values0[value0counter]).arg(m_testSettingsData.values1[value1counter]));
        else
            testLog(QString("Command %1; addr = %2; value[0] = %3; value[1] = %4; ERROR - %5").arg(isPsmu ? QString("PSMU") : QString("MP1_SMU")).arg(address).
                  arg(m_testSettingsData.values0[value0counter]).arg(m_testSettingsData.values1[value1counter]).arg(commandResult));
    }

    ui->labelTestSettingsDescription->setText(QString("Test execution %1/%2 (%3%), value statistic %4/%5").arg(m_testState.testsCounter).arg(allTestsCount).
                                              arg(m_testState.testsCounter * 100 / allTestsCount).arg(m_currentStatisticCounter).arg(m_testSettingsData.oneValuePeriod));
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::testLog(QString message)
{
    QString currentLine = QString("\n");
    if (!message.isEmpty())
        currentLine = QString("%1. %2\n").arg(QDateTime::currentDateTime().toString("hh:mm:ss")).arg(message);
    ui->log->appendPlainText(currentLine.left(currentLine.length() - 1));
    QFile logFile(m_testState.logFileName);
    if (!logFile.open(QFile::Append))
    {
        ui->log->appendPlainText(QString("Cannot open log file %1").arg(m_testState.logFileName));
        return;
    }
    logFile.write(currentLine.toLocal8Bit());
    logFile.close();
}

//-----------------------------------------------------------------------------------------------------------------------------------
