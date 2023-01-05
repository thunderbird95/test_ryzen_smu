#ifndef STATISTIC_H
#define STATISTIC_H

#include <QWidget>

namespace Ui {
class Statistic;
}

typedef struct  {
    qreal min;
    qreal max;
    qreal sum;
//    qreal mid;
//    int count;
}   StatisticData;

class RyzenControl;
class QTableWidget;
class QLabel;

class Statistic : public QWidget
{
    Q_OBJECT

public:

    //
    explicit Statistic(QWidget *parent = nullptr);
    ~Statistic();

    // Displays descriptions
    void setDescriptions(QVector<QString> descriptions);

    // Set link to ryzen control
    void setRyzenControlLink(RyzenControl* link);

public slots:

    // Append new values to statistic
    void handleNewValues(QVector<float> newValues);

signals:

    // Sends error and description
    void error(int level, QString source, QString description);

private:

    // Widget
    Ui::Statistic *ui;

    // Parent
    RyzenControl* m_ryzenControl;

    // Names of all statistic values
    QVector<QString> m_names;

    // Reference statistic data
    QVector<StatisticData> m_referenceStatistic;

    // Reference statistic counter
    int m_referenceStatisticCounter;

    // Current statistic data
    QVector<StatisticData> m_currentStatistic;

    // Current statistic counter
    int m_currentStatisticCounter;

    // Fills reference statistic table
    void displayStatistic(QVector<StatisticData> currentStatistic, int counter, QTableWidget* table, QLabel* counterLabel, QVector<float> currentValues = QVector<float>(), QVector<StatisticData> referenceStatistic = QVector<StatisticData>());

    // Saves statistic data to file (returns error description, or empty string, if no errors detected)
    void saveStatisticToFile(QVector<StatisticData>& currentStatistic, int counter, QString fileName);

    // Loads statistic data from file (returns error description, or empty string, if no errors detected)
    void loadStatisticFromFile(QVector<StatisticData>& currentStatistic, int* counter, QString fileName);
};

#endif // STATISTIC_H
