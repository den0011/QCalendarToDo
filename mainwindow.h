#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QMainWindow>
#include <QTime>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QtCharts>
#include <QPainter>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QColorDialog>
#include <QMenu>
#include <QClipboard>
#include <QApplication>

#include "settings.h"
#include "sql.h"
#include "searchdialog.h"
#include "statisticsdialog.h"
#include "settingsdialog.h"
#include "thememanager.h"

// Делегат для календаря — рисует сегодняшнюю ячейку акцентным цветом темы
class CalendarDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

// Делегат для таблицы задач — явно рисует фон из Qt::BackgroundRole, минуя QSS
class TaskDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

namespace Ui {
class MainWindow;
}

class SearchDialog;
class StatisticsDialog;
class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    int lastDisplayedRow = -1;
    QTimer timeUdate;
    QtCharts::QPieSeries *pieSeries;
    Settings settings;
    int currentYear;
    int currentMonth;
    QString configFilePath;
    DatabaseManager manager;

    QDate currentSelectedDate;
    bool isLoadingData;

    QLabel *saveStatusLabel;
    QTimer *saveStatusTimer;

    SearchDialog *searchDialog;
    StatisticsDialog *statisticsDialog;
    SettingsDialog *settingsDialog;

private:
    void setupUITableToDo();
    void createPiChart();
    void createCalendar();
    void setParametrSettings();
    void setupSaveIndicator();

    void saveTasksForCurrentDate();
    void loadTasksForDate(const QDate &date);
    void loadNotesForDate(const QDate &date);
    void saveNotesForCurrentDate();
    void showSaveStatus(const QString &message, bool success = true);

    bool createBackup(const QString &filePath);
    bool restoreBackup(const QString &filePath);
    bool exportToCSV(const QString &filePath);

    void highlightCurrentDay();
    void updateStatsPanel(int done, int progress);
    void refreshDbStats();
    void scrollToCurrentTime();

private slots:
    void updateCellColors();
    void updateCalendar();
    void quitApp();

    void onDateChanged(const QDate &newDate);
    void onCellClicked(int row, int col);
    void onDateEditChanged(const QDate &newDate);

    void onTableItemChanged(QTableWidgetItem *item);
    void hideSaveStatus();

    void onBackupCreate();
    void onBackupRestore();
    void onExportCSV();

    void onSearchTriggered();
    void onSearchDateSelected(const QDate &date);

    void onStatisticsTriggered();
    void onSettingsTriggered();
    void applySettings();

    void onPrevMonth();
    void onNextMonth();
    void onTodayClicked();
    void onTaskTableContextMenu(const QPoint &pos);

    void onLightTheme();
    void onDarkTheme();
    void onBlueTheme();
    void onGreenTheme();
};

#endif
