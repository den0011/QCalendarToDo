#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QColorDialog>
#include "settings.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(Settings &settings, QWidget *parent = nullptr);
    ~SettingsDialog();

    Settings getSettings() const;

signals:
    void settingsChanged();

private slots:
    void on_pushButtonTasks_clicked();
    void on_pushButtonNotes_clicked();
    void on_pushButtonDoneColor_clicked();
    void on_pushButtonProgressColor_clicked();
    void on_fontComboBoxTasks_currentFontChanged(const QFont &font);
    void on_spinBoxTasks_valueChanged(int value);
    void on_fontComboBoxNotes_currentFontChanged(const QFont &font);
    void on_spinBoxNotes_valueChanged(int value);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::SettingsDialog *ui;
    Settings currentSettings;
    QColorDialog *colorDialog;

    void loadSettings();
    void updatePreview();
};

#endif // SETTINGSDIALOG_H
