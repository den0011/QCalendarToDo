#ifndef SQL_H
#define SQL_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

class DatabaseManager {
public:
    explicit DatabaseManager(const QString &databaseName = "my_database.db")
    {
        // ВАЖНО: именованное соединение
        db = QSqlDatabase::addDatabase("QSQLITE", "calendar_connection");
        db.setDatabaseName(databaseName);

        if (!db.open()) {
            qDebug() << "Ошибка при открытии/создании базы данных:" << db.lastError().text();
            QMessageBox::critical(nullptr,
                                  QObject::tr("Ошибка открытия базы"),
                                  db.lastError().text());
        }

        createTables();
    }

    ~DatabaseManager()
    {
        QString connectionName = db.connectionName();
        db.close();
        QSqlDatabase::removeDatabase(connectionName);
    }

    QSqlDatabase& getDatabase()
    {
        return db;
    }

    // --------------------------------------------------------
    // Demo / legacy functions (оставляем)
    // --------------------------------------------------------
    bool insertData()
    {
        QSqlQuery insertDate(db);
        insertDate.prepare("INSERT INTO Dates (date) VALUES (?)");
        insertDate.addBindValue("2023-08-18");

        if (!insertDate.exec()) {
            qDebug() << "Ошибка при вставке в Dates:" << insertDate.lastError().text();
            return false;
        }

        QSqlQuery insertTime(db);
        insertTime.prepare(
            "INSERT INTO Times (date_id, time, info1, info2, info3) "
            "VALUES ((SELECT id FROM Dates WHERE date = ?), ?, ?, ?, ?)"
        );
        insertTime.addBindValue("2023-08-18");
        insertTime.addBindValue("12:00");
        insertTime.addBindValue("Some info1");
        insertTime.addBindValue("More info2");
        insertTime.addBindValue("More info3");

        if (!insertTime.exec()) {
            qDebug() << "Ошибка при вставке в Times:" << insertTime.lastError().text();
            return false;
        }

        return true;
    }

    bool deleteData(const QString &targetDate)
    {
        QSqlQuery deleteQuery(db);
        deleteQuery.prepare(
            "DELETE FROM Times WHERE date_id IN "
            "(SELECT id FROM Dates WHERE date = ?)"
        );
        deleteQuery.addBindValue(targetDate);

        if (!deleteQuery.exec()) {
            qDebug() << "Ошибка при удалении:" << deleteQuery.lastError().text();
            return false;
        }
        return true;
    }

    bool updateString(const QString &targetDate, const QString &newValue)
    {
        int dateId = ensureDateExists(targetDate);
        if (dateId < 0)
            return false;

        QSqlQuery updateQuery(db);
        updateQuery.prepare(
            "UPDATE Strings SET value = ? WHERE date_id = ?"
        );
        updateQuery.addBindValue(newValue);
        updateQuery.addBindValue(dateId);

        if (!updateQuery.exec() || updateQuery.numRowsAffected() == 0) {
            QSqlQuery insertQuery(db);
            insertQuery.prepare(
                "INSERT INTO Strings (date_id, value) VALUES (?, ?)"
            );
            insertQuery.addBindValue(dateId);
            insertQuery.addBindValue(newValue);

            if (!insertQuery.exec()) {
                qDebug() << "Ошибка при вставке строки:" << insertQuery.lastError().text();
                return false;
            }
        }
        return true;
    }

    int ensureDateExists(const QString &date)
    {
        QSqlQuery query(db);
        query.prepare("SELECT id FROM Dates WHERE date = ?");
        query.addBindValue(date);

        if (query.exec() && query.next())
            return query.value(0).toInt();

        query.prepare("INSERT INTO Dates (date) VALUES (?)");
        query.addBindValue(date);

        if (query.exec())
            return query.lastInsertId().toInt();

        qDebug() << "Ошибка ensureDateExists:" << query.lastError().text();
        return -1;
    }

private:
    QSqlDatabase db;

    void createTables()
    {
        QSqlQuery q(db);

        q.exec(
            "CREATE TABLE IF NOT EXISTS Dates ("
            "id INTEGER PRIMARY KEY, "
            "date TEXT UNIQUE)"
        );

        q.exec(
            "CREATE TABLE IF NOT EXISTS Times ("
            "id INTEGER PRIMARY KEY, "
            "date_id INTEGER, "
            "time TEXT, "
            "info1 TEXT, "
            "info2 TEXT, "
            "info3 TEXT, "
            "FOREIGN KEY(date_id) REFERENCES Dates(id))"
        );

        q.exec(
            "CREATE TABLE IF NOT EXISTS Strings ("
            "id INTEGER PRIMARY KEY, "
            "date_id INTEGER UNIQUE, "
            "value TEXT, "
            "FOREIGN KEY(date_id) REFERENCES Dates(id))"
        );
    }
};

#endif // SQL_H
