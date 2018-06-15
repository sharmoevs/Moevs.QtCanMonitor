#include "gsgeneralstate.h"
#include "ui_gsgeneralstate.h"
#include "defines.h"

GsGeneralState::GsGeneralState(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GsGeneralState)
{
    ui->setupUi(this);
    resetFreshDataFlags();

    ui->btnGsEngineOff->setStyleSheet("color: rgb(250, 0, 0)");

    checkFreshDataTimer = new QTimer();
    checkFreshDataTimer->start(CHECK_FRESH_DATA_TIMER_INTERVAL);

    // Таймер запроса состояния КУС
    connect(checkFreshDataTimer, SIGNAL(timeout()), this, SLOT(checkFreshData()));

    // Выключить двигатели
    connect(ui->btnGsEngineOff, SIGNAL(clicked()), this, SIGNAL(gs_engineOff()));

    // Установить ГОТ = 1
    connect(ui->btnSetGOT, SIGNAL(clicked()), this, SIGNAL(gs_setGOT()));
}

GsGeneralState::~GsGeneralState()
{
    delete ui;
}

// Сбросить флаги свежести данных
void GsGeneralState::resetFreshDataFlags()
{
    gsStateFreshData = false;
}

// Проверка наличия новых данных
void GsGeneralState::checkFreshData()
{
    if(!gsStateFreshData)
    {
        ui->labelGsState->setStyleSheet(TIMEOUT_SETTING_STYLE);
    }

    resetFreshDataFlags();
}


// Второе слово состояния ГС
void GsGeneralState::updateGsState(const quint8 *pBuf, quint8 len)
{
    if(len != 2) return;
    gsStateFreshData = true;
    GSFailure_t failure = (GSFailure_t)pBuf[1];

    if(failure == GgFailure_None)
    {
        ui->labelGsState->setStyleSheet(ACTIVE_SETTING_STYLE);
        ui->labelGsState->setText("Нормальная работа");
    }
    else
    {
        QString errorDescription = "";
        int errCount = 0;
        if(failure & GgFailure_FuncTestError)
        {
            if(errCount++ > 0) errorDescription += "\r\n";
            errorDescription += "Функциональный тест завершен с ошибкой";
        }
        if(failure & GgFailure_CourseSpeedProtect)
        {
            if(errCount++ > 0) errorDescription += "\r\n";
            errorDescription += "Сработала защита по скорости курсового контура";
        }
        if(failure & GgFailure_TangageSpeedProtect)
        {
            if(errCount++ > 0) errorDescription += "\r\n";
            errorDescription += "Сработала защита по скорости тангажного контура";
        }
        if(failure & GgFailure_StartUpCalibration)
        {
            if(errCount++ > 0) errorDescription += "\r\n";
            errorDescription += "Калибровка по включению питания не завершена за отведенное время";
        }
        ui->labelGsState->setStyleSheet(ERROR_SETTING_STYLE);
        ui->labelGsState->setText(errorDescription);
    }
}

// Сохранить количество отсчетов
void GsGeneralState::on_btnSaveSamples_clicked()
{
    emit saveSamples(ui->spinBoxNumberOfSamples->value());
}
