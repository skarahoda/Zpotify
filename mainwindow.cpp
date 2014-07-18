#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtMultimediaWidgets>
#include <QtWidgets>
#include <QtMultimedia>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
