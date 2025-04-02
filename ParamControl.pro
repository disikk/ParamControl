QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    AlertManager.cpp \
    KaZsDialog.cpp \
    LogManager.cpp \
    LogTableModel.cpp \
    LogDialog.cpp \
    MainWindow.cpp \
    MessageBox.cpp \
    MonitoringService.cpp \
    Parameter.cpp \
    ParameterChanged.cpp \
    ParameterDialog.cpp \
    ParameterEquals.cpp \
    ParameterInLimits.cpp \
    ParameterModel.cpp \
    ParameterNotEquals.cpp \
    ParameterOutOfLimits.cpp \
    ParameterTableModel.cpp \
    SotmClient.cpp \
    XmlParser.cpp \
    ConnectionDialog.cpp \
    main.cpp

HEADERS += \
    AlertManager.h \
    KaZsDialog.h \
    LogManager.h \
    LogTableModel.h \
    LogDialog.h \
    MainWindow.h \
    MessageBox.h \
    MonitoringService.h \
    Parameter.h \
    ParameterChanged.h \
    ParameterDialog.h \
    ParameterEquals.h \
    ParameterInLimits.h \
    ParameterModel.h \
    ParameterNotEquals.h \
    ParameterOutOfLimits.h \
    ParameterTableModel.h \
    SotmClient.h \
    XmlParser.h \
    ConnectionDialog.h

FORMS += \
    ConnectionDialog.ui \
    KaZsDialog.ui \
    LogDialog.ui \
    MainWindow.ui \
    MessageBox.ui \
    ParameterDialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

# Настройки компиляции для Astra Linux
linux {
    QMAKE_CXXFLAGS += -std=c++17
    QMAKE_LFLAGS += -no-pie
}

# Иконка приложения
RC_ICONS = icons/app_icon.ico
