#ifndef EXTENDEDCOMMANDDIALOG_H
#define EXTENDEDCOMMANDDIALOG_H

#include <QWidget>
#include <QtWidgets>
#include <QDateTime>
#include "defines.h"


#define EXTENDED_CMD_MAX_LEN     256
#define DEBUG_MSG_ID             0xDB
#define EXTENDED_CMD_STRING_ID   0xBB

#define CURRENT_TIME (QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))

#define CAN_TX_MSG_COLOR    ("#0083FF")
#define CAN_RX_MSG_COLOR    ("#FF6E00")


class ExtendedCommandDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExtendedCommandDialog(QWidget *parent = 0);
    void setHelpDescription(QString txt) { helpDescription = txt; }
    bool isIrParent;
    bool isTvParent;

private:
    QTextEdit *txtLog;
    QLineEdit *txtSend;
    QCheckBox *checkBoxExtendedCmd;
    QCheckBox *checkBoxNeedAnswer;
    QPushButton *btnSend;
    QPushButton *btnTextMode;   // режим ввода текста - text/hex
    QString helpDescription;

    quint8 txBuf[EXTENDED_CMD_MAX_LEN];
    quint8 rxBuf[EXTENDED_CMD_MAX_LEN];
    quint8 rxTextBuf[EXTENDED_CMD_MAX_LEN];
    quint8 indexInBytes;
    quint8 indexInText;

    QString arrToStr(const quint8 *pData, quint8 len);
    void parseDebugMsg(const quint8 *pData, quint8 len);
    void parseDebugStr(const quint8 *pData, quint8 len);

signals:
    void sendTestManualCommand(quint8 *pBuf, quint8 len, bool extendedCmd, bool needAnswer);

private slots:
    void onBtnSendClicked();
    void onBtnTextModeClicked();
    void onCmdTypeChanged();


public slots:
    void processReceivedCmd(const quint8 *pBuf, quint8 len);
    void processDbgText(const quint8 *pBuf, quint8 len);
};

#endif // EXTENDEDCOMMANDDIALOG_H
