#include "statistic.h"
#include "ui_statistic.h"

#include "definitions.h"
#include "ryzen_control.h"

#include <QScrollBar>
#include <QMessageBox>

//-----------------------------------------------------------------------------------------------------------------------------------
Statistic::Statistic(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Statistic)
{
    ui->setupUi(this);

    m_ryzenControl = static_cast<RyzenControl*>(this->parent());

    connect(ui->clearReferenceStatistic, &QPushButton::clicked, this, [ this ] () { m_referenceStatisticCounter = 0; m_referenceStatistic.fill({ .min = 0, .max = 0, .sum = 0 }); } );

    connect(ui->referenceTable->verticalScrollBar(), &QScrollBar::sliderMoved, [this] (int position) { ui->currentTable->verticalScrollBar()->setValue(position); });
}

//-----------------------------------------------------------------------------------------------------------------------------------
Statistic::~Statistic()
{
    delete ui;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::setDescriptions(QVector<QString> descriptions)
{
    m_names = descriptions;
    m_referenceStatistic.resize(descriptions.length());
    m_referenceStatisticCounter = 0;
    m_referenceStatistic.fill({ .min = 0, .max = 0, .sum = 0 });
    ui->referenceTable->setRowCount(descriptions.length());
    for (int i = 0; i < descriptions.length(); i++)
        ui->referenceTable->setVerticalHeaderItem(i, new QTableWidgetItem(QString("%1: %2").arg(i).arg(descriptions[i])));
    ui->currentTable->setRowCount(descriptions.length());
    for (int i = 0; i < descriptions.length(); i++)
        ui->currentTable->setVerticalHeaderItem(i, new QTableWidgetItem(QString("%1").arg(i)));
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Statistic::setRyzenControlLink(RyzenControl *link)
{
    m_ryzenControl = link;
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
    return;

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
            table->setItem(i, 0, new QTableWidgetItem(QString::number(currentStatistic[i].min, 'f', 3)));
            table->setItem(i, 1, new QTableWidgetItem(QString::number(currentStatistic[i].max, 'f', 3)));
            table->setItem(i, 2, new QTableWidgetItem(QString::number(currentStatistic[i].sum / counter, 'f', 3)));
        }
    }
    else
    {
        for (int i = 0; i < currentStatistic.length(); i++)
        {
            for (int j = 0; j < 2; j++)
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
        dataToSave.append(QString("%1. %2: %3 - %4 - %5").arg(i).arg(m_names[i]).arg(currentStatistic[i].min).arg(currentStatistic[i].sum / counter).arg(currentStatistic[i].max));

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
void Statistic::loadStatisticFromFile(QVector<StatisticData> &currentStatistic, int *counter, QString fileName)
{
    QVector<StatisticData> tempStatistic(currentStatistic.length());
    int tempStatisticCounter = 0;
    bool conversionSuccess = false;

    QFile loadedFile(fileName);
    if (!loadedFile.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, QString("Open file error"), QString("Cannot open file %1").arg(fileName));
        return;
    }
    QString data = QString::fromLocal8Bit(loadedFile.readAll()).simplified();
    loadedFile.close();

    QStringList dataLines = data.simplified().split("\r\n");
    if (dataLines.length() != (1 + currentStatistic.length()))
    {
        QMessageBox::warning(this, QString("File parsing error"), QString("File %1 has %2 lines, need %3 lines").arg(fileName).
                             arg(dataLines.length()).arg(1 + currentStatistic.length()));
        return;
    }

    QStringList counterStringSplitted = dataLines[0].split(" ");
    if ((!dataLines[0].contains(QString("Counter:"))) || (counterStringSplitted.length() < 2))
    {
        QMessageBox::warning(this, QString("File parsing error"), QString("File %1 has first line (%2) in uncorrect format (correct - Counter: X, where X - statistic counter)").
                             arg(fileName).arg(dataLines[0]));
        return;
    }

    tempStatisticCounter = counterStringSplitted[1].toInt(&conversionSuccess);
    if ((tempStatisticCounter <= 0) || (conversionSuccess == false))
    {
        QMessageBox::warning(this, QString("File parsing error"), QString("File %1 has uncorrect value (%2) of statistic counter (must be positive number)").
                             arg(fileName).arg(counterStringSplitted[1]));
        return;
    }

    for (int i = 0; i < tempStatistic.length(); i++)
    {

    }
}

//-----------------------------------------------------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------------------------------------------------
