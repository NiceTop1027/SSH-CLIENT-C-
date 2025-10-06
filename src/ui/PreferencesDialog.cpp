#include "PreferencesDialog.h"
#include "AppSettings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QFontDatabase>
#include <QPixmap>
#include <QDir>
#include <QRegularExpression>

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
    , m_tabWidget(nullptr)
{
    setupUi();
    loadSettings();
    setWindowTitle("Preferences");
    setMinimumSize(600, 500);
    resize(700, 600);
}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::setupUi()
{
    setStyleSheet(R"(
        QDialog {
            background: #1E1E1E;
        }
        QTabWidget::pane {
            border: 1px solid #3A3A3A;
            border-radius: 10px;
            background: #1E1E1E;
            padding: 20px;
        }
        QTabBar::tab {
            background: transparent;
            color: #9E9E9E;
            padding: 12px 24px;
            margin-right: 2px;
            border: none;
            border-bottom: 2px solid transparent;
            font-size: 13px;
        }
        QTabBar::tab:selected {
            color: #64B5F6;
            border-bottom: 2px solid #64B5F6;
            font-weight: 500;
        }
        QTabBar::tab:hover {
            background: #252525;
            color: #BDBDBD;
        }
        QGroupBox {
            font-weight: 500;
            color: #BDBDBD;
            border: 1px solid #3A3A3A;
            border-radius: 10px;
            margin-top: 14px;
            padding-top: 18px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 10px;
            color: #64B5F6;
            font-weight: 500;
        }
        QLineEdit, QSpinBox, QDoubleSpinBox {
            background: #2A2A2A;
            color: #E8E8E8;
            border: 1px solid #3A3A3A;
            border-radius: 6px;
            padding: 10px 14px;
            font-size: 13px;
            selection-background-color: #1976D2;
        }
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {
            border: 1px solid #64B5F6;
            background: #2F2F2F;
        }
        QComboBox {
            background: #2A2A2A;
            color: #E8E8E8;
            border: 1px solid #3A3A3A;
            border-radius: 6px;
            padding: 10px 14px;
            font-size: 13px;
        }
        QComboBox:focus {
            border: 1px solid #64B5F6;
            background: #2F2F2F;
        }
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #9E9E9E;
            margin-right: 8px;
        }
        QComboBox QAbstractItemView {
            background: #2A2A2A;
            color: #E8E8E8;
            border: 1px solid #3A3A3A;
            selection-background-color: #3A3A3A;
            border-radius: 6px;
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #42A5F5, stop:1 #1E88E5);
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 11px 26px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #64B5F6, stop:1 #42A5F5);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #1976D2, stop:1 #1565C0);
        }
        QCheckBox {
            color: #BDBDBD;
            font-size: 13px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border-radius: 4px;
            border: 1px solid #3A3A3A;
            background: #2A2A2A;
        }
        QCheckBox::indicator:checked {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #42A5F5, stop:1 #1E88E5);
            border: 1px solid #42A5F5;
        }
        QSlider::groove:horizontal {
            border: none;
            height: 6px;
            background: #2A2A2A;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #64B5F6, stop:1 #42A5F5);
            border: none;
            width: 18px;
            height: 18px;
            margin: -6px 0;
            border-radius: 9px;
        }
        QSlider::handle:horizontal:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #90CAF9, stop:1 #64B5F6);
        }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                       stop:0 #42A5F5, stop:1 #1E88E5);
            border-radius: 3px;
        }
        QLabel {
            color: #9E9E9E;
            font-size: 13px;
        }
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    m_tabWidget = new QTabWidget(this);
    createAppearanceTab();
    createTerminalTab();
    createConnectionTab();

    mainLayout->addWidget(m_tabWidget);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_restoreDefaultsButton = new QPushButton("Restore Defaults");
    m_restoreDefaultsButton->setStyleSheet("background: #2A2A2A; color: #9E9E9E; border: 1px solid #3A3A3A;");
    buttonLayout->addWidget(m_restoreDefaultsButton);
    buttonLayout->addStretch();

    m_applyButton = new QPushButton("Apply");
    m_okButton = new QPushButton("OK");
    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setStyleSheet("background: #2A2A2A; color: #9E9E9E; border: 1px solid #3A3A3A;");

    m_okButton->setDefault(true);

    buttonLayout->addWidget(m_applyButton);
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);

    connect(m_applyButton, &QPushButton::clicked, this, &PreferencesDialog::onApply);
    connect(m_okButton, &QPushButton::clicked, this, &PreferencesDialog::onOk);
    connect(m_cancelButton, &QPushButton::clicked, this, &PreferencesDialog::onCancel);
    connect(m_restoreDefaultsButton, &QPushButton::clicked, this, &PreferencesDialog::onRestoreDefaults);
}

void PreferencesDialog::createAppearanceTab()
{
    m_appearanceTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_appearanceTab);

    // Background Image Group
    QGroupBox* imageGroup = new QGroupBox("Terminal Background Image");
    QVBoxLayout* imageLayout = new QVBoxLayout(imageGroup);

    QHBoxLayout* imageFileLayout = new QHBoxLayout();
    m_backgroundImageEdit = new QLineEdit();
    m_backgroundImageEdit->setPlaceholderText("No background image");
    m_backgroundImageEdit->setReadOnly(true);
    m_browseButton = new QPushButton("Browse...");
    m_clearImageButton = new QPushButton("Clear");

    imageFileLayout->addWidget(m_backgroundImageEdit);
    imageFileLayout->addWidget(m_browseButton);
    imageFileLayout->addWidget(m_clearImageButton);
    imageLayout->addLayout(imageFileLayout);

    // Image opacity
    QHBoxLayout* opacityLayout = new QHBoxLayout();
    opacityLayout->addWidget(new QLabel("Image Opacity:"));
    m_backgroundImageOpacitySlider = new QSlider(Qt::Horizontal);
    m_backgroundImageOpacitySlider->setRange(0, 100);
    m_backgroundImageOpacitySlider->setValue(30);
    m_backgroundImageOpacityLabel = new QLabel("30%");
    opacityLayout->addWidget(m_backgroundImageOpacitySlider);
    opacityLayout->addWidget(m_backgroundImageOpacityLabel);
    imageLayout->addLayout(opacityLayout);

    // Image preview
    m_imagePreviewLabel = new QLabel();
    m_imagePreviewLabel->setMinimumHeight(200);
    m_imagePreviewLabel->setMaximumHeight(250);
    m_imagePreviewLabel->setAlignment(Qt::AlignCenter);
    m_imagePreviewLabel->setStyleSheet("QLabel { border: 1px solid gray; background-color: black; }");
    m_imagePreviewLabel->setText("No image selected");
    imageLayout->addWidget(m_imagePreviewLabel);

    layout->addWidget(imageGroup);
    layout->addStretch();

    connect(m_browseButton, &QPushButton::clicked, this, &PreferencesDialog::onBrowseBackgroundImage);
    connect(m_clearImageButton, &QPushButton::clicked, this, &PreferencesDialog::onClearBackgroundImage);
    connect(m_backgroundImageOpacitySlider, &QSlider::valueChanged, this, &PreferencesDialog::onBackgroundImageOpacityChanged);

    m_tabWidget->addTab(m_appearanceTab, "Appearance");
}

void PreferencesDialog::createTerminalTab()
{
    m_terminalTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_terminalTab);

    // Font Group
    QGroupBox* fontGroup = new QGroupBox("Font");
    QFormLayout* fontLayout = new QFormLayout(fontGroup);

    m_fontFamilyCombo = new QComboBox();
    QStringList monospaceFonts;

    // Add common monospace fonts
    QStringList preferredFonts = {"Monaco", "Consolas", "Courier New", "Menlo", "DejaVu Sans Mono", "Liberation Mono", "Monospace"};
    QStringList availableFonts = QFontDatabase::families();
    for (const QString& font : preferredFonts) {
        if (availableFonts.contains(font)) {
            monospaceFonts.append(font);
        }
    }

    // Add other monospace fonts
    for (const QString& family : availableFonts) {
        if (QFontDatabase::isFixedPitch(family) && !monospaceFonts.contains(family)) {
            monospaceFonts.append(family);
        }
    }

    m_fontFamilyCombo->addItems(monospaceFonts);
    fontLayout->addRow("Font Family:", m_fontFamilyCombo);

    m_fontSizeSpin = new QSpinBox();
    m_fontSizeSpin->setRange(8, 32);
    m_fontSizeSpin->setValue(12);
    fontLayout->addRow("Font Size:", m_fontSizeSpin);

    layout->addWidget(fontGroup);

    // Colors Group
    QGroupBox* colorGroup = new QGroupBox("Colors");
    QFormLayout* colorLayout = new QFormLayout(colorGroup);

    QHBoxLayout* fgLayout = new QHBoxLayout();
    m_foregroundColorButton = new QPushButton("Choose Color...");
    m_foregroundColorPreview = new QLabel();
    m_foregroundColorPreview->setFixedSize(50, 25);
    m_foregroundColorPreview->setStyleSheet("background-color: white; border: 1px solid gray;");
    fgLayout->addWidget(m_foregroundColorButton);
    fgLayout->addWidget(m_foregroundColorPreview);
    fgLayout->addStretch();
    colorLayout->addRow("Foreground:", fgLayout);

    QHBoxLayout* bgLayout = new QHBoxLayout();
    m_backgroundColorButton = new QPushButton("Choose Color...");
    m_backgroundColorPreview = new QLabel();
    m_backgroundColorPreview->setFixedSize(50, 25);
    m_backgroundColorPreview->setStyleSheet("background-color: black; border: 1px solid gray;");
    bgLayout->addWidget(m_backgroundColorButton);
    bgLayout->addWidget(m_backgroundColorPreview);
    bgLayout->addStretch();
    colorLayout->addRow("Background:", bgLayout);

    layout->addWidget(colorGroup);

    // Opacity Group
    QGroupBox* opacityGroup = new QGroupBox("Window Transparency");
    QHBoxLayout* opacityLayout = new QHBoxLayout(opacityGroup);
    opacityLayout->addWidget(new QLabel("Opacity:"));
    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(50, 100);
    m_opacitySlider->setValue(100);
    m_opacityLabel = new QLabel("100%");
    opacityLayout->addWidget(m_opacitySlider);
    opacityLayout->addWidget(m_opacityLabel);

    layout->addWidget(opacityGroup);
    layout->addStretch();

    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferencesDialog::onFontSizeChanged);
    connect(m_opacitySlider, &QSlider::valueChanged, this, &PreferencesDialog::onOpacityChanged);
    connect(m_foregroundColorButton, &QPushButton::clicked, this, &PreferencesDialog::onForegroundColorClicked);
    connect(m_backgroundColorButton, &QPushButton::clicked, this, &PreferencesDialog::onBackgroundColorClicked);

    m_tabWidget->addTab(m_terminalTab, "Terminal");
}

void PreferencesDialog::createConnectionTab()
{
    m_connectionTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_connectionTab);

    QGroupBox* historyGroup = new QGroupBox("Connection History");
    QFormLayout* historyLayout = new QFormLayout(historyGroup);

    m_maxRecentSpin = new QSpinBox();
    m_maxRecentSpin->setRange(5, 50);
    m_maxRecentSpin->setValue(10);
    historyLayout->addRow("Max Recent Connections:", m_maxRecentSpin);

    layout->addWidget(historyGroup);

    QGroupBox* windowGroup = new QGroupBox("Window Settings");
    QVBoxLayout* windowLayout = new QVBoxLayout(windowGroup);

    m_rememberWindowSizeCheck = new QCheckBox("Remember window size");
    m_rememberWindowSizeCheck->setChecked(true);
    windowLayout->addWidget(m_rememberWindowSizeCheck);

    layout->addWidget(windowGroup);
    layout->addStretch();

    m_tabWidget->addTab(m_connectionTab, "Connection");
}

void PreferencesDialog::onBrowseBackgroundImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select Background Image",
        QDir::homePath(),
        "Images (*.png *.jpg *.jpeg *.bmp *.gif);;All Files (*)",
        nullptr,
        QFileDialog::DontUseNativeDialog  // Force Qt dialog instead of native
    );

    if (!fileName.isEmpty()) {
        m_backgroundImageEdit->setText(fileName);

        // Update preview
        QPixmap pixmap(fileName);
        if (!pixmap.isNull()) {
            QPixmap scaled = pixmap.scaled(m_imagePreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_imagePreviewLabel->setPixmap(scaled);
        }
    }
}

void PreferencesDialog::onClearBackgroundImage()
{
    m_backgroundImageEdit->clear();
    m_imagePreviewLabel->clear();
    m_imagePreviewLabel->setText("No image selected");
}

void PreferencesDialog::onBackgroundImageOpacityChanged(int value)
{
    m_backgroundImageOpacityLabel->setText(QString("%1%").arg(value));
}

void PreferencesDialog::onFontSizeChanged(int /* value */)
{
    // Preview could be added here
}

void PreferencesDialog::onOpacityChanged(int value)
{
    m_opacityLabel->setText(QString("%1%").arg(value));
}

void PreferencesDialog::onForegroundColorClicked()
{
    QColor currentColor = m_foregroundColorPreview->palette().color(QPalette::Window);
    QColor color = QColorDialog::getColor(currentColor, this, "Select Foreground Color");

    if (color.isValid()) {
        m_foregroundColorPreview->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(color.name()));
    }
}

void PreferencesDialog::onBackgroundColorClicked()
{
    QColor currentColor = m_backgroundColorPreview->palette().color(QPalette::Window);
    QColor color = QColorDialog::getColor(currentColor, this, "Select Background Color");

    if (color.isValid()) {
        m_backgroundColorPreview->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(color.name()));
    }
}

void PreferencesDialog::onApply()
{
    saveSettings();
}

void PreferencesDialog::onOk()
{
    saveSettings();
    accept();
}

void PreferencesDialog::onCancel()
{
    reject();
}

void PreferencesDialog::onRestoreDefaults()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Restore Defaults",
                                                               "Are you sure you want to restore all settings to defaults?",
                                                               QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        AppSettings* settings = AppSettings::instance();
        settings->setTerminalBackgroundImage("");
        settings->setTerminalFontSize(12);
#ifdef Q_OS_MAC
        settings->setTerminalFontFamily("Monaco");
#elif defined(Q_OS_WIN)
        settings->setTerminalFontFamily("Consolas");
#else
        settings->setTerminalFontFamily("Monospace");
#endif
        settings->setTerminalOpacity(1.0);
        settings->setTerminalForegroundColor(Qt::white);
        settings->setTerminalBackgroundColor(Qt::black);
        settings->setBackgroundImageOpacity(0.3);
        settings->setRememberWindowSize(true);
        settings->setMaxRecentConnections(10);

        loadSettings();
    }
}

void PreferencesDialog::loadSettings()
{
    AppSettings* settings = AppSettings::instance();

    // Appearance
    m_backgroundImageEdit->setText(settings->terminalBackgroundImage());
    m_backgroundImageOpacitySlider->setValue(static_cast<int>(settings->backgroundImageOpacity() * 100));

    if (!settings->terminalBackgroundImage().isEmpty()) {
        QPixmap pixmap(settings->terminalBackgroundImage());
        if (!pixmap.isNull()) {
            QPixmap scaled = pixmap.scaled(m_imagePreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_imagePreviewLabel->setPixmap(scaled);
        }
    }

    // Terminal
    int fontIndex = m_fontFamilyCombo->findText(settings->terminalFontFamily());
    if (fontIndex >= 0) {
        m_fontFamilyCombo->setCurrentIndex(fontIndex);
    }
    m_fontSizeSpin->setValue(settings->terminalFontSize());
    m_opacitySlider->setValue(static_cast<int>(settings->terminalOpacity() * 100));

    QColor fgColor = settings->terminalForegroundColor();
    m_foregroundColorPreview->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(fgColor.name()));

    QColor bgColor = settings->terminalBackgroundColor();
    m_backgroundColorPreview->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(bgColor.name()));

    // Connection
    m_maxRecentSpin->setValue(settings->maxRecentConnections());
    m_rememberWindowSizeCheck->setChecked(settings->rememberWindowSize());
}

void PreferencesDialog::saveSettings()
{
    AppSettings* settings = AppSettings::instance();

    // Appearance
    settings->setTerminalBackgroundImage(m_backgroundImageEdit->text());
    settings->setBackgroundImageOpacity(m_backgroundImageOpacitySlider->value() / 100.0);

    // Terminal
    settings->setTerminalFontFamily(m_fontFamilyCombo->currentText());
    settings->setTerminalFontSize(m_fontSizeSpin->value());
    settings->setTerminalOpacity(m_opacitySlider->value() / 100.0);

    // Extract colors from preview labels
    QString fgStyle = m_foregroundColorPreview->styleSheet();
    QRegularExpression fgRegex("background-color:\\s*([^;]+)");
    QRegularExpressionMatch fgMatch = fgRegex.match(fgStyle);
    if (fgMatch.hasMatch()) {
        settings->setTerminalForegroundColor(QColor(fgMatch.captured(1).trimmed()));
    }

    QString bgStyle = m_backgroundColorPreview->styleSheet();
    QRegularExpressionMatch bgMatch = fgRegex.match(bgStyle);
    if (bgMatch.hasMatch()) {
        settings->setTerminalBackgroundColor(QColor(bgMatch.captured(1).trimmed()));
    }

    // Connection
    settings->setMaxRecentConnections(m_maxRecentSpin->value());
    settings->setRememberWindowSize(m_rememberWindowSizeCheck->isChecked());

    settings->save();
}
