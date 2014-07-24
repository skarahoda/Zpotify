#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QDialogButtonBox;
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QString IP, QString userName, QString pass, QWidget *parent = 0);
    QString serverIP() const;
    QString username() const;
    QString password() const;
signals:

public slots:
    void verify();
private:
    QLabel *serverIPLabel;
    QLabel *usernameLabel;
    QLabel *passwordLabel;

    QLineEdit *serverIPEdit;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;

    QDialogButtonBox *buttons;
};

#endif // SETTINGSDIALOG_H
