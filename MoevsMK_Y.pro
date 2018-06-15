#-------------------------------------------------
#
# Project created by QtCreator 2017-05-11T10:01:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = MoevsMK_Y
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    srcUSB_to_CAN_II/module_can_ixxat.cpp \
    candeviceselectorwidget.cpp \
    canmodule.cpp \
    mkyinfowidget.cpp \
    qcustomplot.cpp \
    extendedcommanddialog.cpp \
    gsgeneralstate.cpp

HEADERS  += mainwindow.h \
    srcUSB_to_CAN_II/module_can_ixxat.h \
    srcUSB_to_CAN_II/sdk_can_module/vciver.h \
    srcUSB_to_CAN_II/sdk_can_module/CANtype.h \
    srcUSB_to_CAN_II/sdk_can_module/vciCtrlType.h \
    srcUSB_to_CAN_II/sdk_can_module/KLNtype.h \
    srcUSB_to_CAN_II/sdk_can_module/LINtype.h \
    srcUSB_to_CAN_II/sdk_can_module/vcierr.h \
    srcUSB_to_CAN_II/sdk_can_module/vciguid.h \
    srcUSB_to_CAN_II/sdk_can_module/vciIdType.h \
    srcUSB_to_CAN_II/sdk_can_module/vcinpl.h \
    srcUSB_to_CAN_II/sdk_can_module/vcitype.h \
    srcUSB_to_CAN_II/sdk_can_module/vci3.h \
    candeviceselectorwidget.h \
    canmodule.h \
    defines.h \
    mkyinfowidget.h \
    qcustomplot.h \
    extendedcommanddialog.h \
    gsgeneralstate.h

FORMS    += mainwindow.ui \
    mkyinfowidget.ui \
    gsgeneralstate.ui





INCLUDEPATH += "srcUSB_to_CAN_II/" \
               "srcUSB_to_CAN_II/sdk_can_module/"

win32:LIBS += "$$PWD\srcUSB_to_CAN_II\sdk_can_module/vcisdk32.lib"



RESOURCES += \
    resources.qrc
