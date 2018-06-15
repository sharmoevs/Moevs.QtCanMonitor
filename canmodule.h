#ifndef CANMODULE_H
#define CANMODULE_H

#include <QObject>
#include <module_can_ixxat.h>
#include <QTimer>
#include <QDebug>
#include "defines.h"


// CAN-идентификаторы кадра для приема
#define CAN_FRAME_ID_MKY_RX               0x70
#define CAN_FRAME_ID_MKZ_RX               0x71
#define CAN_ID_DBGTEXT_MKY_RX             0x90  // отладочный текст от МКУ
#define CAN_ID_DBGTEXT_MKZ_RX             0x91  // отладочный текст от МКZ

// CAN-идентификаторы кадра для передачи
#define CAN_FRAME_ID_MKY                  0x80
#define CAN_FRAME_ID_MKZ                  0x81




//  =========================== MK Y ===========================

// Идентификаторы команд MKY
#define MKY_STATE_REQUEST                 0x01    // запрос состояния
#define MKY_PID_KOEF_REQUEST              0x02    // запрос коэффициентов ПИД-регулятора
#define MKY_UPDATE_PID_KOEF_P             0x03    // установить новые значения ПИД-коэф.
#define MKY_UPDATE_PID_KOEF_I             0x04    // установить новые значения ПИД-коэф.
#define MKY_UPDATE_PID_KOEF_D             0x05    // установить новые значения ПИД-коэф.
#define MKY_UPDATE_PID_KOEF_N             0x51    // установить новые значения ПИД-коэф.
#define MKY_UPDATE_T_DISKR                0x06    // установить новое значение периода дискретизации
#define MKY_SAVE_PID_KOEF_IN_FLASH        0x07    // сохранить коэффициенты ПИД-регулятора во флешь-памяти
#define MKY_VELOCITY_CALIBRATION          0x08    // калибровка по скорости
#define MKY_SEND_EXTENDED_COMMAND         0xF0    // расширенная команда
#define MKY_SET_MODE_TUDA_SUDA_VUS        0x09    // движение туда-сюда ВУС
#define MKY_SET_OUTPUT_DEBUGING_INFO      0x0A    // задать вывод отладочной информации
#define MKY_ENGINE_SWITCH_OFF             0x0B    // выключить двигатели
#define MKY_SAVE_NEW_HARDWARE_SCALE_COURSE  0x0C    // сохранить новую аппаратную шкалу курсового контура
#define MKY_SAVE_NEW_HARDWARE_SCALE_TANGAGE 0x0D    // сохранить новую аппаратную шкалу тангажного контура
#define MKY_HARDWARE_SCALE_OFFSETS_REQUEST  0x0E    // запрос смещений аппаратной шкалы курсового и тангажного  контура относительно железной
#define MKY_SET_MODE_TUDA_SUDA_AR         0x0F    // движение туда-сюда АР
#define MKY_SET_MODE_DUS_TEMP_CALIBRATION 0x12    // калибровка ДУС по температуре


// Идентификаторы ответов от MKY
#define MKY_ANSWER_STATE                  0x0E    // состояние дальномера
#define MKY_ANSWER_TIME                   0x50    // время работы
#define MKY_ANSWER_COURSE_PID_P           0x53
#define MKY_ANSWER_COURSE_PID_I           0x54
#define MKY_ANSWER_COURSE_PID_D           0x55
#define MKY_ANSWER_COURSE_PID_N           0x59
#define MKY_ANSWER_COURSE_T_DISKR         0x56
#define MKY_RX_EXTENDED_ANSWER            0xF0    // ответ на расширенную команду
#define MKY_ANSWER_COURSE_VELOCITY        0x51    // угловая скорость по курсу
#define MKY_ANSWER_COURSE_UPR             0x52    // управляющий сигнал по курсу
#define MKY_CURRENT_ANGLE                 0x0A    // текущая скорость
#define MKY_TEST_VALUE_1                  0x0B
#define MKY_SAVED_VALUE                   0x0C
#define MKY_COURSE_HARDWARE_SCALE_OFFSET  0x0D     // смещение аппаратной шкалы курсового контура относительно железной
#define MKY_TANGAGE_HARDWARE_SCALE_OFFSET 0x0F     // смещение аппаратной шкалы тангажного контура относительно железной
#define MKY_ANGLE_SAMPLE                  0x60    // отсчет датчика угла


//  =========================== MK Z ===========================

// Идентификаторы команд MKZ
#define MKZ_STATE_REQUEST                 0x01    // запрос состояния
#define MKZ_PID_KOEF_REQUEST              0x02    // запрос коэффициентов ПИД-регулятора
#define MKZ_UPDATE_PID_KOEF_P             0x03    // установить новые значения ПИД-коэф.
#define MKZ_UPDATE_PID_KOEF_I             0x04    // установить новые значения ПИД-коэф.
#define MKZ_UPDATE_PID_KOEF_D             0x05    // установить новые значения ПИД-коэф.
#define MKZ_UPDATE_PID_KOEF_N             0x51    // установить новые значения ПИД-коэф.
#define MKZ_UPDATE_T_DISKR                0x06    // установить новое значение периода дискретизации
#define MKZ_SAVE_PID_KOEF_IN_FLASH        0x07    // сохранить коэффициенты ПИД-регулятора во флешь-памяти
#define MKZ_VELOCITY_CALIBRATION          0x08    // калибровка по скорости
#define MKZ_SEND_EXTENDED_COMMAND         0xF0    // расширенная команда
#define MKZ_SET_MODE_TUDA_SUDA            0x09    // движение туда-сюда
#define MKZ_SET_OUTPUT_DEBUGING_INFO      0x0A    // задать вывод отладочной информации
#define MKZ_ENGINE_SWITCH_OFF             0x0B    // выключить двигатели
#define MKZ_SET_MODE_DUS_TEMP_CALIBRATION 0x12    // калибровка ДУС по температуре

// Идентификаторы ответов от MKZ
#define MKZ_ANSWER_STATE                  0x0E    // первое слово состояни
#define MKZ_ANSWER_TIME                   0x50    // время работы
#define MKZ_ANSWER_COURSE_PID_P           0x53
#define MKZ_ANSWER_COURSE_PID_I           0x54
#define MKZ_ANSWER_COURSE_PID_D           0x55
#define MKZ_ANSWER_COURSE_PID_N           0x59
#define MKZ_ANSWER_COURSE_T_DISKR         0x56
#define MKZ_RX_EXTENDED_ANSWER            0xF0    // ответ на расширенную команду
#define MKZ_ANSWER_COURSE_VELOCITY        0x51    // угловая скорость по курсу
#define MKZ_ANSWER_COURSE_UPR             0x52    // управляющий сигнал по курсу
#define MKZ_CURRENT_ANGLE                 0x0A    // текущая скорость
#define MKZ_TEST_VALUE_1                  0x0B
#define MKZ_SAVED_VALUE                   0x0C



//  =========================== GS general ===========================
// Запросы
#define GS_ENGINE_OFF                     0x10    // выключить двигатели
#define GS_SET_GOT_BIT                    0x11    // установить бит ГОТ в 1. Для разрешения управления после ошибки

// Ответы
#define GS_ANSWER_CURENT_STATE            0xE2    // состояние ГС (общее, включая тангажный и курсовой контур)







class CanModule : public QObject
{
    Q_OBJECT
public:
    explicit CanModule(QObject *parent = 0);

    QStringList getDeviceList() { return module_CAN_IXXAT->getListDev(); }
    QStringList getBusNameList(int deviceIndex) { return module_CAN_IXXAT->getListBusDev(deviceIndex); }
    void closeDevice();
    bool openDevice(int deviceIndex, int busIndex);
    bool isOpen() { return isOpened; }



private:
    void sendUserCommand(quint8 *pBuf, quint8 len,bool extendedCmd, bool needAnswer, quint8 canFrameID, quint8 extendedCmdID);

private:
    bool isOpened;
    bool receivedCanFrame;
    Module_CAN_IXXAT *module_CAN_IXXAT;
    QTimer *checkConnectionTimer;


    // General
    void _statusRequest(quint8 canId);
    void _PIDKoefRequest(quint8 canId);
    void _setPIDKoef(quint8 canId, float p, float i, float d, float n, float tDiskr);
    void _savePIDKoefInFlash(quint8 canId);
    void _startVelocityCalibration(quint8 canId);
    void _setModeTudaSudaVUS(quint8 canId, quint16 period, float velocity);
    void _setModeTudaSudaAR(quint8 canId, quint16 period, qint16 angle1, qint16 angle2);
    void _setOutputDebugInfo(quint8 canId, quint32 word);
    void _engineSwitchOff(quint8 canId);


private slots:
    void connectionTimerTimeout();  // проверка соединения
    void onRxCANFrame(bool self, bool rtr, quint16 id,const quint8 *pData, quint8 dlc);

signals:
    void resetModuleRequest();  // запрос на сброс модуля CAN
    void frameReceived();             // принят пакет

    // MK Y
    void onMKYState1RX(const quint8* pData, quint8 len);
    void onMKYWorkingTimeRx(quint32 totalMs);
    void onMKYCourseVelocityRx(qint16 dusCode);
    void onMKYCourseAverageVelocityRx(float averVel);
    void onMKYCourseUprRx(float upr);
    void onMKYCourseRxPidP(float P);
    void onMKYCourseRxPidI(float I);
    void onMKYCourseRxPidD(float D);
    void onMKYCourseRxPidN(float N);
    void onMKYCourseRxTdiskr(float T);
    void onMKYCourseRxCurrentAngle(float value);
    void onMKYCourseRxTestValue2(float value);
    void onMKYAnswerOnExtendedCmdRX(const quint8* pData, quint8 len);
    void onMKYCourseRxSavedValue(float value);
    void onMKYCourseHardwareScaleOffsetRx(float value);
    void onMKYCourseAngleAndVelocitySampleRx(quint32 angleCode, qint16 dusAmplitude);

    // MK Z
    void onMKZState1RX(const quint8* pData, quint8 len);
    void onMKZWorkingTimeRx(quint32 totalMs);
    void onMKZCourseVelocityRx(qint16 dusCode);
    void onMKZCourseAverageVelocityRx(float averVel);
    void onMKZCourseUprRx(float upr);
    void onMKZCourseRxPidP(float P);
    void onMKZCourseRxPidI(float I);
    void onMKZCourseRxPidD(float D);
    void onMKZCourseRxPidN(float N);
    void onMKZCourseRxTdiskr(float T);
    void onMKZCourseRxCurrentAngle(float value);
    void onMKZCourseRxTestValue2(float value);
    void onMKZAnswerOnExtendedCmdRX(const quint8* pData, quint8 len);
    void onMKZCourseRxSavedValue(float value);
    void onMKZTangageHardwareScaleOffsetRx(float value);

    // GS general
    void onGsGeneralStateReceived(const quint8* pData, quint8 len);

    // MKY text
    void onMKYDbgTextRX(const quint8* pData, quint8 len);

    // MKZ text
    void onMKZDbgTextRX(const quint8* pData, quint8 len);


public slots:
    void hardwareScaleOffsetsRequest();  // запрос

    // Команды MKY
    void mkyStatusRequest();
    void mkyPIDKoefRequest();
    void mkySetPIDKoef(float p, float i, float d, float n, float tDiskr);
    void mkySavePIDKoefInFlash();
    void mkyStartVelocityCalibration();
    void mkySendExtendedCommand(quint8 *pBuf, quint8 len, bool extendedCmd, bool needAnswer);
    void mkySetModeTudaSudaVUS(quint16 period, float velocity);
    void mkySetOutputDebugInfo(quint32 cmd);
    void mkyEngineSwitchOff();
    void mkySaveNewHardwareScale();
    void mkySetModeTudaSudaAR(quint16 period, qint16 angle1, qint16 angle2);
    void mkySetModeDusTemperatureCalibtarion();

    // Команды MKZ
    void mkzStatusRequest();
    void mkzPIDKoefRequest();
    void mkzSetPIDKoef(float p, float i, float d, float n, float tDiskr);
    void mkzSavePIDKoefInFlash();
    void mkzStartVelocityCalibration();
    void mkzSendExtendedCommand(quint8 *pBuf, quint8 len, bool extendedCmd, bool needAnswer);
    void mkzSetModeTudaSudaVUS(quint16 period, float velocity);
    void mkzSetOutputDebugInfo(quint32 cmd);
    void mkzEngineSwitchOff();
    void mkzSaveNewHardwareScale();
    void mkzSetModeTudaSudaAR(quint16 period, qint16 angle1, qint16 angle2);
    void mkzSetModeDusTemperatureCalibtarion();


    // GS General
    void gsEngineOff();
    void gsSetGOT();
};

#endif // CANMODULE_H
