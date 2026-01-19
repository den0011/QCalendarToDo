#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QDate>
#include <QMap>

namespace Ui {
class StatisticsDialog;
}

class StatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDialog(QSqlDatabase &db, QWidget *parent = nullptr);
    ~StatisticsDialog();

private slots:
    void on_refreshButton_clicked();
    void on_generateReportButton_clicked();
    void on_exportReportButton_clicked();
    void on_closeButton_clicked();
    void on_reportStartDate_dateChanged(const QDate &date);
    void on_reportEndDate_dateChanged(const QDate &date);
    void on_tabWidget_currentChanged(int index);

private:
    Ui::StatisticsDialog *ui;
    QSqlDatabase &database;

    void updateStatistics();
    void updateCharts();
    void updateAnalysis();
    void generateReport();
    void exportReport();

    struct Statistics {
        int totalTasks = 0;
        int totalDates = 0;
        int totalNotes = 0;
        qint64 dbSize = 0;
        QMap<QString, int> tasksByMonth;
        QMap<QString, int> tasksByWeekday;
        double avgTasksPerDay = 0.0;
        QString mostActiveDate;
        int mostTasksInDay = 0;
    };

    Statistics calculateStatistics(const QDate &start, const QDate &end);
};

#endif // STATISTICSDIALOG_H
