#include "FileTypeColorizer.h"
#include <QFileInfo>

FileTypeColorizer::FileTypeColorizer()
{
    initializeExtensions();
}

void FileTypeColorizer::initializeExtensions()
{
    // Executable extensions
    m_executableExts = {
        "exe", "sh", "bat", "cmd", "com", "app", "bin", "run",
        "elf", "out", "py", "pl", "rb", "js", "jar"
    };

    // Archive extensions
    m_archiveExts = {
        "zip", "tar", "gz", "bz2", "xz", "7z", "rar", "tgz",
        "tbz", "tbz2", "tar.gz", "tar.bz2", "tar.xz", "iso",
        "dmg", "pkg", "deb", "rpm"
    };

    // Image extensions
    m_imageExts = {
        "jpg", "jpeg", "png", "gif", "bmp", "svg", "ico", "webp",
        "tiff", "tif", "psd", "ai", "eps", "raw", "cr2", "nef"
    };

    // Video extensions
    m_videoExts = {
        "mp4", "avi", "mkv", "mov", "wmv", "flv", "webm", "m4v",
        "mpg", "mpeg", "3gp", "ogv", "ts", "vob"
    };

    // Audio extensions
    m_audioExts = {
        "mp3", "wav", "flac", "aac", "ogg", "wma", "m4a", "ape",
        "opus", "aiff", "alac", "mid", "midi"
    };

    // Document extensions
    m_documentExts = {
        "pdf", "doc", "docx", "txt", "rtf", "odt", "tex", "md",
        "ppt", "pptx", "xls", "xlsx", "csv", "ods", "odp"
    };

    // Code/source extensions
    m_codeExts = {
        "c", "cpp", "cc", "cxx", "h", "hpp", "hxx", "java", "py",
        "js", "ts", "jsx", "tsx", "go", "rs", "swift", "kt", "scala",
        "rb", "php", "pl", "lua", "r", "m", "mm", "cs", "vb", "sql",
        "html", "htm", "xml", "css", "scss", "sass", "less", "vue",
        "asm", "s", "f", "f90", "for", "hs", "ml", "clj", "lisp",
        "el", "vim", "sh", "bash", "zsh", "fish"
    };

    // Config extensions
    m_configExts = {
        "conf", "config", "cfg", "ini", "yaml", "yml", "toml", "json",
        "properties", "env", "editorconfig", "gitignore", "dockerignore"
    };

    // Database extensions
    m_databaseExts = {
        "db", "sqlite", "sqlite3", "sql", "mdb", "accdb", "dbf"
    };
}

FileTypeColorizer::FileType FileTypeColorizer::detectFileType(const QString& filename) const
{
    // Check for special file indicators from ls -la output
    if (filename.startsWith('d')) {
        return FileType::Directory;
    }
    if (filename.startsWith('l')) {
        return FileType::Symlink;
    }
    if (filename.startsWith('s')) {
        return FileType::Socket;
    }
    if (filename.startsWith('p')) {
        return FileType::Pipe;
    }
    if (filename.startsWith('b')) {
        return FileType::BlockDevice;
    }
    if (filename.startsWith('c')) {
        return FileType::CharDevice;
    }

    // Check by extension
    if (hasExtension(filename, m_archiveExts)) {
        return FileType::Archive;
    }
    if (hasExtension(filename, m_imageExts)) {
        return FileType::Image;
    }
    if (hasExtension(filename, m_videoExts)) {
        return FileType::Video;
    }
    if (hasExtension(filename, m_audioExts)) {
        return FileType::Audio;
    }
    if (hasExtension(filename, m_documentExts)) {
        return FileType::Document;
    }
    if (hasExtension(filename, m_codeExts)) {
        return FileType::Code;
    }
    if (hasExtension(filename, m_configExts)) {
        return FileType::Config;
    }
    if (hasExtension(filename, m_databaseExts)) {
        return FileType::Database;
    }
    if (hasExtension(filename, m_executableExts)) {
        return FileType::Executable;
    }

    // Check for executable permission (no extension but executable)
    // This would need to be checked from file permissions in ls output

    return FileType::Regular;
}

QColor FileTypeColorizer::getColorForType(FileType type) const
{
    switch (type) {
        case FileType::Directory:
            return m_colorScheme.directory;
        case FileType::Executable:
            return m_colorScheme.executable;
        case FileType::Archive:
            return m_colorScheme.archive;
        case FileType::Image:
            return m_colorScheme.image;
        case FileType::Video:
            return m_colorScheme.video;
        case FileType::Audio:
            return m_colorScheme.audio;
        case FileType::Document:
            return m_colorScheme.document;
        case FileType::Code:
            return m_colorScheme.code;
        case FileType::Config:
            return m_colorScheme.config;
        case FileType::Database:
            return m_colorScheme.database;
        case FileType::Symlink:
            return m_colorScheme.symlink;
        case FileType::Socket:
            return m_colorScheme.socket;
        case FileType::Pipe:
            return m_colorScheme.pipe;
        case FileType::BlockDevice:
            return m_colorScheme.blockDevice;
        case FileType::CharDevice:
            return m_colorScheme.charDevice;
        default:
            return QColor(170, 170, 170); // Default gray
    }
}

QString FileTypeColorizer::colorToAnsi(const QColor& color, bool bold) const
{
    QString ansiCode = QString("\x1b[");

    if (bold) {
        ansiCode += "1;";
    }

    // Use 38;2;R;G;B for RGB color
    ansiCode += QString("38;2;%1;%2;%3m")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue());

    return ansiCode;
}

QString FileTypeColorizer::colorizeFilename(const QString& filename) const
{
    FileType type = detectFileType(filename);
    QColor color = getColorForType(type);

    bool bold = (type == FileType::Directory ||
                 type == FileType::Executable ||
                 type == FileType::Symlink ||
                 type == FileType::Archive);

    QString ansiColor = colorToAnsi(color, bold);
    QString reset = "\x1b[0m";

    return ansiColor + filename + reset;
}

QString FileTypeColorizer::colorizeLsOutput(const QString& output) const
{
    if (output.isEmpty()) {
        return output;
    }

    QStringList lines = output.split('\n');
    QStringList colorizedLines;

    for (const QString& line : lines) {
        if (line.trimmed().isEmpty()) {
            colorizedLines.append(line);
            continue;
        }

        // Parse ls -l format: permissions links owner group size date time filename
        QStringList parts = line.split(QRegularExpression("\\s+"));

        if (parts.size() < 9 && !line.startsWith("total")) {
            // Simple format (just filenames) or not ls -l
            QStringList files = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            QStringList colorizedFiles;

            for (const QString& file : files) {
                colorizedFiles.append(colorizeFilename(file));
            }

            colorizedLines.append(colorizedFiles.join("  "));
        } else if (line.startsWith("total")) {
            // Total line, don't colorize
            colorizedLines.append(line);
        } else {
            // ls -l format
            QString permissions = parts[0];
            QString filename;

            // Filename starts at index 8 (or later if there are spaces in the name)
            if (parts.size() >= 9) {
                QStringList filenameParts = parts.mid(8);
                filename = filenameParts.join(" ");

                // Handle symlinks (name -> target)
                if (filename.contains(" -> ")) {
                    QStringList symlinkParts = filename.split(" -> ");
                    if (symlinkParts.size() == 2) {
                        QString name = colorizeFilename(symlinkParts[0]);
                        QString target = colorizeFilename(symlinkParts[1]);
                        filename = name + " -> " + target;
                    }
                } else {
                    // Determine file type from permissions
                    FileType type = FileType::Regular;

                    if (permissions.startsWith('d')) {
                        type = FileType::Directory;
                    } else if (permissions.startsWith('l')) {
                        type = FileType::Symlink;
                    } else if (permissions[3] == 'x' || permissions[6] == 'x' || permissions[9] == 'x') {
                        type = FileType::Executable;
                    } else {
                        type = detectFileType(filename);
                    }

                    QColor color = getColorForType(type);
                    bool bold = (type == FileType::Directory ||
                               type == FileType::Executable ||
                               type == FileType::Symlink ||
                               type == FileType::Archive);

                    QString ansiColor = colorToAnsi(color, bold);
                    QString reset = "\x1b[0m";
                    filename = ansiColor + filename + reset;
                }

                // Reconstruct line with colorized filename
                QStringList beforeFilename = parts.mid(0, 8);
                colorizedLines.append(beforeFilename.join(" ") + " " + filename);
            } else {
                colorizedLines.append(line);
            }
        }
    }

    return colorizedLines.join('\n');
}

void FileTypeColorizer::setColorScheme(const ColorScheme& scheme)
{
    m_colorScheme = scheme;
}

bool FileTypeColorizer::hasExtension(const QString& filename, const QStringList& extensions) const
{
    QString lower = filename.toLower();

    for (const QString& ext : extensions) {
        if (lower.endsWith('.' + ext)) {
            return true;
        }
    }

    return false;
}
