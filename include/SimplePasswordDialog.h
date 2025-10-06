#ifndef SIMPLEPASSWORDDIALOG_H
#define SIMPLEPASSWORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class SimplePasswordDialog : public QDialog {
    Q_OBJECT

public:
    explicit SimplePasswordDialog(const QString& username, const QString& hostname, QWidget* parent = nullptr);
    QString password() const;

private:
    QLineEdit* m_passwordEdit;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
};

#endif // SIMPLEPASSWORDDIALOG_H
