#ifndef GSGENERALSTATE_H
#define GSGENERALSTATE_H

#include <QWidget>
#include "QTimer.h"

namespace Ui {
class GsGeneralState;
}

#define CHECK_FRESH_DATA_TIMER_INTERVAL     1000

class GsGeneralState : public QWidget
{
    Q_OBJECT

public:
    explicit GsGeneralState(QWidget *parent = 0);
    ~GsGeneralState();

private:
    void resetFreshDataFlags();

private:
    Ui::GsGeneralState *ui;
    QTimer *checkFreshDataTimer;      // проверка

    bool gsStateFreshData;

signals:
    void gs_engineOff();
    void gs_setGOT();
    void saveSamples(int count);

private slots:
    void checkFreshData();

    void on_btnSaveSamples_clicked();

public slots:
    void updateGsState(const quint8 *pBuf, quint8 len);

};

#endif // GSGENERALSTATE_H
