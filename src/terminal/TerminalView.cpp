#include "TerminalView.h"
#include "AppSettings.h"
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QApplication>
#include <QClipboard>

TerminalView::TerminalView(QWidget* parent)
    : QWidget(parent)
    , m_emulator(24, 80)
    , m_charWidth(0)
    , m_charHeight(0)
    , m_rows(24)
    , m_columns(80)
    , m_cursorVisible(true)
    , m_hasFocus(false)
    , m_backgroundImageOpacity(0.3)
    , m_customForeground(Qt::white)
    , m_customBackground(Qt::black)
    , m_useCustomColors(false)
{
    setupTerminal();
    setupFont();
    calculateMetrics();

    m_cursorTimer = new QTimer(this);
    connect(m_cursorTimer, &QTimer::timeout, this, &TerminalView::blinkCursor);
    m_cursorTimer->start(500);  // Blink every 500ms

    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);

    // Set size policy to expand with window
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Connect to settings changes
    connect(AppSettings::instance(), &AppSettings::settingsChanged, this, &TerminalView::onSettingsChanged);
    connect(AppSettings::instance(), &AppSettings::backgroundImageChanged, this, [this](const QString& path) {
        loadBackgroundImage(path);
        update();
    });

    // Load initial settings
    onSettingsChanged();
}

TerminalView::~TerminalView()
{
}

void TerminalView::setupTerminal()
{
    // Set background color
    QPalette p = palette();
    p.setColor(QPalette::Window, QColor(0, 0, 0));
    setPalette(p);
    setAutoFillBackground(true);
}

void TerminalView::setupFont()
{
#ifdef Q_OS_MAC
    m_font = QFont("Monaco", 12);
#elif defined(Q_OS_WIN)
    m_font = QFont("Consolas", 10);
#else
    m_font = QFont("Monospace", 10);
#endif

    m_font.setFixedPitch(true);
    m_font.setStyleHint(QFont::Monospace);
}

void TerminalView::calculateMetrics()
{
    QFontMetrics fm(m_font);
    m_charWidth = fm.horizontalAdvance('M');
    m_charHeight = fm.height();

    // Set minimum size to accommodate at least 24 rows x 80 columns
    int minWidth = m_charWidth * 80;
    int minHeight = m_charHeight * 24;
    setMinimumSize(minWidth, minHeight);
}

void TerminalView::displayOutput(const QString& text)
{
    m_emulator.processData(text);
    update();
}

void TerminalView::displayOutput(const QByteArray& data)
{
    m_emulator.processData(data);
    update();
}

void TerminalView::setDimensions(int rows, int columns)
{
    m_rows = rows;
    m_columns = columns;
    m_emulator.resize(rows, columns);
    calculateMetrics();
    emit dimensionsChanged(rows, columns);
    update();
}

int TerminalView::rows() const
{
    return m_rows;
}

int TerminalView::columns() const
{
    return m_columns;
}

void TerminalView::clearDisplay()
{
    m_emulator.screen().clearScreen();
    update();
}

void TerminalView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setFont(m_font);

    const TerminalScreen& screen = m_emulator.screen();

    // Draw background image if set
    if (!m_backgroundImage.isNull()) {
        painter.setOpacity(m_backgroundImageOpacity);
        QPixmap scaled = m_backgroundImage.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        // Center the image
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
        painter.setOpacity(1.0);
    }

    // Get default colors
    QColor defaultBg = m_useCustomColors ? m_customBackground : QColor(0, 0, 0);
    QColor defaultFg = m_useCustomColors ? m_customForeground : QColor(170, 170, 170);

    // Draw all cells
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_columns; ++col) {
            const TerminalScreen::Cell& cell = screen.cellAt(row, col);
            QRect cellRect = getCellRect(row, col);

            // Draw background (with transparency if background image is set)
            QColor bgColor = cell.bgColor.isValid() ? cell.bgColor : defaultBg;
            if (!m_backgroundImage.isNull() && !cell.bgColor.isValid()) {
                bgColor.setAlpha(200); // Semi-transparent for background image visibility
            }
            painter.fillRect(cellRect, bgColor);

            // Draw character
            if (cell.character != ' ' && cell.character != QChar(0)) {
                QColor fgColor = cell.fgColor.isValid() ? cell.fgColor : defaultFg;
                painter.setPen(fgColor);

                QFont font = m_font;
                font.setBold(cell.bold);
                font.setItalic(cell.italic);
                font.setUnderline(cell.underline);
                painter.setFont(font);

                painter.drawText(cellRect, Qt::AlignLeft | Qt::AlignTop, QString(cell.character));
            }
        }
    }

    // Draw cursor
    if (m_cursorVisible && m_hasFocus && screen.cursorVisible()) {
        int cursorRow = screen.cursorRow();
        int cursorCol = screen.cursorCol();

        if (cursorRow >= 0 && cursorRow < m_rows && cursorCol >= 0 && cursorCol < m_columns) {
            QRect cursorRect = getCellRect(cursorRow, cursorCol);
            painter.fillRect(cursorRect, defaultFg);

            // Redraw character in inverse color
            const TerminalScreen::Cell& cell = screen.cellAt(cursorRow, cursorCol);
            if (cell.character != ' ' && cell.character != QChar(0)) {
                painter.setPen(defaultBg);
                painter.drawText(cursorRect, Qt::AlignLeft | Qt::AlignTop, QString(cell.character));
            }
        }
    }
}

void TerminalView::keyPressEvent(QKeyEvent* event)
{
    QString data = keyEventToString(event);
    if (!data.isEmpty()) {
        emit sendData(data);
    }
    event->accept();
}

void TerminalView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    if (m_charWidth <= 0 || m_charHeight <= 0) {
        return;
    }

    int newCols = width() / m_charWidth;
    int newRows = height() / m_charHeight;

    if (newCols > 0 && newRows > 0 && (newCols != m_columns || newRows != m_rows)) {
        setDimensions(newRows, newCols);
    }
}

void TerminalView::mousePressEvent(QMouseEvent* event)
{
    setFocus();
    QWidget::mousePressEvent(event);
}

void TerminalView::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
}

void TerminalView::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
}

void TerminalView::focusInEvent(QFocusEvent* event)
{
    m_hasFocus = true;
    update();
    QWidget::focusInEvent(event);
}

void TerminalView::focusOutEvent(QFocusEvent* event)
{
    m_hasFocus = false;
    update();
    QWidget::focusOutEvent(event);
}

void TerminalView::blinkCursor()
{
    m_cursorVisible = !m_cursorVisible;
    update();
}

QString TerminalView::keyEventToString(QKeyEvent* event)
{
    int key = event->key();
    Qt::KeyboardModifiers modifiers = event->modifiers();

    // Handle Ctrl combinations
    if (modifiers & Qt::ControlModifier) {
        if (key >= Qt::Key_A && key <= Qt::Key_Z) {
            int ctrlChar = key - Qt::Key_A + 1;
            return QString(QChar(ctrlChar));
        }

        switch (key) {
        case Qt::Key_Space:
            return QString(QChar(0x00));
        case Qt::Key_BracketLeft:
            return QString(QChar(0x1B));
        case Qt::Key_Backslash:
            return QString(QChar(0x1C));
        case Qt::Key_BracketRight:
            return QString(QChar(0x1D));
        case Qt::Key_AsciiCircum:
            return QString(QChar(0x1E));
        case Qt::Key_Underscore:
            return QString(QChar(0x1F));
        }
    }

    // Handle arrow keys
    switch (key) {
    case Qt::Key_Up:
        return "\x1b[A";
    case Qt::Key_Down:
        return "\x1b[B";
    case Qt::Key_Right:
        return "\x1b[C";
    case Qt::Key_Left:
        return "\x1b[D";
    }

    // Handle function keys
    switch (key) {
    case Qt::Key_F1:
        return "\x1bOP";
    case Qt::Key_F2:
        return "\x1bOQ";
    case Qt::Key_F3:
        return "\x1bOR";
    case Qt::Key_F4:
        return "\x1bOS";
    case Qt::Key_F5:
        return "\x1b[15~";
    case Qt::Key_F6:
        return "\x1b[17~";
    case Qt::Key_F7:
        return "\x1b[18~";
    case Qt::Key_F8:
        return "\x1b[19~";
    case Qt::Key_F9:
        return "\x1b[20~";
    case Qt::Key_F10:
        return "\x1b[21~";
    case Qt::Key_F11:
        return "\x1b[23~";
    case Qt::Key_F12:
        return "\x1b[24~";
    }

    // Handle special keys
    switch (key) {
    case Qt::Key_Home:
        return "\x1b[H";
    case Qt::Key_End:
        return "\x1b[F";
    case Qt::Key_Insert:
        return "\x1b[2~";
    case Qt::Key_Delete:
        return "\x1b[3~";
    case Qt::Key_PageUp:
        return "\x1b[5~";
    case Qt::Key_PageDown:
        return "\x1b[6~";
    case Qt::Key_Backspace:
        return "\x7f";
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return "\r";
    case Qt::Key_Tab:
        return "\t";
    case Qt::Key_Escape:
        return "\x1b";
    }

    // Handle printable characters
    if (!event->text().isEmpty()) {
        return event->text();
    }

    return QString();
}

QRect TerminalView::getCellRect(int row, int col) const
{
    int x = col * m_charWidth;
    int y = row * m_charHeight;
    return QRect(x, y, m_charWidth, m_charHeight);
}

void TerminalView::setBackgroundImage(const QString& imagePath)
{
    loadBackgroundImage(imagePath);
    update();
}

void TerminalView::setBackgroundImageOpacity(double opacity)
{
    m_backgroundImageOpacity = qBound(0.0, opacity, 1.0);
    update();
}

void TerminalView::setCustomFont(const QFont& font)
{
    m_font = font;
    m_font.setFixedPitch(true);
    m_font.setStyleHint(QFont::Monospace);
    calculateMetrics();
    update();
}

void TerminalView::setCustomColors(const QColor& fg, const QColor& bg)
{
    m_customForeground = fg;
    m_customBackground = bg;
    m_useCustomColors = true;

    // Update palette
    QPalette p = palette();
    p.setColor(QPalette::Window, bg);
    setPalette(p);

    update();
}

void TerminalView::loadBackgroundImage(const QString& path)
{
    m_backgroundImagePath = path;

    if (path.isEmpty()) {
        m_backgroundImage = QPixmap();
    } else {
        m_backgroundImage.load(path);
        if (m_backgroundImage.isNull()) {
            qWarning() << "Failed to load background image:" << path;
        }
    }
}

void TerminalView::onSettingsChanged()
{
    AppSettings* settings = AppSettings::instance();

    // Load background image
    QString bgImage = settings->terminalBackgroundImage();
    if (bgImage != m_backgroundImagePath) {
        loadBackgroundImage(bgImage);
    }

    // Load opacity
    m_backgroundImageOpacity = settings->backgroundImageOpacity();

    // Load font
    QFont font(settings->terminalFontFamily(), settings->terminalFontSize());
    setCustomFont(font);

    // Load colors
    setCustomColors(settings->terminalForegroundColor(), settings->terminalBackgroundColor());

    update();
}
