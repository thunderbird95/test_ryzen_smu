#ifndef RYZENCONTROL_H
#define RYZENCONTROL_H

#include <QMainWindow>
#include <QTimer>

#include "apu_driver.h"

#include "graphs.h"
#include "statistic.h"

#define CORES_COUNT 8

#define DEBUG_MODE
#define STATISTIC_MEASURES_COUNT    100
#define STATISTIC_MEASURES_INTERVAL 10
#define MAX_AVALUABLE_DIFF_MULT     0.01

#define GFX_CLK_SET_TIMEOUT     3
#define GFX_CLOCKS_COUNT        4
#define GFX_BUSY_REG            374 // 66, 70 or 374
#define CCLK_BUSY_REG           64
#define GFX_FREQUENCIES_LIST    { 1500, 1600, 1700, 1800 }
// COEFFICIENTS TO SLOW DOWN FREQUENCY
#define MIN_GFX_BUSY_VALUES     { -1, 90, 92, 94 }
#define MAX_CCLK_BUSY_VALUES    { 101, 80, 75, 70 }
// COEFFICIENTS TO GROW UP FREQUENCY
#define MAX_GFX_BUSY_VALUES     { 98, 98, 98, 101 }
#define MIN_CCLK_BUSY_VALUES    { 80, 80, 75, -1 }

//#define MAX_GFX_CLK     1800
//#define MIN_GFX_CLK     1500
//#define GFX_CLK_STEP    100
//#define GFX_BUSY_REG        374
//#define MIN_GFX_BUSY_VALUE  95.0
//#define MAX_GFX_BUSY_VALUE  98.0

QT_BEGIN_NAMESPACE
namespace Ui { class RyzenControl; }
QT_END_NAMESPACE

class RyzenControl : public QMainWindow
{
    Q_OBJECT

public:
    RyzenControl(QWidget *parent = nullptr);
    ~RyzenControl();

    // Sets value of the register
    QString setRegValue(bool isPsmu, uint32_t address, uint32_t value0, uint32_t value1 = 0, uint32_t value2 = 0);

signals:

    // Display data
    void displayData(QVector<QVector<float>> values, QVector<int> times, QVector<QString> descriptions, int startIndex);

    // Update values
    void currentValuesUpdate(QVector<float> values);

private slots:

    // Closes all windows
    void closeEvent(QCloseEvent *event);

private:

    // Refleshing all data
    void updateData();

    // Set data to ryzen
    void setData();

    // GUI
    Ui::RyzenControl *ui;

    // Widget with graphs
    Graphs m_graphsWidget;

    // Widget with statistic data
    Statistic m_statisticWidget;

    // AMD APU driver
    ApuDriver m_apuDriver;

    // AMD APU driver data
    ryzen_access m_ry;

    // Timer for refresh info
    QTimer m_periodicTimer;

    // Driver functions
    std::vector<std::function<int(ryzen_access,int)>> m_functions;

    // Text descriptions for every function
    QVector<QString> m_functionDescriptions;

    // Last copy of all table
    QVector<float> m_lastTableValues;

    // Descriptions of table values
    QVector<QString> m_tableDescriptions;

    // Current index of statistic vectors
    int m_currentIndex;

    // Times of statistic data read
    QVector<int> m_tableReadTimes;

    // Values of statistic data
    QVector<QVector<float>> m_tableValues;

    // Data for calc statistic
    QVector<QVector<float>> m_tableStatisticData;

    // Min values of the each table value
    QVector<float> m_tableStatisticMin;

    // Max values of the each table value
    QVector<float> m_tableStatisticMax;

    // Max values of the each table value
    QVector<float> m_tableStatisticMid;

    // Current gfx clock
    int32_t m_currentGfxClk;

    // Index of current gfx clk
    int32_t m_currentGfxClkIndex;

    // Timeout after set gfx clk
    int32_t m_gfxClkSetTimeout;
};
#endif // RYZENCONTROL_H
