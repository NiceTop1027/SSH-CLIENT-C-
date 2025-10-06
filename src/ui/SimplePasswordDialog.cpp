#include "SimplePasswordDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

SimplePasswordDialog::SimplePasswordDialog(const QString& username, const QString& hostname, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("SSH Password");
    setModal(true);
    setFixedSize(350, 150);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Label
    QLabel* label = new QLabel(QString("Enter password for %1@%2:").arg(username).arg(hostname));
    mainLayout->addWidget(label);

    // Password field
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    mainLayout->addWidget(m_passwordEdit);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton("Cancel");
    m_okButton = new QPushButton("OK");
    m_okButton->setDefault(true);

    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);

    mainLayout->addLayout(buttonLayout);

    // Connections
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &QDialog::accept);

    m_passwordEdit->setFocus();
}

QString SimplePasswordDialog::password() const
{
    return m_passwordEdit->text();
}
