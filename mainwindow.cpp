#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <sstream>
#include <cstring>
#include <iomanip>

#include <QScroller>
#include <QLabel>
#include <QDir>
#include <QDebug>

QT_CHARTS_USE_NAMESPACE

bool sortPairs(const std::pair<short int, short int> &a, const std::pair<short int, short int> &b)
{
  return (a.second < b.second);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QLabel* colorsHintLabel = new QLabel(this);
    QPixmap colorsHint = QPixmap(":/ColorsHint.png");
    colorsHintLabel->setParent(this);
    colorsHintLabel->resize(1480, 40);
    colorsHintLabel->raise();
    colorsHintLabel->setPixmap(colorsHint);
    colorsHintLabel->move(8,720);
    colorsHintLabel->show();

    QStringList allFiles = QDir().entryList(QDir::Files); // получаем список всех файлов в текущей папке
    QStringList dataFiles;
    for(int i = 0; i < allFiles.size(); i++) // перебираем эти файлы, выбираем те что имеют начало "BNI"
    {
        if (allFiles[i][0] == 'B' && allFiles[i][1] == 'N' && allFiles[i][2] == 'I')
        {
            dataFiles.append(allFiles[i]);
        }
    }
    //qDebug()<<allFiles;
    //qDebug()<<dataFiles;

    fontUsed = new QFont();
    fontUsed->setBold(true);
    fontUsed->setPointSize(18);
    fontUsed2 = new QFont();
    fontUsed2->setBold(true);
    fontUsed2->setPointSize(14);
    fontUsed3 = new QFont();
    fontUsed3->setBold(true);
    fontUsed3->setPointSize(12);

    selectionOfTheFile = new QComboBox(this);
    selectionOfTheFile->resize(200, 40);
    selectionOfTheFile->move(0, 0);
    selectionOfTheFile->setFont(*fontUsed2);
    for(int i = 0; i < dataFiles.size(); i++)
    {
        selectionOfTheFile->addItem(dataFiles[i]);
    }
    selectionOfTheFile->show();

    currentAxisX = new QValueAxis();
    currentAxisY = new QValueAxis();
    for(int i = 0; i < 9; i++)
    {
        series[i]= new QLineSeries();
        //series[i]= new QScatterSeries();

    }
    chart = new QChart();
    chart->legend()->hide();
    chart->setTitle("Oscillogram");
    chart->setTitleFont(*fontUsed);
    chart->QChart::addAxis(currentAxisX, Qt::AlignBottom);
    chart->QChart::addAxis(currentAxisY, Qt::AlignLeft);
    for(int i = 0; i < 9; i++)
    {
        chart->addSeries(series[i]);
        series[i]->attachAxis(currentAxisX);
        series[i]->attachAxis(currentAxisY);
        //series[i]->setMarkerSize(5);
    }
    series[0]->setColor(QColor(255, 0, 0, 255));//red, красный, 6 пл.
    series[1]->setColor(QColor(255,165,0, 255));//orange, оранжевый, 5 пл.
    series[2]->setColor(QColor(255,255,0, 255));//yellow, желтый, 2 пл.
    series[3]->setColor(QColor(0,128,0, 255));//green, зеленый, 1 пл.
    series[4]->setColor(QColor(0,255,255, 255));//cyan, голубой, 8 пл.
    series[5]->setColor(QColor(0,0,255, 255));//blue, синий, 7 пл.
    series[6]->setColor(QColor(238,130,238, 255));//violet, фиолетовый, 4 пл.
    series[7]->setColor(QColor(0, 0, 0, 255));//black, черный, 3 пл.

    series[8]->setColor(QColor(128, 128, 128, 127));//gray, серый, шумовая линия

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    //chartView->setRubberBand(QChartView::RectangleRubberBand);
    chartView->resize(1280, 720);
    chartView->setParent(this);
    chartView->move(200, 0);
    chartView->viewport()->installEventFilter(this);
    chartView->viewport()->setMouseTracking(false);

    listOfKadrs = new QListWidget();
    listOfKadrs->setParent(this);
    listOfKadrs->move(0,42);
    listOfKadrs->resize(200, 670);
    listOfKadrs->setFont(*fontUsed3);
    listOfKadrs->show();

    if(dataFiles.size() > 0)
    {
        readFileParametersThenDraw();
    }
    else
    {
        chart->setTitle("Файлы не найдены. Скопируйте файлы в папку с программой и перезапустите.");
    }

    isFileChanged = false;//При запуске программы этот флажок false
    isRightMouseButtonPressed = false;//При запуске программы этот флажок false
    connect(selectionOfTheFile, SIGNAL(currentIndexChanged(int)),
            this, SLOT(readFileParametersThenDraw() ) );
    connect(listOfKadrs, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(changeNumberOfKadr() ) );
}

void MainWindow::drawData()
{
    short int DATA;
    char DATAChar[sizeof(DATA)];
    int offsetDATA;
    //DATA = new short int[BlockSize];
    offsetDATA=zagolovok+( block ) * ( ( numberOfKadr ) - 1 );

    QFile ADCFile(this->selectionOfTheFile->currentText());
    ADCFile.open(QIODevice::ReadOnly);

    QDataStream dataStreamFromADCFile(&ADCFile);    // read the data serialized from the file
    ADCFile.seek(offsetDATA + 2);//В отличии от кода Курени А. здесь нужно
    //сместиться еще на 2 байта для корректного чтения последующих данных
    //qDebug()<<ADCFile.pos();
    //qDebug()<<i;

    dataStreamFromADCFile.readRawData(term1Char, sizeof(term1));
    dataStreamFromADCFile.readRawData(godChar, sizeof(god));
    dataStreamFromADCFile.readRawData(denChar, sizeof(den));
    dataStreamFromADCFile.readRawData(chsChar, sizeof(chs));
    dataStreamFromADCFile.readRawData(runChar, sizeof(run));
    dataStreamFromADCFile.readRawData(minChar, sizeof(min));
    dataStreamFromADCFile.readRawData(secChar, sizeof(sec));
    dataStreamFromADCFile.readRawData(mscChar, sizeof(msc));
    dataStreamFromADCFile.readRawData(mksChar, sizeof(mks));
    dataStreamFromADCFile.readRawData(term2Char, sizeof(term2));
    //qDebug()<<ADCFile.pos();

    std::memcpy(&term1, term1Char, sizeof(term1));
    std::memcpy(&god, godChar, sizeof(god));
    std::memcpy(&den, denChar, sizeof(den));
    std::memcpy(&chs, chsChar, sizeof(chs));
    std::memcpy(&run, runChar, sizeof(run));
    std::memcpy(&min, minChar, sizeof(min));
    std::memcpy(&sec, secChar, sizeof(sec));
    std::memcpy(&msc, mscChar, sizeof(msc));
    std::memcpy(&mks, mksChar, sizeof(mks));
    std::memcpy(&term2, term2Char, sizeof(term2));

//![1]
//!
//![1]

//![2]
    ADCValues.clear();
    short int minValue = 2048, maxValue = -2048;
    short int maxValueHorizontal;
    noiseLevel = 2048;
    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j < BlockSize/2; j++)
        {
          dataStreamFromADCFile.readRawData(DATAChar, sizeof(DATA));
          std::memcpy(&DATA, DATAChar, sizeof(DATA));
          if(DATA < 2048 && DATA > -2048)
          {
              if(minValue > DATA) {minValue = DATA; maxValueHorizontal = j;}
              if(maxValue < DATA) {maxValue = DATA;}
              series[i]->append(j, DATA);
          }
          ADCValues.push_back(DATA);
          //qDebug()<<j;
          //qDebug()<<DATA;
        }
        for(int j = BlockSize/2; j < BlockSize; j++)
        {
            dataStreamFromADCFile.readRawData(DATAChar, sizeof(DATA));
            std::memcpy(&DATA, DATAChar, sizeof(DATA));
            if(DATA < 2048 && DATA > -2048)
            {
                if(minValue > DATA) {minValue = DATA; maxValueHorizontal = j;}
                if(maxValue < DATA) {maxValue = DATA;}
                series[i]->append(j, DATA);
                if(noiseLevel > DATA) {noiseLevel = DATA;}
            }
        }
        //qDebug()<<minValue;
        //qDebug()<<maxValue;
        dataStreamFromADCFile.readRawData(term2Char, sizeof(term2));
        std::memcpy(&term2, term2Char, sizeof(term2));
    }
//![2]

//![3]
    //currentAxisX->setRange(0, BlockSize);
    currentAxisX->setRange(maxValueHorizontal-30, maxValueHorizontal+50);
    currentAxisY->setRange(minValue-10, maxValue+10);

    for(int i = 0; i < 8; i++)
    {
        chart->addSeries(series[i]);
        series[i]->attachAxis(currentAxisX);
        series[i]->attachAxis(currentAxisY);
    }
    series[8]->append(0, noiseLevel * 1.1);//Сделаем условие чтобы сигнал на 10% превышал уровень шумов
    series[8]->append(BlockSize - 1, noiseLevel * 1.1);
    chart->addSeries(series[8]);
    series[8]->attachAxis(currentAxisX);
    series[8]->attachAxis(currentAxisY);

    timesAndPlanes.clear();
    ADCValuesIterator = ADCValues.begin();
    for(int i = 0; i < 8; i++)
    {
        isPlaneTriggered = false;
        for(int j = 0; j < BlockSize/2; j++)
        {
            if(*ADCValuesIterator < noiseLevel * 1.1)
            {
                if(isPlaneTriggered == false)
                {
                    triggerTime = j;
                }
                isPlaneTriggered = true;
            }
            ADCValuesIterator++;
        }
        if (isPlaneTriggered == true)
        {
            timesAndPlanes.push_back(std::make_pair(i, triggerTime));
        }
    }
    std::sort(timesAndPlanes.begin(), timesAndPlanes.end(), sortPairs);

    if(min >= 45) { minutesInsideRun = min - 45; }
    else if(min >= 30) { minutesInsideRun = min - 30; }
    else if(min >= 15) { minutesInsideRun = min - 15; }
    else if(min >= 0) { minutesInsideRun = min; }
    std::stringstream information;
    information << "RUN " << currentRun << ", "
                << minutesInsideRun << "мин, " << sec << "сек, " << msc << "мс, " << mks << "мкс."
                << " Плоскости:";
    for(timesAndPlanesIterator = timesAndPlanes.begin();
        timesAndPlanesIterator != timesAndPlanes.end();
        timesAndPlanesIterator++)
    {
        if (timesAndPlanesIterator->first == 0)
        {
            information << " 6 (" << timesAndPlanesIterator->second << ")";
        }
        else if (timesAndPlanesIterator->first == 1)
        {
            information << " 5 (" << timesAndPlanesIterator->second << ")";
        }
        else if (timesAndPlanesIterator->first == 2)
        {
            information << " 2 (" << timesAndPlanesIterator->second << ")";
        }
        else if (timesAndPlanesIterator->first == 3)
        {
            information << " 1 (" << timesAndPlanesIterator->second << ")";
        }
        else if (timesAndPlanesIterator->first == 4)
        {
            information << " 8 (" << timesAndPlanesIterator->second << ")";
        }
        else if (timesAndPlanesIterator->first == 5)
        {
            information << " 7 (" << timesAndPlanesIterator->second << ")";
        }
        else if (timesAndPlanesIterator->first == 6)
        {
            information << " 4 (" << timesAndPlanesIterator->second << ")";
        }
        else if (timesAndPlanesIterator->first == 7)
        {
            information << " 3 (" << timesAndPlanesIterator->second << ")";
        }
    }
    chart->setTitle(QString::fromStdString(information.str()));
//![3]

    ADCFile.close();
}

void MainWindow::readFileParametersThenDraw()
{
    isFileChanged = true;//Ставим флажок перед тем как очистить список с кадрами
    char DATA;
    numberOfKadr = 1;
    QFile ADCFile(this->selectionOfTheFile->currentText());
    ADCFile.open(QIODevice::ReadOnly);

    //Время хранится в формате UTC+3 (Московское)
    QDataStream dataStreamFromADCFile(&ADCFile);
    dataStreamFromADCFile.readRawData(godChar, sizeof(god));
    dataStreamFromADCFile.readRawData(denChar, sizeof(den));
    dataStreamFromADCFile.readRawData(chsChar, sizeof(chs));
    dataStreamFromADCFile.readRawData(runChar, sizeof(run));
    dataStreamFromADCFile.readRawData(adcChar, sizeof(adc));
    dataStreamFromADCFile.readRawData(BlockSizeChar, sizeof(BlockSize));
    dataStreamFromADCFile.readRawData(term0Char, sizeof(term0));

    std::memcpy(&god, godChar, sizeof(god));
    std::memcpy(&den, denChar, sizeof(den));
    std::memcpy(&chs, chsChar, sizeof(chs));
    std::memcpy(&run, runChar, sizeof(run));
    std::memcpy(&adc, adcChar, sizeof(adc));
    std::memcpy(&BlockSize, BlockSizeChar, sizeof(BlockSize));
    std::memcpy(&term0, term0Char, sizeof(term0));

//    qDebug()<<god;
//    qDebug()<<den;
//    qDebug()<<chs;
//    qDebug()<<run;
//    qDebug()<<adc;

    zagolovok=sizeof(god)
            +sizeof(den)+sizeof(chs)
            +sizeof(run)+sizeof(adc)
            +sizeof(BlockSize)+sizeof(term0);
    block=sizeof(term1)+sizeof(god)
            +sizeof(den)+sizeof(chs)
            +sizeof(run)+sizeof(min)
            +sizeof(sec)+sizeof(msc)
            +sizeof(mks)+9*sizeof(term2)
            +2*8*BlockSize;
    razmer=ADCFile.size() - zagolovok;
    max=razmer/block;

    listOfKadrs->clear();
    dataStreamFromADCFile.readRawData(term1Char, sizeof(term1));//Перескакиваем через те 2 байта, которые мешают
    for(unsigned long i = 0; i < max; i++)
    {
        dataStreamFromADCFile.readRawData(term1Char, sizeof(term1));
        dataStreamFromADCFile.readRawData(godChar, sizeof(god));
        dataStreamFromADCFile.readRawData(denChar, sizeof(den));
        dataStreamFromADCFile.readRawData(chsChar, sizeof(chs));
        dataStreamFromADCFile.readRawData(runChar, sizeof(run));
        dataStreamFromADCFile.readRawData(minChar, sizeof(min));
        dataStreamFromADCFile.readRawData(secChar, sizeof(sec));
        dataStreamFromADCFile.readRawData(mscChar, sizeof(msc));
        dataStreamFromADCFile.readRawData(mksChar, sizeof(mks));
        dataStreamFromADCFile.readRawData(term2Char, sizeof(term2));

        std::memcpy(&term1, term1Char, sizeof(term1));
        std::memcpy(&god, godChar, sizeof(god));
        std::memcpy(&den, denChar, sizeof(den));
        std::memcpy(&chs, chsChar, sizeof(chs));
        std::memcpy(&run, runChar, sizeof(run));
        std::memcpy(&min, minChar, sizeof(min));
        std::memcpy(&sec, secChar, sizeof(sec));
        std::memcpy(&msc, mscChar, sizeof(msc));
        std::memcpy(&mks, mksChar, sizeof(mks));
        std::memcpy(&term2, term2Char, sizeof(term2));

//        qDebug()<<"BEGIN";
//        qDebug()<<god;
//        qDebug()<<den;
//        qDebug()<<chs;
//        qDebug()<<run;
//        qDebug()<<min;
//        qDebug()<<sec;
//        qDebug()<<msc;
//        qDebug()<<mks;
//        qDebug()<<"END";

        if(min >= 45) { minutesInsideRun = min - 45; }
        else if(min >= 30) { minutesInsideRun = min - 30; }
        else if(min >= 15) { minutesInsideRun = min - 15; }
        else if(min >= 0) { minutesInsideRun = min; }
        std::stringstream information;
        information << std::left << std::setw(10) << i + 1
                    << std::setw(4) << minutesInsideRun
                    << std::setw(4) << sec
                    << std::setw(5) << msc
                    << std::setw(3) << mks;
        new QListWidgetItem(QString::fromStdString(information.str()) , listOfKadrs);

        for(int j = 0; j < 8; j++)
        {
            for(int k = 0; k < 2 * BlockSize; k++)
            {
                dataStreamFromADCFile.readRawData(&DATA, 1);
            }
            dataStreamFromADCFile.readRawData(term2Char, sizeof(term2));
            std::memcpy(&term2, term2Char, sizeof(term2));
        }
    }
    ADCFile.close();

    unsigned long int unixTimeOfTheRUN = 0;
    for (int year = 1970; year < god; year++)
    {
        if (year % 4 != 0) { unixTimeOfTheRUN += 365 * 86400; }
        else if (year % 4 == 0 && year % 100 != 0) { unixTimeOfTheRUN += 366 * 86400; }
        else if (year % 4 == 0 && year % 100 == 0 && year % 400 != 0) { unixTimeOfTheRUN += 365 * 86400; }
        else if (year % 4 == 0 && year % 100 == 0 && year % 400 == 0) { unixTimeOfTheRUN += 366 * 86400; }
    }
    unixTimeOfTheRUN += ( ( den - 1 ) * 86400 ) + ( chs * 3600 ) + ( min * 60 ) - ( 3 * 3600 );
    currentRun = ( unixTimeOfTheRUN - 189290700 ) / ( 15 * 60 );

    for(int i = 0; i < 9; i++)
    {
        series[i]->clear();
        chart->removeSeries(series[i]);
    }

    drawData();
    listOfKadrs->setCurrentRow(0);
    listOfKadrs->setFocus();
}

void MainWindow::changeNumberOfKadr()
{
    //qDebug()<< "SLOT CHANGE NumberOfKadr WORKED";
    if(isFileChanged == false)
    {
        numberOfKadr = listOfKadrs->currentRow() + 1;
    }

    for(int i = 0; i < 9; i++)
    {
        series[i]->clear();
        chart->removeSeries(series[i]);
    }
    drawData();
    isFileChanged = false;
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if(event->angleDelta().y() > 0)
    {
        // Zoom in
        chartView->chart()->zoomIn();
    }
    else
    {
        // Zooming out
        chartView->chart()->zoomOut();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        //qDebug()<<"Right Button Pressed";
        isRightMouseButtonPressed = true;
        mousePreviousPosition = QCursor::pos();
        //qDebug()<<mousePreviousPosition;
    }
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (isRightMouseButtonPressed == true && event->type() == QEvent::MouseMove)
    {
        //qDebug()<<"Event Filter Worked On MOVE";
        mouseCurrentPosition = QCursor::pos();
        chartView->chart()->scroll(mousePreviousPosition.x() - mouseCurrentPosition.x(),
                                   mouseCurrentPosition.y() - mousePreviousPosition.y());
        mousePreviousPosition = mouseCurrentPosition;
        return true;
    }
    if (isRightMouseButtonPressed == true && event->type() == QEvent::MouseButtonRelease)
    {
        //qDebug()<<"Event Filter Worked On Release";
        isRightMouseButtonPressed = false;
        return true;
    }
    return false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

