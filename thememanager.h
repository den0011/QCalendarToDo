#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QString>
#include <QApplication>
#include <QColor>
#include <QSettings>

class ThemeManager
{
public:
    enum Theme {
        LightTheme,
        DarkTheme,
        BlueTheme,
        GreenTheme
    };

    static ThemeManager& instance();

    void applyTheme(Theme theme);
    void loadTheme(const QString &themeName);
    Theme currentTheme() const { return m_currentTheme; }
    QString currentThemeName() const;

    void saveTheme(QSettings &settings);
    void loadTheme(QSettings &settings);

    QColor accentColor() const;

    static QString getLightTheme();
    static QString getDarkTheme();
    static QString getBlueTheme();
    static QString getGreenTheme();

private:
    ThemeManager();
    Theme m_currentTheme;

    QString getThemeStyleSheet(Theme theme);
};

#endif // THEMEMANAGER_H
