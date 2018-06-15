#include "canmodule.h"

CanModule::CanModule(QObject *parent) : QObject(parent)
{
    isOpened = false;
    receivedCanFrame = false;
    module_CAN_IXXAT = new Module_CAN_IXXAT(this);
    checkConnectionTimer = new QTimer(this);



    connect(checkConnectionTimer, SIGNAL(timeout()), this, SLOT(connectionTimerTimeout()));
    checkConnectionTimer->start(2000);


    // moduleCAN
    QObject::connect(module_CAN_IXXAT,  SIGNAL(receiveData(bool,bool,quint16,const quint8*,quint8)),
                     this, SLOT(onRxCANFrame(bool,bool,quint16,const quint8*,quint8)));
}



// Проверка соединения
void CanModule::connectionTimerTimeout()
{
    if(isOpened)
    {
        if(!receivedCanFrame)
        {
           qDebug() << "can module reseted\n";
           emit resetModuleRequest();
        }
        else receivedCanFrame = false;
    }
}

// Отключиться от CAN-шины
void CanModule::closeDevice()
{
    module_CAN_IXXAT->closeDev();
    isOpened = false;
}

// Подключиться к CAN-шине
bool CanModule::openDevice(int deviceIndex, int busIndex)
{
    isOpened = module_CAN_IXXAT->openDev(deviceIndex, busIndex);
    return isOpened;
}

// Принят CAN-пакет
void CanModule::onRxCANFrame(bool self, bool rtr, quint16 id, const quint8 *pData, quint8 dlc)
{
    if(self || rtr) return;
    receivedCanFrame = true;
    emit frameReceived();

    // Сообщение от MKY
    if(id == CAN_FRAME_ID_MKY_RX)
    {
        switch(pData[0])
        {        
            case MKY_ANSWER_STATE:
            {
                emit onMKYState1RX(pData, dlc);
            }
            break;

            case MKY_ANSWER_TIME:
            {
                quint32 totalMsAtStart = pData[1]<<24 | pData[2]<<16 | pData[3]<<8 | pData[4];
                emit onMKYWorkingTimeRx(totalMsAtStart);
            }
            break;

            case MKY_ANSWER_COURSE_VELOCITY:
            {
                if(dlc == 3)
                {
                    qint16 dusCode = pData[1]<<8 | pData[2];
                    emit onMKYCourseVelocityRx(dusCode);
                }
                else if(dlc == 7)
                {
                    qint16 dusCode = pData[1]<<8 | pData[2];
                    float averVelocity = *((float*)(pData+3));
                    emit onMKYCourseVelocityRx(dusCode);
                    emit onMKYCourseAverageVelocityRx(averVelocity);
                }
            }
            break;

            case MKY_ANSWER_COURSE_UPR:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float upr = *pFloat;
                emit onMKYCourseUprRx(upr);
            }
            break;

            case MKY_ANSWER_COURSE_PID_P:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float koefP = *pFloat;
                emit onMKYCourseRxPidP(koefP);
            }
            break;

            case MKY_ANSWER_COURSE_PID_I:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float koefI = *pFloat;
                emit onMKYCourseRxPidI(koefI);
            }
            break;

            case MKY_ANSWER_COURSE_PID_D:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float koefD = *pFloat;
                emit onMKYCourseRxPidD(koefD);
            }
            break;

            case MKY_ANSWER_COURSE_PID_N:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float koefN = *pFloat;
                emit onMKYCourseRxPidN(koefN);
            }
            break;


            case MKY_ANSWER_COURSE_T_DISKR:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float Tdiskr = *pFloat;
                emit onMKYCourseRxTdiskr(Tdiskr);
            }
            break;

            case MKY_CURRENT_ANGLE:
            {
                if(dlc != 5) return;
                quint32 angleCode = pData[1]<<24 | pData[2]<<16 | pData[3]<<8 | pData[4];
                float angle = (double)angleCode*360/0x7FFFF;
                emit onMKYCourseRxCurrentAngle(angle);

                //qDebug() << angleCode;
            }
            break;

            case MKY_TEST_VALUE_1:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float value = *pFloat;
                emit onMKYCourseRxTestValue2(value);
            }
            break;

            case MKY_RX_EXTENDED_ANSWER: emit onMKYAnswerOnExtendedCmdRX(pData, dlc); break;

            case MKY_SAVED_VALUE:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float value = *pFloat;
                emit onMKYCourseRxSavedValue(value);
            }
            break;

            case MKY_COURSE_HARDWARE_SCALE_OFFSET:
            {
                if(dlc != 5) return;
                quint32 angleCode = pData[1]<<24 | pData[2]<<16 | pData[3]<<8 | pData[4];
                float angle = (double)angleCode*360/0x7FFFF;
                emit onMKYCourseHardwareScaleOffsetRx(angle);
            }
            break;

            case MKY_TANGAGE_HARDWARE_SCALE_OFFSET:
            {
                if(dlc != 5) return;
                quint32 angleCode = pData[1]<<24 | pData[2]<<16 | pData[3]<<8 | pData[4];
                float angle = (double)angleCode*360/0x3FFFF;
                emit onMKZTangageHardwareScaleOffsetRx(angle);
            }
            break;


            // Общее состояние двух контуров
            case GS_ANSWER_CURENT_STATE:
            {
                emit onGsGeneralStateReceived(pData, dlc);
            }
            break;

            // Отсчет с датчика угла
            case MKY_ANGLE_SAMPLE:
            {
                //quint32 angleCode = pData[1]<<16 | pData[2]<<8 | pData[3];
                //qint16  dusAmplitude = (qint16)(pData[4]<<8 | pData[5]);
                //emit onMKYCourseAngleAndVelocitySampleRx(angleCode, dusAmplitude);
                //qDebug() << angle << endl << velocity;
            }
            break;



        }
    }

    // Сообщение от MKZ
    if(id == CAN_FRAME_ID_MKZ_RX)
    {
        switch(pData[0])
        {
            case MKZ_ANSWER_STATE:
            {
                emit onMKZState1RX(pData, dlc);
            }
            break;

            case MKZ_ANSWER_TIME:
            {
                quint32 totalMsAtStart = pData[1]<<24 | pData[2]<<16 | pData[3]<<8 | pData[4];
                emit onMKZWorkingTimeRx(totalMsAtStart);
            }
            break;

            case MKZ_ANSWER_COURSE_VELOCITY:
            {
                if(dlc == 3)
                {
                    qint16 dusCode = pData[1]<<8 | pData[2];
                    emit onMKZCourseVelocityRx(dusCode);
                }
                else if(dlc == 7)
                {
                    qint16 dusCode = pData[1]<<8 | pData[2];
                    float averVelocity = *((float*)(pData+3));
                    emit onMKZCourseVelocityRx(dusCode);
                    emit onMKZCourseAverageVelocityRx(averVelocity);
                }
            }
            break;

            case MKZ_ANSWER_COURSE_UPR:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float upr = *pFloat;
                emit onMKZCourseUprRx(upr);
            }
            break;

            case MKZ_ANSWER_COURSE_PID_P:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float koefP = *pFloat;
                emit onMKZCourseRxPidP(koefP);
            }
            break;

            case MKZ_ANSWER_COURSE_PID_I:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float koefI = *pFloat;
                emit onMKZCourseRxPidI(koefI);
            }
            break;

            case MKZ_ANSWER_COURSE_PID_D:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float koefD = *pFloat;
                emit onMKZCourseRxPidD(koefD);
            }
            break;

            case MKZ_ANSWER_COURSE_PID_N:
            {
                //if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float koefN = *pFloat;
                emit onMKZCourseRxPidN(koefN);
            }
            break;


            case MKZ_ANSWER_COURSE_T_DISKR:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float Tdiskr = *pFloat;
                emit onMKZCourseRxTdiskr(Tdiskr);
            }
            break;

            case MKZ_CURRENT_ANGLE:
            {
  /*
            if(dlc != 3) return;
                quint32 angle = pData[1]<<8 | pData[2];
                emit onMKZCourseRxCurrentAngle(angle);
*/
            /*
                if(dlc != 3) return;
                quint32 angleCode = pData[1]<<8 | pData[2];
                float angle = (double)angleCode*360/0xFFFF;
                emit onMKZCourseRxCurrentAngle(angle);
                */

                if(dlc != 5) return;
                quint32 angleCode = pData[1]<<24 | pData[2]<<16 | pData[3]<<8 | pData[4];
                float angle = (double)angleCode*360/0x3FFFF;
                emit onMKZCourseRxCurrentAngle(angle);
            }
            break;

            case MKZ_TEST_VALUE_1:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float value = *pFloat;
                emit onMKZCourseRxTestValue2(value);
            }
            break;

            case MKZ_RX_EXTENDED_ANSWER: emit onMKZAnswerOnExtendedCmdRX(pData, dlc); break;

            case MKZ_SAVED_VALUE:
            {
                if(dlc != 5) return;
                float *pFloat = (float*)(pData+1);
                float value = *pFloat;
                emit onMKZCourseRxSavedValue(value);
            }
            break;
        }
    }


    // Отладочный текст от MKY
    if(id == CAN_ID_DBGTEXT_MKY_RX)
    {
        //QString s ="";
        //for(int i=0; i<dlc; i++) s += QString::number(pData[i], 16) + " ";
        //qDebug() << "MKY TEXT " + s;

        emit onMKYDbgTextRX(pData, dlc);
    }

    if(id == CAN_ID_DBGTEXT_MKZ_RX)
    {
        //QString s ="";
        //for(int i=0; i<dlc; i++) s += QString::number(pData[i], 16) + " ";
        //qDebug() << "MKZ TEXT " + s;

        emit onMKZDbgTextRX(pData, dlc);
    }

}






// Расширенная команда. Обобщенная функция для различных девайсов
void CanModule::sendUserCommand(quint8 *pBuf,           // буфер с данными
                                quint8 len,             // длина передаваемых данных
                                bool extendedCmd,       // если false, то данные передаются как есть,
                                                        // обычными пакетами, пачками по 8 байт
                                                        // Если true, то данные передаются в расширенной команде,
                                                        // имеющий свой идентификатор
                                bool needAnswer,        // необходим ответ на расширенную команду от подчиненного
                                quint8 canFrameID,      // идентификатор пакета CAN
                                quint8 extendedCmdID)   // идентификатор расширенной команды
{
    if(!extendedCmd)    // обычная команда
    {
        quint8 frameLen;
        quint8 *p = pBuf;
        while(len > 0)
        {
           if(len > 8) frameLen = 8;
           else frameLen = len;

           module_CAN_IXXAT->transmitData(canFrameID, false, p, frameLen);
           p+=frameLen;
           len -= frameLen;
        }
    }
    else                // расширенная команда
    {
        quint8 cmd[8];
        quint8 needToSend = 0;          // сколько байт осталось отправить
        quint8 sendedBytes = 0;         // кол-во отправленных байт данных
        quint8 dataBytesInFrame = 0;    // кол-во байт в передаваемом пакете
        int indx = 0;                   // индекс

        cmd[0] = extendedCmdID;

        while(sendedBytes != len)
        {
           needToSend = len - sendedBytes;
           if(needToSend <= EXTCMD_MAX_DATA_BYTES) // если оставшиеся данные влезают в один пакет - то он последний
           {
               cmd[1] = needAnswer ? (EXTCMD_END_MSG | 1<<7) : EXTCMD_END_MSG;
           }
           else
           {
               if(sendedBytes == 0) cmd[1] = EXTCMD_START_MSG;
               else cmd[1] = EXTCMD_CONT_MSG;
           }

           dataBytesInFrame = needToSend <= EXTCMD_MAX_DATA_BYTES ? needToSend : EXTCMD_MAX_DATA_BYTES;
           sendedBytes += dataBytesInFrame;

           for(int i=0; i<dataBytesInFrame; i++)
           {
              cmd[i+2] = pBuf[indx++];
           }
           module_CAN_IXXAT->transmitData(canFrameID, false, cmd, dataBytesInFrame+2); // +2 байта заголовка
        }
    }
}




// ================================== Общее ==================================

// Запросить статус
void CanModule::_statusRequest(quint8 canId)
{
    quint8 pData[] = { MKY_STATE_REQUEST };
    module_CAN_IXXAT->transmitData(canId, false, pData, sizeof(pData));
}

// Запросить значения ПИД-регулятора
void CanModule::_PIDKoefRequest(quint8 canId)
{
    quint8 pData[] = { MKY_PID_KOEF_REQUEST };
    module_CAN_IXXAT->transmitData(canId, false, pData, sizeof(pData));
}

// Установить новые значения коэффициентов ПИД-регулятора
void CanModule::_setPIDKoef(quint8 canId, float p, float i, float d, float n, float tDiskr)
{
    quint8 *pointer = (quint8*)&p;
    quint8 pDataP[] =
    {
        MKY_UPDATE_PID_KOEF_P,
        pointer[0],
        pointer[1],
        pointer[2],
        pointer[3]
    };
    module_CAN_IXXAT->transmitData(canId, false, pDataP, sizeof(pDataP));


    pointer = (quint8*)&i;
    quint8 pDataI[] =
    {
         MKY_UPDATE_PID_KOEF_I,
         pointer[0],
         pointer[1],
         pointer[2],
         pointer[3]
    };
    module_CAN_IXXAT->transmitData(canId, false, pDataI, sizeof(pDataI));


    pointer = (quint8*)&d;
    quint8 pDataD[] =
    {
         MKY_UPDATE_PID_KOEF_D,
         pointer[0],
         pointer[1],
         pointer[2],
         pointer[3]
    };
    module_CAN_IXXAT->transmitData(canId, false, pDataD, sizeof(pDataD));

    pointer = (quint8*)&n;
    quint8 pDataN[] =
    {
         MKY_UPDATE_PID_KOEF_N,
         pointer[0],
         pointer[1],
         pointer[2],
         pointer[3]
    };
    module_CAN_IXXAT->transmitData(canId, false, pDataN, sizeof(pDataN));


    pointer = (quint8*)&tDiskr;
    quint8 pDataT[] =
    {
         MKY_UPDATE_T_DISKR,
         pointer[0],
         pointer[1],
         pointer[2],
         pointer[3]
    };
    module_CAN_IXXAT->transmitData(canId, false, pDataT, sizeof(pDataT));
}


// Сохранить настройки во флешь-памяти
void CanModule::_savePIDKoefInFlash(quint8 canId)
{
    quint8 pData[] = { MKY_SAVE_PID_KOEF_IN_FLASH };
    module_CAN_IXXAT->transmitData(canId, false, pData, sizeof(pData));
}

// Запустить калибровку по скорости
void CanModule::_startVelocityCalibration(quint8 canId)
{
    quint8 pData[] = { MKY_VELOCITY_CALIBRATION };
    module_CAN_IXXAT->transmitData(canId, false, pData, sizeof(pData));
}

// Установить режим работы туда-сюда ВУС
void CanModule::_setModeTudaSudaVUS(quint8 canId, quint16 period, float velocity)
{
    quint8 *pointer = (quint8*)&velocity;
    quint8 pData[] =
    {
        MKY_SET_MODE_TUDA_SUDA_VUS,
        pointer[0],
        pointer[1],
        pointer[2],
        pointer[3],
        period,
        period>>8
    };
    module_CAN_IXXAT->transmitData(canId, false, pData, sizeof(pData));
}

// Установить режим работы туда-сюда АР
void CanModule::_setModeTudaSudaAR(quint8 canId, quint16 period, qint16 angle1, qint16 angle2)
{
    quint8 pData[] =
    {
        MKY_SET_MODE_TUDA_SUDA_AR,
        period>>8,
        period,
        angle1>>8,
        angle1,
        angle2>>8,
        angle2
    };
    module_CAN_IXXAT->transmitData(canId, false, pData, sizeof(pData));
}


// Установить параметры, выводимые для отладки
void CanModule::_setOutputDebugInfo(quint8 canId, quint32 word)
{
    quint8 pData[] =
    {
        MKY_SET_OUTPUT_DEBUGING_INFO,
        word,
        word>>8,
        word>>16,
        word>>24
    };

    module_CAN_IXXAT->transmitData(canId, false, pData, sizeof(pData));
}


// Выключение двигателей
void CanModule::_engineSwitchOff(quint8 canId)
{
    quint8 pData[] = { MKY_ENGINE_SWITCH_OFF };
    module_CAN_IXXAT->transmitData(canId, false, pData, sizeof(pData));
}









// ================================== MKY ==================================


// Запросить статус КУС
void CanModule::mkyStatusRequest()
{
    _statusRequest(CAN_FRAME_ID_MKY);
    //quint8 pData[] = { MKY_STATE_REQUEST };
    //module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
}

// Запросить значения ПИД-регулятора
void CanModule::mkyPIDKoefRequest()
{
    _PIDKoefRequest(CAN_FRAME_ID_MKY);
    //quint8 pData[] = { MKY_PID_KOEF_REQUEST };
    //module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
}

// Установить новые значения коэффициентов ПИД-регулятора
void CanModule::mkySetPIDKoef(float p, float i, float d, float n, float tDiskr)
{
    _setPIDKoef(CAN_FRAME_ID_MKY, p,i,d,n,tDiskr);
    /*
    quint8 *pointer = (quint8*)&p;
    quint8 pDataP[] =
    {
        MKY_UPDATE_PID_KOEF_P,
        pointer[0],
        pointer[1],
        pointer[2],
        pointer[3]
    };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pDataP, sizeof(pDataP));


    pointer = (quint8*)&i;
    quint8 pDataI[] =
    {
         MKY_UPDATE_PID_KOEF_I,
         pointer[0],
         pointer[1],
         pointer[2],
         pointer[3]
    };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pDataI, sizeof(pDataI));


    pointer = (quint8*)&d;
    quint8 pDataD[] =
    {
         MKY_UPDATE_PID_KOEF_D,
         pointer[0],
         pointer[1],
         pointer[2],
         pointer[3]
    };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pDataD, sizeof(pDataD));

    pointer = (quint8*)&n;
    quint8 pDataN[] =
    {
         MKY_UPDATE_PID_KOEF_N,
         pointer[0],
         pointer[1],
         pointer[2],
         pointer[3]
    };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pDataN, sizeof(pDataN));


    pointer = (quint8*)&tDiskr;
    quint8 pDataT[] =
    {
         MKY_UPDATE_T_DISKR,
         pointer[0],
         pointer[1],
         pointer[2],
         pointer[3]
    };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pDataT, sizeof(pDataT));
*/
}

// Сохранить настройки во флешь-памяти
void CanModule::mkySavePIDKoefInFlash()
{
    _savePIDKoefInFlash(CAN_FRAME_ID_MKY);
    //quint8 pData[] = { MKY_SAVE_PID_KOEF_IN_FLASH };
    //module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
}

// Запустить калибровку по скорости
void CanModule::mkyStartVelocityCalibration()
{
    _startVelocityCalibration(CAN_FRAME_ID_MKY);
    //quint8 pData[] = { MKY_VELOCITY_CALIBRATION };
    //module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
}

// Отпраивть рассширенную команду КУС
void CanModule::mkySendExtendedCommand(quint8 *pBuf, quint8 len, bool extendedCmd, bool needAnswer)
{
    sendUserCommand(pBuf, len, extendedCmd, needAnswer, CAN_FRAME_ID_MKY, MKY_SEND_EXTENDED_COMMAND);
}

// Установить режим работы туда-сюда ВУС
void CanModule::mkySetModeTudaSudaVUS(quint16 period, float velocity)
{
    _setModeTudaSudaVUS(CAN_FRAME_ID_MKY, period, velocity);
    /*
    quint8 *pointer = (quint8*)&velocity;
    quint8 pData[] =
    {
        MKY_SET_MODE_TUDA_SUDA,
        pointer[0],
        pointer[1],
        pointer[2],
        pointer[3],
        period,
        period>>8
    };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
*/
}

// Установить режим работы туда-сюда АР
void CanModule::mkySetModeTudaSudaAR(quint16 period, qint16 angle1, qint16 angle2)
{
    _setModeTudaSudaAR(CAN_FRAME_ID_MKY, period, angle1, angle2);
}

// Установить параметры, выводимые для отладки
void CanModule::mkySetOutputDebugInfo(quint32 word)
{
    _setOutputDebugInfo(CAN_FRAME_ID_MKY, word);
    /*
    quint8 pData[] =
    {
        MKY_SET_OUTPUT_DEBUGING_INFO,
        word,
        word>>8,
        word>>16,
        word>>24
    };

    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
*/
}

// Выключение двигателей
void CanModule::mkyEngineSwitchOff()
{
    _engineSwitchOff(CAN_FRAME_ID_MKY);
    //quint8 pData[] = { MKY_ENGINE_SWITCH_OFF };
    //module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
}


// Сохранить новую аппаратную шкалу
void CanModule::mkySaveNewHardwareScale()
{
    quint8 pData[] =    {        MKY_SAVE_NEW_HARDWARE_SCALE_COURSE    };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
}



// Запросить смещения аппаратных шкал относительно железных тангажного и курсового контуров
void CanModule::hardwareScaleOffsetsRequest()
{
    quint8 pData[] =    {        MKY_HARDWARE_SCALE_OFFSETS_REQUEST    };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
}


// Калибровка ДУС по температуре
void CanModule::mkySetModeDusTemperatureCalibtarion()
{
    quint8 pData[] =    {        MKY_SET_MODE_DUS_TEMP_CALIBRATION    };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
}















// ================================== MKZ ==================================


// Запросить статус MKZ
void CanModule::mkzStatusRequest()
{
    _statusRequest(CAN_FRAME_ID_MKZ);
}

// Запросить значения ПИД-регулятора
void CanModule::mkzPIDKoefRequest()
{
    _PIDKoefRequest(CAN_FRAME_ID_MKZ);}

// Установить новые значения коэффициентов ПИД-регулятора
void CanModule::mkzSetPIDKoef(float p, float i, float d, float n, float tDiskr)
{
    _setPIDKoef(CAN_FRAME_ID_MKZ, p,i,d,n,tDiskr);
}

// Сохранить настройки во флешь-памяти
void CanModule::mkzSavePIDKoefInFlash()
{
    _savePIDKoefInFlash(CAN_FRAME_ID_MKZ);}

// Запустить калибровку по скорости
void CanModule::mkzStartVelocityCalibration()
{
    _startVelocityCalibration(CAN_FRAME_ID_MKZ);}

// Отпраивть рассширенную команду КУС
void CanModule::mkzSendExtendedCommand(quint8 *pBuf, quint8 len, bool extendedCmd, bool needAnswer)
{
    sendUserCommand(pBuf, len, extendedCmd, needAnswer, CAN_FRAME_ID_MKZ, MKY_SEND_EXTENDED_COMMAND);
}

// Установить режим работы туда-сюда
void CanModule::mkzSetModeTudaSudaVUS(quint16 period, float velocity)
{
    _setModeTudaSudaVUS(CAN_FRAME_ID_MKZ, period, velocity);
}

// Установить параметры, выводимые для отладки
void CanModule::mkzSetOutputDebugInfo(quint32 word)
{
    _setOutputDebugInfo(CAN_FRAME_ID_MKZ, word);
}

// Выключение двигателей
void CanModule::mkzEngineSwitchOff()
{
    _engineSwitchOff(CAN_FRAME_ID_MKZ);
}

// Сохранить ноль аппаратной шкалы
void CanModule::mkzSaveNewHardwareScale()
{
    quint8 pData[] =    {        MKY_SAVE_NEW_HARDWARE_SCALE_TANGAGE    };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
}

// Установить режим работы туда-сюда АР
void CanModule::mkzSetModeTudaSudaAR(quint16 period, qint16 angle1, qint16 angle2)
{
    _setModeTudaSudaAR(CAN_FRAME_ID_MKZ, period, angle1, angle2);
}


// Калибровка ДУС по температуре
void CanModule::mkzSetModeDusTemperatureCalibtarion()
{
    quint8 pData[] =    {        MKZ_SET_MODE_DUS_TEMP_CALIBRATION    };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKZ, false, pData, sizeof(pData));
}







// ========================= GENERAL =========================

// Выключить двигатели
void CanModule::gsEngineOff()
{
    quint8 pData[] =    {  GS_ENGINE_OFF  };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
}

// Установить ГОТ в 1
void CanModule::gsSetGOT()
{
    quint8 pData[] =    {  GS_SET_GOT_BIT  };
    module_CAN_IXXAT->transmitData(CAN_FRAME_ID_MKY, false, pData, sizeof(pData));
}



