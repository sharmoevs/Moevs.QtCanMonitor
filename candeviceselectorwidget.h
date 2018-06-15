#ifndef CANDEVICESELECTORWIDGET_H
#define CANDEVICESELECTORWIDGET_H

#include <QObject>
#include <QtWidgets>
#include <QWidget>

class CANDeviceSelectorWidget : public QToolBar
{
    Q_OBJECT

public:
    CANDeviceSelectorWidget();//QToolBar *parent = 0);

    void onConnectionStateChanged(bool opened);
    void onDeviceListUpdated(QStringList deviceList);
    void onBusListUpdated(QStringList busList);

    int getDevicesCount() const { return comboBox_moduleCAN_DevName->count(); }
    int getDeviceIndex()const { return comboBox_moduleCAN_DevName->currentIndex(); }
    int getBusIndex() const { return comboBox_moduleCAN_BusName->currentIndex(); }


private:
    QComboBox *comboBox_moduleCAN_DevName;
    QComboBox *comboBox_moduleCAN_BusName;
    QAction *action_UpdateListDev;
    QAction *action_OpenCloseDev;

protected:
    void contextMenuEvent(QContextMenuEvent *event);

signals:
    // Список CAN-устройств
    void refreshDeviceListRequest();        // обновили список устройств
    void selectedDeviceChanged(int index);  // выбрали другое устройство
    void openCloseRequest();                // подключить/отключить устройство


};








#endif // CANDEVICESELECTORWIDGET_H
