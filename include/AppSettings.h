#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QFont>
#include <QColor>
#include <QSize>

class AppSettings : public QObject {
    Q_OBJECT

public:
    static AppSettings* instance();

    // Terminal settings
    QString terminalBackgroundImage() const;
    void setTerminalBackgroundImage(const QString& path);

    int terminalFontSize() const;
    void setTerminalFontSize(int size);

    QString terminalFontFamily() const;
    void setTerminalFontFamily(const QString& family);

    double terminalOpacity() const;
    void setTerminalOpacity(double opacity);

    QColor terminalForegroundColor() const;
    void setTerminalForegroundColor(const QColor& color);

    QColor terminalBackgroundColor() const;
    void setTerminalBackgroundColor(const QColor& color);

    double backgroundImageOpacity() const;
    void setBackgroundImageOpacity(double opacity);

    // Window settings
    bool rememberWindowSize() const;
    void setRememberWindowSize(bool remember);

    QSize lastWindowSize() const;
    void setLastWindowSize(const QSize& size);

    // Connection settings
    int maxRecentConnections() const;
    void setMaxRecentConnections(int max);

    QStringList recentConnections() const;
    void addRecentConnection(const QString& profileName);

    // Save/Load
    void save();
    void load();

signals:
    void settingsChanged();
    void backgroundImageChanged(const QString& path);
    void fontSizeChanged(int size);
    void opacityChanged(double opacity);
    void colorsChanged();

private:
    explicit AppSettings(QObject* parent = nullptr);
    ~AppSettings();
    AppSettings(const AppSettings&) = delete;
    AppSettings& operator=(const AppSettings&) = delete;

    static AppSettings* s_instance;
    QSettings* m_settings;

    // Cached values
    QString m_backgroundImage;
    int m_fontSize;
    QString m_fontFamily;
    double m_opacity;
    QColor m_foregroundColor;
    QColor m_backgroundColor;
    double m_backgroundImageOpacity;
    bool m_rememberWindowSize;
    QSize m_lastWindowSize;
    int m_maxRecentConnections;
    QStringList m_recentConnections;
};

#endif // APPSETTINGS_H
