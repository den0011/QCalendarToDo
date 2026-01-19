#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(Settings &settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    currentSettings(settings)
{
    ui->setupUi(this);
    colorDialog = new QColorDialog(this);
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::loadSettings()
{
    // Загружаем шрифты
    ui->fontComboBoxTasks->setCurrentFont(currentSettings.tasks);
    ui->spinBoxTasks->setValue(currentSettings.tasks.pointSize());
    ui->fontComboBoxNotes->setCurrentFont(currentSettings.notes);
    ui->spinBoxNotes->setValue(currentSettings.notes.pointSize());

    // Загружаем цвета
    ui->pushButtonDoneColor->setStyleSheet("background-color: " + currentSettings.done.name() + ";");
    ui->pushButtonProgressColor->setStyleSheet("background-color: " + currentSettings.progress.name() + ";");

    // Загружаем настройки уведомлений
    ui->checkBoxSystemTray->setChecked(currentSettings.messages);
    ui->spinBoxTraySec->setValue(currentSettings.sec);

    updatePreview();
}

void SettingsDialog::updatePreview()
{
    // Обновляем предварительный просмотр шрифтов
    QFont tasksFont = ui->fontComboBoxTasks->currentFont();
    tasksFont.setPointSize(ui->spinBoxTasks->value());
    ui->plainTextEditTasksPreview->setFont(tasksFont);

    QFont notesFont = ui->fontComboBoxNotes->currentFont();
    notesFont.setPointSize(ui->spinBoxNotes->value());
    ui->plainTextEditNotesPreview->setFont(notesFont);
}

Settings SettingsDialog::getSettings() const
{
    return currentSettings;
}

void SettingsDialog::on_fontComboBoxTasks_currentFontChanged(const QFont &font)
{
    Q_UNUSED(font);
    updatePreview();
}

void SettingsDialog::on_spinBoxTasks_valueChanged(int value)
{
    Q_UNUSED(value);
    updatePreview();
}

void SettingsDialog::on_fontComboBoxNotes_currentFontChanged(const QFont &font)
{
    Q_UNUSED(font);
    updatePreview();
}

void SettingsDialog::on_spinBoxNotes_valueChanged(int value)
{
    Q_UNUSED(value);
    updatePreview();
}

void SettingsDialog::on_pushButtonTasks_clicked()
{
    QFont font = ui->fontComboBoxTasks->currentFont();
    font.setPointSize(ui->spinBoxTasks->value());
    currentSettings.tasks = font;
    updatePreview();
}

void SettingsDialog::on_pushButtonNotes_clicked()
{
    QFont font = ui->fontComboBoxNotes->currentFont();
    font.setPointSize(ui->spinBoxNotes->value());
    currentSettings.notes = font;
    updatePreview();
}

void SettingsDialog::on_pushButtonDoneColor_clicked()
{
    QColor currentColor = currentSettings.done;
    colorDialog->setCurrentColor(currentColor);
    colorDialog->setWindowTitle("Установить цвет для завершенных задач");

    if (colorDialog->exec() == QDialog::Accepted) {
        QColor color = colorDialog->selectedColor();
        if (color.isValid()) {
            currentSettings.done = color;
            ui->pushButtonDoneColor->setStyleSheet("background-color: " + color.name() + ";");
        }
    }
}

void SettingsDialog::on_pushButtonProgressColor_clicked()
{
    QColor currentColor = currentSettings.progress;
    colorDialog->setCurrentColor(currentColor);
    colorDialog->setWindowTitle("Установить цвет для текущих задач");

    if (colorDialog->exec() == QDialog::Accepted) {
        QColor color = colorDialog->selectedColor();
        if (color.isValid()) {
            currentSettings.progress = color;
            ui->pushButtonProgressColor->setStyleSheet("background-color: " + color.name() + ";");
        }
    }
}

void SettingsDialog::on_buttonBox_accepted()
{
    // Сохраняем все настройки
    QFont tasksFont = ui->fontComboBoxTasks->currentFont();
    tasksFont.setPointSize(ui->spinBoxTasks->value());
    currentSettings.tasks = tasksFont;

    QFont notesFont = ui->fontComboBoxNotes->currentFont();
    notesFont.setPointSize(ui->spinBoxNotes->value());
    currentSettings.notes = notesFont;

    currentSettings.messages = ui->checkBoxSystemTray->isChecked();
    currentSettings.sec = ui->spinBoxTraySec->value();

    emit settingsChanged();
    accept();
}

void SettingsDialog::on_buttonBox_rejected()
{
    reject();
}
