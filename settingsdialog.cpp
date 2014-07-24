#include <QtWidgets>
#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QString IP, QString userName, QString pass, QWidget *parent) :
    QDialog(parent)
{
    QGridLayout *layout= new QGridLayout;

    serverIPLabel = new QLabel(tr("Server IP and Port:"));
    serverIPEdit = new QLineEdit(IP);
    layout->addWidget(serverIPLabel,0,0);
    layout->addWidget(serverIPEdit,0,1);

    usernameLabel = new QLabel(tr("Username:"));
    usernameEdit = new QLineEdit(userName);
    layout->addWidget(usernameLabel,1,0);
    layout->addWidget(usernameEdit,1,1);

    passwordLabel = new QLabel(tr("Password:"));
    passwordEdit = new QLineEdit(pass);
    layout->addWidget(passwordLabel,2,0);
    layout->addWidget(passwordEdit,2,1);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons,3,0);
    connect(buttons, SIGNAL(accepted()),this,SLOT(verify()));
    connect(buttons,SIGNAL(rejected()),this,SLOT(reject()));

    setWindowTitle(tr("Settings"));
    setLayout(layout);
}



void SettingsDialog::verify()
{
    if(serverIPEdit->text().isEmpty() || usernameEdit->text().isEmpty())
        QMessageBox::warning(this,tr("Incomplete"),tr("You have to fill IP and username"));
    else
        accept();
}

QString SettingsDialog::serverIP() const{
    return serverIPEdit->text();
}

QString SettingsDialog::username() const{
    return usernameEdit->text();
}

QString SettingsDialog::password() const{
    return passwordEdit->text();
}
