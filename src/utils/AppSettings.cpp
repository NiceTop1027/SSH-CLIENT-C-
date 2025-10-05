#include "AppSettings.h"
#include <QApplication>

AppSettings* AppSettings::s_instance = nullptr;

AppSettings* AppSettings::instance()
{
    if (!s_instance) {
        s_instance = new AppSettings(qApp);
    }
    return s_instance;
}

AppSettings::AppSettings(QObject* parent)
    : QObject(parent)
    , m_settings(new QSettings("SSHClient", "SSHClient", this))
    , m_fontSize(12)
    , m_fontFamily("Monaco")
    , m_opacity(1.0)
    , m_foregroundColor(Qt::white)
    , m_backgroundColor(Qt::black)
    , m_backgroundImageOpacity(0.3)
    , m_rememberWindowSize(true)
    , m_lastWindowSize(1024, 768)
    , m_maxRecentConnections(10)
{
    load();
}

AppSettings::~AppSettings()
{
    save();
}

QString AppSettings::terminalBackgroundImage() const
{
    return m_backgroundImage;
}

void AppSettings::setTerminalBackgroundImage(const QString& path)
{
    if (m_backgroundImage != path) {
        m_backgroundImage = path;
        emit backgroundImageChanged(path);
        emit settingsChanged();
    }
}

int AppSettings::terminalFontSize() const
{
    return m_fontSize;
}

void AppSettings::setTerminalFontSize(int size)
{
    if (m_fontSize != size) {
        m_fontSize = size;
        emit fontSizeChanged(size);
        emit settingsChanged();
    }
}

QString AppSettings::terminalFontFamily() const
{
    return m_fontFamily;
}

void AppSettings::setTerminalFontFamily(const QString& family)
{
    if (m_fontFamily != family) {
        m_fontFamily = family;
        emit settingsChanged();
    }
}

double AppSettings::terminalOpacity() const
{
    return m_opacity;
}

void AppSettings::setTerminalOpacity(double opacity)
{
    if (m_opacity != opacity) {
        m_opacity = opacity;
        emit opacityChanged(opacity);
        emit settingsChanged();
    }
}

QColor AppSettings::terminalForegroundColor() const
{
    return m_foregroundColor;
}

void AppSettings::setTerminalForegroundColor(const QColor& color)
{
    if (m_foregroundColor != color) {
        m_foregroundColor = color;
        emit colorsChanged();
        emit settingsChanged();
    }
}

QColor AppSettings::terminalBackgroundColor() const
{
    return m_backgroundColor;
}

void AppSettings::setTerminalBackgroundColor(const QColor& color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        emit colorsChanged();
        emit settingsChanged();
    }
}

double AppSettings::backgroundImageOpacity() const
{
    return m_backgroundImageOpacity;
}

void AppSettings::setBackgroundImageOpacity(double opacity)
{
    if (m_backgroundImageOpacity != opacity) {
        m_backgroundImageOpacity = opacity;
        emit settingsChanged();
    }
}

bool AppSettings::rememberWindowSize() const
{
    return m_rememberWindowSize;
}

void AppSettings::setRememberWindowSize(bool remember)
{
    m_rememberWindowSize = remember;
}

QSize AppSettings::lastWindowSize() const
{
    return m_lastWindowSize;
}

void AppSettings::setLastWindowSize(const QSize& size)
{
    m_lastWindowSize = size;
}

int AppSettings::maxRecentConnections() const
{
    return m_maxRecentConnections;
}

void AppSettings::setMaxRecentConnections(int max)
{
    m_maxRecentConnections = max;
}

QStringList AppSettings::recentConnections() const
{
    return m_recentConnections;
}

void AppSettings::addRecentConnection(const QString& profileName)
{
    m_recentConnections.removeAll(profileName);
    m_recentConnections.prepend(profileName);

    while (m_recentConnections.size() > m_maxRecentConnections) {
        m_recentConnections.removeLast();
    }
}

void AppSettings::save()
{
    m_settings->beginGroup("Terminal");
    m_settings->setValue("backgroundImage", m_backgroundImage);
    m_settings->setValue("fontSize", m_fontSize);
    m_settings->setValue("fontFamily", m_fontFamily);
    m_settings->setValue("opacity", m_opacity);
    m_settings->setValue("foregroundColor", m_foregroundColor);
    m_settings->setValue("backgroundColor", m_backgroundColor);
    m_settings->setValue("backgroundImageOpacity", m_backgroundImageOpacity);
    m_settings->endGroup();

    m_settings->beginGroup("Window");
    m_settings->setValue("rememberSize", m_rememberWindowSize);
    m_settings->setValue("lastSize", m_lastWindowSize);
    m_settings->endGroup();

    m_settings->beginGroup("Connection");
    m_settings->setValue("maxRecentConnections", m_maxRecentConnections);
    m_settings->setValue("recentConnections", m_recentConnections);
    m_settings->endGroup();

    m_settings->sync();
}

void AppSettings::load()
{
    m_settings->beginGroup("Terminal");
    m_backgroundImage = m_settings->value("backgroundImage", "").toString();
    m_fontSize = m_settings->value("fontSize", 12).toInt();
#ifdef Q_OS_MAC
    m_fontFamily = m_settings->value("fontFamily", "Monaco").toString();
#elif defined(Q_OS_WIN)
    m_fontFamily = m_settings->value("fontFamily", "Consolas").toString();
#else
    m_fontFamily = m_settings->value("fontFamily", "Monospace").toString();
#endif
    m_opacity = m_settings->value("opacity", 1.0).toDouble();
    m_foregroundColor = m_settings->value("foregroundColor", QColor(Qt::white)).value<QColor>();
    m_backgroundColor = m_settings->value("backgroundColor", QColor(Qt::black)).value<QColor>();
    m_backgroundImageOpacity = m_settings->value("backgroundImageOpacity", 0.3).toDouble();
    m_settings->endGroup();

    m_settings->beginGroup("Window");
    m_rememberWindowSize = m_settings->value("rememberSize", true).toBool();
    m_lastWindowSize = m_settings->value("lastSize", QSize(1024, 768)).toSize();
    m_settings->endGroup();

    m_settings->beginGroup("Connection");
    m_maxRecentConnections = m_settings->value("maxRecentConnections", 10).toInt();
    m_recentConnections = m_settings->value("recentConnections").toStringList();
    m_settings->endGroup();
}
