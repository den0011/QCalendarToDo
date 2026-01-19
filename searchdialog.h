#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QDate>

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QSqlDatabase &db, QWidget *parent = nullptr);
    ~SearchDialog();

signals:
    void dateSelected(const QDate &date);

private slots:
    void on_searchButton_clicked();
    void on_clearButton_clicked();
    void on_closeButton_clicked();
    void on_searchLineEdit_returnPressed();
    void on_searchLineEdit_textChanged(const QString &text);
    void on_resultsTable_cellDoubleClicked(int row, int column);
    void on_caseSensitiveCheckBox_stateChanged(int arg1);
    void on_searchTitleCheckBox_stateChanged(int arg1);
    void on_searchMessageCheckBox_stateChanged(int arg1);
    void on_searchDescriptionCheckBox_stateChanged(int arg1);
    void on_searchNotesCheckBox_stateChanged(int arg1);

private:
    Ui::SearchDialog *ui;
    QSqlDatabase &database;

    void performSearch(const QString &searchText);
    void clearResults();
    void setupTable();
};

#endif // SEARCHDIALOG_H
