#include "mkyinfowidget.h"


MKYinfoWidget::MKYinfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MKYinfoWidget)
{
    ui->setupUi(this);
    resetNewDataFlags();
    customPlotInit();

    plotVelocityTime = QTime::currentTime();
    plotUprTime = QTime::currentTime();
    plotAngleTime = QTime::currentTime();
    plotDemo2Time = QTime::currentTime();

    hardwareScaleOffset = 0;
    testValue1Index = testValue2Index = 0;
    connectionLost = true;

    numOfAngleSamples = 0;
    numOfVelocitySamples = 0;

    saveAngleSamplesEnable = false;
    saveVelocitySamplesEnable = false;
    maxNumberOfSavedSamples = 0; // кол-во отсчетов, которые нужно сохранить
    iSaveAngleSample = 0;       // кол-во отсчетов угла
    iSaveVelocitySample = 0;     // кол-во отсчетов угла
    angleSaveSamplesList = NULL;
    velocitySaveSamplesList = NULL;


    ui->btnCopyAngleSamples->setVisible(false);
    ui->btnCopyVelocitySamples->setVisible(false);

    ui->groupBoxWorkingMode->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->pushButtonEngineSwithOff->setStyleSheet("color: rgb(250, 0, 0)");

    // Текущие ошибки ГС
    ui->labelGsErrorDescription->setText("");
    ui->labelGsErrorDescription->setStyleSheet("color: rgb(255, 0, 0)");

    // Контекстное меню диаграмм
    plotContextMenu = new QMenu();
    actionRescalePlot = plotContextMenu->addAction("Rescale");


    // Контекстное меню расширенных команд
    contextMenuExtendedDialog = new QMenu();
    actionExtentedCommand = contextMenuExtendedDialog->addAction("Расширенная команда");

    extendedCmdDialog = new ExtendedCommandDialog(this);
    extendedCmdDialog->setWindowTitle("Расширенная команда КУС");



    // Скрыть ненужные графики
    on_checkBoxVelocityPlot_clicked();
    on_checkBoxPlotUpr_clicked();
    on_checkBoxPlotAngle_clicked();
    on_checkBoxPlotDemo1_clicked();


    // Контекстное меню расширенной команды
    connect(ui->groupBoxWorkingMode, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(showExtDialogContextMenu(const QPoint)));
    connect(actionExtentedCommand, SIGNAL(triggered(bool)), extendedCmdDialog, SLOT(show()));

    // Диалог расширенной команды
    connect(extendedCmdDialog, SIGNAL(sendTestManualCommand(quint8*,quint8,bool,bool)),
            this, SIGNAL(sendTestManualCommand(quint8*,quint8,bool,bool)));
    connect(this, SIGNAL(answerOnExtendedCmdReceived(const quint8*,quint8)),
            extendedCmdDialog, SLOT(processReceivedCmd(const quint8*,quint8)));




    // Таймер запроса состояния КУС
    mkyStateRequestTimer = new QTimer(this);
    connect(mkyStateRequestTimer, SIGNAL(timeout()), this, SIGNAL(mkyStatusRequest()));

    // Таймер проверки приема ответа от КУС
    mkyCheckFreshDataTimer = new QTimer(this);
    connect(mkyCheckFreshDataTimer, SIGNAL(timeout()), this, SLOT(onCheckFreshDataTimeout()));

    // Таймер обновления графиков
    plotRefreshTimer = new QTimer(this);
    connect(plotRefreshTimer, SIGNAL(timeout()), this, SLOT(refreshPlots()));


    // Контекстное меню
    connect(ui->customPlotVelocity, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(showPlotContextMenu(const QPoint)));
    connect(actionRescalePlot, SIGNAL(triggered(bool)), this, SLOT(rescalePlot()));

    // Запрос ПИД-коэффициентов
    connect(ui->pushButtonGetPIDKoef, SIGNAL(clicked(bool)), this, SIGNAL(mkyPidKoefRequest()));

    // Установка новых значений коэффициентов ПИД-регулятора
    connect(ui->pushButtonSetPIDkoef, SIGNAL(clicked(bool)), this, SLOT(setNewPidKoef()));

    // Сохранить коэффициенты ПИД-регулятора во флеш-памяти контроллера
    connect(ui->pushButtonSavePIDKoefInFlash, SIGNAL(clicked(bool)), this, SIGNAL(mkySavePidKoefInFlashRequest()));

    // Калибровка по скорости
    connect(ui->pushButtonVelocityCalibration, SIGNAL(clicked(bool)), this, SIGNAL(mkyVelocityCalibration()));

    // Выключение двигателей
    connect(ui->pushButtonEngineSwithOff, SIGNAL(clicked(bool)), this, SIGNAL(engineSwithOff()));

    // Изменить аппаратную шкалу угла
    connect(ui->btnSaveCurrentHardwareAngle, SIGNAL(clicked(bool)), this, SIGNAL(saveNewHardwareScale()));

    // Туда-сюда АР
    connect(ui->pushButtonSetModeTudaSudaAR, SIGNAL(clicked(bool)), this, SLOT(on_pushButtonSetModeTudaSudaAR_clicked()));

    // Калибровка ДУС по температуре
    connect(ui->pushButtonDusTemperatureCalibration, SIGNAL(clicked(bool)), this, SIGNAL(setModeDusTemperatureCalibration()));
}

MKYinfoWidget::~MKYinfoWidget()
{
    delete ui;
}


// Инициализация плота
void MKYinfoWidget::customPlotInit()
{
    // Улговая скорость
    ui->customPlotVelocity->xAxis2->setLabel("ТЕКУЩАЯ СКОРОСТЬ");
    ui->customPlotVelocity->addGraph(); // blue line
    ui->customPlotVelocity->graph(0)->setPen(QPen(QColor(40, 110, 255)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->customPlotVelocity->xAxis->setTicker(timeTicker);
    ui->customPlotVelocity->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->customPlotVelocity->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlotVelocity->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlotVelocity->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlotVelocity->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlotVelocity->yAxis2, SLOT(setRange(QCPRange)));



    // Управляющее воздействие
    ui->customPlotUpr->xAxis2->setLabel("УПРАВЛЯЮЩИЙ СИГНАЛ");
    ui->customPlotUpr->addGraph(); // blue line
    ui->customPlotUpr->graph(0)->setPen(QPen(QColor(40, 110, 255)));

    QSharedPointer<QCPAxisTickerTime> timeTickerUpr(new QCPAxisTickerTime);
    timeTickerUpr->setTimeFormat("%h:%m:%s");
    ui->customPlotUpr->xAxis->setTicker(timeTickerUpr);
    ui->customPlotUpr->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlotUpr->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlotUpr->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlotUpr->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlotUpr->yAxis2, SLOT(setRange(QCPRange)));





    // Угол
    ui->customPlotAngle->xAxis2->setLabel("УГОЛ В АППАРАТНОЙ ШКАЛЕ");
    ui->customPlotAngle->addGraph(); // blue line
    ui->customPlotAngle->graph(0)->setPen(QPen(QColor(40, 110, 255)));

    QSharedPointer<QCPAxisTickerTime> timeTickerAngle(new QCPAxisTickerTime);
    timeTickerAngle->setTimeFormat("%h:%m:%s");
    ui->customPlotAngle->xAxis->setTicker(timeTickerAngle);
    //ui->customPlotAngle->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
    ui->customPlotAngle->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlotAngle->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlotAngle->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlotAngle->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlotAngle->yAxis2, SLOT(setRange(QCPRange)));



    // Тестовые значения 2
    ui->customPlotDemo2->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->customPlotDemo2->xAxis2->setLabel("ТЕСТ");
    ui->customPlotDemo2->addGraph(); // blue line
    ui->customPlotDemo2->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    ui->customPlotDemo2->yAxis2->setVisible(true);

    QSharedPointer<QCPAxisTickerTime> timeTickerTest1(new QCPAxisTickerTime);
    timeTickerTest1->setTimeFormat("%h:%m:%s");
    ui->customPlotDemo2->xAxis->setTicker(timeTickerTest1);

    //ui->customPlotDemo2->graph(0)->setLineStyle(QCPGraph::lsNone);
    //ui->customPlotDemo2->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
    ui->customPlotDemo2->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlotDemo2->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlotDemo2->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlotDemo2->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlotDemo2->yAxis2, SLOT(setRange(QCPRange)));

}



// Обновление графика скорости
void MKYinfoWidget::updatePlotVelocity(float velocity)
{
    // calculate two new data points:
    double key = plotVelocityTime.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    bool scaleInKeyRange = ui->checkBoxPlotScaleInKeyRange->isChecked();

    ui->customPlotVelocity->graph(0)->addData(key, velocity);// add data to lines:
    if(scaleInKeyRange) ui->customPlotVelocity->graph(0)->rescaleValueAxis(false, true);
    else ui->customPlotVelocity->graph(0)->rescaleValueAxis(); // rescale value (vertical) axis to fit the current data:

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlotVelocity->xAxis->setRange(key, 8, Qt::AlignRight);
    //ui->customPlotVelocity->replot();
}

// Обновление графика управляющего сигнала
void MKYinfoWidget::updatePlotUpr(float upr)
{
    double key = plotUprTime.elapsed()/1000.0; // time elapsed since start of demo, in seconds

    ui->customPlotUpr->graph(0)->addData(key, upr);
    bool scaleInKeyRange = ui->checkBoxPlotScaleInKeyRange->isChecked();
    if(scaleInKeyRange) ui->customPlotUpr->graph(0)->rescaleValueAxis(false, true);
    else ui->customPlotUpr->graph(0)->rescaleValueAxis(); // rescale value (vertical) axis to fit the current data:

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlotUpr->xAxis->setRange(key, 8, Qt::AlignRight);
    //ui->customPlotUpr->replot();
}

// Обновление графика угла
void MKYinfoWidget::updatePlotAngle(float value)
{
    double key = plotAngleTime.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    bool scaleInKeyRange = ui->checkBoxPlotScaleInKeyRange->isChecked();

    ui->customPlotAngle->graph(0)->addData(key, value);// add data to lines:
    if(scaleInKeyRange) ui->customPlotAngle->graph(0)->rescaleValueAxis(false, true);
    else ui->customPlotAngle->graph(0)->rescaleValueAxis(); // rescale value (vertical) axis to fit the current data:

    ui->customPlotAngle->xAxis->setRange(key, 8, Qt::AlignRight);
}


// Обновление графика Demo2
void MKYinfoWidget::updatePlotDemo2(float value)
{
    //static QTime plotDemo2Time(QTime::currentTime());
    double key = plotDemo2Time.elapsed()/1000.0; // time elapsed since start of demo, in seconds

    ui->customPlotDemo2->graph(0)->addData(key, value);
    bool scaleInKeyRange = ui->checkBoxPlotScaleInKeyRange->isChecked();
    if(scaleInKeyRange) ui->customPlotDemo2->graph(0)->rescaleValueAxis(false, true);
    else ui->customPlotDemo2->graph(0)->rescaleValueAxis(); // rescale value (vertical) axis to fit the current data:

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlotDemo2->xAxis->setRange(key, 8, Qt::AlignRight);
}

// Обновить графики
void MKYinfoWidget::refreshPlots()
{
    ui->customPlotVelocity->replot();
    ui->customPlotUpr->replot();
    ui->customPlotAngle->replot();
    ui->customPlotDemo2->replot();// Тестовое значение 2
}





// Открыть контекстное меню
void MKYinfoWidget::showPlotContextMenu(const QPoint &pos)
{
   QWidget *_sender = (QWidget*)sender();
   plotContextMenu->exec(_sender->mapToGlobal(pos));
}

// Отмасштабировать
void MKYinfoWidget::rescalePlot()
{
    //QCustomPlot *plot = (QCustomPlot*)plotContextMenu->window();

    //customPlot->graph(0)->data().clear();
    //customPlot->graph(0)->rescaleValueAxis(false, true);
}


// сбросить флаги приема новых данных
void MKYinfoWidget::resetNewDataFlags()
{
    timeFreshData = false;
}

// Включить/отключить отпрос состояния КУС
void MKYinfoWidget::setStateRequestTimerEnable(bool enable)
{
    if(enable)
    {
        mkyStateRequestTimer->start(MKY_CHECK_STATE_INTERVAL);          // опрос девайса
        mkyCheckFreshDataTimer->start(MKY_CHECK_FRESH_DATA_INTERVAL);   // проверка ответов от девайса
        plotRefreshTimer->start(MKY_REFRESH_PLOT_INTERVAL);             // обновление графиков


        updateOutputDebugingInfo();
        emit mkyStatusRequest();
        emit mkyPidKoefRequest();
        emit hardwareScaleOffsetRequest();       
    }
    else
    {
        // Остановить таймеры и сбросить актуальность данных
        mkyStateRequestTimer->stop();
        mkyCheckFreshDataTimer->stop();
        plotRefreshTimer->stop();
        resetNewDataFlags();                // сбросить флаг приема новых данных
        onCheckFreshDataTimeout();          // сбросить актуальность принятых данных
        emit setOutputDebugingInfo(0);      // отключить вывод отладочной информации
    }
}



// Проверить прием данных от MKY
void MKYinfoWidget::onCheckFreshDataTimeout()
{
    // Время работы
    if(!timeFreshData)
    {
        ui->labelWorkingTime->setStyleSheet(TIMEOUT_SETTING_STYLE);
        connectionLost = true;
    }

    resetNewDataFlags();
}


// Общее время работы
void MKYinfoWidget::updateWorkingTimeRx(quint32 totalms)
{
    timeFreshData = true;
    quint32 timeH = 0, timeM = 0, timeS = 0, timeMs = 0;

    timeMs = totalms % 1000;     // кол-во миллисекунд (0...999)
    timeS = totalms / 1000;      // кол-во прошедших секунд (>60)
    timeM = timeS / 60;          // кол-во минут (>60)
    timeH = timeM/60;            // кол-во часов
    timeS = timeS%60;            // кол-во секунд (0..59)
    timeM = timeM%60;            // кол-во минут (0..60)

    ui->labelWorkingTime->setText(QString("Время работы %1:%2:%3").arg(timeH).arg(timeM,2,10,QLatin1Char('0')).arg(timeS,2,10,QLatin1Char('0')));
    ui->labelWorkingTime->setStyleSheet(ACTIVE_SETTING_STYLE);

    if(connectionLost)
    {
        connectionLost = false;
        updateOutputDebugingInfo();
    }
}


// Средняя скорость по углу
void MKYinfoWidget::updateCourseAverageVelocity(float averVel)
{
    QString format = averVel<0 ? QString("Vср (по углу):%1°/c") : QString("Vср (по углу): %1°/c");
    ui->labelAverageVelocity->setText(format.arg(QString::number(averVel, 10, 3)));
}



// Обновить текущее значение коэффициента P PID-регулятора
void MKYinfoWidget::updateCoursePID_P(float P)
{
    //ui->label_current_P->setText(QString("%1").arg(QString::number(P)));
    ui->doubleSpinBox_Pkoef->setValue(P);
}

// Обновить текущее значение коэффициента I PID-регулятора
void MKYinfoWidget::updateCoursePID_I(float I)
{
    //ui->label_current_I->setText(QString("%1").arg(QString::number(I)));
    ui->doubleSpinBox_Ikoef->setValue(I);
}

// Обновить текущее значение коэффициента D PID-регулятора
void MKYinfoWidget::updateCoursePID_D(float D)
{
    //ui->label_current_D->setText(QString("%1").arg(QString::number(D)));
    ui->doubleSpinBox_Dkoef->setValue(D);
}

// Обновить текущее значение коэффициента N PID-регулятора
void MKYinfoWidget::updateCoursePID_N(float N)
{
    //ui->label_current_N->setText(QString("%1").arg(QString::number(D)));
    ui->doubleSpinBox_Nkoef->setValue(N);
}

// Обновить текущее значение интервала дискретизации
void MKYinfoWidget::updateCourseTdiskr(float t)
{
    //ui->label_current_Tdiskr->setText(QString("%1").arg(QString::number(t)));
    ui->doubleSpinBox_Tdiskr->setValue(t);
}


// Установка новых значений коэффициентов ПИД-регулятора
void MKYinfoWidget::setNewPidKoef()
{
    float p = ui->doubleSpinBox_Pkoef->value();
    float i = ui->doubleSpinBox_Ikoef->value();
    float d = ui->doubleSpinBox_Dkoef->value();
    float n = ui->doubleSpinBox_Nkoef->value();
    float tdiskr = ui->doubleSpinBox_Tdiskr->value();

    emit mkyUpdatePidKoef(p, i, d, n, tdiskr);
}



// Обновить скорость по курсу
void MKYinfoWidget::updateCourseVelocity(qint16 dusAmplitude)
{
    // Рассчет скорости
    float velocity = (float)dusAmplitude/32.0;

    // Сохранение отсчетов
    if(saveVelocitySamplesEnable)
    {
        iSaveVelocitySample++;
        if(iSaveVelocitySample <= maxNumberOfSavedSamples) // не скопированы все отсчеты
        {
            //velocitySaveSamplesList->append(velocity);
            velocitySaveSamplesList->append(dusAmplitude);
        }
        else    // скопированы все отсчеты
        {
            saveVelocitySamplesEnable = false;
            ui->btnCopyVelocitySamples->setVisible(true);
        }
    }

    // Децимация
    numOfVelocitySamples++;
    if(numOfVelocitySamples < ui->spinBoxDecimation->value()) return;
    numOfVelocitySamples = 0;

    // Отображение
    if(ui->checkBoxVelocityPlot->isChecked()) updatePlotVelocity(velocity);
    QString format = (velocity<0) ? QString("V:%1°/c") : QString("V: %1°/c");
    ui->labelCurrentVelocity->setText(format.arg(QString::number(velocity, 10, 3)));

    //updatePlotVelocity(dusAmplitude);
    //ui->labelCurrentVelocity->setText(QString("Амплидута ДУС(код): %1").arg(QString::number(dusAmplitude)));
}

// Обновить управляющее воздействие
void MKYinfoWidget::updateCourseUpr(float upr)
{
    if(ui->checkBoxPlotUpr->isChecked())updatePlotUpr(upr);
    //ui->textEditDebug->append(QString("%1").arg(QString::number(upr)));
}


// Отобразить следующее значение угла
void MKYinfoWidget::updateCurrentAngle(float angle360)
{    
    // Рассчет угла
    float angle = (angle360 > hardwareScaleOffset) ?
                  (angle360 - hardwareScaleOffset) :
                  (360 - (hardwareScaleOffset-angle360));
    if(angle > 180) angle = angle-360;

    // Сохранение отсчетов
    if(saveAngleSamplesEnable)
    {
        iSaveAngleSample++;
        if(iSaveAngleSample <= maxNumberOfSavedSamples) // не скопированы все отсчеты
        {
            angleSaveSamplesList->append(angle);
        }
        else    // скопированы все отсчеты
        {
            saveAngleSamplesEnable = false;
            ui->btnCopyAngleSamples->setVisible(true);
        }
    }

    // Децимация
    numOfAngleSamples++;
    if(numOfAngleSamples < ui->spinBoxDecimation->value()) return;
    numOfAngleSamples = 0;

    // Отображение
    if(ui->checkBoxPlotAngle->isChecked()) updatePlotAngle(angle);
    QString format = (angle<0) ? QString("φ:%1°") : QString("φ: %1°");
    ui->labelCurrentAngle->setText(format.arg(QString::number(angle,10,6)));

    //qDebug() << angle;
    //ui->textEditValue1->append(format.arg(QString::number(angle,10,6)));
}

// Отобразить следующее тестовое значение 1
void MKYinfoWidget::updateTestValue2(float value)
{
    //ui->textEditValue1->append(QString("%1").arg(value));
    if(ui->checkBoxPlotDemo1->isChecked()) updatePlotDemo2(value);
}










// открыть контекстное меню расширенного диалога
void MKYinfoWidget::showExtDialogContextMenu(const QPoint &pos)
{
    contextMenuExtendedDialog->exec(ui->groupBoxWorkingMode->mapToGlobal(pos));
}


// Включить режим работы "Туда-сюда ВУС"
void MKYinfoWidget::on_pushButtonSetModeTudaSudaVUS_clicked()
{
    quint16 period = ui->spinBoxDutaSudaVUSTime->value();
    float vel = ui->doubleSpinBoxTudaSudaVelocity->value();
    emit setModeTudaSudaVUS(period, vel);
}

// Включить режим работы "Туда-сюда АР"
void MKYinfoWidget::on_pushButtonSetModeTudaSudaAR_clicked()
{
    quint16 period = ui->spinBoxDutaSudaARTime->value();
    qint16 angle1, angle2;

    angle1 = (qint16)(ui->doubleSpinBoxTudaSudaArAngle1->value()*128);
    angle2 = (qint16)(ui->doubleSpinBoxTudaSudaArAngle2->value()*128);

    emit setModeTudaSudaAR(period, angle1,  angle2);
}


// Отображение графика скорости
void MKYinfoWidget::on_checkBoxVelocityPlot_clicked()
{
    bool isVisible = ui->checkBoxVelocityPlot->isChecked();
    ui->customPlotVelocity->setVisible(isVisible);
    updateOutputDebugingInfo();
}

// Отображение графика управляющего воздействия
void MKYinfoWidget::on_checkBoxPlotUpr_clicked()
{
    bool isVisible = ui->checkBoxPlotUpr->isChecked();
    ui->customPlotUpr->setVisible(isVisible);
    updateOutputDebugingInfo();
}

// Отображение графика угла
void MKYinfoWidget::on_checkBoxPlotAngle_clicked()
{
    bool isVisible = ui->checkBoxPlotAngle->isChecked();
    ui->customPlotAngle->setVisible(isVisible);
    updateOutputDebugingInfo();
}

// Отображение графика тестового значения 1
void MKYinfoWidget::on_checkBoxPlotDemo1_clicked()
{
    bool isVisible = ui->checkBoxPlotDemo1->isChecked();
    ui->customPlotDemo2->setVisible(isVisible);
    updateOutputDebugingInfo();
}

// Обновить список параметров для отображения
void MKYinfoWidget::updateOutputDebugingInfo()
{
    bool enableVelocity = ui->checkBoxVelocityPlot->isChecked();
    bool enableUpr = ui->checkBoxPlotUpr->isChecked();
    bool enableAngle = ui->checkBoxPlotAngle->isChecked();
    bool enableTest1 = ui->checkBoxPlotDemo1->isChecked();

    bool globalEnable = (int)enableVelocity | (int)enableUpr | (int)enableAngle | (int)enableTest1;       // логическое "ИЛИ" по всем параметрам

    quint32 word = (int)globalEnable |
                   (int)enableVelocity<<1 |
                   (int)enableUpr<<2 |
                   (int)enableAngle<<3 |
                   (int)enableTest1<<4;
    emit setOutputDebugingInfo(word);
}

// Получено сохраненное контроллером в ходе теста значение
void MKYinfoWidget::updateSavedValue(float value)
{
    ui->textEditValue1->append(QString("%1").arg(value));
}


// Получено состояние
void MKYinfoWidget::updateGsState_word1(const quint8 *pBuf, quint8 len)
{
    (void)len;
    ui->label_GkCurrentMode->setStyleSheet("color: rgb(0, 0, 0)");

    switch(pBuf[1])
    {
        case GK_MODE_ENGINE_OFF_BY_CMD:
        {
            ui->label_GkCurrentMode->setText("Двигатели выключены");
        }
        break;

        case GK_MODE_VUS:
        {
            qint16 velCode128 = (qint16)(pBuf[2]<<8 | pBuf[3]);
            float velocity = (float)velCode128/128.0;
            ui->label_GkCurrentMode->setText(QString("ВУС\t%1°/c").arg(velocity));
        }
        break;

        case GK_MODE_AR:
        {
            qint16 angleCode128 = (qint16)(pBuf[2]<<8 | pBuf[3]);
            float angle = (float)angleCode128/128.0;
            ui->label_GkCurrentMode->setText(QString("Арретирование\t%1°").arg(angle));
        }
        break;

        case GK_MODE_CALIBRATION:
        {
            ui->label_GkCurrentMode->setText("Калибровка");
        }
        break;

        case GK_MODE_TUDA_SUDA_VUS:
        {
            float velocityF = *((float*)(&pBuf[2]));
            int period = (quint16)(pBuf[6]<<8 | pBuf[7]);
            ui->label_GkCurrentMode->setText(QString("Туда-сюда ВУС    T=%1мс   V=%2°/c").arg(period).arg(velocityF));
        }
        break;

        case GK_MODE_TUDA_SUDA_AR:
        {
            //float velocityF = *((float*)(&pBuf[2]));
            //int period = (quint16)(pBuf[6]<<8 | pBuf[7]);
            //ui->label_GkCurrentMode->setText(QString("Туда-сюда ВУС    T=%1мс   V=%2°/c").arg(period).arg(velocityF));
            ui->label_GkCurrentMode->setText(QString("Туда-сюда АР"));
        }
        break;


        case GK_MODE_ENGINE_OFF_BY_SPEED_PROTECTION:
        {
            ui->label_GkCurrentMode->setText("Защита по скорости");
            ui->label_GkCurrentMode->setStyleSheet("color: rgb(255, 0, 0)");
        }
        break;

        case GK_MODE_ENGINE_DISABLED:
        {
            ui->label_GkCurrentMode->setText("Запрет управления");
            ui->label_GkCurrentMode->setStyleSheet("color: rgb(255, 0, 0)");
        }
        break;

        case GK_MODE_TP:
        {
            ui->label_GkCurrentMode->setText("Транспортное положение");
        }
        break;


        case GK_MODE_VUO:
        {
            qint16 angleCode128 = (qint16)(pBuf[2]<<8 | pBuf[3]);
            float angle = (float)angleCode128/128.0;

            qint16 velCode128 = (qint16)(pBuf[4]<<8 | pBuf[5]);
            float velocity = (float)velCode128/128.0;


            ui->label_GkCurrentMode->setText(QString("ВУО\t%1°"
                                                     "\t%2°/c").arg(angle).arg(velocity));
        }
        break;

        case GK_MODE_SELFCONTROL:
        {
            quint8 selfControlStage = pBuf[2];
            FuncTestErrorCode_t errCode = (FuncTestErrorCode_t)(pBuf[3]<<8 | pBuf[4]);


            ui->label_GkCurrentMode->setStyleSheet("color: rgb(0, 0, 0)");
            QString description;

            switch(selfControlStage)
            {
                case FUNCTEST_INIT: description = "Функциональный тест"; break;

                case FUNCTEST_VUS_TEST:
                    {
                        qint16 velocity32 = (qint16)(pBuf[5]<<8 | pBuf[6]);
                        float velocity = (float)velocity32/32.0;
                        description = QString("Функциональный тест. ВУС %1°/c").arg(velocity);
                    }
                    break;

                case FUNCTEST_AR_TEST:
                    {
                        qint16 angle128 = (qint16)(pBuf[5]<<8 | pBuf[6]);
                        float angle = (float)angle128/128.0;
                        description = QString("Функциональный тест. АР %1°").arg(angle);
                    }
                    break;

                case FUNCTEST_COMPLETE_SUCCESS:
                    description = QString("Функциональный тест завершен");
                    ui->label_GkCurrentMode->setStyleSheet("color: rgb(0, 100, 0)");
                    break;

                case FUNCTEST_COMPLETE_WITH_ERROR:
                    {
                        QString error = "";
                        if(errCode & FuncTestErrorCode_TestTimeout)
                        {
                            error += "выполняется дольше 60с\r\n";
                        }

                        if(errCode & FuncTestErrorCode_CourseMoveToStartTimeout)
                        {
                            error += "КУРС: истекло время выхода на стартовую позицию\r\n";
                        }
                        if(errCode & FuncTestErrorCode_CourseArrieterTimeout)
                        {
                            error += "КУРС: истекло время выхода в арретир\r\n";
                        }
                        if(errCode & FuncTestErrorCode_CourseRotationTimeout)
                        {
                            error += "КУРС: истекло время выполнения одного оборота\r\n";
                        }
                        if(errCode & FuncTestErrorCode_CourseVusVelocityIncorrect)
                        {
                            error += "КУРС: скорость ВУС не соответствует заданной\r\n";
                        }


                        if(errCode & FuncTestErrorCode_TangageMoveToStartTimeout)
                        {
                            error += "ТАНГАЖ: истекло время выхода на стартовую позицию\r\n";
                        }
                        if(errCode & FuncTestErrorCode_TangageArrieterTimeout)
                        {
                            error += "ТАНГАЖ: истекло время выхода в арретир\r\n";
                        }
                        if(errCode & FuncTestErrorCode_TangageRotationTimeout)
                        {
                            error += "ТАНГАЖ: истекло время выполнения одного оборота\r\n";
                        }
                        if(errCode & FuncTestErrorCode_TangageVusVelocityIncorrect)
                        {
                            error += "ТАНГАЖ: скорость ВУС не соответствует заданной\r\n";
                        }

                        description = QString("Функциональный тест завершен\n%1").arg(error);
                        ui->label_GkCurrentMode->setStyleSheet("color: rgb(255, 0, 0)");
                    }
                    break;

                    default:
                        description = "Самоконтроль. UNKNOWN STATE";
                        ui->label_GkCurrentMode->setStyleSheet("color: rgb(255, 0, 0)");
                        break;
            }

            ui->label_GkCurrentMode->setText(description);


/*
 *          //quint8 numOfArretierPosition = pBuf[4]; // номер сектора при функциональном тесте
            if(selfControlErrorCode != 0)
            {
                QString reason = "unknown";
                switch(selfControlErrorCode)
                {
                    case SC_ERRORCODE_SELFCONTROL_TIMEOUT:// самоконтроль не завершился в течении 60с
                        reason = "выполнялся дольше 60с";
                    break;

                    case SC_ERRORCODE_MOVE_TO_START_TIMEOUT:// шар не успел выйти на стартовую позицию
                        reason = "истекло время выхода на стартовую позицию";
                    break;

                    case SC_ERRORCODE_ARRETIER_TIMEOUT:// шар не успел выйти в арретир за отведенное время
                        reason = "истекло время выхода в арретир";
                    break;

                    case SC_ERRORCODE_ROTATION_TIMEOUT:// шар не успел сделать оборот за заданное время
                        reason = "истекло время выполнения одного оборота";
                    break;

                    case SC_ERRORCODE_VUS_VELOCITY_ERROR:// шар сделал оборот, но скорость не соответствует заданной (40 градусов вместо 20, например)
                        reason = "ошибка скорости в режиме ВУС";
                    break;
                }
                ui->label_GkCurrentMode->setText(QString("Самоконтроль\n%1").arg(reason));
                ui->label_GkCurrentMode->setStyleSheet("color: rgb(255, 0, 0)");
            }
            else
            {
                ui->label_GkCurrentMode->setText("Самоконтроль");
                ui->label_GkCurrentMode->setStyleSheet("color: rgb(0, 0, 0)");
            }
            */
        }
        break;

        case GK_MODE_INITIALIZE:
            ui->label_GkCurrentMode->setText("Инициализация");
        break;

        case GK_MODE_DUS_TEMP_CALIBRATION:
            ui->label_GkCurrentMode->setText("Калибровка ДУС по температуре");
        break;

        case GK_MODE_HARDFAULT:
            ui->label_GkCurrentMode->setText("КРИТИЧЕСКАЯ ОШИБКА!");
            ui->label_GkCurrentMode->setStyleSheet("color: rgb(255, 0, 0)");
            break;
    }
}

// Обновить смещение аппаратной шкалы
void MKYinfoWidget::updateHardwareScaleOffset(float offset)
{
    hardwareScaleOffset = offset;
}



// Сохранить отсчеты
void MKYinfoWidget::startSaveSamples(int count)
{
    // Сохранение отсчетов
    maxNumberOfSavedSamples = count; // кол-во отсчетов, которые нужно сохранить

    saveAngleSamplesEnable = true;
    saveVelocitySamplesEnable = true;
    iSaveAngleSample = 0;       // кол-во отсчетов угла
    iSaveVelocitySample = 0;     // кол-во отсчетов угла


    ui->btnCopyAngleSamples->setVisible(false);
    ui->btnCopyVelocitySamples->setVisible(false);

    if(angleSaveSamplesList != NULL) delete angleSaveSamplesList;
    if(velocitySaveSamplesList != NULL) delete velocitySaveSamplesList;

    angleSaveSamplesList = new QList<float>();
    velocitySaveSamplesList = new QList<float>();

}


// Скопировать значения угловой скорости
void MKYinfoWidget::on_btnCopyVelocitySamples_clicked()
{
    if(velocitySaveSamplesList == NULL) return;
    QString s = "";
    for(int i=0; i< velocitySaveSamplesList->count(); i++)
    {
        s += QString::number(velocitySaveSamplesList->at(i)) + "\r\n";
    }
    QApplication::clipboard()->setText(s);
}

// Скопировать значения угла
void MKYinfoWidget::on_btnCopyAngleSamples_clicked()
{
    if(angleSaveSamplesList == NULL) return;
    QString s = "";
    for(int i=0; i< angleSaveSamplesList->count(); i++)
    {
        s += QString::number(angleSaveSamplesList->at(i)) + "\r\n";
    }
    QApplication::clipboard()->setText(s);
}
