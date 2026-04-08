#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSet>
#include <QSqlQuery>

// ── CalendarDelegate ──────────────────────────────────────────────────────────
void CalendarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    bool isToday    = index.data(Qt::UserRole).toBool();
    bool isWeekend  = index.data(Qt::UserRole + 1).toBool();
    bool hasTasks   = index.data(Qt::UserRole + 2).toBool();

    if (isToday) {
        painter->save();
        painter->fillRect(option.rect, ThemeManager::instance().accentColor());
        QFont f = option.font;
        f.setBold(true);
        painter->setFont(f);
        painter->setPen(Qt::white);
        QRect textRect = option.rect.adjusted(0, 0, 0, hasTasks ? -6 : 0);
        painter->drawText(textRect, Qt::AlignCenter, index.data().toString());
        if (hasTasks) {
            painter->setBrush(Qt::white);
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(option.rect.center().x() - 2, option.rect.bottom() - 6, 4, 4);
        }
        painter->restore();
    } else {
        if (isWeekend) {
            painter->save();
            painter->fillRect(option.rect, QColor(220, 80, 80, 35));
            painter->restore();
        }
        QStyledItemDelegate::paint(painter, option, index);
        if (hasTasks && !index.data().toString().isEmpty()) {
            painter->save();
            painter->setBrush(ThemeManager::instance().accentColor());
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(option.rect.center().x() - 2, option.rect.bottom() - 6, 4, 4);
            painter->restore();
        }
    }
}

// ── TaskDelegate ──────────────────────────────────────────────────────────────
void TaskDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    bool isNow = index.data(Qt::UserRole).toBool();

    // Явно рисуем фон из BackgroundRole — минуем ограничение QSS
    QVariant bgData = index.data(Qt::BackgroundRole);
    if (bgData.isValid() && !(option.state & QStyle::State_Selected)) {
        QBrush bgBrush = bgData.value<QBrush>();
        if (bgBrush.style() != Qt::NoBrush) {
            painter->fillRect(option.rect, bgBrush);
            opt.backgroundBrush = QBrush(Qt::NoBrush);
        }
    }

    QStyledItemDelegate::paint(painter, opt, index);

    // Линия-индикатор текущего времени — рисуем в каждой ячейке строки
    if (isNow) {
        painter->save();
        QPen pen(ThemeManager::instance().accentColor(), 2);
        painter->setPen(pen);
        painter->drawLine(option.rect.left(), option.rect.top(),
                          option.rect.right(), option.rect.top());
        painter->restore();
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    isLoadingData(false)
{
    ui->setupUi(this);

    trayIcon = new QSystemTrayIcon(QIcon(":/png/res/calendar.png"), this);
    ui->mainToolBar->setMovable(false);

    configFilePath = "config.ini";
    QSettings appSettings(configFilePath, QSettings::IniFormat);

    settings.load(appSettings);
    settings.printValues();

    // Загружаем тему
    ThemeManager::instance().loadTheme(appSettings);
    ThemeManager::instance().applyTheme(ThemeManager::instance().currentTheme());

    setParametrSettings();
    createPiChart();
    createCalendar();
    setupUITableToDo();
    setupSaveIndicator();
    updateCellColors();

    currentSelectedDate = QDate::currentDate();
    loadTasksForDate(currentSelectedDate);
    loadNotesForDate(currentSelectedDate);
    refreshDbStats();
    scrollToCurrentTime();

    connect(ui->prevMonthButton, &QPushButton::clicked, this, &MainWindow::onPrevMonth);
    connect(ui->nextMonthButton, &QPushButton::clicked, this, &MainWindow::onNextMonth);
    connect(ui->todayButton,     &QPushButton::clicked, this, &MainWindow::onTodayClicked);

    QObject::connect(&timeUdate, &QTimer::timeout, this, &MainWindow::updateCellColors);
    timeUdate.start(1000);

    if (settings.messages) {
        trayIcon->show();
        trayIcon->showMessage("Включены всплывающие сообщения",
                             "Отключить можно в настройках",
                             QIcon(":/png/res/calendar.png"), 5000);
    }

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(quitApp()));
    connect(ui->tableWidgetToDo, &QTableWidget::itemChanged,
            this, &MainWindow::onTableItemChanged);

    // Создаём меню
    QMenu *fileMenu = ui->menuBar->addMenu("Файл");
    QAction *backupAction = new QAction("Создать резервную копию...", this);
    connect(backupAction, &QAction::triggered, this, &MainWindow::onBackupCreate);
    fileMenu->addAction(backupAction);

    QAction *restoreAction = new QAction("Восстановить из резервной копии...", this);
    connect(restoreAction, &QAction::triggered, this, &MainWindow::onBackupRestore);
    fileMenu->addAction(restoreAction);

    fileMenu->addSeparator();

    QAction *exportAction = new QAction("Экспорт в CSV...", this);
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportCSV);
    fileMenu->addAction(exportAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction("Выход", this);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(exitAction);

    QMenu *searchMenu = ui->menuBar->addMenu("Поиск");
    QAction *searchAction = new QAction("Найти задачу...", this);
    searchAction->setShortcut(QKeySequence("Ctrl+F"));
    connect(searchAction, &QAction::triggered, this, &MainWindow::onSearchTriggered);
    searchMenu->addAction(searchAction);

    QMenu *statsMenu = ui->menuBar->addMenu("Статистика");
    QAction *statsAction = new QAction("Статистика и отчёты...", this);
    statsAction->setShortcut(QKeySequence("Ctrl+T"));
    connect(statsAction, &QAction::triggered, this, &MainWindow::onStatisticsTriggered);
    statsMenu->addAction(statsAction);

    QMenu *settingsMenu = ui->menuBar->addMenu("Настройки");
    QAction *settingsAction = new QAction("Настройки...", this);
    settingsAction->setShortcut(QKeySequence("Ctrl+,"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettingsTriggered);
    settingsMenu->addAction(settingsAction);

    settingsMenu->addSeparator();

    // Подменю тем
    QMenu *themeMenu = settingsMenu->addMenu("Тема");

    QActionGroup *themeGroup = new QActionGroup(this);

    QAction *lightThemeAction = new QAction("Светлая", this);
    lightThemeAction->setCheckable(true);
    lightThemeAction->setActionGroup(themeGroup);
    connect(lightThemeAction, &QAction::triggered, this, &MainWindow::onLightTheme);
    themeMenu->addAction(lightThemeAction);

    QAction *darkThemeAction = new QAction("Тёмная", this);
    darkThemeAction->setCheckable(true);
    darkThemeAction->setActionGroup(themeGroup);
    connect(darkThemeAction, &QAction::triggered, this, &MainWindow::onDarkTheme);
    themeMenu->addAction(darkThemeAction);

    QAction *blueThemeAction = new QAction("Синяя", this);
    blueThemeAction->setCheckable(true);
    blueThemeAction->setActionGroup(themeGroup);
    connect(blueThemeAction, &QAction::triggered, this, &MainWindow::onBlueTheme);
    themeMenu->addAction(blueThemeAction);

    QAction *greenThemeAction = new QAction("Зелёная", this);
    greenThemeAction->setCheckable(true);
    greenThemeAction->setActionGroup(themeGroup);
    connect(greenThemeAction, &QAction::triggered, this, &MainWindow::onGreenTheme);
    themeMenu->addAction(greenThemeAction);

    // Устанавливаем текущую тему как выбранную
    switch (ThemeManager::instance().currentTheme()) {
        case ThemeManager::DarkTheme: darkThemeAction->setChecked(true); break;
        case ThemeManager::BlueTheme: blueThemeAction->setChecked(true); break;
        case ThemeManager::GreenTheme: greenThemeAction->setChecked(true); break;
        default: lightThemeAction->setChecked(true); break;
    }

    searchDialog = nullptr;
    statisticsDialog = nullptr;
    settingsDialog = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupSaveIndicator()
{
    saveStatusLabel = new QLabel(this);
    ui->statusBar->addPermanentWidget(saveStatusLabel);

    saveStatusTimer = new QTimer(this);
    saveStatusTimer->setSingleShot(true);
    connect(saveStatusTimer, &QTimer::timeout, this, &MainWindow::hideSaveStatus);
}

void MainWindow::showSaveStatus(const QString &message, bool success)
{
    if (success) {
        saveStatusLabel->setStyleSheet("QLabel { color: green; padding: 5px; font-weight: bold; }");
        saveStatusLabel->setText("✓ " + message);
    } else {
        saveStatusLabel->setStyleSheet("QLabel { color: red; padding: 5px; font-weight: bold; }");
        saveStatusLabel->setText("✗ " + message);
    }
    saveStatusTimer->start(3000);
}

void MainWindow::hideSaveStatus()
{
    saveStatusLabel->setText("");
}

void MainWindow::onTableItemChanged(QTableWidgetItem *item)
{
    if (!isLoadingData && item) {
        saveTasksForCurrentDate();
        refreshDbStats();
        showSaveStatus("Автосохранение выполнено", true);
    }
}

void MainWindow::saveTasksForCurrentDate()
{
    QString dateString = currentSelectedDate.toString("yyyy-MM-dd");
    manager.deleteData(dateString);

    int savedCount = 0;
    for (int row = 0; row < ui->tableWidgetToDo->rowCount(); ++row) {
        QTableWidgetItem *itemTitle = ui->tableWidgetToDo->item(row, 0);
        QTableWidgetItem *itemMessage = ui->tableWidgetToDo->item(row, 1);
        QTableWidgetItem *itemDescription = ui->tableWidgetToDo->item(row, 2);

        QString title = itemTitle ? itemTitle->text() : "";
        QString message = itemMessage ? itemMessage->text() : "";
        QString description = itemDescription ? itemDescription->text() : "";

        if (!title.isEmpty() || !message.isEmpty() || !description.isEmpty()) {
            QTableWidgetItem *headerItem = ui->tableWidgetToDo->verticalHeaderItem(row);
            QString time = headerItem ? headerItem->text() : "";

            manager.ensureDateExists(dateString);

            QSqlQuery insertQuery(manager.getDatabase());
            insertQuery.prepare("INSERT INTO Times (date_id, time, info1, info2, info3) "
                              "VALUES ((SELECT id FROM Dates WHERE date = ?), ?, ?, ?, ?)");
            insertQuery.addBindValue(dateString);
            insertQuery.addBindValue(time);
            insertQuery.addBindValue(title);
            insertQuery.addBindValue(message);
            insertQuery.addBindValue(description);

            if (insertQuery.exec()) {
                savedCount++;
            }
        }
    }
}

void MainWindow::loadTasksForDate(const QDate &date)
{
    isLoadingData = true;
    QString dateString = date.toString("yyyy-MM-dd");

    for (int row = 0; row < ui->tableWidgetToDo->rowCount(); ++row) {
        if (ui->tableWidgetToDo->item(row, 0))
            ui->tableWidgetToDo->item(row, 0)->setText("");
        if (ui->tableWidgetToDo->item(row, 1))
            ui->tableWidgetToDo->item(row, 1)->setText("");
        if (ui->tableWidgetToDo->item(row, 2))
            ui->tableWidgetToDo->item(row, 2)->setText("");
    }

    QSqlQuery query(manager.getDatabase());
    query.prepare("SELECT time, info1, info2, info3 FROM Times "
                 "WHERE date_id IN (SELECT id FROM Dates WHERE date = ?)");
    query.addBindValue(dateString);

    if (query.exec()) {
        while (query.next()) {
            QString time = query.value(0).toString();
            QString info1 = query.value(1).toString();
            QString info2 = query.value(2).toString();
            QString info3 = query.value(3).toString();

            for (int row = 0; row < ui->tableWidgetToDo->rowCount(); ++row) {
                QTableWidgetItem *headerItem = ui->tableWidgetToDo->verticalHeaderItem(row);
                if (headerItem && headerItem->text() == time) {
                    if (ui->tableWidgetToDo->item(row, 0))
                        ui->tableWidgetToDo->item(row, 0)->setText(info1);
                    if (ui->tableWidgetToDo->item(row, 1))
                        ui->tableWidgetToDo->item(row, 1)->setText(info2);
                    if (ui->tableWidgetToDo->item(row, 2))
                        ui->tableWidgetToDo->item(row, 2)->setText(info3);
                    break;
                }
            }
        }
    }
    isLoadingData = false;
}

void MainWindow::loadNotesForDate(const QDate &date)
{
    QString dateString = date.toString("yyyy-MM-dd");
    QSqlQuery query(manager.getDatabase());
    query.prepare("SELECT value FROM Strings "
                 "WHERE date_id IN (SELECT id FROM Dates WHERE date = ?)");
    query.addBindValue(dateString);

    if (query.exec() && query.next()) {
        ui->plainTextEditNotes->setPlainText(query.value(0).toString());
    } else {
        ui->plainTextEditNotes->clear();
    }
}

void MainWindow::saveNotesForCurrentDate()
{
    QString dateString = currentSelectedDate.toString("yyyy-MM-dd");
    QString notes = ui->plainTextEditNotes->toPlainText();
    manager.updateString(dateString, notes);
}

void MainWindow::quitApp()
{
    saveTasksForCurrentDate();
    saveNotesForCurrentDate();

    QSettings appSettings(configFilePath, QSettings::IniFormat);
    settings.save(appSettings);
    ThemeManager::instance().saveTheme(appSettings);
}

void MainWindow::setParametrSettings()
{
    ui->tableWidgetToDo->setFont(settings.tasks);
    ui->plainTextEditNotes->setFont(settings.notes);
}

void MainWindow::setupUITableToDo()
{
    ui->tableWidgetToDo->setItemDelegate(new TaskDelegate(this));
    ui->tableWidgetToDo->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidgetToDo->setAlternatingRowColors(true);
    ui->tableWidgetToDo->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidgetToDo, &QTableWidget::customContextMenuRequested,
            this, &MainWindow::onTaskTableContextMenu);
    ui->tableWidgetToDo->setRowCount(96);
    ui->tableWidgetToDo->setColumnCount(3);

    QTime currentTime(0, 0);
    for (int row = 0; row < 96; ++row) {
        ui->tableWidgetToDo->setVerticalHeaderItem(row, new QTableWidgetItem(currentTime.toString("hh:mm")));
        currentTime = currentTime.addSecs(15 * 60);
    }

    for (int row = 0; row < 96; ++row) {
        QTableWidgetItem *item1 = new QTableWidgetItem;
        QTableWidgetItem *item2 = new QTableWidgetItem;
        QTableWidgetItem *item3 = new QTableWidgetItem;
        ui->tableWidgetToDo->setItem(row, 0, item1);
        ui->tableWidgetToDo->setItem(row, 1, item2);
        ui->tableWidgetToDo->setItem(row, 2, item3);
        item1->setTextAlignment(Qt::AlignCenter);
        item2->setTextAlignment(Qt::AlignCenter);
        item3->setTextAlignment(Qt::AlignCenter);
    }

    ui->tableWidgetToDo->verticalHeader()->setDefaultSectionSize(30);
    QStringList headerLabels;
    headerLabels << "Заголовок" << "Сообщение" << "Описание";
    ui->tableWidgetToDo->setHorizontalHeaderLabels(headerLabels);
    ui->tableWidgetToDo->setShowGrid(false);
    ui->tableWidgetToDo->setColumnWidth(0, 100);
    ui->tableWidgetToDo->setColumnWidth(1, 100);
    ui->tableWidgetToDo->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::updateCellColors()
{
    QTime currentTime = QTime::currentTime();
    int done = 0, progress = 0;
    int currentRow = currentTime.hour() * 4 + currentTime.minute() / 15;

    for (int row = 0; row < 96; ++row) {
        QTableWidgetItem *item1 = ui->tableWidgetToDo->item(row, 0);
        QTableWidgetItem *item2 = ui->tableWidgetToDo->item(row, 1);
        QTableWidgetItem *item3 = ui->tableWidgetToDo->item(row, 2);
        if (item1 && item2) {
            QTableWidgetItem *headerItem = ui->tableWidgetToDo->verticalHeaderItem(row);
            QTime cellTime = QTime::fromString(headerItem->text(), "hh:mm");
            bool isPast = (cellTime <= currentTime);
            if (isPast) done++; else progress++;

            bool hasContent = !item1->text().isEmpty() || !item2->text().isEmpty()
                              || (item3 && !item3->text().isEmpty());
            QBrush rowBrush;
            if (hasContent) {
                rowBrush = isPast ? settings.done : settings.progress;
            } else if (isPast) {
                QColor tint = settings.done;
                tint.setAlpha(55);
                rowBrush = tint;
            }
            // future + empty: rowBrush остаётся дефолтным (alternating)

            if (rowBrush.style() != Qt::NoBrush) {
                item1->setBackground(rowBrush);
                item2->setBackground(rowBrush);
                if (item3) item3->setBackground(rowBrush);
            } else {
                item1->setData(Qt::BackgroundRole, QVariant());
                item2->setData(Qt::BackgroundRole, QVariant());
                if (item3) item3->setData(Qt::BackgroundRole, QVariant());
            }

            // Маркер строки «сейчас» для делегата
            bool isNow = (row == currentRow && currentSelectedDate == QDate::currentDate());
            item1->setData(Qt::UserRole, isNow);
            item2->setData(Qt::UserRole, isNow);
            if (item3) item3->setData(Qt::UserRole, isNow);

            if (row > lastDisplayedRow && isPast) {
                if (!item1->text().isEmpty() && !item2->text().isEmpty() && settings.messages) {
                    trayIcon->show();
                    trayIcon->showMessage(item1->text(), item2->text(),
                                         QIcon(":/png/res/calendar.png"), settings.sec*1000);
                }
                lastDisplayedRow = row;
            }
        }
    }

    pieSeries->slices().at(0)->setValue(done);
    pieSeries->slices().at(1)->setValue(progress);
    pieSeries->slices().at(0)->setLabel(QString("Завершено: %1 инт.").arg(done));
    pieSeries->slices().at(1)->setLabel(QString("В процессе: %1 инт.").arg(progress));
    pieSeries->slices().at(0)->setColor(settings.done);
    pieSeries->slices().at(1)->setColor(settings.progress);

    updateStatsPanel(done, progress);
}

void MainWindow::createPiChart()
{
    pieSeries = new QtCharts::QPieSeries();
    pieSeries->append("Завершено", 30);
    pieSeries->append("В процессе", 50);

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->addSeries(pieSeries);
    chart->legend()->setAlignment(Qt::AlignRight);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layout = new QVBoxLayout(ui->widgetPiCharts);
    layout->addWidget(chartView);
    ui->widgetPiCharts->setLayout(layout);
}

void MainWindow::createCalendar()
{
    QDate currentDate = QDate::currentDate();
    currentYear = currentDate.year();
    currentMonth = currentDate.month();

    ui->dateEdit->setDate(currentDate);
    ui->dateEdit->setCalendarPopup(true);
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &MainWindow::onDateChanged);

    ui->monthLabel->setAlignment(Qt::AlignCenter);
    ui->tableWidgetCalendar->setRowCount(6);
    ui->tableWidgetCalendar->setColumnCount(7);
    ui->tableWidgetCalendar->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidgetCalendar->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidgetCalendar->setItemDelegate(new CalendarDelegate(this));
    ui->plainTextEditNotes->setPlaceholderText("Введите заметку для выбранного дня...");

    QStringList daysOfWeek = {"Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};
    ui->tableWidgetCalendar->setHorizontalHeaderLabels(daysOfWeek);

    updateCalendar();

    connect(ui->tableWidgetCalendar, &QTableWidget::cellClicked, this, &MainWindow::onCellClicked);
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &MainWindow::onDateEditChanged);
}

void MainWindow::highlightCurrentDay()
{
    QDate today = QDate::currentDate();

    // Сбрасываем UserRole у всех ячеек
    for (int row = 0; row < ui->tableWidgetCalendar->rowCount(); ++row) {
        for (int col = 0; col < ui->tableWidgetCalendar->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidgetCalendar->item(row, col);
            if (item) {
                QFont font = item->font();
                font.setBold(false);
                item->setFont(font);
                item->setData(Qt::UserRole, false);
            }
        }
    }

    // Помечаем сегодняшний день через UserRole — CalendarDelegate нарисует акцентный цвет
    if (today.year() == currentYear && today.month() == currentMonth) {
        QDate date(currentYear, currentMonth, 1);
        int dayOfWeek = date.dayOfWeek();
        int day = today.day();

        int row = (day + dayOfWeek - 2) / 7;
        int col = (day + dayOfWeek - 2) % 7;

        QTableWidgetItem *item = ui->tableWidgetCalendar->item(row, col);
        if (item) {
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            item->setData(Qt::UserRole, true);
        }
    }
}

void MainWindow::updateCalendar()
{
    QDate date(currentYear, currentMonth, 1);
    int dayOfWeek = date.dayOfWeek();
    int daysInMonth = date.daysInMonth();

    ui->tableWidgetCalendar->clearContents();

    // Дни текущего месяца с задачами
    QSet<int> daysWithTasks;
    {
        QString monthPrefix = QString("%1-%2-")
            .arg(currentYear).arg(currentMonth, 2, 10, QChar('0'));
        QSqlQuery q(manager.getDatabase());
        q.prepare("SELECT date FROM Dates WHERE date LIKE ?");
        q.addBindValue(monthPrefix + "%");
        if (q.exec()) {
            while (q.next()) {
                QDate d = QDate::fromString(q.value(0).toString(), "yyyy-MM-dd");
                if (d.isValid()) daysWithTasks.insert(d.day());
            }
        }
    }

    for (int day = 1; day <= daysInMonth; ++day) {
        int row = (day + dayOfWeek - 2) / 7;
        int col = (day + dayOfWeek - 2) % 7;
        QTableWidgetItem *item = new QTableWidgetItem(QString::number(day));
        item->setTextAlignment(Qt::AlignCenter);
        item->setData(Qt::UserRole,     false);                      // isToday (highlightCurrentDay заполнит)
        item->setData(Qt::UserRole + 1, col >= 5);                   // isWeekend (Сб=5, Вс=6)
        item->setData(Qt::UserRole + 2, daysWithTasks.contains(day)); // hasTasks
        ui->tableWidgetCalendar->setItem(row, col, item);
    }

    highlightCurrentDay();
    ui->tableWidgetCalendar->setCurrentCell(-1, -1);
    ui->monthLabel->setText(date.toString("MMMM yyyy"));
}

void MainWindow::onDateEditChanged(const QDate &newDate)
{
    for (int row = 0; row < ui->tableWidgetCalendar->rowCount(); ++row) {
        for (int col = 0; col < ui->tableWidgetCalendar->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidgetCalendar->item(row, col);
            if (item) {
                int day = item->text().toInt();
                if (QDate(currentYear, currentMonth, day) == newDate) {
                    ui->tableWidgetCalendar->setCurrentCell(row, col);
                    return;
                }
            }
        }
    }
}

void MainWindow::onCellClicked(int row, int col)
{
    QTableWidgetItem *item = ui->tableWidgetCalendar->item(row, col);
    if (item) {
        int day = item->text().toInt();
        ui->dateEdit->setDate(QDate(currentYear, currentMonth, day));
    }
}

void MainWindow::onDateChanged(const QDate &newDate)
{
    saveTasksForCurrentDate();
    saveNotesForCurrentDate();
    showSaveStatus("Данные сохранены", true);

    currentSelectedDate = newDate;
    currentYear = newDate.year();
    currentMonth = newDate.month();

    loadTasksForDate(newDate);
    loadNotesForDate(newDate);
    updateCalendar();
    refreshDbStats();
    scrollToCurrentTime();
    lastDisplayedRow = -1;
}

// Резервное копирование
bool MainWindow::createBackup(const QString &filePath)
{
    QString dbPath = "my_database.db";
    if (!QFile::exists(dbPath)) {
        QMessageBox::warning(this, "Ошибка", "База данных не найдена!");
        return false;
    }

    if (QFile::exists(filePath)) QFile::remove(filePath);

    if (QFile::copy(dbPath, filePath)) {
        showSaveStatus("Резервная копия создана: " + QFileInfo(filePath).fileName(), true);
        QMessageBox::information(this, "Успех", "Резервная копия успешно создана!\n\nФайл: " + filePath);
        return true;
    } else {
        showSaveStatus("Ошибка создания резервной копии", false);
        QMessageBox::critical(this, "Ошибка", "Не удалось создать резервную копию!");
        return false;
    }
}

bool MainWindow::restoreBackup(const QString &filePath)
{
    if (!QFile::exists(filePath)) {
        QMessageBox::warning(this, "Ошибка", "Файл резервной копии не найден!");
        return false;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение",
        "Вы уверены, что хотите восстановить данные из резервной копии?\n\nТекущие данные будут перезаписаны!",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return false;

    QString dbPath = "my_database.db";
    manager.close(); // закрываем соединение перед заменой файла
    if (QFile::exists(dbPath)) QFile::remove(dbPath);

    if (QFile::copy(filePath, dbPath)) {
        manager.reset();
        loadTasksForDate(currentSelectedDate);
        loadNotesForDate(currentSelectedDate);
        showSaveStatus("Данные восстановлены из резервной копии", true);
        QMessageBox::information(this, "Успех", "Данные успешно восстановлены из резервной копии!");
        return true;
    } else {
        showSaveStatus("Ошибка восстановления из резервной копии", false);
        QMessageBox::critical(this, "Ошибка", "Не удалось восстановить данные из резервной копии!");
        return false;
    }
}

bool MainWindow::exportToCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл для экспорта!");
        return false;
    }

    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif
    out << "Дата;Время;Заголовок;Сообщение;Описание\n";

    QSqlQuery datesQuery(manager.getDatabase());
    datesQuery.exec("SELECT date FROM Dates ORDER BY date");

    while (datesQuery.next()) {
        QString date = datesQuery.value(0).toString();
        QSqlQuery tasksQuery(manager.getDatabase());
        tasksQuery.prepare("SELECT time, info1, info2, info3 FROM Times "
                          "WHERE date_id IN (SELECT id FROM Dates WHERE date = ?) ORDER BY time");
        tasksQuery.addBindValue(date);

        if (tasksQuery.exec()) {
            while (tasksQuery.next()) {
                QString time = tasksQuery.value(0).toString();
                QString info1 = tasksQuery.value(1).toString();
                QString info2 = tasksQuery.value(2).toString();
                QString info3 = tasksQuery.value(3).toString();

                info1.replace("\"", "\"\"");
                info2.replace("\"", "\"\"");
                info3.replace("\"", "\"\"");

                out << date << ";" << time << ";\"" << info1 << "\";\"" << info2 << "\";\"" << info3 << "\"\n";
            }
        }
    }

    out << "\n\n=== ЗАМЕТКИ ===\nДата;Заметка\n";
    QSqlQuery notesQuery(manager.getDatabase());
    notesQuery.exec("SELECT Dates.date, Strings.value FROM Strings "
                   "INNER JOIN Dates ON Strings.date_id = Dates.id ORDER BY Dates.date");

    while (notesQuery.next()) {
        QString date = notesQuery.value(0).toString();
        QString note = notesQuery.value(1).toString();
        note.replace("\"", "\"\"");
        note.replace("\n", " ");
        out << date << ";\"" << note << "\"\n";
    }

    file.close();
    showSaveStatus("Данные экспортированы в CSV", true);
    QMessageBox::information(this, "Успех", "Данные успешно экспортированы в CSV!\n\nФайл: " + filePath);
    return true;
}

void MainWindow::onBackupCreate()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Создать резервную копию",
        QDir::homePath() + "/calendar_backup_" + QDate::currentDate().toString("yyyy-MM-dd") + ".db",
        "Database Files (*.db);;All Files (*)");
    if (!fileName.isEmpty()) createBackup(fileName);
}

void MainWindow::onBackupRestore()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Восстановить из резервной копии",
        QDir::homePath(), "Database Files (*.db);;All Files (*)");
    if (!fileName.isEmpty()) restoreBackup(fileName);
}

void MainWindow::onExportCSV()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт в CSV",
        QDir::homePath() + "/calendar_export_" + QDate::currentDate().toString("yyyy-MM-dd") + ".csv",
        "CSV Files (*.csv);;All Files (*)");
    if (!fileName.isEmpty()) exportToCSV(fileName);
}

// Поиск
void MainWindow::onSearchTriggered()
{
    if (!searchDialog) {
        searchDialog = new SearchDialog(manager.getDatabase(), this);
        connect(searchDialog, &SearchDialog::dateSelected, this, &MainWindow::onSearchDateSelected);
    }
    searchDialog->show();
    searchDialog->raise();
    searchDialog->activateWindow();
}

void MainWindow::onSearchDateSelected(const QDate &date)
{
    ui->dateEdit->setDate(date);
    currentSelectedDate = date;
    currentYear = date.year();
    currentMonth = date.month();
    loadTasksForDate(date);
    loadNotesForDate(date);
    updateCalendar();
    onDateEditChanged(date);
    showSaveStatus("Перешли к дате: " + date.toString("dd.MM.yyyy"), true);
}

void MainWindow::onStatisticsTriggered()
{
    if (!statisticsDialog) {
        statisticsDialog = new StatisticsDialog(manager.getDatabase(), this);
    }
    statisticsDialog->show();
    statisticsDialog->raise();
    statisticsDialog->activateWindow();
}

void MainWindow::onSettingsTriggered()
{
    if (!settingsDialog) {
        settingsDialog = new SettingsDialog(settings, this);
        connect(settingsDialog, &SettingsDialog::settingsChanged, this, &MainWindow::applySettings);
    }
    settingsDialog->show();
    settingsDialog->raise();
    settingsDialog->activateWindow();
}

void MainWindow::applySettings()
{
    if (settingsDialog) {
        settings = settingsDialog->getSettings();
        setParametrSettings();
        updateCellColors();

        QSettings appSettings(configFilePath, QSettings::IniFormat);
        settings.save(appSettings);

        showSaveStatus("Настройки применены", true);
    }
}

// Методы переключения тем
void MainWindow::onLightTheme()
{
    ThemeManager::instance().applyTheme(ThemeManager::LightTheme);
    showSaveStatus("Применена светлая тема", true);
}

void MainWindow::onDarkTheme()
{
    ThemeManager::instance().applyTheme(ThemeManager::DarkTheme);
    showSaveStatus("Применена тёмная тема", true);
}

void MainWindow::onBlueTheme()
{
    ThemeManager::instance().applyTheme(ThemeManager::BlueTheme);
    showSaveStatus("Применена синяя тема", true);
}

void MainWindow::onGreenTheme()
{
    ThemeManager::instance().applyTheme(ThemeManager::GreenTheme);
    showSaveStatus("Применена зелёная тема", true);
}

void MainWindow::onTaskTableContextMenu(const QPoint &pos)
{
    int row = ui->tableWidgetToDo->rowAt(pos.y());
    if (row < 0) return;

    QMenu menu(this);
    QAction *clearAction = menu.addAction("Очистить строку");
    QAction *copyAction  = menu.addAction("Копировать в буфер обмена");

    QAction *chosen = menu.exec(ui->tableWidgetToDo->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    if (chosen == clearAction) {
        isLoadingData = true;
        for (int col = 0; col < 3; ++col) {
            QTableWidgetItem *it = ui->tableWidgetToDo->item(row, col);
            if (it) it->setText("");
        }
        isLoadingData = false;
        saveTasksForCurrentDate();
        refreshDbStats();
        showSaveStatus("Строка очищена", true);

    } else if (chosen == copyAction) {
        QString time  = ui->tableWidgetToDo->verticalHeaderItem(row)
                            ? ui->tableWidgetToDo->verticalHeaderItem(row)->text() : "";
        auto cell = [&](int col) -> QString {
            QTableWidgetItem *it = ui->tableWidgetToDo->item(row, col);
            return it ? it->text() : "";
        };
        QApplication::clipboard()->setText(
            QString("%1\t%2\t%3\t%4").arg(time, cell(0), cell(1), cell(2)));
        showSaveStatus("Скопировано в буфер", true);
    }
}

void MainWindow::scrollToCurrentTime()
{
    if (currentSelectedDate != QDate::currentDate()) return;
    QTime now = QTime::currentTime();
    int row = qMax(0, now.hour() * 4 + now.minute() / 15 - 2);
    QTableWidgetItem *item = ui->tableWidgetToDo->item(row, 0);
    if (item)
        ui->tableWidgetToDo->scrollToItem(item, QAbstractItemView::PositionAtTop);
}

void MainWindow::onPrevMonth()
{
    QDate d(currentYear, currentMonth, 1);
    d = d.addMonths(-1);
    currentYear = d.year();
    currentMonth = d.month();
    updateCalendar();
}

void MainWindow::onNextMonth()
{
    QDate d(currentYear, currentMonth, 1);
    d = d.addMonths(1);
    currentYear = d.year();
    currentMonth = d.month();
    updateCalendar();
}

void MainWindow::onTodayClicked()
{
    ui->dateEdit->setDate(QDate::currentDate());
}

void MainWindow::updateStatsPanel(int done, int progress)
{
    // Временные метки — обновляются каждую секунду (без запросов к БД)
    ui->labelStatCompleted->setText(QString("✓ Прошло: %1 интервалов").arg(done));
    ui->labelStatInProgress->setText(QString("⏳ Впереди: %1 интервалов").arg(progress));
}

void MainWindow::refreshDbStats()
{
    QString dateStr = currentSelectedDate.toString("yyyy-MM-dd");

    QSqlQuery qDay(manager.getDatabase());
    qDay.prepare("SELECT COUNT(*) FROM Times WHERE date_id IN (SELECT id FROM Dates WHERE date = ?)");
    qDay.addBindValue(dateStr);
    int tasksDay = 0;
    if (qDay.exec() && qDay.next()) tasksDay = qDay.value(0).toInt();

    QSqlQuery qDates(manager.getDatabase());
    int totalDates = 0;
    if (qDates.exec("SELECT COUNT(*) FROM Dates") && qDates.next())
        totalDates = qDates.value(0).toInt();

    QSqlQuery qTasks(manager.getDatabase());
    int totalTasks = 0;
    if (qTasks.exec("SELECT COUNT(*) FROM Times") && qTasks.next())
        totalTasks = qTasks.value(0).toInt();

    ui->labelStatTasksDay->setText(QString("Задач выбранного дня: %1").arg(tasksDay));
    ui->labelStatTotalDates->setText(QString("Активных дней: %1").arg(totalDates));
    ui->labelStatTotalTasks->setText(QString("Задач в базе: %1").arg(totalTasks));
}
