#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "candeviceselectorwidget.h"
#include "canmodule.h"
#include "mkyInfoWidget.h"
#include "gsgeneralstate.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    CanModule *canModule;
    CANDeviceSelectorWidget *canDeviceSelector;
    QTabWidget *mainTabWidget;
    MKYinfoWidget *mkyInfoWidget;
    MKYinfoWidget *mkzInfoWidget;
    GsGeneralState *gsGeneralState;


private slots:

    void onMainTabWidgetTabChanged(int index);
    void requestStateTimersSetEnable(int selectedTab);

    // CANDeviceSelectorWidget
    void updateCANDeviceList();    // получить список доступный устройств
    void updateCANBusList();       // получить список доступных шин выбранного устройства
    void onCANOpenClose();         // обработка запроса на подключение/отключение модуля
    void onCANDeviceConnectionChanged(bool isOpen);     // при подлючении/отключении CAN-устройства

    void onCANModuleResetRequest(); // запрос на сброс can-модуля



};

#endif // MAINWINDOW_H
