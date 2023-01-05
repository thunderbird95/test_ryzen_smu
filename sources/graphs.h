#ifndef GRAPHS_H
#define GRAPHS_H

#include <QWidget>
#include <QGridLayout>
#include <QList>
#include <QCheckBox>

namespace Ui {
class Graphs;
}

class Graphs : public QWidget
{
    Q_OBJECT

public:
    explicit Graphs(QWidget *parent = nullptr);
    ~Graphs();

public slots:

    // Displays descriptions
    void setDescriptions(QVector<QString> descriptions);

    // Displays data of all graphs
    void displayGraph(QVector<QVector<float>> values, QVector<int> times, QVector<QString> descriptions, int startIndex);

signals:

    // Sends error and description
    void error(int level, QString source, QString description);

private slots:

    // Display graph
    void displayGraphInternal();

    // Load current displayed graph indexes
    void loadPreset();

    // Save current displayed graph indexes
    void savePreset();

private:

    // GUI
    Ui::Graphs *ui;

    // Widget for scrollarea
    QWidget* m_scrollAreaWidget;

    // Layout with many checkboxes
    QGridLayout* m_plotDescriptionsLayout;

    // Checkboxes for scrollarea widget
    QList<QCheckBox*> m_plotVisibleGraphs;

    // All values
    QVector<QVector<float>> m_values;

    // All times
    QVector<int> m_times;

    // Start index for graph
    int m_startIndex;

    // Avaluable colors for graphs
    QList<QColor> m_avaluableColors;

    // First display flag
    bool m_isFirstRun;
};

#endif // GRAPHS_H
