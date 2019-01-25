#include "mainwindow.h"
#include "ui_mainwindow.h"
using namespace cv;

// 不按save 直接關視窗的話會怎樣?
// 加黑白和彩色的選項
// pdf creater到底怎麼出來的快把它關掉=_=


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

void MainWindow::on_BrowseBtn_In_clicked()
{
    if(imaP != nullptr){
        delete imaP;
    }
    imaP = new ImageProcessor();
    connect(this,SIGNAL(modifyFinished(QString)),imaP,SLOT(doneModify(QString)) );

    QString fileName = QFileDialog::getOpenFileName(this, "Open Image",
                    "C:\\", "Image Files (*.png *.jpg)");
    ui->textEdit_In->setText(fileName);
}

void MainWindow::on_RunBtn_In_clicked()
{
    QString path = ui->textEdit_In->toPlainText();
    qDebug()<<path;
    ui->Status->setText("Press the Blue dots, move them to desired vertices");
    bool colorful = ui->checkBox->checkState();
    imaP->run(path, colorful);
}

void MainWindow::on_Save_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Pdf"),
                    "C:/Users/jenny/Desktop/temp.pdf", "Image Files (*.png *.jpg)");
    emit modifyFinished(fileName);
    ui->Status->setText("Done saving");
    delete imaP;
    imaP = nullptr;
}

