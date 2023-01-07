#include "graphs.h"
#include "ui_graphs.h"

#include "definitions.h"

#include <QColor>
#include <QFileDialog>

//-----------------------------------------------------------------------------------------------------------------------------------
Graphs::Graphs(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Graphs)
{
    ui->setupUi(this);

    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    ui->mainPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    // Coordinate axis usage time scale
   QSharedPointer<QCPAxisTickerTime> timeTicker(new   QCPAxisTickerTime);
   timeTicker->setTimeFormat("%h:%m:%s");
   //timeTicker->setTickCount();
   ui->mainPlot->xAxis->setTicker(timeTicker);
   ui->mainPlot->xAxis->setTickLabelRotation(30); // Set the X-axis time rotation angle of 30 degrees
    //ui->mainPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    //ui->mainPlot->xAxis->setDateTimeFormat("hh:mm:ss");

   ui->mainPlot->legend->setVisible(true);

   this->setObjectName("CustomPlotWidget");

   m_avaluableColors.push_back(QColorConstants::Black);
   m_avaluableColors.push_back(QColorConstants::DarkGray);
   m_avaluableColors.push_back(QColorConstants::Gray);
   m_avaluableColors.push_back(QColorConstants::LightGray);
   m_avaluableColors.push_back(QColorConstants::Red);
   m_avaluableColors.push_back(QColorConstants::Green);
   m_avaluableColors.push_back(QColorConstants::Blue);
   m_avaluableColors.push_back(QColorConstants::Cyan);
   m_avaluableColors.push_back(QColorConstants::Magenta);
   m_avaluableColors.push_back(QColorConstants::Yellow);
   m_avaluableColors.push_back(QColorConstants::DarkRed);
   m_avaluableColors.push_back(QColorConstants::DarkGreen);
   m_avaluableColors.push_back(QColorConstants::DarkBlue);
   m_avaluableColors.push_back(QColorConstants::DarkCyan);
   m_avaluableColors.push_back(QColorConstants::DarkMagenta);
   m_avaluableColors.push_back(QColorConstants::DarkYellow);

//   FlowLayout* flowLayout = new FlowLayout;
//   QWidget* scrollAreaContent = new QWidget;
//   scrollAreaContent->setLayout( flowLayout );
//   QScrollArea* scrollArea = new QScrollArea;
//   scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
//   scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
//   scrollArea->setWidgetResizable( true );
//   scrollArea->setWidget( scrollAreaContent );

//   // add your buttons the flowLayout ....
//   flowLayout->addWidget( button );

//   connect(ui->mainPlot, &QCustomPlot::selectionChangedByUser, this, [ this ] () { for (int i = 0; i < ui->mainPlot->graphCount(); i++)
//       { if (ui->mainPlot->graph(i)->selected() && (!(ui->mainPlot->graph(i)->name().left(4) == QString("(S)"))))
//               ui->mainPlot->graph(i)->setName(QString("(S) %1").arg(ui->mainPlot->graph(i)->name())); } });

   connect(ui->mainPlot, &QCustomPlot::selectionChangedByUser, this, [ this ] () { for (int i = 0; i < ui->mainPlot->graphCount(); i++)
       { QElapsedTimer workTimer;
         workTimer.start();
         QPen temp = ui->mainPlot->graph(i)->pen(); temp.setWidth(ui->mainPlot->graph(i)->selected() ? 10 : 1); ui->mainPlot->graph(i)->setPen(temp);
         ui->info->setText(QString("Selecting time %1 ms").arg(workTimer.elapsed()));
       } });

   connect(ui->loadPreset, &QPushButton::clicked, this, &Graphs::loadPreset);
   connect(ui->savePreset, &QPushButton::clicked, this, &Graphs::savePreset);


}

//-----------------------------------------------------------------------------------------------------------------------------------
Graphs::~Graphs()
{
    delete ui;
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Graphs::setDescriptions(QVector<QString> descriptions)
{
    m_scrollAreaWidget = new QWidget();
    m_plotDescriptionsLayout = new QGridLayout(m_scrollAreaWidget);
    //m_plotDescriptionsLayout->setDefaultPositioning()

    for (int i = 0; i < descriptions.length(); i++)
    {
        m_plotVisibleGraphs.push_back(new QCheckBox(m_scrollAreaWidget));
        m_plotVisibleGraphs.last()->setText(QString("%1. %2").arg(i).arg(descriptions[i]));
        m_plotDescriptionsLayout->addWidget(m_plotVisibleGraphs.last());
        if (!((descriptions[i].contains(QString("UNKNOWN"))) || (descriptions[i].contains(QString("limit"))) || (descriptions[i].contains(QString("time")))))
            m_plotVisibleGraphs.last()->setChecked(true);
        connect(m_plotVisibleGraphs.last(), &QCheckBox::clicked, this, &Graphs::displayGraphInternal);
        connect(m_plotVisibleGraphs.last(), &QCheckBox::clicked, this, &Graphs::savePreset);
    }

    ui->scrollArea->setWidget(m_scrollAreaWidget);

    m_isFirstRun = true;

    loadPreset();
//    QCheckBox* widget;
//    for (int i = 0; i < descriptions.length(); i++)
//    {
//        widget = new QCheckBox();
//        widget->setText(QString("%1. %2").arg(i).arg(descriptions[i]));
//        //ui->listView->addScrollBarWidget()
//        //ui->scrollArea->addScrollBarWidget(widget, Qt::AlignVCenter);
//        //ui->verticalLayout_2->addWidget(widget);
//        //break;
//        //if (i > 20) break;
//    }
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Graphs::displayGraph(QVector<QVector<float>> values, QVector<int> times, QVector<QString> descriptions, int startIndex)
{
    m_values = values;
    m_times = times;
    m_startIndex = startIndex;

    if (m_isFirstRun)
    {
        for (int i = 0; i < m_values[(startIndex + m_values.length() - 1) % m_values.length()].length(); i++)
        {
            if (m_values[(startIndex + m_values.length() - 1) % m_values.length()][i] < 0.00000001)
            {
                if (i < m_plotVisibleGraphs.length())
                    m_plotVisibleGraphs[i]->setChecked(false);
            }
        }
        m_isFirstRun = false;
    }

    displayGraphInternal();
    return;

    QElapsedTimer workTimer;
    workTimer.start();

    ui->mainPlot->clearGraphs();

    int timesCount = times.length();
    if (timesCount <= 0)
        return;
    QVector<double> timesForGraph(timesCount);
    QVector<double> valuesForGraph(timesCount);
    for (int i = 0; i < timesCount; i++)
        timesForGraph[i] = times[(i + startIndex) % timesCount];

#if 0
    if (descriptions.length() != values.length())
    {
        emit error(FATAL_ERROR, objectName(), QString("displayGraph: descriptions length != values length"));
        return;
    }
    int graphsCount = values.length();

    for (int i = 0; i < graphsCount; i++)
    {
        if (values[i].length() != timesCount)
        {
            emit error(FATAL_ERROR, objectName(), QString("displayGraph: values[%1] length != timesCount").arg(i));
            return;
        }

        for (int j = 0; j < timesCount; j++)
            valuesForGraph[j] = values[i][(j + startIndex) % timesCount];

        ui->mainPlot->addGraph();
        ui->mainPlot->graph(i)->setData(timesForGraph, valuesForGraph, true);
        ui->mainPlot->graph(i)->setName(descriptions[i]);
    }
#else
    if (timesCount != values.length())
    {
        emit error(FATAL_ERROR, objectName(), QString("displayGraph: timesCount != values length"));
        return;
    }
    int graphsCount = values.first().length();
    for (int i = 0; i < timesCount; i++)
    {
        if (values[i].length() != graphsCount)
        {
            emit error(FATAL_ERROR, objectName(), QString("displayGraph: values[%1] length != graphsCount").arg(i));
            return;
        }
    }
    if (graphsCount != descriptions.length())
    {
        emit error(FATAL_ERROR, objectName(), QString("displayGraph: graphsCount != descriptions length"));
        return;
    }

    for (int i = 0; i < graphsCount; i++)
    {
        if (descriptions[i] == QString("UNKNOWN"))
            continue;
        for (int j = 0; j < timesCount; j++)
            valuesForGraph[j] = values[(j + startIndex) % timesCount][i];

        ui->mainPlot->addGraph();
        ui->mainPlot->graph(ui->mainPlot->graphCount() - 1)->setData(timesForGraph, valuesForGraph, true);
        ui->mainPlot->graph(ui->mainPlot->graphCount() - 1)->setName(descriptions[i]);

        QColor randomColor;
        randomColor.setBlue(random() % 128);
        randomColor.setGreen(random() % 128);
        randomColor.setRed(random() % 128);
//        randomColor.setBlue(128);
//        randomColor.setGreen(128);
//        randomColor.setRed(128);
        ui->mainPlot->graph(ui->mainPlot->graphCount() - 1)->setPen(QPen(randomColor));
    }
#endif

    ui->mainPlot->xAxis->setRange(timesForGraph.first() - (timesForGraph.last() - timesForGraph.first()) * 0.1, timesForGraph.last() + (timesForGraph.last() - timesForGraph.first()) * 0.1);
    ui->mainPlot->replot();

    ui->info->setText(QString("Processing time %1 ms").arg(workTimer.elapsed()));
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Graphs::displayGraphInternal()
{
    if ((m_values.isEmpty()) || (m_times.isEmpty()))
        return;

    QElapsedTimer workTimer;
    workTimer.start();

    QList<int> lastSelectedGraphIndexes;
    QList<QCPDataSelection> lastSelections;
    for (int i = 0; i < ui->mainPlot->graphCount(); i++)
    {
        if (!ui->mainPlot->graph(i)->selected())
            continue;
        QStringList splittedGraphName = ui->mainPlot->graph(i)->name().split(QString(". "));
        if (splittedGraphName.isEmpty())
            continue;
        lastSelectedGraphIndexes.append(splittedGraphName.first().toInt());
        lastSelections.append(ui->mainPlot->graph(i)->selection());
    }

    ui->mainPlot->clearGraphs();

    int timesCount = m_times.length();
    if (timesCount <= 0)
        return;
    QVector<double> timesForGraph(timesCount);
    QVector<double> valuesForGraph(timesCount);
    for (int i = 0; i < timesCount; i++)
        timesForGraph[i] = m_times[(i + m_startIndex) % timesCount];

#if 0
    if (descriptions.length() != values.length())
    {
        emit error(FATAL_ERROR, objectName(), QString("displayGraph: descriptions length != values length"));
        return;
    }
    int graphsCount = values.length();

    for (int i = 0; i < graphsCount; i++)
    {
        if (values[i].length() != timesCount)
        {
            emit error(FATAL_ERROR, objectName(), QString("displayGraph: values[%1] length != timesCount").arg(i));
            return;
        }

        for (int j = 0; j < timesCount; j++)
            valuesForGraph[j] = values[i][(j + startIndex) % timesCount];

        ui->mainPlot->addGraph();
        ui->mainPlot->graph(i)->setData(timesForGraph, valuesForGraph, true);
        ui->mainPlot->graph(i)->setName(descriptions[i]);
    }
#else
    if (timesCount != m_values.length())
    {
        emit error(FATAL_ERROR, objectName(), QString("displayGraph: timesCount != values length"));
        return;
    }
    int graphsCount = m_values.first().length();
    for (int i = 0; i < timesCount; i++)
    {
        if (m_values[i].length() != graphsCount)
        {
            emit error(FATAL_ERROR, objectName(), QString("displayGraph: values[%1] length != graphsCount").arg(i));
            return;
        }
    }
    if (graphsCount != m_plotVisibleGraphs.length())
    {
        emit error(FATAL_ERROR, objectName(), QString("displayGraph: graphsCount != descriptions length"));
        return;
    }

    for (int i = 0; i < graphsCount; i++)
    {
        if (!m_plotVisibleGraphs[i]->isChecked())
            continue;
        for (int j = 0; j < timesCount; j++)
            valuesForGraph[j] = m_values[(j + m_startIndex) % timesCount][i];

        ui->mainPlot->addGraph();
        ui->mainPlot->graph(ui->mainPlot->graphCount() - 1)->setData(timesForGraph, valuesForGraph, true);
        ui->mainPlot->graph(ui->mainPlot->graphCount() - 1)->setName(QString("%1 (%2)").arg(m_plotVisibleGraphs[i]->text()).arg(valuesForGraph[timesCount - 1]));
        ui->mainPlot->graph(ui->mainPlot->graphCount() - 1)->setPen(QPen(m_avaluableColors[(ui->mainPlot->graphCount() - 1) % (m_avaluableColors.length())]));
        if (lastSelectedGraphIndexes.contains(i))
        {
            QPen tempPen = ui->mainPlot->graph(ui->mainPlot->graphCount() - 1)->pen();
            tempPen.setWidth(10);
            ui->mainPlot->graph(ui->mainPlot->graphCount() - 1)->setPen(tempPen);
            ui->mainPlot->graph(ui->mainPlot->graphCount() - 1)->setSelection(lastSelections[lastSelectedGraphIndexes.indexOf(i)]);
        }
//        QPen tempPen = ui->mainPlot->graph(ui->mainPlot->graphCount() - 1)->pen();
//        tempPen.setWidth(20);
//        ui->mainPlot->graph(ui->mainPlot->graphCount() - 1)->setPen(tempPen);

        QColor randomColor;
        randomColor.setBlue(random() % 128);
        randomColor.setGreen(random() % 128);
        randomColor.setRed(random() % 128);
//        randomColor.setBlue(128);
//        randomColor.setGreen(128);
//        randomColor.setRed(128);
        //ui->mainPlot->graph(ui->mainPlot->graphCount() - 1)->setPen(QPen(randomColor));
    }
#endif

    ui->mainPlot->xAxis->setRange(timesForGraph.first() - (timesForGraph.last() - timesForGraph.first()) * 0.1, timesForGraph.last() + (timesForGraph.last() - timesForGraph.first()) * 0.3);
    ui->mainPlot->replot();

    ui->info->setText(QString("Processing time %1 ms").arg(workTimer.elapsed()));
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Graphs::loadPreset()
{
    QPushButton* sender = qobject_cast<QPushButton*>(this->sender());
    QString loadFileName;
    if (sender == 0)
        loadFileName = QString("saved_graph_preset.txt");
    else
    {
        loadFileName = QFileDialog::getOpenFileName(this);
        if (loadFileName.isEmpty())
            return;
    }

//    QString loadFileName = QFileDialog::getOpenFileName(this);
//    if (loadFileName.isEmpty())
//        return;

    QFile loadedFile(loadFileName);
    if (!loadedFile.open(QFile::ReadOnly))
    {
        if (sender != 0)
            QMessageBox::warning(this, QString("Open file error"), QString("Cannot open file %1").arg(loadFileName));
        return;
    }
    QString data = QString::fromLocal8Bit(loadedFile.readAll()).simplified();
    loadedFile.close();
    QStringList splittedData = data.split(" ");
    QVector<int> checkedIndexes;
    foreach (QString indexString, splittedData)
    {
        bool conversionSuccess = false;
        int index = indexString.toInt(&conversionSuccess);
        if (!conversionSuccess)
        {
            QMessageBox::warning(this, QString("File parsing error"), QString("File %1: value %2 is not a number").arg(loadFileName).arg(indexString));
            return;
        }
        checkedIndexes.append(index);
    }
    for (int i = 0; i < m_plotVisibleGraphs.length(); i++)
        m_plotVisibleGraphs[i]->setChecked(checkedIndexes.contains(i));
}

//-----------------------------------------------------------------------------------------------------------------------------------
void Graphs::savePreset()
{
    QPushButton* sender = qobject_cast<QPushButton*>(this->sender());
    QString saveFileName;
    if (sender == 0)
        saveFileName = QString("saved_graph_preset.txt");
    else
    {
        saveFileName = QFileDialog::getSaveFileName(this);
        if (saveFileName.isEmpty())
            return;
    }

    QStringList data;
    for (int i = 0; i < m_plotVisibleGraphs.length(); i++)
    {
        if (m_plotVisibleGraphs[i]->isChecked())
            data.append(QString::number(i));
    }

    QFile fileForSave(saveFileName);
    if (!fileForSave.open(QFile::WriteOnly))
    {
        QMessageBox::warning(this, QString("Open file error"), QString("Cannot open file %1").arg(saveFileName));
        return;
    }
    fileForSave.write(data.join(" ").toLocal8Bit());
    fileForSave.close();
}

//-----------------------------------------------------------------------------------------------------------------------------------
