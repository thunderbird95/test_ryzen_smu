#ifndef TEST_SETTINGS_H
#define TEST_SETTINGS_H

#include <QWidget>

namespace Ui {
class TestSettings;
}

typedef struct  {
    QVector<int> mp1smuAddresses;
    QVector<int> psmuAddresses;
    QVector<int> values0;
    QVector<int> values1;
    int oneValuePeriod;
    qreal avaluableDiff;
}   TestSettingsData;

class TestSettings : public QWidget
{
    Q_OBJECT

public:
    explicit TestSettings(QWidget *parent = nullptr);
    ~TestSettings();

    // Description of all settings
    QString description();

    // Gets all settings
    bool getSettings(TestSettingsData* data/*QVector<int>& mp1smuAddresses, QVector<int>& psmuAddresses, QVector<int>& values0, QVector<int>& values1, int* statisticPeriod*/);

    // Requests settings description signal
    void requestSettingsDescription();


    // Vector to ranges cast
    static QVector<QPair<int,int>> vectorToRanges(QVector<int> input);

    // Ranges to vector cast
    static QVector<int> rangesToVector(QVector<QPair<int,int>> input);

signals:

    // Emits settings flag and description
    void settingsChanged(bool rightFlag, QString description, QString extendedDescription);

private slots:

    // Load settings from file
    void loadSettings();

    // Save settings to file
    void saveSettings();

private:

    // This widget
    Ui::TestSettings *ui;

    // Mp1 SMU addresses
    QVector<QPair<int,int>> m_mp1smuAddresses;

    // psmu addresses
    QVector<QPair<int,int>> m_psmuAddresses;

    // values 0
    QVector<QPair<int,int>> m_values0;

    // values 1
    QVector<QPair<int,int>> m_values1;

    // Period of statistic for each value changing
    int m_statisticPeriod;

    // Avaluable difference in percents
    qreal m_avaluableDifference;

    // Flag of applicable settings
    bool m_rightFlag;

    // Displays all settings
    void displaySettings();

    // Appends new range to vector with current ranges
    void appendRangeToVector(QPair<int,int> range, QVector<QPair<int,int>>& vector);

    // Removes range from vector with current ranges
    void removeRangeFromVector(QPair<int,int> range, QVector<QPair<int,int>>& vector);

    // Ranges to string cast
    bool rangesToString(QVector<QPair<int,int>>& input, QString& output);
    QString rangesToString(QVector<QPair<int,int>>& input);

    // String to ranges cast
    bool stringToRanges(QString input, QVector<QPair<int,int>>& output);

    // Range to string cast
    static bool rangeToString(QPair<int,int>& input, QString& output);
    static QString rangeToString(QPair<int,int>& input);

    // String to range cast
    static bool stringToRange(QString input, QPair<int, int> &output);
};

#endif // TEST_SETTINGS_H
