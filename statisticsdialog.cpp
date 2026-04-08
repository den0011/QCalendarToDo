#include "statisticsdialog.h"
#include "ui_statisticsdialog.h"
#include "thememanager.h"
#include <QSqlQuery>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDir>
#include <QVBoxLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

StatisticsDialog::StatisticsDialog(QSqlDatabase &db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StatisticsDialog),
    database(db)
{
    ui->setupUi(this);

    // Применяем цвета карточек статистики с учётом темы
    bool isDark = (ThemeManager::instance().currentTheme() == ThemeManager::DarkTheme);
    QString colTasks  = isDark ? "#42A5F5" : "#2196F3";
    QString colDates  = isDark ? "#66BB6A" : "#4CAF50";
    QString colNotes  = isDark ? "#FFA726" : "#FF9800";
    QString colDbSize = isDark ? "#AB47BC" : "#9C27B0";
    ui->totalTasksLabel->setStyleSheet(QString("font-size: 36px; font-weight: bold; color: %1;").arg(colTasks));
    ui->totalDatesLabel->setStyleSheet(QString("font-size: 36px; font-weight: bold; color: %1;").arg(colDates));
    ui->totalNotesLabel->setStyleSheet(QString("font-size: 36px; font-weight: bold; color: %1;").arg(colNotes));
    ui->dbSizeLabel->setStyleSheet(QString("font-size: 36px; font-weight: bold; color: %1;").arg(colDbSize));

    // Устанавливаем начальные даты
    ui->reportStartDate->setDate(QDate::currentDate().addMonths(-1));
    ui->reportEndDate->setDate(QDate::currentDate());

    // Обновляем статистику при открытии
    updateStatistics();
}

StatisticsDialog::~StatisticsDialog()
{
    delete ui;
}

StatisticsDialog::Statistics StatisticsDialog::calculateStatistics(const QDate &start, const QDate &end)
{
    Statistics stats;

    // Получаем общее количество задач
    QSqlQuery query(database);
    query.exec("SELECT COUNT(*) FROM Times");
    if (query.next()) {
        stats.totalTasks = query.value(0).toInt();
    }

    // Получаем количество дат
    query.exec("SELECT COUNT(*) FROM Dates");
    if (query.next()) {
        stats.totalDates = query.value(0).toInt();
    }

    // Получаем количество заметок
    query.exec("SELECT COUNT(*) FROM Strings");
    if (query.next()) {
        stats.totalNotes = query.value(0).toInt();
    }

    // Получаем размер базы данных
    QFile dbFile("my_database.db");
    if (dbFile.exists()) {
        stats.dbSize = dbFile.size();
    } else {
        stats.dbSize = 0;
    }

    // Собираем статистику по месяцам
    stats.tasksByMonth.clear();
    query.exec(QString("SELECT substr(date, 1, 7) as month, COUNT(Times.id) "
              "FROM Dates LEFT JOIN Times ON Dates.id = Times.date_id "
              "WHERE date BETWEEN '%1' AND '%2' "
              "GROUP BY month ORDER BY month")
              .arg(start.toString("yyyy-MM-dd"))
              .arg(end.toString("yyyy-MM-dd")));
    while (query.next()) {
        QString month = query.value(0).toString();
        int count = query.value(1).toInt();
        stats.tasksByMonth[month] = count;
    }

    // Собираем статистику по дням недели
    stats.tasksByWeekday.clear();
    query.exec(QString("SELECT CAST(strftime('%w', date) AS INTEGER) as weekday, COUNT(Times.id) "
              "FROM Dates LEFT JOIN Times ON Dates.id = Times.date_id "
              "WHERE date BETWEEN '%1' AND '%2' "
              "GROUP BY weekday ORDER BY weekday")
              .arg(start.toString("yyyy-MM-dd"))
              .arg(end.toString("yyyy-MM-dd")));
    while (query.next()) {
        int weekday = query.value(0).toInt();
        int count = query.value(1).toInt();
        QString weekdayStr;
        switch (weekday) {
            case 0: weekdayStr = "Вс"; break;
            case 1: weekdayStr = "Пн"; break;
            case 2: weekdayStr = "Вт"; break;
            case 3: weekdayStr = "Ср"; break;
            case 4: weekdayStr = "Чт"; break;
            case 5: weekdayStr = "Пт"; break;
            case 6: weekdayStr = "Сб"; break;
            default: weekdayStr = QString::number(weekday);
        }
        stats.tasksByWeekday[weekdayStr] = count;
    }

    // Самый активный день
    query.exec(QString("SELECT Dates.date, COUNT(Times.id) as cnt "
              "FROM Dates LEFT JOIN Times ON Dates.id = Times.date_id "
              "WHERE date BETWEEN '%1' AND '%2' "
              "GROUP BY Dates.date ORDER BY cnt DESC LIMIT 1")
              .arg(start.toString("yyyy-MM-dd"))
              .arg(end.toString("yyyy-MM-dd")));
    if (query.next()) {
        stats.mostActiveDate = query.value(0).toString();
        stats.mostTasksInDay = query.value(1).toInt();
    }

    // Среднее количество задач в день
    query.exec(QString("SELECT COUNT(Times.id) as task_count, COUNT(DISTINCT Dates.date) as date_count "
              "FROM Dates LEFT JOIN Times ON Dates.id = Times.date_id "
              "WHERE date BETWEEN '%1' AND '%2'")
              .arg(start.toString("yyyy-MM-dd"))
              .arg(end.toString("yyyy-MM-dd")));
    if (query.next()) {
        int taskCount = query.value(0).toInt();
        int dateCount = query.value(1).toInt();
        if (dateCount > 0) {
            stats.avgTasksPerDay = static_cast<double>(taskCount) / dateCount;
        }
    }

    return stats;
}

void StatisticsDialog::updateStatistics()
{
    QDate endDate = QDate::currentDate();
    QDate startDate = endDate.addYears(-1);

    Statistics stats = calculateStatistics(startDate, endDate);

    ui->totalTasksLabel->setText(QString::number(stats.totalTasks));
    ui->totalDatesLabel->setText(QString::number(stats.totalDates));
    ui->totalNotesLabel->setText(QString::number(stats.totalNotes));

    QString sizeText;
    if (stats.dbSize < 1024) {
        sizeText = QString::number(stats.dbSize) + " Б";
    } else if (stats.dbSize < 1024 * 1024) {
        sizeText = QString::number(stats.dbSize / 1024.0, 'f', 2) + " КБ";
    } else {
        sizeText = QString::number(stats.dbSize / (1024.0 * 1024.0), 'f', 2) + " МБ";
    }
    ui->dbSizeLabel->setText(sizeText);

    QString detailText = "<h3>Детальная информация</h3>";
    detailText += QString("<p><b>Среднее задач в день:</b> %1</p>")
        .arg(QString::number(stats.avgTasksPerDay, 'f', 2));
    detailText += QString("<p><b>Самый активный день:</b> %1 (%2 задач)</p>")
        .arg(stats.mostActiveDate.isEmpty() ? "Нет данных" : stats.mostActiveDate)
        .arg(stats.mostTasksInDay);

    ui->generalStatsText->setHtml(detailText);

    updateCharts();
    updateAnalysis();
}

void StatisticsDialog::updateCharts()
{
    // Удаляем старые графики
    QLayoutItem* item;
    while ((item = ui->tasksChartWidget->layout()) != nullptr) {
        delete item;
    }
    while ((item = ui->activityChartWidget->layout()) != nullptr) {
        delete item;
    }

    // График задач по месяцам
    QtCharts::QBarSeries *series = new QtCharts::QBarSeries();
    QtCharts::QBarSet *set = new QtCharts::QBarSet("Задачи");

    QDate endDate = QDate::currentDate();
    QDate startDate = endDate.addMonths(-6);

    QStringList categories;

    for (int i = 0; i < 6; i++) {
        QDate monthDate = startDate.addMonths(i);
        QString monthKey = monthDate.toString("yyyy-MM");
        categories << monthDate.toString("MMM yyyy");

        QSqlQuery query(database);
        query.prepare("SELECT COUNT(Times.id) "
                     "FROM Dates LEFT JOIN Times ON Dates.id = Times.date_id "
                     "WHERE substr(date, 1, 7) = ?");
        query.addBindValue(monthKey);

        int count = 0;
        if (query.exec() && query.next()) {
            count = query.value(0).toInt();
        }

        *set << count;
    }

    series->append(set);

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle("Задачи за последние 6 месяцев");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    QtCharts::QBarCategoryAxis *axisX = new QtCharts::QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setTitleText("Количество задач");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *tasksLayout = new QVBoxLayout(ui->tasksChartWidget);
    tasksLayout->addWidget(chartView);
    ui->tasksChartWidget->setLayout(tasksLayout);

    // График по дням недели
    QtCharts::QBarSeries *weekSeries = new QtCharts::QBarSeries();
    QtCharts::QBarSet *weekSet = new QtCharts::QBarSet("Задачи");

    QStringList weekDays = {"Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};

    for (int i = 1; i <= 7; i++) {
        QSqlQuery query(database);
        query.prepare("SELECT COUNT(Times.id) "
                     "FROM Dates INNER JOIN Times ON Dates.id = Times.date_id "
                     "WHERE CAST(strftime('%w', date) AS INTEGER) = ?");
        query.addBindValue(i % 7);

        int count = 0;
        if (query.exec() && query.next()) {
            count = query.value(0).toInt();
        }

        *weekSet << count;
    }

    weekSeries->append(weekSet);

    QtCharts::QChart *weekChart = new QtCharts::QChart();
    weekChart->addSeries(weekSeries);
    weekChart->setTitle("Распределение задач по дням недели");
    weekChart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    QtCharts::QBarCategoryAxis *weekAxisX = new QtCharts::QBarCategoryAxis();
    weekAxisX->append(weekDays);
    weekAxisX->setTitleText("День недели");
    weekChart->addAxis(weekAxisX, Qt::AlignBottom);
    weekSeries->attachAxis(weekAxisX);

    QtCharts::QValueAxis *weekAxisY = new QtCharts::QValueAxis();
    weekAxisY->setTitleText("Количество задач");
    weekChart->addAxis(weekAxisY, Qt::AlignLeft);
    weekSeries->attachAxis(weekAxisY);

    QtCharts::QChartView *weekChartView = new QtCharts::QChartView(weekChart);
    weekChartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *activityLayout = new QVBoxLayout(ui->activityChartWidget);
    activityLayout->addWidget(weekChartView);
    ui->activityChartWidget->setLayout(activityLayout);
}

void StatisticsDialog::updateAnalysis()
{
    QString analysis = "<h3>Анализ продуктивности</h3>";

    QDate endDate = QDate::currentDate();
    QDate startDate = endDate.addMonths(-1);

    Statistics stats = calculateStatistics(startDate, endDate);

    if (stats.avgTasksPerDay >= 10) {
        analysis += "<p style='color: green;'><b>✓ Отлично!</b> Вы очень продуктивны, среднее количество задач в день высокое.</p>";
    } else if (stats.avgTasksPerDay >= 5) {
        analysis += "<p style='color: orange;'><b>○ Хорошо!</b> Средняя продуктивность, есть куда расти.</p>";
    } else {
        analysis += "<p style='color: red;'><b>✗ Низкая активность</b> Попробуйте ставить больше задач.</p>";
    }

    analysis += "<h4>Рекомендации:</h4><ul>";

    if (stats.totalNotes < stats.totalDates / 2) {
        analysis += "<li>Попробуйте чаще делать заметки - это помогает не забывать важные детали</li>";
    }

    if (stats.avgTasksPerDay < 3) {
        analysis += "<li>Разбивайте большие задачи на подзадачи - так проще контролировать прогресс</li>";
    }

    if (stats.mostTasksInDay > stats.avgTasksPerDay * 3) {
        analysis += "<li>Распределяйте задачи более равномерно по дням</li>";
    }

    analysis += "<li>Регулярно делайте резервные копии данных</li>";
    analysis += "<li>Используйте поиск для быстрого доступа к старым задачам</li>";
    analysis += "</ul>";

    ui->analysisText->setHtml(analysis);
}

void StatisticsDialog::generateReport()
{
    QDate start = ui->reportStartDate->date();
    QDate end = ui->reportEndDate->date();

    if (start > end) {
        QMessageBox::warning(this, "Ошибка", "Начальная дата должна быть раньше конечной!");
        return;
    }

    Statistics stats = calculateStatistics(start, end);

    QString report = "<html><head><style>";
    report += "body { font-family: Arial, sans-serif; margin: 20px; }";
    report += "h1 { color: #2196F3; }";
    report += "h2 { color: #4CAF50; border-bottom: 2px solid #4CAF50; padding-bottom: 5px; }";
    report += "table { border-collapse: collapse; width: 100%; margin: 20px 0; }";
    report += "th { background-color: #2196F3; color: white; padding: 10px; text-align: left; }";
    report += "td { border: 1px solid #ddd; padding: 8px; }";
    report += "tr:nth-child(even) { background-color: #f2f2f2; }";
    report += ".stats { display: flex; justify-content: space-between; margin: 20px 0; }";
    report += ".stat-card { background-color: white; border: 1px solid #ddd; border-radius: 5px; padding: 15px; text-align: center; flex: 1; margin: 0 10px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }";
    report += ".stat-value { font-size: 24px; font-weight: bold; color: #2196F3; }";
    report += ".stat-label { color: #666; margin-top: 5px; }";
    report += "</style></head><body>";

    report += QString("<h1>Отчёт за период с %1 по %2</h1>")
        .arg(start.toString("dd.MM.yyyy"))
        .arg(end.toString("dd.MM.yyyy"));

    report += "<div class='stats'>";
    report += QString("<div class='stat-card'><div class='stat-value'>%1</div><div class='stat-label'>Задач</div></div>")
        .arg(stats.totalTasks);
    report += QString("<div class='stat-card'><div class='stat-value'>%1</div><div class='stat-label'>Дней</div></div>")
        .arg(stats.totalDates);
    report += QString("<div class='stat-card'><div class='stat-value'>%1</div><div class='stat-label'>Заметок</div></div>")
        .arg(stats.totalNotes);
    report += QString("<div class='stat-card'><div class='stat-value'>%1</div><div class='stat-label'>Среднее в день</div></div>")
        .arg(QString::number(stats.avgTasksPerDay, 'f', 2));
    report += "</div>";

    report += "<h2>Детальный список задач</h2>";

    QSqlQuery query(database);
    query.prepare("SELECT Dates.date, Times.time, Times.info1, Times.info2, Times.info3 "
                 "FROM Times INNER JOIN Dates ON Times.date_id = Dates.id "
                 "WHERE Dates.date BETWEEN ? AND ? "
                 "ORDER BY Dates.date, Times.time");
    query.addBindValue(start.toString("yyyy-MM-dd"));
    query.addBindValue(end.toString("yyyy-MM-dd"));

    if (query.exec()) {
        report += "<table>";
        report += "<tr><th>Дата</th><th>Время</th><th>Заголовок</th><th>Сообщение</th><th>Описание</th></tr>";

        int taskCount = 0;
        while (query.next()) {
            taskCount++;
            report += "<tr>";
            report += QString("<td>%1</td>").arg(query.value(0).toString());
            report += QString("<td>%1</td>").arg(query.value(1).toString());
            report += QString("<td>%1</td>").arg(query.value(2).toString().toHtmlEscaped());
            report += QString("<td>%1</td>").arg(query.value(3).toString().toHtmlEscaped());
            report += QString("<td>%1</td>").arg(query.value(4).toString().toHtmlEscaped());
            report += "</tr>";
        }

        if (taskCount == 0) {
            report += "<tr><td colspan='5' style='text-align: center; padding: 20px;'>Нет задач за выбранный период</td></tr>";
        }

        report += "</table>";
    }

    report += QString("<p><i>Отчёт сформирован: %1</i></p>")
        .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"));
    report += "</body></html>";

    ui->reportText->setHtml(report);
}

void StatisticsDialog::exportReport()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Экспорт отчёта",
        QDir::homePath() + "/report_" + QDate::currentDate().toString("yyyy-MM-dd") + ".html",
        "HTML Files (*.html);;All Files (*)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        out.setCodec("UTF-8");
#endif
        out << ui->reportText->toHtml();
        file.close();

        QMessageBox::information(this, "Успех", "Отчёт успешно экспортирован!");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл!");
    }
}

// Слоты
void StatisticsDialog::on_refreshButton_clicked()
{
    updateStatistics();
}

void StatisticsDialog::on_generateReportButton_clicked()
{
    generateReport();
}

void StatisticsDialog::on_exportReportButton_clicked()
{
    exportReport();
}

void StatisticsDialog::on_closeButton_clicked()
{
    accept();
}

void StatisticsDialog::on_reportStartDate_dateChanged(const QDate &date)
{
    Q_UNUSED(date);
}

void StatisticsDialog::on_reportEndDate_dateChanged(const QDate &date)
{
    Q_UNUSED(date);
}

void StatisticsDialog::on_tabWidget_currentChanged(int index)
{
    Q_UNUSED(index);
    if (ui->tabWidget->currentIndex() == 1) { // Графики
        updateCharts();
    }
}
