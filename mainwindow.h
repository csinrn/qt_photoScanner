#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QPixmap>
#include "imageprocessor.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    ImageProcessor* imaP = nullptr;
private slots:
    void on_BrowseBtn_In_clicked();

    void on_RunBtn_In_clicked();

    void on_Save_clicked();

private:
    Ui::MainWindow *ui;
signals:
    void startPic(QString path);
    void modifyFinished(QString pdfPath);
};

#endif // MAINWINDOW_H
