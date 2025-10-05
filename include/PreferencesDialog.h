#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>

class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget* parent = nullptr);
    ~PreferencesDialog();

private slots:
    void onBrowseBackgroundImage();
    void onClearBackgroundImage();
    void onBackgroundImageOpacityChanged(int value);
    void onFontSizeChanged(int value);
    void onOpacityChanged(int value);
    void onForegroundColorClicked();
    void onBackgroundColorClicked();
    void onApply();
    void onOk();
    void onCancel();
    void onRestoreDefaults();

private:
    void setupUi();
    void createAppearanceTab();
    void createTerminalTab();
    void createConnectionTab();
    void loadSettings();
    void saveSettings();

    QTabWidget* m_tabWidget;

    // Appearance tab
    QWidget* m_appearanceTab;
    QLineEdit* m_backgroundImageEdit;
    QPushButton* m_browseButton;
    QPushButton* m_clearImageButton;
    QSlider* m_backgroundImageOpacitySlider;
    QLabel* m_backgroundImageOpacityLabel;
    QLabel* m_imagePreviewLabel;

    // Terminal tab
    QWidget* m_terminalTab;
    QComboBox* m_fontFamilyCombo;
    QSpinBox* m_fontSizeSpin;
    QSlider* m_opacitySlider;
    QLabel* m_opacityLabel;
    QPushButton* m_foregroundColorButton;
    QPushButton* m_backgroundColorButton;
    QLabel* m_foregroundColorPreview;
    QLabel* m_backgroundColorPreview;

    // Connection tab
    QWidget* m_connectionTab;
    QSpinBox* m_maxRecentSpin;
    QCheckBox* m_rememberWindowSizeCheck;

    // Buttons
    QPushButton* m_applyButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_restoreDefaultsButton;
};

#endif // PREFERENCESDIALOG_H
