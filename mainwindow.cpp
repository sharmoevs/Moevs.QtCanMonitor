#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setMinimumSize(600, 700);
    this->setWindowTitle(QString("Программа контроля ГК (CAN) тест отсчеты с ДУС")/*+ QString(APP_VERSION)*/);



    // Тулбар
    canDeviceSelector = new CANDeviceSelectorWidget();
    this->addToolBar(canDeviceSelector);

    mkyInfoWidget = new MKYinfoWidget(this);
    mkyInfoWidget->setExtendedDialogTitle("Расширенная команда МК управления по КУРСУ");
    mkzInfoWidget = new MKYinfoWidget(this);
    mkzInfoWidget->setExtendedDialogTitle("Расширенная команда МК управления по ТАНГАЖУ");
    canModule = new CanModule(this);

    // Вкладки контуров
    mainTabWidget = new QTabWidget(this);
    mainTabWidget->addTab(mkyInfoWidget, "Курс");
    mainTabWidget->addTab(mkzInfoWidget, "Тангаж");

    // Компонент для отображения общего состояния ГС
    gsGeneralState = new GsGeneralState();


    // Компоновка
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(gsGeneralState);
    mainLayout->addWidget(mainTabWidget);
    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(mainLayout);
    this->setCentralWidget(mainWidget);





    // CAN deviceSelector
    connect(canDeviceSelector,  SIGNAL(selectedDeviceChanged(int)),
                     this, SLOT(updateCANBusList()));
    connect(canDeviceSelector,  SIGNAL(refreshDeviceListRequest()),
                     this, SLOT(updateCANDeviceList()));
    connect(canDeviceSelector,  SIGNAL(openCloseRequest()),
                     this, SLOT(onCANOpenClose()));

    // mainTabWidget
    connect(mainTabWidget, SIGNAL(currentChanged(int)), this, SLOT(onMainTabWidgetTabChanged(int)));

    // MKY info widget
    connect(mkyInfoWidget, SIGNAL(mkyStatusRequest()), canModule, SLOT(mkyStatusRequest()));
    connect(mkyInfoWidget, SIGNAL(mkyPidKoefRequest()), canModule, SLOT(mkyPIDKoefRequest()));
    connect(mkyInfoWidget, SIGNAL(mkyUpdatePidKoef(float,float,float,float,float)), canModule, SLOT(mkySetPIDKoef(float,float,float,float,float)));
    connect(mkyInfoWidget, SIGNAL(mkySavePidKoefInFlashRequest()), canModule, SLOT(mkySavePIDKoefInFlash()));
    connect(mkyInfoWidget, SIGNAL(mkyVelocityCalibration()), canModule, SLOT(mkyStartVelocityCalibration()));
    connect(mkyInfoWidget, SIGNAL(sendTestManualCommand(quint8*,quint8,bool,bool)), canModule, SLOT(mkySendExtendedCommand(quint8*,quint8,bool,bool)));
    connect(mkyInfoWidget, SIGNAL(setModeTudaSudaVUS(quint16, float)), canModule, SLOT(mkySetModeTudaSudaVUS(quint16,float)));
    connect(mkyInfoWidget, SIGNAL(setOutputDebugingInfo(quint32)), canModule, SLOT(mkySetOutputDebugInfo(quint32)));
    connect(mkyInfoWidget, SIGNAL(engineSwithOff()), canModule, SLOT(mkyEngineSwitchOff()));
    connect(mkyInfoWidget, SIGNAL(saveNewHardwareScale()), canModule, SLOT(mkySaveNewHardwareScale()));
    connect(mkyInfoWidget, SIGNAL(hardwareScaleOffsetRequest()), canModule, SLOT(hardwareScaleOffsetsRequest()));
    connect(mkyInfoWidget, SIGNAL(setModeTudaSudaAR(quint16, qint16, qint16)), canModule, SLOT(mkySetModeTudaSudaAR(quint16,qint16, qint16)));
    connect(mkyInfoWidget, SIGNAL(setModeDusTemperatureCalibration()), canModule, SLOT(mkySetModeDusTemperatureCalibtarion()));

    connect(canModule, SIGNAL(onMKYWorkingTimeRx(quint32)), mkyInfoWidget, SLOT(updateWorkingTimeRx(quint32)));
    connect(canModule, SIGNAL(onMKYCourseVelocityRx(qint16)), mkyInfoWidget, SLOT(updateCourseVelocity(qint16)));
    connect(canModule, SIGNAL(onMKYCourseAverageVelocityRx(float)), mkyInfoWidget, SLOT(updateCourseAverageVelocity(float)));
    connect(canModule, SIGNAL(onMKYCourseUprRx(float)), mkyInfoWidget, SLOT(updateCourseUpr(float)));
    connect(canModule, SIGNAL(onMKYCourseRxPidP(float)), mkyInfoWidget, SLOT(updateCoursePID_P(float)));
    connect(canModule, SIGNAL(onMKYCourseRxPidI(float)), mkyInfoWidget, SLOT(updateCoursePID_I(float)));
    connect(canModule, SIGNAL(onMKYCourseRxPidD(float)), mkyInfoWidget, SLOT(updateCoursePID_D(float)));
    connect(canModule, SIGNAL(onMKYCourseRxPidN(float)), mkyInfoWidget, SLOT(updateCoursePID_N(float)));
    connect(canModule, SIGNAL(onMKYCourseRxTdiskr(float)), mkyInfoWidget, SLOT(updateCourseTdiskr(float)));
    connect(canModule, SIGNAL(onMKYCourseRxCurrentAngle(float)), mkyInfoWidget, SLOT(updateCurrentAngle(float)));
    connect(canModule, SIGNAL(onMKYCourseRxTestValue2(float)), mkyInfoWidget, SLOT(updateTestValue2(float)));
    connect(canModule, SIGNAL(onMKYAnswerOnExtendedCmdRX(const quint8*,quint8)), mkyInfoWidget, SIGNAL(answerOnExtendedCmdReceived(const quint8*,quint8)));
    connect(canModule, SIGNAL(onMKYCourseRxSavedValue(float)), mkyInfoWidget, SLOT(updateSavedValue(float)));
    connect(canModule, SIGNAL(onMKYState1RX(const quint8*,quint8)), mkyInfoWidget, SLOT(updateGsState_word1(const quint8*,quint8)));
    connect(canModule, SIGNAL(onMKYCourseHardwareScaleOffsetRx(float)), mkyInfoWidget, SLOT(updateHardwareScaleOffset(float)));

    // MKZ info widget
    connect(mkzInfoWidget, SIGNAL(mkyStatusRequest()), canModule, SLOT(mkzStatusRequest()));
    connect(mkzInfoWidget, SIGNAL(mkyPidKoefRequest()), canModule, SLOT(mkzPIDKoefRequest()));
    connect(mkzInfoWidget, SIGNAL(mkyUpdatePidKoef(float,float,float,float,float)), canModule, SLOT(mkzSetPIDKoef(float,float,float,float,float)));
    connect(mkzInfoWidget, SIGNAL(mkySavePidKoefInFlashRequest()), canModule, SLOT(mkzSavePIDKoefInFlash()));
    connect(mkzInfoWidget, SIGNAL(mkyVelocityCalibration()), canModule, SLOT(mkzStartVelocityCalibration()));
    connect(mkzInfoWidget, SIGNAL(sendTestManualCommand(quint8*,quint8,bool,bool)), canModule, SLOT(mkzSendExtendedCommand(quint8*,quint8,bool,bool)));
    connect(mkzInfoWidget, SIGNAL(setModeTudaSudaVUS(quint16, float)), canModule, SLOT(mkzSetModeTudaSudaVUS(quint16,float)));
    connect(mkzInfoWidget, SIGNAL(setOutputDebugingInfo(quint32)), canModule, SLOT(mkzSetOutputDebugInfo(quint32)));
    connect(mkzInfoWidget, SIGNAL(engineSwithOff()), canModule, SLOT(mkzEngineSwitchOff()));
    connect(mkzInfoWidget, SIGNAL(saveNewHardwareScale()), canModule, SLOT(mkzSaveNewHardwareScale()));
    connect(mkzInfoWidget, SIGNAL(hardwareScaleOffsetRequest()), canModule, SLOT(hardwareScaleOffsetsRequest()));
    connect(mkzInfoWidget, SIGNAL(setModeTudaSudaAR(quint16, qint16, qint16)), canModule, SLOT(mkzSetModeTudaSudaAR(quint16,qint16, qint16)));
    connect(mkzInfoWidget, SIGNAL(setModeDusTemperatureCalibration()), canModule, SLOT(mkzSetModeDusTemperatureCalibtarion()));


    connect(canModule, SIGNAL(onMKZWorkingTimeRx(quint32)), mkzInfoWidget, SLOT(updateWorkingTimeRx(quint32)));
    connect(canModule, SIGNAL(onMKZCourseVelocityRx(qint16)), mkzInfoWidget, SLOT(updateCourseVelocity(qint16)));
    connect(canModule, SIGNAL(onMKZCourseAverageVelocityRx(float)), mkzInfoWidget, SLOT(updateCourseAverageVelocity(float)));
    connect(canModule, SIGNAL(onMKZCourseUprRx(float)), mkzInfoWidget, SLOT(updateCourseUpr(float)));
    connect(canModule, SIGNAL(onMKZCourseRxPidP(float)), mkzInfoWidget, SLOT(updateCoursePID_P(float)));
    connect(canModule, SIGNAL(onMKZCourseRxPidI(float)), mkzInfoWidget, SLOT(updateCoursePID_I(float)));
    connect(canModule, SIGNAL(onMKZCourseRxPidD(float)), mkzInfoWidget, SLOT(updateCoursePID_D(float)));
    connect(canModule, SIGNAL(onMKZCourseRxPidN(float)), mkzInfoWidget, SLOT(updateCoursePID_N(float)));
    connect(canModule, SIGNAL(onMKZCourseRxTdiskr(float)), mkzInfoWidget, SLOT(updateCourseTdiskr(float)));
    connect(canModule, SIGNAL(onMKZCourseRxCurrentAngle(float)), mkzInfoWidget, SLOT(updateCurrentAngle(float)));
    connect(canModule, SIGNAL(onMKZCourseRxTestValue2(float)), mkzInfoWidget, SLOT(updateTestValue2(float)));
    connect(canModule, SIGNAL(onMKZAnswerOnExtendedCmdRX(const quint8*,quint8)), mkzInfoWidget, SIGNAL(answerOnExtendedCmdReceived(const quint8*,quint8)));
    connect(canModule, SIGNAL(onMKZCourseRxSavedValue(float)), mkzInfoWidget, SLOT(updateSavedValue(float)));    
    connect(canModule, SIGNAL(onMKZState1RX(const quint8*,quint8)), mkzInfoWidget, SLOT(updateGsState_word1(const quint8*,quint8)));
    connect(canModule, SIGNAL(onMKZTangageHardwareScaleOffsetRx(float)), mkzInfoWidget, SLOT(updateHardwareScaleOffset(float)));

    // Gs general state
    connect(gsGeneralState, SIGNAL(gs_engineOff()), canModule, SLOT(gsEngineOff()));
    connect(gsGeneralState, SIGNAL(gs_setGOT()), canModule, SLOT(gsSetGOT()));
    connect(gsGeneralState, SIGNAL(saveSamples(int)), mkyInfoWidget, SLOT(startSaveSamples(int)));
    connect(gsGeneralState, SIGNAL(saveSamples(int)), mkzInfoWidget, SLOT(startSaveSamples(int)));

    connect(canModule, SIGNAL(onGsGeneralStateReceived(const quint8*,quint8)), gsGeneralState, SLOT(updateGsState(const quint8*,quint8)));

    // Текстовые сообщения
    connect(canModule, SIGNAL(onMKYDbgTextRX(const quint8*,quint8)), mkyInfoWidget->getExtendedCmdDialog(), SLOT(processDbgText(const quint8*,quint8)));
    connect(canModule, SIGNAL(onMKZDbgTextRX(const quint8*,quint8)), mkzInfoWidget->getExtendedCmdDialog(), SLOT(processDbgText(const quint8*,quint8)));


    updateCANDeviceList();
}

MainWindow::~MainWindow()
{
    delete ui;
}


// Выбрано другое устройство для наблюдения
void MainWindow::onMainTabWidgetTabChanged(int index)
{
    if(!canModule->isOpen()) return;

    requestStateTimersSetEnable(index);  // запустить таймер
}

// Запустить/остановить таймер опроса текущего устройтсва
void MainWindow::requestStateTimersSetEnable(int selectedTab)
{
    switch(selectedTab)
    {
        case 0:  // MKY
            mkyInfoWidget->setStateRequestTimerEnable(true);
            mkzInfoWidget->setStateRequestTimerEnable(false);
        break;

        case 1:  // MKZ
            mkyInfoWidget->setStateRequestTimerEnable(false);
            mkzInfoWidget->setStateRequestTimerEnable(true);
            break;

        default:
            mkyInfoWidget->setStateRequestTimerEnable(false);
            mkzInfoWidget->setStateRequestTimerEnable(false);
            break;
    }
}



// CAN

// ===================== CAN =====================
// Получить список CAN-устройств, доступных в системе
void MainWindow::updateCANDeviceList()
{
    QStringList strListDevName = canModule->getDeviceList();
    canDeviceSelector->onDeviceListUpdated(strListDevName);
    updateCANBusList();
}

// Получить список CAN-шин выбранного устройства
void MainWindow::updateCANBusList()
{
    int deviceIndex = canDeviceSelector->getDeviceIndex();
    QStringList strListBusName = canModule->getBusNameList(deviceIndex);
    canDeviceSelector->onBusListUpdated(strListBusName);
}

// Обработка запроса на подключение/отключение модуля
void MainWindow::onCANOpenClose()
{
    if(canModule->isOpen()) // если устройство подключено - отключить
    {
        onCANDeviceConnectionChanged(false);
        canModule->closeDevice();
    }
    else
    {
        int deviceIndex = canDeviceSelector->getDeviceIndex();
        int busIndex = canDeviceSelector->getBusIndex();
        if(!canModule->openDevice(deviceIndex, busIndex))
        {
            updateCANDeviceList();
        }
    }

    bool canDeviceOpened = canModule->isOpen();
    canDeviceSelector->onConnectionStateChanged(canDeviceOpened);
    QThread::msleep(100);
    this->onCANDeviceConnectionChanged(canDeviceOpened);
}

// Не было ответа от устройства - перезагрузить его
void MainWindow::onCANModuleResetRequest()
{
    int deviceIndex = canDeviceSelector->getDeviceIndex();
    int busIndex = canDeviceSelector->getBusIndex();

    canModule->closeDevice();
    canModule->openDevice(deviceIndex, busIndex);
}



// При подключении/отключении CAN-устройства
void MainWindow::onCANDeviceConnectionChanged(bool isOpen)
{
    switch(mainTabWidget->currentIndex())
    {
        case 0: mkyInfoWidget->setStateRequestTimerEnable(isOpen); break;
        case 1: mkzInfoWidget->setStateRequestTimerEnable(isOpen); break;
    }
}
