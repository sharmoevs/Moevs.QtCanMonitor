#ifndef MODULE_CAN_IXXAT_H
#define MODULE_CAN_IXXAT_H

#include <QThread>
#include <QStringList>
#include <vcinpl.h>
#include <QList>

typedef struct{
    quint16 id;
    bool rtr;
    quint8 pData[8];
    quint8 len;
}MsgCAN_type;

class Module_CAN_IXXAT : public QThread
{
    Q_OBJECT
public:
    explicit Module_CAN_IXXAT(QObject *parent = 0);
    ~Module_CAN_IXXAT();

    QStringList getListDev();
    QStringList getListBusDev(quint32 i);
    bool openDev(quint32 iDev,quint32 iBus);
    void closeDev();

    void test()
    {
      //canControlReset(hCanCtl);
        UINT16 wRxFifoSize  = 1024;
        UINT16 wRxThreshold = 1;
        UINT16 wTxFifoSize  = 128;
        UINT16 wTxThreshold = 1;
        canChannelInitialize( hCanChn,
                                        wRxFifoSize, wRxThreshold,
                                        wTxFifoSize, wTxThreshold);
    }

public slots:
    void transmitData(quint16 id, bool rtr, const quint8 *pData, quint8 len);
    void transmitData(QList<MsgCAN_type> listMsgCAN);

signals:
    void receiveData(bool self, bool rtr, quint16 id,const quint8 *pData, quint8 dlc);

protected:
    void run();

private:
    volatile bool stopped;
    bool devOpened;
    HANDLE hDevice;       // device handle
    HANDLE hCanChn;       // channel handle
    HANDLE hCanCtl;       // controller handle

    quint8 *bufData;
    quint16 iBuf;
};

#endif // MODULE_CAN_IXXAT_H
