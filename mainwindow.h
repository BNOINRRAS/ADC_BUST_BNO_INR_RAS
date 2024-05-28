#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QListWidget>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
//#include <QtCharts/QScatterSeries>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QMainWindow *currentWindow;
    QFont *fontUsed;
    QFont *fontUsed2;
    QFont *fontUsed3;
    QComboBox *selectionOfTheFile;
    QListWidget *listOfKadrs;

//    QLineSeries *series[9];
//    QChart *chart;
//    QChartView *chartView;
//    QValueAxis *currentAxisX;
//    QValueAxis *currentAxisY;

    QtCharts::QLineSeries *series[9];
    //QtCharts::QScatterSeries *series[9];
    QtCharts::QChart *chart;
    QtCharts::QChartView *chartView;
    QtCharts::QValueAxis *currentAxisX;
    QtCharts::QValueAxis *currentAxisY;

    unsigned short int zagolovok, block;
    unsigned long int numberOfKadr, razmer, max;

    unsigned short int god, den, chs, min, sec, msc, mks, run;
    unsigned short int term0, term1, term2, adc, BlockSize;

    char godChar[sizeof(god)], denChar[sizeof(den)], chsChar[sizeof(chs)],
         minChar[sizeof(min)], secChar[sizeof(sec)], mscChar[sizeof(msc)],
         mksChar[sizeof(mks)], runChar[sizeof(run)];
    char term0Char[sizeof(term0)], term1Char[sizeof(term1)],
         term2Char[sizeof(term2)], adcChar[sizeof(adc)],
         BlockSizeChar[sizeof(BlockSize)];

    int currentRun;
    int minutesInsideRun;
    QPointF mousePreviousPosition;
    QPointF mouseCurrentPosition;
    bool isFileChanged;
    bool isRightMouseButtonPressed;
    int noiseLevel;
    std::vector<short int> ADCValues;
    std::vector<short int> :: iterator ADCValuesIterator;
    std::vector<std::pair<short int, short int>> timesAndPlanes;
    std::vector<std::pair<short int, short int>> :: iterator timesAndPlanesIterator;
    bool isPlaneTriggered;
    short int triggerTime;

    void drawData();
    void scrollChart();

    bool eventFilter(QObject *object, QEvent *event);

    virtual void wheelEvent(QWheelEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

private slots:
    void readFileParametersThenDraw();
    void changeNumberOfKadr();

};
#endif // MAINWINDOW_H
