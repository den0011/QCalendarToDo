#include "searchdialog.h"
#include "ui_searchdialog.h"
#include <QSqlQuery>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QTime>

SearchDialog::SearchDialog(QSqlDatabase &db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog),
    database(db)
{
    ui->setupUi(this);

    // Настраиваем таблицу
    setupTable();

    // Устанавливаем иконку
    ui->iconLabel->setPixmap(QIcon(":/png/res/search.png").pixmap(16, 16));
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::setupTable()
{
    // Настраиваем ширину колонок
    ui->resultsTable->setColumnWidth(0, 100);  // Дата
    ui->resultsTable->setColumnWidth(1, 80);   // Время
    ui->resultsTable->setColumnWidth(2, 150);  // Заголовок
    ui->resultsTable->setColumnWidth(3, 200);  // Сообщение
    ui->resultsTable->setColumnWidth(4, 250);  // Описание
    ui->resultsTable->setColumnWidth(5, 80);   // Тип

    // Настраиваем поведение таблицы
    ui->resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->resultsTable->setSortingEnabled(true);
}

void SearchDialog::performSearch(const QString &searchText)
{
    ui->resultsTable->setRowCount(0);
    ui->resultsTable->setSortingEnabled(false);

    Qt::CaseSensitivity cs = ui->caseSensitiveCheckBox->isChecked()
        ? Qt::CaseSensitive : Qt::CaseInsensitive;

    int foundCount = 0;

    // Поиск в задачах
    if (ui->searchTitleCheckBox->isChecked() ||
        ui->searchMessageCheckBox->isChecked() ||
        ui->searchDescriptionCheckBox->isChecked()) {

        QSqlQuery query(database);
        QString queryStr = "SELECT Dates.date, Times.time, Times.info1, Times.info2, Times.info3 "
                          "FROM Times "
                          "INNER JOIN Dates ON Times.date_id = Dates.id ";

        // Добавляем условия поиска
        QStringList conditions;
        if (ui->searchTitleCheckBox->isChecked()) {
            conditions << "Times.info1 LIKE '%' || ? || '%'";
        }
        if (ui->searchMessageCheckBox->isChecked()) {
            conditions << "Times.info2 LIKE '%' || ? || '%'";
        }
        if (ui->searchDescriptionCheckBox->isChecked()) {
            conditions << "Times.info3 LIKE '%' || ? || '%'";
        }

        if (!conditions.isEmpty()) {
            queryStr += "WHERE (" + conditions.join(" OR ") + ") ";
        }

        queryStr += "ORDER BY Dates.date, Times.time";

        query.prepare(queryStr);

        // Привязываем значения
        if (ui->searchTitleCheckBox->isChecked()) {
            query.addBindValue(searchText);
        }
        if (ui->searchMessageCheckBox->isChecked()) {
            query.addBindValue(searchText);
        }
        if (ui->searchDescriptionCheckBox->isChecked()) {
            query.addBindValue(searchText);
        }

        if (query.exec()) {
            while (query.next()) {
                QString date = query.value(0).toString();
                QString time = query.value(1).toString();
                QString title = query.value(2).toString();
                QString message = query.value(3).toString();
                QString description = query.value(4).toString();

                bool found = false;

                // Проверяем, в каком поле найдено
                if (ui->searchTitleCheckBox->isChecked() && title.contains(searchText, cs)) {
                    found = true;
                }
                if (ui->searchMessageCheckBox->isChecked() && message.contains(searchText, cs)) {
                    found = true;
                }
                if (ui->searchDescriptionCheckBox->isChecked() && description.contains(searchText, cs)) {
                    found = true;
                }

                if (found) {
                    int row = ui->resultsTable->rowCount();
                    ui->resultsTable->insertRow(row);

                    QTableWidgetItem *dateItem = new QTableWidgetItem(date);
                    QTableWidgetItem *timeItem = new QTableWidgetItem(time);
                    QTableWidgetItem *titleItem = new QTableWidgetItem(title);
                    QTableWidgetItem *messageItem = new QTableWidgetItem(message);
                    QTableWidgetItem *descItem = new QTableWidgetItem(description);
                    QTableWidgetItem *typeItem = new QTableWidgetItem("Задача");

                    // Устанавливаем данные для сортировки
                    dateItem->setData(Qt::UserRole, QDate::fromString(date, "yyyy-MM-dd"));
                    timeItem->setData(Qt::UserRole, QTime::fromString(time, "hh:mm"));

                    ui->resultsTable->setItem(row, 0, dateItem);
                    ui->resultsTable->setItem(row, 1, timeItem);
                    ui->resultsTable->setItem(row, 2, titleItem);
                    ui->resultsTable->setItem(row, 3, messageItem);
                    ui->resultsTable->setItem(row, 4, descItem);
                    ui->resultsTable->setItem(row, 5, typeItem);

                    // Подсвечиваем найденный текст
                    QList<QTableWidgetItem*> items = {titleItem, messageItem, descItem};
                    for (QTableWidgetItem *item : items) {
                        if (item && item->text().contains(searchText, cs)) {
                            item->setBackground(QColor(255, 255, 200));
                            item->setForeground(QColor(0, 0, 0));
                            item->setToolTip("Найдено: " + searchText);
                        }
                    }

                    foundCount++;
                }
            }
        }
    }

    // Поиск в заметках
    if (ui->searchNotesCheckBox->isChecked()) {
        QSqlQuery query(database);
        query.prepare("SELECT Dates.date, Strings.value "
                     "FROM Strings "
                     "INNER JOIN Dates ON Strings.date_id = Dates.id "
                     "WHERE Strings.value LIKE '%' || ? || '%' "
                     "ORDER BY Dates.date");
        query.addBindValue(searchText);

        if (query.exec()) {
            while (query.next()) {
                QString date = query.value(0).toString();
                QString note = query.value(1).toString();

                if (note.contains(searchText, cs)) {
                    int row = ui->resultsTable->rowCount();
                    ui->resultsTable->insertRow(row);

                    QString shortNote = note;
                    if (note.length() > 100) {
                        shortNote = note.left(97) + "...";
                    }

                    QTableWidgetItem *dateItem = new QTableWidgetItem(date);
                    QTableWidgetItem *typeItem = new QTableWidgetItem("Заметка");
                    QTableWidgetItem *descItem = new QTableWidgetItem(shortNote);

                    dateItem->setData(Qt::UserRole, QDate::fromString(date, "yyyy-MM-dd"));

                    ui->resultsTable->setItem(row, 0, dateItem);
                    ui->resultsTable->setItem(row, 1, new QTableWidgetItem("-"));
                    ui->resultsTable->setItem(row, 2, new QTableWidgetItem(""));
                    ui->resultsTable->setItem(row, 3, new QTableWidgetItem(""));
                    ui->resultsTable->setItem(row, 4, descItem);
                    ui->resultsTable->setItem(row, 5, typeItem);

                    descItem->setBackground(QColor(200, 255, 200));
                    descItem->setForeground(QColor(0, 0, 0));
                    descItem->setToolTip("Полный текст заметки:\n" + note);

                    foundCount++;
                }
            }
        }
    }

    ui->resultsTable->setSortingEnabled(true);

    // Обновляем статус
    if (foundCount > 0) {
        ui->statusLabel->setText(QString("Найдено результатов: %1").arg(foundCount));
        ui->statusLabel->setStyleSheet("QLabel { color: #2E7D32; font-weight: bold; }");
    } else {
        ui->statusLabel->setText("Ничего не найдено");
        ui->statusLabel->setStyleSheet("QLabel { color: #C62828; font-weight: bold; }");
    }
}

void SearchDialog::clearResults()
{
    ui->resultsTable->setRowCount(0);
    ui->searchLineEdit->clear();
    ui->statusLabel->setText("Введите текст для поиска");
    ui->statusLabel->setStyleSheet("QLabel { color: #666666; font-style: italic; }");
    ui->searchLineEdit->setFocus();
}

// Слоты
void SearchDialog::on_searchButton_clicked()
{
    QString searchText = ui->searchLineEdit->text().trimmed();

    if (searchText.isEmpty()) {
        QMessageBox::information(this, "Поиск",
            "Пожалуйста, введите текст для поиска.");
        return;
    }

    if (!ui->searchTitleCheckBox->isChecked() &&
        !ui->searchMessageCheckBox->isChecked() &&
        !ui->searchDescriptionCheckBox->isChecked() &&
        !ui->searchNotesCheckBox->isChecked()) {
        QMessageBox::information(this, "Поиск",
            "Пожалуйста, выберите хотя бы одну область для поиска.");
        return;
    }

    performSearch(searchText);
}

void SearchDialog::on_clearButton_clicked()
{
    clearResults();
}

void SearchDialog::on_closeButton_clicked()
{
    accept();
}

void SearchDialog::on_searchLineEdit_returnPressed()
{
    on_searchButton_clicked();
}

void SearchDialog::on_searchLineEdit_textChanged(const QString &text)
{
    if (text.isEmpty()) {
        clearResults();
    }
}

void SearchDialog::on_resultsTable_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);

    QTableWidgetItem *dateItem = ui->resultsTable->item(row, 0);
    QTableWidgetItem *typeItem = ui->resultsTable->item(row, 5);

    if (dateItem && typeItem) {
        QString dateString = dateItem->text();
        QString type = typeItem->text();

        QDate date = QDate::fromString(dateString, "yyyy-MM-dd");

        if (date.isValid()) {
            if (type == "Заметка") {
                // Для заметок показываем полный текст
                QTableWidgetItem *noteItem = ui->resultsTable->item(row, 4);
                if (noteItem) {
                    QMessageBox::information(this, "Заметка от " + dateString,
                        noteItem->toolTip());
                }
            }

            emit dateSelected(date);
            accept();
        }
    }
}

void SearchDialog::on_caseSensitiveCheckBox_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    QString searchText = ui->searchLineEdit->text().trimmed();
    if (!searchText.isEmpty()) {
        performSearch(searchText);
    }
}

void SearchDialog::on_searchTitleCheckBox_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    QString searchText = ui->searchLineEdit->text().trimmed();
    if (!searchText.isEmpty()) {
        performSearch(searchText);
    }
}

void SearchDialog::on_searchMessageCheckBox_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    QString searchText = ui->searchLineEdit->text().trimmed();
    if (!searchText.isEmpty()) {
        performSearch(searchText);
    }
}

void SearchDialog::on_searchDescriptionCheckBox_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    QString searchText = ui->searchLineEdit->text().trimmed();
    if (!searchText.isEmpty()) {
        performSearch(searchText);
    }
}

void SearchDialog::on_searchNotesCheckBox_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    QString searchText = ui->searchLineEdit->text().trimmed();
    if (!searchText.isEmpty()) {
        performSearch(searchText);
    }
}
