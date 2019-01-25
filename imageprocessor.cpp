#include "imageprocessor.h"
using namespace cv;

//加 try catch
// 加滿版或置中或維持比例滿版

void onMouseWrap(int Event,int x,int y,int flags,void* param);

ImageProcessor::ImageProcessor()
{
    catched = -1;
}

void ImageProcessor::run(QString path, bool colorful){
    // read to get verticed
    // imshow and change verticed ( run in while(flag) )
    // mainWindow save clicked, end while
    // prospective change
    // save and to close run

    src = imread(path.toStdString());
    Mat a = src.clone();

    // calculate shrinks
    int factor = setFactor(a.cols,a.rows);

    // canny
    cv::cvtColor(a,a,CV_BGR2GRAY);
    double otsu_thresh_val = cv::threshold(a, a, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
    cv::resize(src,a,cv::Size(a.rows/factor,a.cols/factor));
    //imshow("origin",a);
    Mat dst;
    cv::Canny(a,dst,otsu_thresh_val*0.5,otsu_thresh_val);
    //imshow("test_canny",dst);
    dilate(dst,dst,Mat());

    // make shrink mask by canny
    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;
    findContours(dst,contours,hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    Mat mask = makeMask(dst,contours);
//    imshow("test_mask",mask);

    //make a more correct mask by grabcut
    Mat t,f,g,mask_g;
    mask_g = mask.clone();
    for(int i = 0 ;i<mask_g.rows;i++){
        for(int j=0;j<mask_g.cols;j++){
            if(mask_g.at<char>(i,j) == 0)
                mask_g.at<char>(i,j)=GC_BGD;
            else
                mask_g.at<char>(i,j)=GC_PR_FGD;
        }
    }
    grabCut(a,mask_g,Rect(0,0,0,0),f,g,2,GC_INIT_WITH_MASK);
    compare(mask_g,GC_PR_FGD,mask,CMP_EQ);
    //imshow("test_mask2",mask);


    Mat masked = src.clone();
    cvtColor(masked,masked,CV_BGR2GRAY);// cut if colorful

    // find the correct mask_small contours for perspectiveTransform
    findContours(mask,contours,hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    double max = 0;
    int num = 0;
    for(int i = 0; i<contours.size();i++){
        if(contourArea(contours[i])>max){
            max = contourArea(contours[i]);
            num = i;
        }
    }

    // do adaptiveThresh and perspectiveTransform        // cut if colorful
    adaptiveThreshold(masked, masked, 255, ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 21, 10);
    findVertices(contours,num,mask,masked);

    // run 用彩色or黑白縮圖標完點(畫邊線?)顯示後 開始空轉while

    this->shrink = a.clone();
    if(!colorful){
        this->src = masked;
    }
    for(int i=0; i<4; i++){
        circle(a,vertices[i],radius,Scalar(255),-1);
        qDebug()<<vertices[i].x;
    }
    namedWindow("vertices check",WINDOW_KEEPRATIO);
    resizeWindow("vertices check", a.rows, a.cols);
    setMouseCallback("vertices check",onMouseWrap,this);
    imshow("vertices check",a);
}

Mat ImageProcessor::makeMask(Mat origin_c, vecs contours){
    int num;
    double max = 0;
    for(int i = 0; i<contours.size();i++){
        if(contourArea(contours[i])>max){
            max = contourArea(contours[i]);
            num = i;
        }
    }
    Mat mask(origin_c.size(), CV_8U, Scalar(0));
    drawContours(mask, contours, num, Scalar(255),CV_FILLED);

    return mask;
}

void ImageProcessor::findVertices(vecs contours,int num, Mat mask_shrink, Mat masked_big){
    //imshow("per_beforeT",masked_big);

    // draw convex for goodFeatureTrack to track
    vecs hull(1);
    convexHull(Mat(contours[num]),hull[0]);  //消除mask_shrink中可能的內凹角點
    Mat hu(mask_shrink.rows,mask_shrink.cols,CV_8UC1,Scalar(0));
    //for(int i = 0; i< hull.size(); i++)
    drawContours(hu, hull, 0, Scalar(255),2,8);
//    imshow("hull_test",hu);

    // make mask for goodFeatureTrack
    Mat mask(mask_shrink.rows,mask_shrink.cols,CV_8UC1,Scalar(0));
    drawContours(mask,contours,num,Scalar(255),30,LINE_8);
//    imshow("corner pre mask_test",mask);

    // goodFeaturesToTrack
    std::vector<Point2i> corner_tracker;
    goodFeaturesToTrack(hu,corner_tracker,4,0.05,50,mask,4,false);  //正常照片的尺寸是多少? 除十後角距設50會太多嗎?
    for(int i = 0; i< corner_tracker.size(); i++){
        circle(hu,corner_tracker[i],30,Scalar(255),-1);
        qDebug()<<"c"<<i;
    }
//    imshow("goodFeatures",hu);

    // renew vertices, vertices is for shrink
    for(int i=0; i<4; i++){
        vertices[i] = Point2f(corner_tracker[i].x, corner_tracker[i].y);
    }
}

Mat ImageProcessor::perspectiveTransform(Mat masked_big, Mat mask_shrink){
    // map vertices to masked_big
    float f_c = (float)masked_big.cols/(float)mask_shrink.cols;
    float f_r = (float)masked_big.rows/(float)mask_shrink.rows;
    Point2f k[4] = {Point2f(vertices[0].x*f_c,vertices[0].y*f_r),
                    Point2f(vertices[1].x*f_c,vertices[1].y*f_r),
                    Point2f(vertices[2].x*f_c,vertices[2].y*f_r),
                    Point2f(vertices[3].x*f_c,vertices[3].y*f_r)};
    std::vector<Point2f> q = sortPoints2Vec(k);
//    imshow("per_m",masked_big);

    // set the final output size
    int length_y = sqrt(pow(q[0].x-q[1].x,2) + pow(q[0].y-q[1].y,2));
    int length_x = sqrt(pow(q[1].x-q[2].x,2) + pow(q[1].y-q[2].y,2));
    Point2f p[4] = {Point(length_x,length_y),
                              Point(0,length_y),
                              Point(0,0),
                              Point(length_x,0)};
    std::vector<Point2f> P = sortPoints2Vec(p);

    // transform
    cv::Mat transmtx = cv::getPerspectiveTransform(q, P);
    cv::warpPerspective(masked_big,masked_big, transmtx, Size(length_x,length_y));

    //cv::resize(masked_big,masked_big,cv::Size(masked_big.rows/4,masked_big.cols/4));
    //imshow("per",masked_big);
    //cv::resize(masked_big,masked_big,cv::Size(masked_big.rows*4,masked_big.cols*4));

    return masked_big;
}

std::vector<Point2f>ImageProcessor::sortPoints2Vec(Point2f* points){
    // leftbot -> lefttop -> righttop -> rightbot
    for(int i = 1; i<4;i++){
        for(int j = i-1; j>=0;j--){
            if(points[j+1].x<points[j].x){
                Point2f temp = points[j+1];
                points[j+1] = points[j];
                points[j] = temp;
            }else{
                break;
            }
        }
    }
    if(points[0].y > points[1].y){
        Point2f temp = points[0];
        points[0] = points[1];
        points[1] = temp;
    }
    if(points[2].y < points[3].y){
        Point2f temp = points[2];
        points[2] = points[3];
        points[3] = temp;
    }

    std::vector<Point2f> cornerPoints;
    cornerPoints.push_back(points[0]);
    cornerPoints.push_back(points[1]);
    cornerPoints.push_back(points[2]);
    cornerPoints.push_back(points[3]);
    return cornerPoints;
}


QString ImageProcessor::GeneratePicWord(int width, int height, QString path){

    QString html;
    QString a = "<div><img align=\"middle\" src = ";
    QString b = "\"" + path + "\" ";
    //QString c = "width=\""+QString::number(width)+"\" height=\""+QString::number(height)+"\"></div>" ;
    QString c = "width=\" "+ QString::number(width) +"\" height=\""+QString::number(height)+ "\"></div>" ;
//?    //1190 1684
    html = a + b + c;
    return html;
}

void ImageProcessor::saveAsPdf(QString pdfPath, QString imgPath, int picWidth, int picHeight){
    QPrinter printer_text(QPrinter::ScreenResolution);
    printer_text.setOutputFormat(QPrinter::PdfFormat);
    printer_text.setOutputFileName(pdfPath);//pdfname为要保存的pdf文件名
    printer_text.setFullPage(true);
    printer_text.setPageMargins(QMarginsF(0,0,0,0));

    float factor;
    float factor_h;
    float factor_w;
    factor_h = (float)picHeight/(float)printer_text.pageRect().height();
    factor_w = (float)picWidth/(float)printer_text.pageRect().width();
    qDebug()<<"f"<<factor_h<<picWidth <<printer_text.pageRect().width()<<factor_w;
    factor = factor_h > factor_w ? factor_h : factor_w;

    QTextDocument text_document;
    QString html = GeneratePicWord(picWidth/factor,picHeight/factor,imgPath);

    text_document.setPageSize(QSizeF(printer_text.pageRect().size()));
    text_document.setDocumentMargin(0);
    text_document.setHtml(html);
    text_document.print(&printer_text);
    text_document.end();
}

int ImageProcessor::setFactor(int cols, int rows){
    int factors[10] = {10,9,8,7,6,5,4,3,2,1};
    for(int i = 0; i<10;i++){
        if(cols%factors[i]==0 && rows%factors[i]==0)
            return factors[i];
    }
    return 1;
}

void onMouseWrap(int Event,int x,int y,int flags,void* param){
    ImageProcessor* mainWin = (ImageProcessor*)(param);
    mainWin->onMouse(Event,x,y,flags);
}

void ImageProcessor::onMouse(int Event,int x,int y,int flags ){
    bool changed = false;

    if( Event == CV_EVENT_LBUTTONUP){
        for(int i=0; i<4; i++){
            //qDebug()<<(x-vertices[i].x)*(x-vertices[i].x)+(y-vertices[i].y)*(y-vertices[i].y)<<radius;
            if(( (x-vertices[i].x)*(x-vertices[i].x)+(y-vertices[i].y)*(y-vertices[i].y) ) < radius*radius){
                if(catched == -1){   //catch in the first time
                    qDebug()<<"catch in the first time";
                    catched = i;
                    color[i] = Scalar(0,255,0,255);
                }
                else{   // place at the other point
                    qDebug()<<"place at the other point";
                    vertices[catched] = Point2f(x,y);
                    color[i] = Scalar(255,0,0,255);
                    catched = -1;
                }
                changed = true;
                break;
            }else{    // place at the other place
                if(catched != -1){
                    qDebug()<<"place at the other place";
                    vertices[catched] = Point2f(x,y);
                    color[catched] = Scalar(255,0,0,255);
                    catched = -1;
                    changed = true;
                    break;
                }
            }
        }
    }
    if(changed){
        Mat a;
        shrink.copyTo(a);
        for(int i=0; i<4; i++)
            circle(a,vertices[i],radius,color[i],-1);
        imshow("vertices check",a);
    }
}

void ImageProcessor::doneModify(QString pdfPath){
    done = true;
    this->pdfPath = pdfPath;
    destroyWindow("vertices check");

    //  perspectiveTransform
    Mat final = perspectiveTransform(src,shrink);

    // save
    imwrite(tempImgPath.toStdString(),final);
    saveAsPdf(this->pdfPath, this->tempImgPath, final.cols, final.rows);
}

void ImageProcessor::test(){
    imshow("tt",shrink);
}
