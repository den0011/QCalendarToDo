QT       += core gui charts sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QCalendarToDo
TEMPLATE = app


DEFINES += QT_DEPRECATED_WARNINGS


CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        searchdialog.cpp \
        settingsdialog.cpp \
        statisticsdialog.cpp \
        thememanager.cpp

HEADERS += \
        mainwindow.h \
        searchdialog.h \
        settings.h \
        settingsdialog.h \
        sql.h \
        statisticsdialog.h \
        thememanager.h

FORMS += \
        mainwindow.ui \
        searchdialog.ui \
        settingsdialog.ui \
        statisticsdialog.ui

RESOURCES += \
    res.qrc

win32 {
        RC_FILE += file.rc
        OTHER_FILES += file.rc
}


