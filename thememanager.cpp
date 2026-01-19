#include "thememanager.h"
#include <QApplication>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager() : m_currentTheme(LightTheme)
{
}

void ThemeManager::applyTheme(Theme theme)
{
    m_currentTheme = theme;
    QString styleSheet = getThemeStyleSheet(theme);
    qApp->setStyleSheet(styleSheet);
}

void ThemeManager::loadTheme(const QString &themeName)
{
    if (themeName == "Dark") {
        applyTheme(DarkTheme);
    } else if (themeName == "Blue") {
        applyTheme(BlueTheme);
    } else if (themeName == "Green") {
        applyTheme(GreenTheme);
    } else {
        applyTheme(LightTheme);
    }
}

QString ThemeManager::currentThemeName() const
{
    switch (m_currentTheme) {
        case DarkTheme: return "Dark";
        case BlueTheme: return "Blue";
        case GreenTheme: return "Green";
        default: return "Light";
    }
}

void ThemeManager::saveTheme(QSettings &settings)
{
    settings.beginGroup("Theme");
    settings.setValue("current", currentThemeName());
    settings.endGroup();
}

void ThemeManager::loadTheme(QSettings &settings)
{
    settings.beginGroup("Theme");
    QString themeName = settings.value("current", "Light").toString();
    settings.endGroup();
    loadTheme(themeName);
}

QString ThemeManager::getThemeStyleSheet(Theme theme)
{
    switch (theme) {
        case DarkTheme: return getDarkTheme();
        case BlueTheme: return getBlueTheme();
        case GreenTheme: return getGreenTheme();
        default: return getLightTheme();
    }
}

QString ThemeManager::getLightTheme()
{
    return R"(
/* Light Theme */
* {
    font-family: "Segoe UI", "Arial", sans-serif;
    font-size: 10pt;
}

QWidget {
    background-color: #f8f9fa;
    color: #333333;
}

QMainWindow, QDialog {
    background-color: #f8f9fa;
    border: none;
}

QGroupBox {
    font-weight: bold;
    border: 2px solid #cccccc;
    border-radius: 6px;
    margin-top: 12px;
    padding-top: 12px;
    background-color: white;
    color: #333333;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 10px;
    padding: 0 8px 0 8px;
    color: #333333;
}

QTableWidget {
    border: 1px solid #cccccc;
    border-radius: 4px;
    background-color: white;
    alternate-background-color: #f9f9f9;
    gridline-color: #e0e0e0;
    color: #333333;
    selection-background-color: #e3f2fd;
    selection-color: black;
}

QTableWidget::item {
    padding: 4px;
    border: none;
}

QTableWidget::item:selected {
    background-color: #e3f2fd;
    color: black;
}

QHeaderView {
    background-color: transparent;
}

QHeaderView::section {
    background-color: #2196F3;
    color: white;
    padding: 8px;
    border: none;
    font-weight: bold;
    border-right: 1px solid #ffffff;
}

QHeaderView::section:last {
    border-right: none;
}

QPlainTextEdit, QTextEdit, QTextBrowser {
    border: 1px solid #cccccc;
    border-radius: 4px;
    background-color: white;
    padding: 8px;
    color: #333333;
    selection-background-color: #2196F3;
    selection-color: white;
}

QLineEdit {
    border: 1px solid #cccccc;
    border-radius: 4px;
    padding: 6px 8px;
    background-color: white;
    color: #333333;
    selection-background-color: #2196F3;
    selection-color: white;
}

QLineEdit:focus {
    border: 2px solid #2196F3;
    padding: 5px 7px;
}

QPushButton {
    background-color: #2196F3;
    color: white;
    border: none;
    border-radius: 4px;
    padding: 8px 16px;
    font-weight: bold;
    min-height: 32px;
}

QPushButton:hover {
    background-color: #1976D2;
}

QPushButton:pressed {
    background-color: #0D47A1;
}

QPushButton:disabled {
    background-color: #cccccc;
    color: #666666;
}

QToolBox {
    background-color: transparent;
}

QToolBox::tab {
    background-color: #e0e0e0;
    border: 1px solid #cccccc;
    border-radius: 4px;
    padding: 8px 12px;
    margin: 2px;
    color: #333333;
}

QToolBox::tab:selected {
    background-color: #2196F3;
    color: white;
    font-weight: bold;
}

QToolBox::tab:hover {
    background-color: #1976D2;
    color: white;
}

QDateEdit, QSpinBox, QDoubleSpinBox, QComboBox, QFontComboBox {
    border: 1px solid #cccccc;
    border-radius: 4px;
    padding: 6px 8px;
    background-color: white;
    color: #333333;
    min-height: 32px;
}

QDateEdit:focus, QSpinBox:focus, QComboBox:focus {
    border: 2px solid #2196F3;
    padding: 5px 7px;
}

QDateEdit::drop-down, QSpinBox::up-button, QSpinBox::down-button,
QComboBox::drop-down {
    border: none;
    background-color: transparent;
    width: 24px;
}

QComboBox QAbstractItemView {
    border: 1px solid #cccccc;
    background-color: white;
    selection-background-color: #e3f2fd;
    selection-color: black;
}

QLabel {
    color: #333333;
    background-color: transparent;
}

QCheckBox, QRadioButton {
    color: #333333;
    spacing: 8px;
}

QCheckBox::indicator, QRadioButton::indicator {
    width: 18px;
    height: 18px;
}

QCheckBox::indicator:checked {
    background-color: #2196F3;
    border: 2px solid #1976D2;
}

QStatusBar {
    background-color: white;
    border-top: 1px solid #cccccc;
    color: #333333;
    padding: 4px;
}

QMenuBar {
    background-color: white;
    border-bottom: 1px solid #cccccc;
    padding: 2px;
}

QMenuBar::item {
    padding: 6px 12px;
    background-color: transparent;
    color: #333333;
    border-radius: 4px;
}

QMenuBar::item:selected {
    background-color: #e3f2fd;
    color: #2196F3;
}

QMenuBar::item:pressed {
    background-color: #2196F3;
    color: white;
}

QMenu {
    background-color: white;
    border: 1px solid #cccccc;
    border-radius: 4px;
    padding: 4px;
}

QMenu::item {
    padding: 8px 24px 8px 30px;
    color: #333333;
    border-radius: 2px;
    margin: 2px;
}

QMenu::item:selected {
    background-color: #e3f2fd;
    color: #2196F3;
}

QMenu::separator {
    height: 1px;
    background-color: #cccccc;
    margin: 4px 8px;
}

QTabWidget {
    background-color: transparent;
}

QTabWidget::pane {
    border: 1px solid #cccccc;
    border-radius: 4px;
    background-color: white;
    top: -1px;
}

QTabBar {
    background-color: transparent;
}

QTabBar::tab {
    background-color: #e0e0e0;
    color: #333333;
    padding: 8px 16px;
    margin-right: 2px;
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
    border: 1px solid #cccccc;
    border-bottom: none;
}

QTabBar::tab:selected {
    background-color: white;
    border-bottom: 2px solid #2196F3;
    font-weight: bold;
}

QTabBar::tab:hover:!selected {
    background-color: #f0f0f0;
}

QSplitter {
    background-color: transparent;
}

QSplitter::handle {
    background-color: #cccccc;
    border: 1px solid #e0e0e0;
}

QSplitter::handle:hover {
    background-color: #2196F3;
}

QSplitter::handle:horizontal {
    width: 4px;
    margin: 0 1px;
}

QSplitter::handle:vertical {
    height: 4px;
    margin: 1px 0;
}

QScrollBar:vertical {
    border: none;
    background-color: #f0f0f0;
    width: 12px;
    margin: 0px;
}

QScrollBar::handle:vertical {
    background-color: #c0c0c0;
    border-radius: 6px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background-color: #a0a0a0;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}

QScrollBar:horizontal {
    border: none;
    background-color: #f0f0f0;
    height: 12px;
    margin: 0px;
}

QScrollBar::handle:horizontal {
    background-color: #c0c0c0;
    border-radius: 6px;
    min-width: 30px;
}

QScrollBar::handle:horizontal:hover {
    background-color: #a0a0a0;
}

QProgressBar {
    border: 1px solid #cccccc;
    border-radius: 4px;
    background-color: #f0f0f0;
    text-align: center;
}

QProgressBar::chunk {
    background-color: #2196F3;
    border-radius: 3px;
}

QToolButton {
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 4px;
    padding: 4px;
}

QToolButton:hover {
    background-color: #e3f2fd;
    border: 1px solid #2196F3;
}

QToolButton:pressed {
    background-color: #2196F3;
    color: white;
}

QToolTip {
    background-color: #333333;
    color: white;
    border: 1px solid #666666;
    border-radius: 4px;
    padding: 4px 8px;
}
)";
}

QString ThemeManager::getDarkTheme()
{
    return R"(
/* Dark Theme */
* {
    font-family: "Segoe UI", "Arial", sans-serif;
    font-size: 10pt;
}

QWidget {
    background-color: #1e1e1e;
    color: #e0e0e0;
}

QMainWindow, QDialog {
    background-color: #1e1e1e;
    border: none;
}

QGroupBox {
    font-weight: bold;
    border: 2px solid #404040;
    border-radius: 6px;
    margin-top: 12px;
    padding-top: 12px;
    background-color: #2d2d2d;
    color: #e0e0e0;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 10px;
    padding: 0 8px 0 8px;
    color: #e0e0e0;
}

QTableWidget {
    border: 1px solid #404040;
    border-radius: 4px;
    background-color: #2d2d2d;
    alternate-background-color: #252525;
    gridline-color: #404040;
    color: #e0e0e0;
    selection-background-color: #0d47a1;
    selection-color: white;
}

QTableWidget::item {
    padding: 4px;
    border: none;
}

QTableWidget::item:selected {
    background-color: #0d47a1;
    color: white;
}

QHeaderView {
    background-color: transparent;
}

QHeaderView::section {
    background-color: #1976D2;
    color: white;
    padding: 8px;
    border: none;
    font-weight: bold;
    border-right: 1px solid #2d2d2d;
}

QHeaderView::section:last {
    border-right: none;
}

QPlainTextEdit, QTextEdit, QTextBrowser {
    border: 1px solid #404040;
    border-radius: 4px;
    background-color: #2d2d2d;
    padding: 8px;
    color: #e0e0e0;
    selection-background-color: #1976D2;
    selection-color: white;
}

QLineEdit {
    border: 1px solid #404040;
    border-radius: 4px;
    padding: 6px 8px;
    background-color: #2d2d2d;
    color: #e0e0e0;
    selection-background-color: #1976D2;
    selection-color: white;
}

QLineEdit:focus {
    border: 2px solid #1976D2;
    padding: 5px 7px;
}

QPushButton {
    background-color: #1976D2;
    color: white;
    border: none;
    border-radius: 4px;
    padding: 8px 16px;
    font-weight: bold;
    min-height: 32px;
}

QPushButton:hover {
    background-color: #2196F3;
}

QPushButton:pressed {
    background-color: #0D47A1;
}

QPushButton:disabled {
    background-color: #404040;
    color: #666666;
}

QToolBox {
    background-color: transparent;
}

QToolBox::tab {
    background-color: #404040;
    border: 1px solid #505050;
    border-radius: 4px;
    padding: 8px 12px;
    margin: 2px;
    color: #e0e0e0;
}

QToolBox::tab:selected {
    background-color: #1976D2;
    color: white;
    font-weight: bold;
}

QToolBox::tab:hover {
    background-color: #2196F3;
    color: white;
}

QDateEdit, QSpinBox, QDoubleSpinBox, QComboBox, QFontComboBox {
    border: 1px solid #404040;
    border-radius: 4px;
    padding: 6px 8px;
    background-color: #2d2d2d;
    color: #e0e0e0;
    min-height: 32px;
}

QDateEdit:focus, QSpinBox:focus, QComboBox:focus {
    border: 2px solid #1976D2;
    padding: 5px 7px;
}

QDateEdit::drop-down, QSpinBox::up-button, QSpinBox::down-button,
QComboBox::drop-down {
    border: none;
    background-color: transparent;
    width: 24px;
}

QComboBox QAbstractItemView {
    border: 1px solid #404040;
    background-color: #2d2d2d;
    color: #e0e0e0;
    selection-background-color: #0d47a1;
    selection-color: white;
}

QLabel {
    color: #e0e0e0;
    background-color: transparent;
}

QCheckBox, QRadioButton {
    color: #e0e0e0;
    spacing: 8px;
}

QCheckBox::indicator, QRadioButton::indicator {
    width: 18px;
    height: 18px;
}

QCheckBox::indicator:checked {
    background-color: #1976D2;
    border: 2px solid #2196F3;
}

QStatusBar {
    background-color: #2d2d2d;
    border-top: 1px solid #404040;
    color: #e0e0e0;
    padding: 4px;
}

QMenuBar {
    background-color: #2d2d2d;
    border-bottom: 1px solid #404040;
    padding: 2px;
}

QMenuBar::item {
    padding: 6px 12px;
    background-color: transparent;
    color: #e0e0e0;
    border-radius: 4px;
}

QMenuBar::item:selected {
    background-color: #404040;
    color: #2196F3;
}

QMenuBar::item:pressed {
    background-color: #1976D2;
    color: white;
}

QMenu {
    background-color: #2d2d2d;
    border: 1px solid #404040;
    border-radius: 4px;
    padding: 4px;
}

QMenu::item {
    padding: 8px 24px 8px 30px;
    color: #e0e0e0;
    border-radius: 2px;
    margin: 2px;
}

QMenu::item:selected {
    background-color: #404040;
    color: #2196F3;
}

QMenu::separator {
    height: 1px;
    background-color: #404040;
    margin: 4px 8px;
}

QTabWidget {
    background-color: transparent;
}

QTabWidget::pane {
    border: 1px solid #404040;
    border-radius: 4px;
    background-color: #2d2d2d;
    top: -1px;
}

QTabBar {
    background-color: transparent;
}

QTabBar::tab {
    background-color: #404040;
    color: #e0e0e0;
    padding: 8px 16px;
    margin-right: 2px;
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
    border: 1px solid #404040;
    border-bottom: none;
}

QTabBar::tab:selected {
    background-color: #2d2d2d;
    border-bottom: 2px solid #2196F3;
    font-weight: bold;
    color: #e0e0e0;
}

QTabBar::tab:hover:!selected {
    background-color: #505050;
}

QSplitter {
    background-color: transparent;
}

QSplitter::handle {
    background-color: #404040;
    border: 1px solid #505050;
}

QSplitter::handle:hover {
    background-color: #1976D2;
}

QSplitter::handle:horizontal {
    width: 4px;
    margin: 0 1px;
}

QSplitter::handle:vertical {
    height: 4px;
    margin: 1px 0;
}

QScrollBar:vertical {
    border: none;
    background-color: #2d2d2d;
    width: 12px;
    margin: 0px;
}

QScrollBar::handle:vertical {
    background-color: #404040;
    border-radius: 6px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background-color: #505050;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}

QScrollBar:horizontal {
    border: none;
    background-color: #2d2d2d;
    height: 12px;
    margin: 0px;
}

QScrollBar::handle:horizontal {
    background-color: #404040;
    border-radius: 6px;
    min-width: 30px;
}

QScrollBar::handle:horizontal:hover {
    background-color: #505050;
}

QProgressBar {
    border: 1px solid #404040;
    border-radius: 4px;
    background-color: #2d2d2d;
    text-align: center;
    color: #e0e0e0;
}

QProgressBar::chunk {
    background-color: #1976D2;
    border-radius: 3px;
}

QToolButton {
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 4px;
    padding: 4px;
}

QToolButton:hover {
    background-color: #404040;
    border: 1px solid #1976D2;
}

QToolButton:pressed {
    background-color: #1976D2;
    color: white;
}

QToolTip {
    background-color: #333333;
    color: white;
    border: 1px solid #666666;
    border-radius: 4px;
    padding: 4px 8px;
}
)";
}

QString ThemeManager::getBlueTheme()
{
    return R"(
/* Blue Theme */
* {
    font-family: "Segoe UI", "Arial", sans-serif;
    font-size: 10pt;
}

QWidget {
    background-color: #e3f2fd;
    color: #0d47a1;
}

QMainWindow, QDialog {
    background-color: #e3f2fd;
    border: none;
}

QGroupBox {
    font-weight: bold;
    border: 2px solid #64b5f6;
    border-radius: 6px;
    margin-top: 12px;
    padding-top: 12px;
    background-color: white;
    color: #0d47a1;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 10px;
    padding: 0 8px 0 8px;
    color: #0d47a1;
}

QTableWidget {
    border: 1px solid #64b5f6;
    border-radius: 4px;
    background-color: white;
    alternate-background-color: #e3f2fd;
    gridline-color: #90caf9;
    color: #1565c0;
    selection-background-color: #2196F3;
    selection-color: white;
}

QTableWidget::item {
    padding: 4px;
    border: none;
}

QTableWidget::item:selected {
    background-color: #2196F3;
    color: white;
}

QHeaderView {
    background-color: transparent;
}

QHeaderView::section {
    background-color: #1976D2;
    color: white;
    padding: 8px;
    border: none;
    font-weight: bold;
    border-right: 1px solid #ffffff;
}

QHeaderView::section:last {
    border-right: none;
}

QPlainTextEdit, QTextEdit, QTextBrowser {
    border: 1px solid #64b5f6;
    border-radius: 4px;
    background-color: white;
    padding: 8px;
    color: #1565c0;
    selection-background-color: #1976D2;
    selection-color: white;
}

QLineEdit {
    border: 1px solid #64b5f6;
    border-radius: 4px;
    padding: 6px 8px;
    background-color: white;
    color: #1565c0;
    selection-background-color: #1976D2;
    selection-color: white;
}

QLineEdit:focus {
    border: 2px solid #1976D2;
    padding: 5px 7px;
}

QPushButton {
    background-color: #2196F3;
    color: white;
    border: none;
    border-radius: 4px;
    padding: 8px 16px;
    font-weight: bold;
    min-height: 32px;
}

QPushButton:hover {
    background-color: #1976D2;
}

QPushButton:pressed {
    background-color: #0D47A1;
}

QPushButton:disabled {
    background-color: #90caf9;
    color: #64b5f6;
}

QToolBox {
    background-color: transparent;
}

QToolBox::tab {
    background-color: #bbdefb;
    border: 1px solid #64b5f6;
    border-radius: 4px;
    padding: 8px 12px;
    margin: 2px;
    color: #0d47a1;
}

QToolBox::tab:selected {
    background-color: #2196F3;
    color: white;
    font-weight: bold;
}

QToolBox::tab:hover {
    background-color: #1976D2;
    color: white;
}

QDateEdit, QSpinBox, QDoubleSpinBox, QComboBox, QFontComboBox {
    border: 1px solid #64b5f6;
    border-radius: 4px;
    padding: 6px 8px;
    background-color: white;
    color: #1565c0;
    min-height: 32px;
}

QDateEdit:focus, QSpinBox:focus, QComboBox:focus {
    border: 2px solid #1976D2;
    padding: 5px 7px;
}

QDateEdit::drop-down, QSpinBox::up-button, QSpinBox::down-button,
QComboBox::drop-down {
    border: none;
    background-color: transparent;
    width: 24px;
}

QComboBox QAbstractItemView {
    border: 1px solid #64b5f6;
    background-color: white;
    color: #1565c0;
    selection-background-color: #e3f2fd;
    selection-color: #0d47a1;
}

QLabel {
    color: #0d47a1;
    background-color: transparent;
}

QCheckBox, QRadioButton {
    color: #0d47a1;
    spacing: 8px;
}

QCheckBox::indicator, QRadioButton::indicator {
    width: 18px;
    height: 18px;
}

QCheckBox::indicator:checked {
    background-color: #2196F3;
    border: 2px solid #1976D2;
}

QStatusBar {
    background-color: white;
    border-top: 1px solid #64b5f6;
    color: #0d47a1;
    padding: 4px;
}

QMenuBar {
    background-color: white;
    border-bottom: 1px solid #64b5f6;
    padding: 2px;
}

QMenuBar::item {
    padding: 6px 12px;
    background-color: transparent;
    color: #0d47a1;
    border-radius: 4px;
}

QMenuBar::item:selected {
    background-color: #bbdefb;
    color: #0d47a1;
}

QMenuBar::item:pressed {
    background-color: #2196F3;
    color: white;
}

QMenu {
    background-color: white;
    border: 1px solid #64b5f6;
    border-radius: 4px;
    padding: 4px;
}

QMenu::item {
    padding: 8px 24px 8px 30px;
    color: #0d47a1;
    border-radius: 2px;
    margin: 2px;
}

QMenu::item:selected {
    background-color: #bbdefb;
    color: #0d47a1;
}

QMenu::separator {
    height: 1px;
    background-color: #64b5f6;
    margin: 4px 8px;
}

QTabWidget {
    background-color: transparent;
}

QTabWidget::pane {
    border: 1px solid #64b5f6;
    border-radius: 4px;
    background-color: white;
    top: -1px;
}

QTabBar {
    background-color: transparent;
}

QTabBar::tab {
    background-color: #bbdefb;
    color: #0d47a1;
    padding: 8px 16px;
    margin-right: 2px;
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
    border: 1px solid #64b5f6;
    border-bottom: none;
}

QTabBar::tab:selected {
    background-color: white;
    border-bottom: 2px solid #2196F3;
    font-weight: bold;
}

QTabBar::tab:hover:!selected {
    background-color: #e3f2fd;
}

QSplitter {
    background-color: transparent;
}

QSplitter::handle {
    background-color: #64b5f6;
    border: 1px solid #90caf9;
}

QSplitter::handle:hover {
    background-color: #1976D2;
}

QSplitter::handle:horizontal {
    width: 4px;
    margin: 0 1px;
}

QSplitter::handle:vertical {
    height: 4px;
    margin: 1px 0;
}

QScrollBar:vertical {
    border: none;
    background-color: #e3f2fd;
    width: 12px;
    margin: 0px;
}

QScrollBar::handle:vertical {
    background-color: #64b5f6;
    border-radius: 6px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background-color: #1976D2;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}

QScrollBar:horizontal {
    border: none;
    background-color: #e3f2fd;
    height: 12px;
    margin: 0px;
}

QScrollBar::handle:horizontal {
    background-color: #64b5f6;
    border-radius: 6px;
    min-width: 30px;
}

QScrollBar::handle:horizontal:hover {
    background-color: #1976D2;
}

QProgressBar {
    border: 1px solid #64b5f6;
    border-radius: 4px;
    background-color: #e3f2fd;
    text-align: center;
    color: #0d47a1;
}

QProgressBar::chunk {
    background-color: #2196F3;
    border-radius: 3px;
}

QToolButton {
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 4px;
    padding: 4px;
}

QToolButton:hover {
    background-color: #bbdefb;
    border: 1px solid #2196F3;
}

QToolButton:pressed {
    background-color: #2196F3;
    color: white;
}

QToolTip {
    background-color: #333333;
    color: white;
    border: 1px solid #666666;
    border-radius: 4px;
    padding: 4px 8px;
}
)";
}

QString ThemeManager::getGreenTheme()
{
    return R"(
/* Green Theme */
* {
    font-family: "Segoe UI", "Arial", sans-serif;
    font-size: 10pt;
}

QWidget {
    background-color: #e8f5e9;
    color: #1b5e20;
}

QMainWindow, QDialog {
    background-color: #e8f5e9;
    border: none;
}

QGroupBox {
    font-weight: bold;
    border: 2px solid #81c784;
    border-radius: 6px;
    margin-top: 12px;
    padding-top: 12px;
    background-color: white;
    color: #1b5e20;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 10px;
    padding: 0 8px 0 8px;
    color: #1b5e20;
}

QTableWidget {
    border: 1px solid #81c784;
    border-radius: 4px;
    background-color: white;
    alternate-background-color: #e8f5e9;
    gridline-color: #a5d6a7;
    color: #2e7d32;
    selection-background-color: #4CAF50;
    selection-color: white;
}

QTableWidget::item {
    padding: 4px;
    border: none;
}

QTableWidget::item:selected {
    background-color: #4CAF50;
    color: white;
}

QHeaderView {
    background-color: transparent;
}

QHeaderView::section {
    background-color: #388E3C;
    color: white;
    padding: 8px;
    border: none;
    font-weight: bold;
    border-right: 1px solid #ffffff;
}

QHeaderView::section:last {
    border-right: none;
}

QPlainTextEdit, QTextEdit, QTextBrowser {
    border: 1px solid #81c784;
    border-radius: 4px;
    background-color: white;
    padding: 8px;
    color: #2e7d32;
    selection-background-color: #388E3C;
    selection-color: white;
}

QLineEdit {
    border: 1px solid #81c784;
    border-radius: 4px;
    padding: 6px 8px;
    background-color: white;
    color: #2e7d32;
    selection-background-color: #388E3C;
    selection-color: white;
}

QLineEdit:focus {
    border: 2px solid #388E3C;
    padding: 5px 7px;
}

QPushButton {
    background-color: #4CAF50;
    color: white;
    border: none;
    border-radius: 4px;
    padding: 8px 16px;
    font-weight: bold;
    min-height: 32px;
}

QPushButton:hover {
    background-color: #388E3C;
}

QPushButton:pressed {
    background-color: #2E7D32;
}

QPushButton:disabled {
    background-color: #a5d6a7;
    color: #81c784;
}

QToolBox {
    background-color: transparent;
}

QToolBox::tab {
    background-color: #c8e6c9;
    border: 1px solid #81c784;
    border-radius: 4px;
    padding: 8px 12px;
    margin: 2px;
    color: #1b5e20;
}

QToolBox::tab:selected {
    background-color: #4CAF50;
    color: white;
    font-weight: bold;
}

QToolBox::tab:hover {
    background-color: #388E3C;
    color: white;
}

QDateEdit, QSpinBox, QDoubleSpinBox, QComboBox, QFontComboBox {
    border: 1px solid #81c784;
    border-radius: 4px;
    padding: 6px 8px;
    background-color: white;
    color: #2e7d32;
    min-height: 32px;
}

QDateEdit:focus, QSpinBox:focus, QComboBox:focus {
    border: 2px solid #388E3C;
    padding: 5px 7px;
}

QDateEdit::drop-down, QSpinBox::up-button, QSpinBox::down-button,
QComboBox::drop-down {
    border: none;
    background-color: transparent;
    width: 24px;
}

QComboBox QAbstractItemView {
    border: 1px solid #81c784;
    background-color: white;
    color: #2e7d32;
    selection-background-color: #e8f5e9;
    selection-color: #1b5e20;
}

QLabel {
    color: #1b5e20;
    background-color: transparent;
}

QCheckBox, QRadioButton {
    color: #1b5e20;
    spacing: 8px;
}

QCheckBox::indicator, QRadioButton::indicator {
    width: 18px;
    height: 18px;
}

QCheckBox::indicator:checked {
    background-color: #4CAF50;
    border: 2px solid #388E3C;
}

QStatusBar {
    background-color: white;
    border-top: 1px solid #81c784;
    color: #1b5e20;
    padding: 4px;
}

QMenuBar {
    background-color: white;
    border-bottom: 1px solid #81c784;
    padding: 2px;
}

QMenuBar::item {
    padding: 6px 12px;
    background-color: transparent;
    color: #1b5e20;
    border-radius: 4px;
}

QMenuBar::item:selected {
    background-color: #c8e6c9;
    color: #1b5e20;
}

QMenuBar::item:pressed {
    background-color: #4CAF50;
    color: white;
}

QMenu {
    background-color: white;
    border: 1px solid #81c784;
    border-radius: 4px;
    padding: 4px;
}

QMenu::item {
    padding: 8px 24px 8px 30px;
    color: #1b5e20;
    border-radius: 2px;
    margin: 2px;
}

QMenu::item:selected {
    background-color: #c8e6c9;
    color: #1b5e20;
}

QMenu::separator {
    height: 1px;
    background-color: #81c784;
    margin: 4px 8px;
}

QTabWidget {
    background-color: transparent;
}

QTabWidget::pane {
    border: 1px solid #81c784;
    border-radius: 4px;
    background-color: white;
    top: -1px;
}

QTabBar {
    background-color: transparent;
}

QTabBar::tab {
    background-color: #c8e6c9;
    color: #1b5e20;
    padding: 8px 16px;
    margin-right: 2px;
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
    border: 1px solid #81c784;
    border-bottom: none;
}

QTabBar::tab:selected {
    background-color: white;
    border-bottom: 2px solid #4CAF50;
    font-weight: bold;
}

QTabBar::tab:hover:!selected {
    background-color: #e8f5e9;
}

QSplitter {
    background-color: transparent;
}

QSplitter::handle {
    background-color: #81c784;
    border: 1px solid #a5d6a7;
}

QSplitter::handle:hover {
    background-color: #388E3C;
}

QSplitter::handle:horizontal {
    width: 4px;
    margin: 0 1px;
}

QSplitter::handle:vertical {
    height: 4px;
    margin: 1px 0;
}

QScrollBar:vertical {
    border: none;
    background-color: #e8f5e9;
    width: 12px;
    margin: 0px;
}

QScrollBar::handle:vertical {
    background-color: #81c784;
    border-radius: 6px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background-color: #388E3C;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}

QScrollBar:horizontal {
    border: none;
    background-color: #e8f5e9;
    height: 12px;
    margin: 0px;
}

QScrollBar::handle:horizontal {
    background-color: #81c784;
    border-radius: 6px;
    min-width: 30px;
}

QScrollBar::handle:horizontal:hover {
    background-color: #388E3C;
}

QProgressBar {
    border: 1px solid #81c784;
    border-radius: 4px;
    background-color: #e8f5e9;
    text-align: center;
    color: #1b5e20;
}

QProgressBar::chunk {
    background-color: #4CAF50;
    border-radius: 3px;
}

QToolButton {
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 4px;
    padding: 4px;
}

QToolButton:hover {
    background-color: #c8e6c9;
    border: 1px solid #4CAF50;
}

QToolButton:pressed {
    background-color: #4CAF50;
    color: white;
}

QToolTip {
    background-color: #333333;
    color: white;
    border: 1px solid #666666;
    border-radius: 4px;
    padding: 4px 8px;
}
)";
}
