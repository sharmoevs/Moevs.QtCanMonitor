#include "candeviceselectorwidget.h"
//#include "defines.h"

CANDeviceSelectorWidget::CANDeviceSelectorWidget()//QToolBar *parent) : QToolBar(parent)
{
    this->setIconSize(QSize(48,48));
    this->setFloatable(false);

    QFont font;
    font.setPointSize(12);

    QLabel *label_ToolBar1 = new QLabel("Модуль ",this);
    label_ToolBar1->setFont(font);

    comboBox_moduleCAN_DevName = new QComboBox;
    comboBox_moduleCAN_BusName = new QComboBox;

    comboBox_moduleCAN_DevName->setFont(font);
    comboBox_moduleCAN_DevName->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    comboBox_moduleCAN_DevName->setToolTip("Выбор модуля");
    comboBox_moduleCAN_BusName->setFont(font);
    comboBox_moduleCAN_BusName->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    comboBox_moduleCAN_BusName->setToolTip("Выбор шины в модуле");

    action_UpdateListDev = new QAction(QIcon(":/images/images/refreshCom.png"),"",this);
    action_UpdateListDev->setToolTip("Обновить список модулей");
    action_OpenCloseDev = new QAction(QIcon(":/images/images/connect.png"),"",this);
    action_OpenCloseDev->setToolTip("Подключиться к шине");

    this->addWidget(label_ToolBar1);
    this->addWidget(comboBox_moduleCAN_DevName);
    this->addWidget(comboBox_moduleCAN_BusName);
    this->addAction(action_UpdateListDev);
    this->addAction(action_OpenCloseDev);

    comboBox_moduleCAN_DevName->setEnabled(false);
    comboBox_moduleCAN_BusName->setEnabled(false);
    action_OpenCloseDev->setEnabled(false);

    QObject::connect(comboBox_moduleCAN_DevName, SIGNAL(currentIndexChanged(int)), this, SIGNAL(selectedDeviceChanged(int)));
    QObject::connect(action_UpdateListDev, SIGNAL(triggered()), this, SIGNAL(refreshDeviceListRequest()));
    QObject::connect(action_OpenCloseDev, SIGNAL(triggered()), this, SIGNAL(openCloseRequest()));
}


// При подключении/отключии коробочки
void CANDeviceSelectorWidget::onConnectionStateChanged(bool opened)
{
    if(opened)      // устройство подключено
    {
        action_OpenCloseDev->setIcon(QIcon(":/images/images/disconnect.png"));
        action_OpenCloseDev->setToolTip("Отключить модуль");
        action_UpdateListDev->setEnabled(false);
        comboBox_moduleCAN_DevName->setEnabled(false);
        comboBox_moduleCAN_BusName->setEnabled(false);
    }
    else            // устройство отключено
    {
        action_OpenCloseDev->setIcon(QIcon(":/images/images/connect.png"));
        action_OpenCloseDev->setToolTip("Подключить модуль");
        action_UpdateListDev->setEnabled(true);
        comboBox_moduleCAN_DevName->setEnabled(true);
        comboBox_moduleCAN_BusName->setEnabled(true);
    }
}

// Обновить список устройств
void CANDeviceSelectorWidget::onDeviceListUpdated(QStringList deviceList)
{
    comboBox_moduleCAN_DevName->clear();

    if(deviceList.size() == 0)
    {
        deviceList += QString("нет доступных");
        comboBox_moduleCAN_DevName->setEnabled(false);
        action_OpenCloseDev->setEnabled(false);
    }
    else
    {
        comboBox_moduleCAN_BusName->setEnabled(true);
        comboBox_moduleCAN_DevName->setEnabled(true);
        action_OpenCloseDev->setEnabled(true);
    }
    comboBox_moduleCAN_DevName->addItems(deviceList);
}

// Обновить список шин
void CANDeviceSelectorWidget::onBusListUpdated(QStringList busList)
{
    comboBox_moduleCAN_BusName->clear();

    if(busList.size() == 0)
    {
        busList += QString("нет шин");
        comboBox_moduleCAN_BusName->setEnabled(false);
    }
    else
    {
        comboBox_moduleCAN_BusName->setEnabled(true);
        comboBox_moduleCAN_DevName->setEnabled(true);
    }
    comboBox_moduleCAN_BusName->addItems(busList);
}



// Контекстное меню
void CANDeviceSelectorWidget::contextMenuEvent(QContextMenuEvent *event)
{
    // QToolBar::contextMenuEvent(event);

    QMenu *menu = new QMenu(this);
    //menu->addAction(QString("Версия ПО %1").arg(APP_VERSION));
    menu->exec(event->globalPos());
    delete menu;
}





