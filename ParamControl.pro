QT += core gui network xml widgets multimedia

CONFIG += c++17

# Условие для сборки под Astra Linux (если необходимо)
# CONFIG(astralinux) {
#     message("Building for Astra Linux")
#     # Дополнительные флаги или настройки для Astra Linux
# }

TARGET = ParamControl
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to fail only for certain releases of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/core/Parameter.cpp \
    src/core/ParameterEquals.cpp \
    src/core/ParameterNotEquals.cpp \
    src/core/ParameterInLimits.cpp \
    src/core/ParameterOutOfLimits.cpp \
    src/core/ParameterChanged.cpp \
    src/core/ParameterModel.cpp \
    src/core/SotmClient.cpp \
    src/core/XmlParser.cpp \
    src/core/MonitoringService.cpp \
    src/core/AlertManager.cpp \
    src/core/LogManager.cpp \
    src/core/TmiAnalyzer.cpp \
    src/core/UpdateManager.cpp \
    src/ui/MainWindow.cpp \
    src/ui/ParameterDialog.cpp \
    src/ui/ConnectionDialog.cpp \
    src/ui/LogDialog.cpp \
    src/ui/KaZsDialog.cpp \
    src/ui/SettingsDialog.cpp \
    src/ui/ParameterCardView.cpp \
    src/ui/ParameterTableModel.cpp \
    src/ui/LogTableModel.cpp \
    src/ui/LogSortFilterProxyModel.cpp # Добавлен новый файл

HEADERS += \
    src/core/Parameter.h \
    src/core/ParameterEquals.h \
    src/core/ParameterNotEquals.h \
    src/core/ParameterInLimits.h \
    src/core/ParameterOutOfLimits.h \
    src/core/ParameterChanged.h \
    src/core/ParameterModel.h \
    src/core/SotmClient.h \
    src/core/XmlParser.h \
    src/core/MonitoringService.h \
    src/core/AlertManager.h \
    src/core/LogManager.h \
    src/core/TmiAnalyzer.h \
    src/core/UpdateManager.h \
    src/ui/MainWindow.h \
    src/ui/ParameterDialog.h \
    src/ui/ConnectionDialog.h \
    src/ui/LogDialog.h \
    src/ui/KaZsDialog.h \
    src/ui/SettingsDialog.h \
    src/ui/ParameterCardView.h \
    src/ui/ParameterTableModel.h \
    src/ui/LogTableModel.h \
    src/ui/LogSortFilterProxyModel.h # Добавлен новый файл

FORMS += \
    src/ui/MainWindow.ui \
    src/ui/ParameterDialog.ui \
    src/ui/ConnectionDialog.ui \
    src/ui/LogDialog.ui \
    src/ui/KaZsDialog.ui \
    src/ui/SettingsDialog.ui \
    src/ui/ParameterCardView.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    src/resources/resources.qrc

# Указываем путь для поиска включаемых файлов
INCLUDEPATH += src/core \
               src/ui

# Дополнительные библиотеки, если потребуются
# LIBS += -lsome_library

# Настройки для конкретных платформ
win32: LIBS += -lws2_32 # Для сокетов в Windows, если потребуется

# Копирование директории data при сборке (пример)
# Необходимо убедиться, что пути корректны для вашей системы сборки
# win32 {
#     COPY_CMD = xcopy /s /i /y
#     DATA_SOURCE = $$PWD/data
#     DATA_DEST = $$OUT_PWD/debug/data # Или release
# } else {
#     COPY_CMD = cp -R
#     DATA_SOURCE = $$PWD/data
#     DATA_DEST = $$OUT_PWD/data
# }
#
# data_deployment.commands = $$COPY_CMD $$shell_path($$DATA_SOURCE) $$shell_path($$DATA_DEST)
# QMAKE_EXTRA_TARGETS += data_deployment
# POST_TARGETDEPS += data_deployment

# Копирование звуков и иконок, если они не в ресурсах
# sound_deployment.files = $$PWD/src/resources/sounds/*
# sound_deployment.path = $$OUT_PWD/sounds # или $$INSTALL_ROOT/sounds
# INSTALLS += sound_deployment
#
# icon_deployment.files = $$PWD/src/resources/icons/*
# icon_deployment.path = $$OUT_PWD/icons # или $$INSTALL_ROOT/icons
# INSTALLS += icon_deployment

