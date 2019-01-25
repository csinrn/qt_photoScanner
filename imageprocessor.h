#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include "QThread"
#include "opencv2/opencv.hpp"
#include <QDebug>
#include <QMainWindow>
#include "opencv2/opencv.hpp"
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QTextBlock>
using namespace cv;

typedef std::vector<std::vector<cv::Point>> vecs;

class ImageProcessor : public QObject//: public QThread
{
    Q_OBJECT
public:
    ImageProcessor();
    void run(QString path, bool colorful);
    int setFactor(int cols, int rows);
    Mat makeMask(Mat origin, vecs contours);
    void findVertices(vecs contour,int num, Mat mask_shrink, Mat masked_big);
    Mat perspectiveTransform(Mat masked_big, Mat mask_shrink);
    std::vector<Point2f> sortPoints2Vec(Point2f* points);
    QString GeneratePicWord(int width, int height,QString path);
    void saveAsPdf(QString pdfPath, QString imgPath, int picWidth, int picHeight);
    void onMouse(int Event,int x,int y,int flags);
    Point2f vertices[4];
    void test();

//private:
    int radius=10;
    int t=0;
    bool done = false;
    QString tempImgPath = "C:/Users/jenny/Desktop/temp.jpg";
    QString pdfPath;
    Mat src;
    Mat shrink;
    int catched;
    Scalar color[4] = {Scalar(255,0,0,255) , Scalar(255,0,0,255) , Scalar(255,0,0,255) , Scalar(255,0,0,255)};
signals :
    void finishPic(QPixmap);
public slots:
    void doneModify(QString pdfPath);
};

#endif // IMAGEPROCESSOR_H
