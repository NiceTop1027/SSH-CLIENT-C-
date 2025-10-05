#ifndef FILETYPECOLORIZER_H
#define FILETYPECOLORIZER_H

#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QColor>

class FileTypeColorizer {
public:
    enum class FileType {
        Directory,
        Executable,
        Archive,
        Image,
        Video,
        Audio,
        Document,
        Code,
        Config,
        Database,
        Symlink,
        Socket,
        Pipe,
        BlockDevice,
        CharDevice,
        Regular
    };

    struct ColorScheme {
        QColor directory = QColor(85, 85, 255);       // Blue (bold)
        QColor executable = QColor(85, 255, 85);      // Green (bold)
        QColor archive = QColor(255, 85, 85);         // Red (bold)
        QColor image = QColor(255, 85, 255);          // Magenta
        QColor video = QColor(255, 85, 255);          // Magenta
        QColor audio = QColor(85, 255, 255);          // Cyan
        QColor document = QColor(170, 170, 170);      // White
        QColor code = QColor(255, 255, 85);           // Yellow
        QColor config = QColor(170, 85, 0);           // Orange
        QColor database = QColor(255, 170, 85);       // Light orange
        QColor symlink = QColor(85, 255, 255);        // Cyan (bold)
        QColor socket = QColor(255, 85, 255);         // Magenta (bold)
        QColor pipe = QColor(255, 255, 85);           // Yellow (bold)
        QColor blockDevice = QColor(255, 255, 85);    // Yellow (bold)
        QColor charDevice = QColor(255, 255, 85);     // Yellow (bold)
    };

    FileTypeColorizer();

    // Detect file type from extension or name
    FileType detectFileType(const QString& filename) const;

    // Get color for file type
    QColor getColorForType(FileType type) const;

    // Apply color to filename in terminal format (ANSI codes)
    QString colorizeFilename(const QString& filename) const;

    // Process ls output and add colors
    QString colorizeLsOutput(const QString& output) const;

    // Set custom color scheme
    void setColorScheme(const ColorScheme& scheme);

private:
    ColorScheme m_colorScheme;

    // Extension lists
    QStringList m_executableExts;
    QStringList m_archiveExts;
    QStringList m_imageExts;
    QStringList m_videoExts;
    QStringList m_audioExts;
    QStringList m_documentExts;
    QStringList m_codeExts;
    QStringList m_configExts;
    QStringList m_databaseExts;

    void initializeExtensions();
    QString colorToAnsi(const QColor& color, bool bold = false) const;
    bool hasExtension(const QString& filename, const QStringList& extensions) const;
};

#endif // FILETYPECOLORIZER_H
