#ifndef MKYINFOWIDGET_H
#define MKYINFOWIDGET_H

#include <QWidget>
#include "defines.h"
#include "QTimer.h"
#include "qcustomplot.h"
#include "extendedcommanddialog.h"
#include "ui_mkyinfowidget.h"

namespace Ui {
class MKYinfoWidget;
}


#define MKY_CHECK_STATE_INTERVAL            300
#define MKY_CHECK_FRESH_DATA_INTERVAL       (MKY_CHECK_STATE_INTERVAL*2)
#define MKY_REFRESH_PLOT_INTERVAL           40



// Режимы работы ГК
#define GK_MODE_ENGINE_DISABLED                     0x00
#define GK_MODE_ENGINE_OFF_BY_SPEED_PROTECTION      0x01
#define GK_MODE_ENGINE_OFF_BY_CMD                   0x02
#define GK_MODE_VUS                                 0x03
#define GK_MODE_AR                                  0x04
#define GK_MODE_CALIBRATION                         0x05
#define GK_MODE_TUDA_SUDA_VUS                       0x06
#define GK_MODE_TP                                  0x07
#define GK_MODE_VUO                                 0x08
#define GK_MODE_SELFCONTROL                         0x09
#define GK_MODE_INITIALIZE                          0x0A
#define GK_MODE_TUDA_SUDA_AR                        0x0B
#define GK_MODE_DUS_TEMP_CALIBRATION                0x0C
#define GK_MODE_HARDFAULT                           0x0D



// Коды ошибок самоконтроля
typedef enum
{
  FuncTestErrorCode_None = 0,
  FuncTestErrorCode_TestTimeout                 = 0x01,         // функциональный тест не завершился в течении 60с

  FuncTestErrorCode_CourseMoveToStartTimeout    = 0x02,         // не успел выйти на стартовую позицию
  FuncTestErrorCode_CourseArrieterTimeout       = 0x04,         // не успел выйти в арретир за отведенное время
  FuncTestErrorCode_CourseRotationTimeout       = 0x08,         // не успел сделать оборот за заданное время
  FuncTestErrorCode_CourseVusVelocityIncorrect  = 0x10,         // сделал оборот, но скорость не соответствует заданной (40 градусов вместо 20, например)

  FuncTestErrorCode_TangageMoveToStartTimeout   = 0x20,
  FuncTestErrorCode_TangageArrieterTimeout      = 0x40,
  FuncTestErrorCode_TangageRotationTimeout      = 0x80,
  FuncTestErrorCode_TangageVusVelocityIncorrect = 0x100,
} FuncTestErrorCode_t;



// Стадии функционального теста
#define FUNCTEST_INIT           0                 // инициализация режима самоконтроля
#define FUNCTEST_VUS_TEST       1                 // тест ВУС (сделать полный оборот)
#define FUNCTEST_AR_TEST        2           // тест АР (встать на заданные углы)
#define FUNCTEST_COMPLETE_SUCCESS 3         // самоконтроль завершен успешно
#define FUNCTEST_COMPLETE_WITH_ERROR 4       // самоконтроль завершен с ошибкой




class MKYinfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MKYinfoWidget(QWidget *parent = 0);
    ~MKYinfoWidget();   
    void setStateRequestTimerEnable(bool enable); // Включить/отключить отпрос состояния КУС
    void setExtendedDialogTitle(QString str) { extendedCmdDialog->setWindowTitle(str); }
    ExtendedCommandDialog *getExtendedCmdDialog() {return extendedCmdDialog;}

private:
    Ui::MKYinfoWidget *ui;
    bool timeFreshData;
    bool connectionLost;

    // Контекстное меню диаграмм
    QMenu *plotContextMenu;
    QAction *actionRescalePlot;

    // Контекстное меню Расширенной команды
    QMenu *contextMenuExtendedDialog;
    QAction *actionExtentedCommand;
    ExtendedCommandDialog *extendedCmdDialog;       // Диалог отправки расширенной команды


    // Таймер-запроса состояния КУС
    QTimer *mkyStateRequestTimer;
    QTimer *mkyCheckFreshDataTimer;      // проверка
    QTimer *plotRefreshTimer;

    QTime plotVelocityTime;
    QTime plotUprTime;
    QTime plotAngleTime;
    QTime plotDemo2Time;

    // Тестовые значения
    int testValue1Index;
    int testValue2Index;

    int numOfAngleSamples;
    int numOfVelocitySamples;

    float hardwareScaleOffset;  // смещение аппаратной шкалы относительно железной

    // Сохранение отсчетов
    int maxNumberOfSavedSamples; // кол-во отсчетов, которые нужно сохранить
    bool saveAngleSamplesEnable;
    bool saveVelocitySamplesEnable;
    int iSaveAngleSample;       // кол-во отсчетов угла
    int iSaveVelocitySample;    // кол-во отсчетов угла

    QList<float> *angleSaveSamplesList;
    QList<float> *velocitySaveSamplesList;


    void resetNewDataFlags();
    void customPlotInit();
    void updatePlotVelocity(float velocity);
    void updatePlotUpr(float upr);
    void updatePlotAngle(float angle360);
    void updatePlotDemo2(float value);

    void updateOutputDebugingInfo();

private slots:
    void showPlotContextMenu(const QPoint &pos);
    void showExtDialogContextMenu(const QPoint &pos);
    void rescalePlot();
    void setNewPidKoef();
    void refreshPlots();
    void on_checkBoxVelocityPlot_clicked();
    void on_checkBoxPlotUpr_clicked();
    void on_checkBoxPlotAngle_clicked();
    void on_checkBoxPlotDemo1_clicked();
    void on_pushButtonSetModeTudaSudaVUS_clicked();
    void on_pushButtonSetModeTudaSudaAR_clicked();


    void on_btnCopyVelocitySamples_clicked();

    void on_btnCopyAngleSamples_clicked();

signals:
    void mkyStatusRequest();
    void mkyPidKoefRequest();
    void mkyUpdatePidKoef(float p, float i, float d, float n, float t);
    void mkySavePidKoefInFlashRequest();
    void mkyVelocityCalibration();

    void sendTestManualCommand(quint8 *pBuf, quint8 len, bool extendedCmd, bool needAnswer);
    void answerOnExtendedCmdReceived(const quint8 *pBuf, quint8 len);
    void setModeTudaSudaVUS(quint16 period, float velocity);
    void setModeTudaSudaAR(quint16 period, qint16 angle1, qint16 angle2);
    void setOutputDebugingInfo(quint32 word);
    void engineSwithOff();
    void saveNewHardwareScale();
    void hardwareScaleOffsetRequest();
    void setModeDusTemperatureCalibration();

public slots:
    void onCheckFreshDataTimeout();
    void updateWorkingTimeRx(quint32 totalms);
    void updateCourseVelocity(qint16 dusAmplitude);
    void updateCourseAverageVelocity(float averVel);
    void updateCourseUpr(float upr);
    void updateCoursePID_P(float P);
    void updateCoursePID_I(float I);
    void updateCoursePID_D(float D);
    void updateCoursePID_N(float N);
    void updateCourseTdiskr(float t);
    void updateCurrentAngle(float angle360);
    void updateTestValue2(float value);
    void updateSavedValue(float value);
    void updateGsState_word1(const quint8 *pBuf, quint8 len);
    void updateHardwareScaleOffset(float offset);

    void startSaveSamples(int count);
};

#endif // MKYINFOWIDGET_H
