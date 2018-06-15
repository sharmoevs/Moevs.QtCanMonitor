#include "extendedcommanddialog.h"

ExtendedCommandDialog::ExtendedCommandDialog(QWidget *parent) : QDialog(parent)
{
   this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::Tool);
   this->resize(800,300);

   isIrParent = false;
   isTvParent = false;
   indexInBytes = 0;
   indexInText = 0;

   QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

   txtLog = new QTextEdit(this);
   txtLog->setFont(fixedFont);
   txtLog->document()->setMaximumBlockCount(100);
   txtSend = new QLineEdit(this);
   txtSend->setToolTip("Введите -h для получения справки");
   checkBoxExtendedCmd = new QCheckBox("Расширенная команда", this);
   checkBoxExtendedCmd->setChecked(true);
   checkBoxNeedAnswer = new QCheckBox("С ответом", this);
   checkBoxNeedAnswer->setChecked(true);
   btnSend = new QPushButton("Отправить", this);
   btnTextMode = new QPushButton("HEX", this);

   //QRegExp bytesArrayRegExp(QString("(-h$)?([0-9A-Fa-f]{1,2})(\\s[0-9A-Fa-f]{1,2}){0,60}\\s?"));
   //txtSend->setValidator(new QRegExpValidator(bytesArrayRegExp, this));


   //Компоновка
   QGridLayout *mainLayout = new QGridLayout;
   mainLayout->setColumnMinimumWidth(2, 70);
   mainLayout->addWidget(txtLog,                0, 0, 1, 4);
   mainLayout->addWidget(checkBoxExtendedCmd,   1, 0);
   mainLayout->addWidget(checkBoxNeedAnswer,    1, 1);
   mainLayout->addWidget(txtSend,               2, 0, 1, 2);
   mainLayout->addWidget(btnTextMode,           2, 2, 1, 1);
   mainLayout->addWidget(btnSend,               2, 3, 1, 1);
   this->setLayout(mainLayout);

   connect(btnSend, SIGNAL(clicked(bool)), this, SLOT(onBtnSendClicked()));
   connect(btnTextMode, SIGNAL(clicked(bool)), this, SLOT(onBtnTextModeClicked()));
   connect(checkBoxExtendedCmd, SIGNAL(clicked(bool)), this, SLOT(onCmdTypeChanged()));

   onBtnTextModeClicked();
}

// При изменении типа команды
void ExtendedCommandDialog::onCmdTypeChanged()
{
    checkBoxNeedAnswer->setVisible(checkBoxExtendedCmd->isChecked());
}

// При измененни типа ввода команды - text/hex
void ExtendedCommandDialog::onBtnTextModeClicked()
{
    if(btnTextMode->text() == "TEXT")
    {
        btnTextMode->setText("HEX");
        checkBoxExtendedCmd->setVisible(true);
        checkBoxNeedAnswer->setVisible(true);

        QRegExp bytesArrayRegExp(QString("(-h$)?([0-9A-Fa-f]{1,2})(\\s[0-9A-Fa-f]{1,2}){0,60}\\s?"));
        txtSend->setValidator(new QRegExpValidator(bytesArrayRegExp, this));
        txtSend->clear();
    }
    else
    {
        btnTextMode->setText("TEXT");
        checkBoxExtendedCmd->setVisible(false);
        checkBoxNeedAnswer->setVisible(false);

        txtSend->setValidator(0);
        txtSend->clear();
    }
}

// Отправить команду
void ExtendedCommandDialog::onBtnSendClicked()
{
    if(txtSend->text() == "-h")
    {
      if(helpDescription.isNull() || helpDescription.isEmpty()) { txtSend->clear(); return; }

      txtLog->append(QString("\n%1").arg(helpDescription));
      txtLog->moveCursor(QTextCursor::End);
      txtSend->clear();
      return;
    }

    bool isExtendedCmd = checkBoxExtendedCmd->isChecked();
    bool needAnswer = checkBoxNeedAnswer->isChecked();
    bool sendHEX = btnTextMode->text() == "HEX";
    int len;
    if(sendHEX)
    {
        QStringList bytesList = txtSend->text().split(' ', QString::SkipEmptyParts);
        len = bytesList.count();
        if(len == 0) return;

        bool ok;
        for(int i=0; i<len; i++)
        {
            txBuf[i] = bytesList[i].toInt(&ok,16);
        }

        QString extCmdSign = isExtendedCmd ? "  (Ext)" : "";
        txtLog->append(QString("%1 ---&gt; <font color=%2> %3 </font> %4").arg(CURRENT_TIME, CAN_TX_MSG_COLOR, arrToStr(txBuf, len), extCmdSign));
        txtLog->moveCursor(QTextCursor::End);
    }
    else // отправить текст
    {
       QByteArray byteArray = txtSend->text().toUtf8();
       len = byteArray.length();
       if(len == 0) return;

       for(int i=0; i<len; i++)
       {
           txBuf[i+1] = byteArray.at(i);
       }
       txBuf[0] = EXTENDED_CMD_STRING_ID;   // вначале передается маркер - "передаем строку"
       len++;

       txtLog->append(QString("%1 ---&gt; <font color=%2> %3 </font>").arg(CURRENT_TIME, CAN_TX_MSG_COLOR,txtSend->text()));
       txtLog->moveCursor(QTextCursor::End);
    }



    emit sendTestManualCommand(txBuf, len, isExtendedCmd, needAnswer);
}

// Преобразовать массив байт в строку
QString ExtendedCommandDialog::arrToStr(const quint8 *pData, quint8 len)
{
    QString bytes;
    for(int i=0; i<len; i++)
        bytes += QString("%1").arg(pData[i], 2, 16, QLatin1Char('0')).toUpper() + "  ";
    return bytes;
}

// Обработка ответа на расширенную команду от камеры
void ExtendedCommandDialog::processReceivedCmd(const quint8 *pBuf, quint8 len)
{
    quint8 rxDone = 0;
    quint8 extAnswerLen;

    switch(pBuf[1] & 0xf)
    {
        case EXTCMD_START_MSG:
            indexInBytes = 0;
            break;

        case EXTCMD_CONT_MSG: break;

        case EXTCMD_END_MSG:
            rxDone = 1;
            break;

        default: return;
    }

    for(quint8 i=2; i<len; i++) // первые 2 байта служебные
    {
       rxBuf[indexInBytes++] = pBuf[i];
    }
    if(rxDone)
    {
        extAnswerLen = indexInBytes;
        indexInBytes = 0;


        if(rxBuf[0] == EXTENDED_CMD_STRING_ID)
        {
            parseDebugStr(&rxBuf[1], extAnswerLen-1);   // -1 -заголовок вначале пакета
        }
        else if(rxBuf[0] == DEBUG_MSG_ID)
        {
            parseDebugMsg(rxBuf, extAnswerLen);
        }
        else
        {
            QString rxBytes = arrToStr(rxBuf, extAnswerLen);
            txtLog->append(QString("%1 &lt;--- <font color=%2> %3 </font>").arg(CURRENT_TIME, CAN_RX_MSG_COLOR, rxBytes));
            txtLog->moveCursor(QTextCursor::End);
        }
    }
}


// Распарсить команду
void ExtendedCommandDialog::parseDebugMsg(const quint8 *pBuf, quint8 len)
{
    switch(pBuf[1])
    {
        case 0x1:     // значения температур, по которым определялся режим
        {
           if(isTvParent || isIrParent)
           {
                if(len != 5) return;
                QString s = QString("<font color=black>Первые показания с датчика температуры:<br>"
                                    "T1 = %1°<br>T2 = %2°<br>T3 = %3°</font>").arg(QString::number((qint8)pBuf[2]),
                                                                                   QString::number((qint8)pBuf[3]),
                                                                                   QString::number((qint8)pBuf[4]));

                txtLog->append(s);
                txtLog->moveCursor(QTextCursor::End);
           }
        }
        break;

        case 0x2:   // для ТВ - настройки, сохраненные во флешке, для ИК - кол-во команд в очереди и текущая выполняемая команда
        {
           if(isTvParent)
           {
               if(len != 4) return;

               qint8 offsetX = (qint8)pBuf[2];
               qint8 offsetY = (qint8)pBuf[3];

               txtLog->append(QString("<font color=black>Сохраненные настройки:<br>"
                                      "Смещение изображения по оси Х: %1<br>"
                                      "Смещение изображения по оси Y: %2</font>").arg(QString::number(offsetX),
                                                                          QString::number(offsetY)));
           }
           else if(isIrParent)
           {
               if(len != 4) return;

               quint8 queueCount = pBuf[2];
               quint8 currentCmdID = pBuf[3];

               QString currentCmd = "Unknown";
               switch(currentCmdID)
               {
                   case 0x00: currentCmd = "---"; break;
                   case 0x01: currentCmd = "Запрос температуры"; break;
                   case 0x02: currentCmd = "Сохранение текущих настроек в энергонезависимой памяти"; break;
                   case 0x04: currentCmd = "Установка режима пространственной фильтрации"; break;
                   case 0x10: currentCmd = "Установка режима временной фильтрации"; break;
                   case 0x20: currentCmd = "Установка режима АРУ"; break;
                   case 0x40: currentCmd = "Расширенная команда без ответа"; break;
                   case 0x80: currentCmd = "Расширенная команда с ответом"; break;
               }

               txtLog->append(QString("<font color=black>Количество команд в очереди - %1</font>").arg(QString::number(queueCount)));

               if(currentCmdID == 0) txtLog->append(QString("<font color=black>Текущих заданий нет</font"));
               else txtLog->append(QString("<font color=black>Текущее задание контроллера - %1</font>").arg(currentCmd));

               txtLog->moveCursor(QTextCursor::End);
           }
        }
        break;
    }
}

// Распарсить принятую строку
void ExtendedCommandDialog::parseDebugStr(const quint8 *pData, quint8 len)
{
    //QString str((QChar*)(&pData[1]), len);

    QString str = QTextCodec::codecForMib(106)->toUnicode((const char*)pData, len);
/*
    static int curVal, prevVal = 0;
    static int curValIRQ, prevValIRQ = 0;
    bool ok;

    int val = str.toInt(&ok);
    if(!ok)
    {
        txtLog->append("ОШИБКА ВХОДНЫХ ДАННЫХ!!!!!\r\n");
    }
    else
    {
        if(val >= 0)
        {
            curVal = val;
            if(curVal != prevVal+1)
            {
                txtLog->append("ОШИБКА НЕСООТВЕТСТВИЕ ЧИСЕЛ +!!!!!\r\n");
            }
            prevVal = curVal;
        }
        else
        {
            curValIRQ = val;
            if(curValIRQ != prevValIRQ-1)
            {
                txtLog->append("ОШИБКА НЕСООТВЕТСТВИЕ ЧИСЕЛ -!!!!!\r\n");
            }
            prevValIRQ = curValIRQ;
        }
    }
*/

    /*
    QStringList strList =str.split(' ');
    if(strList.count() > 1)
    {
        QString numStr = strList.at(1);
        bool ok;

        if(strList.at(0).startsWith('x'))
        {
            curVal = numStr.toInt(&ok);
            if(curVal != prevVal+1)
            {
                txtLog->append("ОШИБКА НЕСООТВЕТСТВИЕ ЧИСЕЛ X!!!!!\r\n");
            }
            prevVal = curVal;
        }
        else
        {
            curValIRQ = numStr.toInt(&ok);
            if(curValIRQ != prevValIRQ+1)
            {
                txtLog->append("ОШИБКА НЕСООТВЕТСТВИЕ ЧИСЕЛ Y!!!!!\r\n");
            }
            prevValIRQ = curValIRQ;
        }
    }
    else txtLog->append("ОШИБКА ВХОДНЫХ ДАННЫХ!!!!!\r\n");
*/
    //txtLog->append(QString("%1 --- %2").arg(CURRENT_TIME, str));


    // Отладка - Сохранить в лог файл
    if(str.startsWith("КАЛИБРОВКА ПО ТЕМПЕРАТУРЕ"))
    //if(str.startsWith("["))
    {
        QString logFilePath = QCoreApplication::applicationDirPath()+ QDir::separator() + "Moevs_CalibrationLog.txt";
        QFile file(logFilePath);
        if (file.open(QIODevice::Append))
        {
           QTextStream writeStream(&file);
           QString string = QString("%1: %2\r\n").arg(CURRENT_TIME, str);;
           writeStream << string;
           file.close();
        }
    }



    str = str.replace("<","&lt;");// вывод foo,bar,baz
    str = str.replace("\n","<br>");// вывод foo,bar,baz
    str = str.replace(" ","&nbsp;");


    QString appendStr = QString("%1 &lt;--- <font color=%2> %3 </font>").arg(CURRENT_TIME, CAN_RX_MSG_COLOR, str);
    txtLog->append(appendStr);
    //txtLog->moveCursor(QTextCursor::End);
}


















// ===================================== Тектовые сообщения  =================================


// Прием тектового сообщения
void ExtendedCommandDialog::processDbgText(const quint8 *pBuf, quint8 len)
{
    quint8 rxDone = 0;
    quint8 extAnswerLen;

    switch(pBuf[0] & 0xf)
    {
        case EXTCMD_START_MSG:
            indexInText = 0;
            break;

        case EXTCMD_CONT_MSG: break;

        case EXTCMD_END_MSG:
            rxDone = 1;
            break;

        default:
        return;
    }

    for(quint8 i=1; i<len; i++) // первые 2 байта служебные
    {
       rxTextBuf[indexInText++] = pBuf[i];
    }

    if(rxDone)
    {
        extAnswerLen = indexInText;
        indexInText = 0;

        parseDebugStr(rxTextBuf, extAnswerLen);
    }
}
