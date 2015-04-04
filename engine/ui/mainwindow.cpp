#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "view.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QGLFormat qglFormat;
    qglFormat.setVersion(3, 3);
    qglFormat.setProfile(QGLFormat::CoreProfile);
    qglFormat.setSampleBuffers(true);

    ui->setupUi(this);

    QGridLayout *gridLayout = new QGridLayout(ui->view);
    View *view = new View(qglFormat, this);
    gridLayout->addWidget(view);
}

MainWindow::~MainWindow()
{
    delete ui;
}

