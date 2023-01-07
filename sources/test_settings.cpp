#include "test_settings.h"
#include "ui_test_settings.h"

#include <QMessageBox>
#include <QFileDialog>

//-----------------------------------------------------------------------------------------------------------------------------------
TestSettings::TestSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestSettings)
{
    ui->setupUi(this);

#if 0
    connect(ui->minRangeValue, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [ this ] (int value) { ui->maxRangeValue->setMinimum(value); } );
    connect(ui->maxRangeValue, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [ this ] (int value) { ui->minRangeValue->setMaximum(value); } );
#else
    connect(ui->minRangeValue, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [ this ] (int value) { disconnect(); ui->maxRangeValue->setMinimum(value); } );
    connect(ui->maxRangeValue, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [ this ] (int value) { ui->minRangeValue->setMaximum(value); } );
#endif

    connect(ui->statisticPeriod, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [ this ] (int value) { m_statisticPeriod = value; displaySettings(); } );
    connect(ui->addMp1SmuAddress, &QPushButton::clicked, [ this ] () {
        appendRangeToVector(QPair<int,int>(ui->minRangeValue->value(), ui->maxRangeValue->value()), m_mp1smuAddresses); displaySettings();
    } );
#if 0
    connect(ui->removeMp1SmuAddress, &QPushButton::clicked, [ this ] () {
        removeRangeFromVector(QPair<int,int>(ui->minRangeValue->value(), ui->maxRangeValue->value()), m_mp1smuAddresses); displaySettings();
    } );
#else
    connect(ui->removeMp1SmuAddress, &QPushButton::clicked, [ this ] () {
        QPair<int,int> rangeToRemove;
        if (!ui->mp1smu_list->currentItem()) return;
        if (!stringToRange(ui->mp1smu_list->currentItem()->text(), rangeToRemove)) return;
        removeRangeFromVector(rangeToRemove, m_mp1smuAddresses); displaySettings();
    } );
#endif

    connect(ui->addPsmuAddress, &QPushButton::clicked, [ this ] () {
        appendRangeToVector(QPair<int,int>(ui->minRangeValue->value(), ui->maxRangeValue->value()), m_psmuAddresses); displaySettings();
    } );
#if 0
    connect(ui->removePsmuAddress, &QPushButton::clicked, [ this ] () {
        removeRangeFromVector(QPair<int,int>(ui->minRangeValue->value(), ui->maxRangeValue->value()), m_psmuAddresses); displaySettings();
    } );
#else
    connect(ui->removePsmuAddress, &QPushButton::clicked, [ this ] () {
        QPair<int,int> rangeToRemove;
        if (!ui->psmu_list->currentItem()) return;
        if (!stringToRange(ui->psmu_list->currentItem()->text(), rangeToRemove)) return;
        removeRangeFromVector(rangeToRemove, m_psmuAddresses); displaySettings();
    } );
#endif

    connect(ui->addValue0, &QPushButton::clicked, [ this ] () {
        appendRangeToVector(QPair<int,int>(ui->minRangeValue->value(), ui->maxRangeValue->value()), m_values0); displaySettings();
    } );
#if 0
    connect(ui->removeValue0, &QPushButton::clicked, [ this ] () {
        removeRangeFromVector(QPair<int,int>(ui->minRangeValue->value(), ui->maxRangeValue->value()), m_values0); displaySettings();
    } );
#else
    connect(ui->removeValue0, &QPushButton::clicked, [ this ] () {
        QPair<int,int> rangeToRemove;
        if (!ui->value0_list->currentItem()) return;
        if (!stringToRange(ui->value0_list->currentItem()->text(), rangeToRemove)) return;
        removeRangeFromVector(rangeToRemove, m_values0); displaySettings();
    } );
#endif

    connect(ui->addValue1, &QPushButton::clicked, [ this ] () {
        appendRangeToVector(QPair<int,int>(ui->minRangeValue->value(), ui->maxRangeValue->value()), m_values1); displaySettings();
    } );
#if 0
    connect(ui->removeValue1, &QPushButton::clicked, [ this ] () {
        removeRangeFromVector(QPair<int,int>(ui->minRangeValue->value(), ui->maxRangeValue->value()), m_values1); displaySettings();
    } );
#else
    connect(ui->removeValue1, &QPushButton::clicked, [ this ] () {
        QPair<int,int> rangeToRemove;
        if (!ui->value1_list->currentItem()) return;
        if (!stringToRange(ui->value1_list->currentItem()->text(), rangeToRemove)) return;
        removeRangeFromVector(rangeToRemove, m_values1); displaySettings();
    } );
#endif

    connect(ui->loadTestSettings, &QPushButton::clicked, this, &TestSettings::loadSettings);
    connect(ui->saveTestSettings, &QPushButton::clicked, this, &TestSettings::saveSettings);

    m_mp1smuAddresses.append(QPair<int,int>(0,0));
    //m_psmuAddresses.append(QPair<int,int>(0,0));
    m_values0.append(QPair<int,int>(-1000,-1000));
    m_values0.append(QPair<int,int>(0,1));
    m_values0.append(QPair<int,int>(5,5));
    m_values0.append(QPair<int,int>(10,10));
    m_values0.append(QPair<int,int>(100,100));
    m_values0.append(QPair<int,int>(1000,1000));
    m_values1.append(QPair<int,int>(0,0));
    m_statisticPeriod = 100;

    loadSettings();
    displaySettings();
}

//-----------------------------------------------------------------------------------------------------------------------------------
TestSettings::~TestSettings()
{
    delete ui;
}

//-----------------------------------------------------------------------------------------------------------------------------------
QString TestSettings::description()
{
    int mp1SmuAddressesCount = rangesToVector(m_mp1smuAddresses).length();
    int psmuAddressesCount = rangesToVector(m_psmuAddresses).length();
    int values0count = rangesToVector(m_values0).length();
    int values1count = rangesToVector(m_values1).length();
    QStringList errors;
    if ((mp1SmuAddressesCount + psmuAddressesCount) <= 0)
        errors.append(QString("no addresses found"));
    if (values0count <= 0)
        errors.append(QString("no values[0] found"));
    if (values1count <= 0)
        errors.append(QString("no values[1] found"));
    QString result;
    m_rightFlag = errors.isEmpty();
    if (!m_rightFlag)
        result = QString("Errors in test settings: %1").arg(errors.join(QString(", ")));
    else
        result = QString("Mp1 SMU + PSMU addresses: %1 + %2 = %3\nValues[0] - %4, values[1] - %5\nOne test period - %6. All tests time ~ %7").
                arg(mp1SmuAddressesCount).arg(psmuAddressesCount).arg(mp1SmuAddressesCount + psmuAddressesCount).arg(values0count).arg(values1count).arg(m_statisticPeriod).
                arg((mp1SmuAddressesCount + psmuAddressesCount) * values0count * values1count * m_statisticPeriod);
    return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
bool TestSettings::getSettings(QVector<int> &mp1smuAddresses, QVector<int> &psmuAddresses, QVector<int> &values0, QVector<int> &values1, int *statisticPeriod)
{
    mp1smuAddresses = rangesToVector(m_mp1smuAddresses);
    psmuAddresses = rangesToVector(m_psmuAddresses);
    values0 = rangesToVector(m_values0);
    values1 = rangesToVector(m_values1);
    *statisticPeriod = ui->statisticPeriod->value();
    return m_rightFlag;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void TestSettings::requestSettingsDescription()
{
    displaySettings();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void TestSettings::loadSettings()
{
    QPushButton* sender = qobject_cast<QPushButton*>(this->sender());
    QString loadFileName;
    if (sender == 0)
        loadFileName = QString("saved_test_settings.txt");
    else
    {
        loadFileName = QFileDialog::getOpenFileName(this);
        if (loadFileName.isEmpty())
            return;
    }

    QFile loadedFile(loadFileName);
    if (!loadedFile.open(QFile::ReadOnly))
    {
        if (sender != 0)
            QMessageBox::warning(this, QString("Open file error"), QString("Cannot open file %1").arg(loadFileName));
        return;
    }
    QString data = QString::fromLocal8Bit(loadedFile.readAll());
    loadedFile.close();
    QStringList dataLines = data.split("\n");

    // Temp values
    QVector<QPair<int,int>> mp1smuAddresses, psmuAddresses, values0, values1;
    int statisticPeriod;
    bool mp1smuAddressesFlag = false;
    bool psmuAddressesFlag = false;
    bool values0flag = false;
    bool values1flag = false;
    bool statisticPeriodFlag = false;

    foreach (QString line, dataLines)
    {
        bool conversionSuccess = true;
        line = line.simplified();
        if (line.left(QString("Statistic period:").length()) == QString("Statistic period:"))
        {
            statisticPeriod = line.mid(QString("Statistic period:").length()).toInt(&conversionSuccess);
            statisticPeriodFlag = true;
            if ((!conversionSuccess) || (statisticPeriod <= 0))
                QMessageBox::warning(this, QString("Parsing error"), QString("Error in line %1: %2 is not positive integer value").
                                     arg(line).arg(line.mid(QString("Statistic period:").length())));
        }
        else if (line.left(QString("Mp1 SMU addresses:").length()) == QString("Mp1 SMU addresses:"))
        {
            mp1smuAddressesFlag = true;
            conversionSuccess = stringToRanges(line.mid(QString("Mp1 SMU addresses:").length()), mp1smuAddresses);
        }
        else if (line.left(QString("PSMU addresses:").length()) == QString("PSMU addresses:"))
        {
            psmuAddressesFlag = true;
            conversionSuccess = stringToRanges(line.mid(QString("PSMU addresses:").length()), psmuAddresses);
        }
        else if (line.left(QString("Values[0]:").length()) == QString("Values[0]:"))
        {
            values0flag = true;
            conversionSuccess = stringToRanges(line.mid(QString("Values[0]:").length()), values0);
        }
        else if (line.left(QString("Values[1]:").length()) == QString("Values[1]:"))
        {
            values1flag = true;
            conversionSuccess = stringToRanges(line.mid(QString("Values[1]:").length()), values1);
        }

        if (!conversionSuccess)
            return;
    }

    QStringList errors;
    if (!mp1smuAddressesFlag)   errors.append(QString("no Mp1 SMU addresses line found"));
    if (!psmuAddressesFlag)   errors.append(QString("no PSMU addresses line found"));
    if (!values0flag)   errors.append(QString("no Values[0] line found"));
    if (!values1flag)   errors.append(QString("no Values[1] line found"));
    if (!statisticPeriodFlag)   errors.append(QString("no Statistic period line found"));

    if (!errors.isEmpty())
    {
        QMessageBox::warning(this, QString("Parsing error"), QString("Errors: %1").arg(errors.join(QString(", "))));
        return;
    }

    m_mp1smuAddresses = mp1smuAddresses;
    m_psmuAddresses = psmuAddresses;
    m_values0 = values0;
    m_values1 = values1;
    m_statisticPeriod = statisticPeriod;

    if (sender != 0)
        displaySettings();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void TestSettings::saveSettings()
{
    QPushButton* sender = qobject_cast<QPushButton*>(this->sender());
    QString saveFileName;
    if (sender == 0)
        saveFileName = QString("saved_test_settings.txt");
    else
    {
        saveFileName = QFileDialog::getSaveFileName(this);
        if (saveFileName.isEmpty())
            return;
    }

    QStringList data;
    data.append(QString("Description: %1").arg(description()));
    data.append(QString("Statistic period: %1").arg(m_statisticPeriod));
    data.append(QString("Mp1 SMU addresses: %1").arg(rangesToString(m_mp1smuAddresses)));
    data.append(QString("PSMU addresses: %1").arg(rangesToString(m_psmuAddresses)));
    data.append(QString("Values[0]: %1").arg(rangesToString(m_values0)));
    data.append(QString("Values[1]: %1").arg(rangesToString(m_values1)));

    QFile fileForSave(saveFileName);
    if (!fileForSave.open(QFile::WriteOnly))
    {
        QMessageBox::warning(this, QString("Open file error"), QString("Cannot open file %1").arg(saveFileName));
        return;
    }
    fileForSave.write(data.join("\n").toLocal8Bit());
    fileForSave.close();
}

//-----------------------------------------------------------------------------------------------------------------------------------
void TestSettings::displaySettings()
{
    ui->mp1smu_list->clear();
    for (int i = 0; i < m_mp1smuAddresses.length(); i++)
        ui->mp1smu_list->addItem(rangeToString(m_mp1smuAddresses[i]));
    ui->psmu_list->clear();
    for (int i = 0; i < m_psmuAddresses.length(); i++)
        ui->psmu_list->addItem(rangeToString(m_psmuAddresses[i]));
    ui->value0_list->clear();
    for (int i = 0; i < m_values0.length(); i++)
        ui->value0_list->addItem(rangeToString(m_values0[i]));
    ui->value1_list->clear();
    for (int i = 0; i < m_values1.length(); i++)
        ui->value1_list->addItem(rangeToString(m_values1[i]));
    ui->statisticPeriod->setValue(m_statisticPeriod);
    ui->labelTestSettingsDescription->setText(description());
    saveSettings();

    if (m_rightFlag)
        emit settingsChanged(true, description(), QString("Mp1 SMU addresses: %1\nPSMU addresses: %2\nValues[0]: %3\nValues[1]: %4").
                             arg(rangesToString(m_mp1smuAddresses)).arg(rangesToString(m_psmuAddresses)).arg(rangesToString(m_values0)).arg(rangesToString(m_values1)));
    else
        emit settingsChanged(false, QString("Errors in test settings"), description());
}

//-----------------------------------------------------------------------------------------------------------------------------------
void TestSettings::appendRangeToVector(QPair<int, int> range, QVector<QPair<int, int>> &vector)
{
    QVector<int> temp = rangesToVector(vector);
    if (range.first > range.second)
        qSwap(range.first, range.second);
    for (int i = range.first; i <= range.second; i++)
    {
        if (!temp.contains(i))
            temp.append(i);
    }
    std::sort(temp.begin(), temp.end());
    vector = vectorToRanges(temp);
}

//-----------------------------------------------------------------------------------------------------------------------------------
void TestSettings::removeRangeFromVector(QPair<int, int> range, QVector<QPair<int, int>> &vector)
{
    QVector<int> temp = rangesToVector(vector);
    if (range.first > range.second)
        qSwap(range.first, range.second);
    for (int i = range.first; i <= range.second; i++)
    {
        while (temp.removeOne(i))   { ; }
    }
    std::sort(temp.begin(), temp.end());
    vector = vectorToRanges(temp);
}

//-----------------------------------------------------------------------------------------------------------------------------------
QVector<QPair<int,int>> TestSettings::vectorToRanges(QVector<int> input)
{
    QVector<QPair<int,int>> result;
    int startPairIndex = -1;
    for (int i = 0; i < input.length(); i++)
    {
        if (startPairIndex == -1)
            startPairIndex = i;
        else
        {
            if (input[i] <= (input[i - 1] + 1))
                continue;
            else
            {
                result.append(QPair<int,int>(input[startPairIndex], input[i - 1]));
                startPairIndex = i;
            }
        }
    }
    if (startPairIndex != -1)
        result.append(QPair<int,int>(input[startPairIndex], input.last()));
    return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
QVector<int> TestSettings::rangesToVector(QVector<QPair<int,int>> input)
{
    QVector<int> result;
#if 0
    Q_FOREACH(QPair<int,int> currentPair, input)
    {
        for (int i = currentPair.first; i < currentPair.second; i++)
        {
            if (!result.contains(i))
                result.append(i);
        }
    }
#else
    for (int i = 0; i < input.length(); i++)
    {
        if (input[i].first > input[i].second)
            qSwap(input[i].first, input[i].second);
        for (int j = input[i].first; j <= input[i].second; j++)
        {
            if (!result.contains(j))
                result.append(j);
        }
    }
#endif
    return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
bool TestSettings::rangesToString(QVector<QPair<int, int>> &input, QString &output)
{
    QStringList result;
#if 0
    for (int i = 0; i < input.length(); i++)
    {
        if (input[i].first == input[i].second)
            result.append(QString("%1").arg(input[i].first));
        else
            result.append(QString("%1 - %2").arg(input[i].first).arg(input[i].second));
    }
#else
    QString temp;
    for (int i = 0; i < input.length(); i++)
    {
        rangeToString(input[i], temp);
        result.append(temp);
    }
#endif
    output = result.join(QString(", "));
    return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------
QString TestSettings::rangesToString(QVector<QPair<int, int>> &input)
{
    QStringList result;
    for (int i = 0; i < input.length(); i++)
        result.append(rangeToString(input[i]));
    return (result.join(QString(", ")));
}

//-----------------------------------------------------------------------------------------------------------------------------------
bool TestSettings::stringToRanges(QString input, QVector<QPair<int, int>> &output)
{
    //bool conversionSuccess = false;
    QPair<int,int> temp;
    QStringList splittedValues = input.simplified().split(",");
    output.clear();
    foreach(QString current, splittedValues)
    {
#if 0
        current.remove(" ");
        if (current.contains("-"))
        {
            QStringList values = current.split("-");
            if (values.length() < 2)
            {
                QMessageBox::warning(this, QString("Parsing error"), QString("Element %1 uncorrect").arg(current));
                return false;
            }
            temp.first = values[0].toInt(&conversionSuccess);
            if (!conversionSuccess)
            {
                QMessageBox::warning(this, QString("Parsing error"), QString("Element %1 in pair %2 uncorrect").arg(values[0]).arg(current));
                return false;
            }
            temp.second = values[1].toInt(&conversionSuccess);
            if (!conversionSuccess)
            {
                QMessageBox::warning(this, QString("Parsing error"), QString("Element %1 in pair %2 uncorrect").arg(values[1]).arg(current));
                return false;
            }
        }
        else
        {
            temp.first = current.toInt(&conversionSuccess);
            if (!conversionSuccess)
            {
                QMessageBox::warning(this, QString("Parsing error"), QString("Element %1 uncorrect").arg(current));
                return false;
            }
            temp.second = temp.first;
        }
#else
        if (current.isEmpty())
            continue;
        if (!stringToRange(current,temp))
        {
            QMessageBox::warning(this, QString("Parsing error"), QString("Element %1 uncorrect").arg(current));
            return false;
        }
#endif
        output.append(temp);
    }

    return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------
bool TestSettings::rangeToString(QPair<int, int> &input, QString &output)
{
#if 0
    if (input.first == input.second)
        output = QString("%1").arg(input.first);
    else
        output = QString("%1 - %2").arg(input.first).arg(input.second);
#else
    output = rangeToString(input);
#endif
    return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------
QString TestSettings::rangeToString(QPair<int,int>& input)
{
    if (input.first == input.second)
        return (QString("%1").arg(input.first));
    else
        return (QString("%1 - %2").arg(input.first).arg(input.second));
}

//-----------------------------------------------------------------------------------------------------------------------------------
bool TestSettings::stringToRange(QString input, QPair<int, int> &output)
{
    QString temp = input;
    bool conversionSuccess = false;
#if 0
    temp.remove(" ");
    if (temp.contains("-"))
    {
        QStringList values = temp.split("-");
        if (values.length() < 2)
            return false;
        output.first = values[0].toInt(&conversionSuccess);
        if (!conversionSuccess)
            return false;
        output.second = values[1].toInt(&conversionSuccess);
        if (!conversionSuccess)
            return false;
    }
    else
    {
        output.first = temp.toInt(&conversionSuccess);
        if (!conversionSuccess)
            return false;
        output.second = output.first;
    }
#else
    QStringList values = temp.simplified().split(" ");
    if (values.contains(QString("-")))
    {
        if (values.length() < 3)
            return false;
        output.first = values[0].toInt(&conversionSuccess);
        if (!conversionSuccess)
            return false;
        output.second = values[2].toInt(&conversionSuccess);
        if (!conversionSuccess)
            return false;
    }
    else
    {
        output.first = temp.toInt(&conversionSuccess);
        if (!conversionSuccess)
            return false;
        output.second = output.first;
    }
#endif
    return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------
