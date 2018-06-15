#include "module_can_ixxat.h"
#include <QString>
#include <QDebug>

#define BUF_SIZE    450000

Module_CAN_IXXAT::Module_CAN_IXXAT(QObject *parent) :
    QThread(parent)
{
    devOpened = false;
    stopped = false;
    bufData = new quint8[8*BUF_SIZE];
    iBuf = 0;
}

Module_CAN_IXXAT::~Module_CAN_IXXAT()
{
   closeDev();
   delete[] bufData;
}

QStringList Module_CAN_IXXAT::getListDev()
{
    QStringList strList;
    HRESULT hResult;        // error code
    HANDLE  hEnum;          // enumerator handle
    VCIDEVICEINFO sInfo;    // device info

    strList.clear();

    hResult = vciEnumDeviceOpen(&hEnum);
    if (hResult == VCI_OK)
    {
        quint32 iDev = 0;
        while(vciEnumDeviceNext(hEnum, &sInfo)==VCI_OK)
        {
            ++iDev;
            QString str = QString("%1 ").arg(iDev) + QString::fromLatin1(sInfo.Description);
            strList.append(str);
        }

        vciEnumDeviceClose(hEnum);
    }

    return strList;
}

QStringList Module_CAN_IXXAT::getListBusDev(quint32 iDev)
{
    QStringList strList;
    HRESULT hResult;        // error code
    HANDLE  hEnum;          // enumerator handle
    VCIDEVICEINFO sInfo;    // device info
    VCIDEVICECAPS caps;

    strList.clear();

    hResult = vciEnumDeviceOpen(&hEnum);
    if (hResult == VCI_OK)
    {
        for(quint32 i=0; i<=iDev; ++i)
        {
          if(vciEnumDeviceNext(hEnum, &sInfo)!=VCI_OK) return strList;
        }

        hResult = vciDeviceOpen(sInfo.VciObjectId, &hDevice);
        if(hResult==VCI_OK)
        {
            hResult = vciDeviceGetCaps(hDevice,&caps);
            if(hResult==VCI_OK)
            {
                for(quint32 i=0;i<caps.BusCtrlCount;++i)
                {
                    QString str;
                    UINT16 busCtrlTypes = caps.BusCtrlTypes[i];
                    switch(VCI_BUS_TYPE(busCtrlTypes))
                    {
                    case VCI_BUS_RES: str += QString("reserved %1").arg(i+1); break;
                    case VCI_BUS_CAN: str += QString("CAN %1").arg(i+1); break;
                    case VCI_BUS_LIN: str += QString("LIN %1").arg(i+1); break;
                    case VCI_BUS_FLX: str += QString("FlexRay %1").arg(i+1); break;
                    case VCI_BUS_KLN: str += QString("K-Line %1").arg(i+1); break;
                    }

                    strList.append(str);
                }
            }
            vciDeviceClose(hDevice);
        }

        vciEnumDeviceClose(hEnum);
    }

    return strList;
}


bool Module_CAN_IXXAT::openDev(quint32 iDev,quint32 iBus)
{
    //(void)iDev,iBus;
    HRESULT hResult;        // error code
    HANDLE  hEnum;          // enumerator handle
    HANDLE hDevice;       // device handle
    VCIDEVICEINFO sInfo;    // device info

    hResult = vciEnumDeviceOpen(&hEnum);
    if (hResult == VCI_OK)
    {
        for(quint32 i=0; i<=iDev; ++i)
        {
          if(vciEnumDeviceNext(hEnum, &sInfo)!=VCI_OK) return devOpened;
        }

        hResult = vciDeviceOpen(sInfo.VciObjectId, &hDevice);
        if(hResult==VCI_OK)
        {
            hResult = canChannelOpen(hDevice, iBus, FALSE, &hCanChn);
            //vciDeviceClose(hDevice);
        }

        if (hResult == VCI_OK)
        {// initialize the message channel
          UINT16 wRxFifoSize  = 1024;
          UINT16 wRxThreshold = 1;
          UINT16 wTxFifoSize  = 128;
          UINT16 wTxThreshold = 1;

          hResult = canChannelInitialize( hCanChn,
                                          wRxFifoSize, wRxThreshold,
                                          wTxFifoSize, wTxThreshold);
        }

        if (hResult == VCI_OK)
        {// activate the CAN channel
            //qDebug()<<"initialize the message channel OK";
          hResult = canChannelActivate(hCanChn, TRUE);
        }

        if (hResult == VCI_OK)
        {// open the CAN controller
            //qDebug()<<"activate the CAN channel OK";
          hResult = canControlOpen(hDevice, iBus, &hCanCtl);
          // this function fails if the controller is in use
          // by another application.
        }

        if (hResult == VCI_OK)
        {// initialize the CAN controller
            //qDebug()<<"open the CAN controller OK";
          /*hResult = */canControlInitialize(hCanCtl, CAN_OPMODE_STANDARD | CAN_OPMODE_ERRFRAME,
                                         CAN_BT0_1000KB, CAN_BT1_1000KB);
        }

        if (hResult == VCI_OK)
        { // set the acceptance filter
            //qDebug()<<"initialize the CAN controller OK";
           /*hResult = */canControlSetAccFilter( hCanCtl, FALSE,
                                             CAN_ACC_CODE_ALL, CAN_ACC_MASK_ALL);
        }

        if (hResult == VCI_OK)
        {// start the CAN controller
            //qDebug()<<"set the acceptance filter OK";
          /*hResult = */canControlStart(hCanCtl, TRUE);
        }

        if (hResult == VCI_OK)
        {
            //qDebug()<<"start the CAN controller OK";
            devOpened = true;
            start();
        }

        vciEnumDeviceClose(hEnum);
    }

    return devOpened;
}

void Module_CAN_IXXAT::closeDev()
{
    if(devOpened)
    {
        if(isRunning())
        {
            stopped = true;
            while(isRunning());
        }
        canControlReset(hCanCtl);
        canChannelClose(hCanChn);
        canControlClose(hCanCtl);
        vciDeviceClose(hDevice);
        devOpened = false;
    }
}

void Module_CAN_IXXAT::transmitData(quint16 id,bool rtr,const quint8 *pData, quint8 len)
{
  HRESULT hResult;
  CANMSG  sCanMsg;
  UINT8   i;

  sCanMsg.dwTime   = 0;
  sCanMsg.dwMsgId  = id;    // CAN message identifier

  sCanMsg.uMsgInfo.Bytes.bType  = CAN_MSGTYPE_DATA;
  sCanMsg.uMsgInfo.Bytes.bFlags = CAN_MAKE_MSGFLAGS(8,0,0,0,0);
  sCanMsg.uMsgInfo.Bits.srr     = 1;
  if(len>8) len=8;
  sCanMsg.uMsgInfo.Bits.dlc = len;

  if(rtr) sCanMsg.uMsgInfo.Bits.rtr = 1;
  else
  {
      sCanMsg.uMsgInfo.Bits.rtr = 0;
      for (i = 0; i < len; i++ )
      {
          sCanMsg.abData[i] = pData[i];
      }
  }

  static int tt=0;
  tt++;

  // write the CAN message into the transmit FIFO
  hResult = canChannelSendMessage(hCanChn, 100 /*INFINITE*/, &sCanMsg);
  if(hResult != VCI_OK)
  {
    qDebug() << "Tx result: " + QString::number(hResult, 16);
  }
}

void Module_CAN_IXXAT::transmitData(QList<MsgCAN_type> listMsgCAN)
{
    quint32 num = listMsgCAN.size();

    for(quint32 n=0; n<num; ++n)
    {
        bool rtr = listMsgCAN[n].rtr;
        quint16 id = listMsgCAN[n].id;
        const quint8 *pData = listMsgCAN[n].pData;
        quint8 len = listMsgCAN[n].len;

        //HRESULT hResult;
        CANMSG  sCanMsg;
        UINT8   i;

        sCanMsg.dwTime   = 0;
        sCanMsg.dwMsgId  = id;    // CAN message identifier

        sCanMsg.uMsgInfo.Bytes.bType  = CAN_MSGTYPE_DATA;
        sCanMsg.uMsgInfo.Bytes.bFlags = CAN_MAKE_MSGFLAGS(8,0,0,0,0);
        sCanMsg.uMsgInfo.Bits.srr     = 1;
        if(len>8) len=8;
        sCanMsg.uMsgInfo.Bits.dlc = len;

        if(rtr) sCanMsg.uMsgInfo.Bits.rtr = 1;
        else
        {
            sCanMsg.uMsgInfo.Bits.rtr = 0;
            for (i = 0; i < len; i++ )
            {
                sCanMsg.abData[i] = pData[i];
            }
        }

        // write the CAN message into the transmit FIFO
        /*hResult = */canChannelSendMessage(hCanChn, INFINITE, &sCanMsg);
    }
}


void Module_CAN_IXXAT::run()
{
    HRESULT hResult;
    CANMSG  sCanMsg;
    struct data;
    //static quint8 bufData[8];

    while(!stopped)
    {
        // read a CAN message from the receive FIFO
        hResult = canChannelReadMessage(hCanChn, 100, &sCanMsg);
        if (hResult == VCI_OK)
        {
            if (sCanMsg.uMsgInfo.Bytes.bType == CAN_MSGTYPE_DATA)
            {
                  bool rtr,self;
                  quint8 dlc = sCanMsg.uMsgInfo.Bits.dlc;
                  if(dlc>8) dlc=8;
                  if (sCanMsg.uMsgInfo.Bits.rtr == 0)
                  {//Not Remote Frame
                      for(quint8 i=0;i<dlc;++i) bufData[iBuf+i]=sCanMsg.abData[i];
                      rtr = false;
                  }
                  else rtr = true;
                  if(sCanMsg.uMsgInfo.Bits.srr) self=true;
                  else self = false;
                  emit receiveData(self,rtr,sCanMsg.dwMsgId,&bufData[iBuf],dlc);
                  iBuf += 8;
                  if(iBuf>=(8*BUF_SIZE)) iBuf=0;
            }
        }
        else
        {
            ////qDebug() << "Rx result: " + QString::number(hResult, 16);
            CANLINESTATUS canLineStatus;
            canControlGetStatus(hCanCtl, &canLineStatus);
            if(canLineStatus.dwStatus & CAN_STATUS_BUSOFF)
            {
               hResult = canControlInitialize(hCanCtl, CAN_OPMODE_STANDARD | CAN_OPMODE_ERRFRAME,
                                                         CAN_BT0_1000KB, CAN_BT1_1000KB);
               hResult = canControlStart(hCanCtl, TRUE);
            }
        }
    }
    stopped = false;
}
