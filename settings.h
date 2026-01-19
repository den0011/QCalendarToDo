#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMainWindow>
#include <QSettings>
#include <QtDebug>
#include <QColor>
#include <QFont>

struct Settings
{
    QColor done;
    QColor progress;
    QFont tasks;
    QFont notes;
    bool messages;
    int sec;

    Settings() : messages(false) {}

    void save(QSettings &settings)
    {
        settings.beginGroup("Settings");
        settings.setValue("Colors/done", done);
        settings.setValue("Colors/progress", progress);
        settings.setValue("Fonts/tasks", tasks);
        settings.setValue("Fonts/notes", notes);
        settings.setValue("General/showMessages", messages);
        settings.setValue("Tray/sec", sec);
        settings.endGroup();
    }

    void load(QSettings &settings)
    {
        settings.beginGroup("Settings");
        done = settings.value("Colors/done", QColor(255, 128, 128)).value<QColor>();
        progress = settings.value("Colors/progress", QColor(0, 128, 192)).value<QColor>();
        tasks = settings.value("Fonts/tasks", QFont("Arial", 12)).value<QFont>();
        notes = settings.value("Fonts/notes", QFont("Arial", 12)).value<QFont>();
        messages = settings.value("General/showMessages", true).toBool();
        sec = settings.value("Tray/sec", 5).toInt();
        settings.endGroup();
    }

    void printValues()
    {
        qDebug() << "Done Color: " << done;
        qDebug() << "Progress Color: " << progress;
        qDebug() << "Tasks Font: " << tasks;
        qDebug() << "Notes Font: " << notes;
        qDebug() << "Show Messages: " << messages;
        qDebug() << "Tray sec: " << sec;
    }
};

#endif // SETTINGS_H
