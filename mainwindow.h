/*  Copyright 2022 Javier Alvarez
    This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStatusBar>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QTextEdit>
#include <QCheckBox>
#include <QTimer>
#include <QCamera>
#include <QCameraViewfinder>
#include <QListView>
#include <QPushButton>
#include <QMutex>
#include <QStandardItemModel>



#include "tesseract/baseapi.h"

#include "opencv2/opencv.hpp"
#include "opencv2/dnn.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include "usb_camera.h"
#include "necta_camera.h"
#include "oakd_camera.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow();
    void showImage(QPixmap);
    QImage extractTextVideo(QImage frame);

private:
    void initUI();
    void createActions();
    void showImage(QString);
    void showImage(cv::Mat);
    void setupShortcuts();

    void decode(const cv::Mat& scores, const cv::Mat& geometry, float scoreThresh,
        std::vector<cv::RotatedRect>& detections, std::vector<float>& confidences);
    cv::Mat detectTextAreas(QImage &image, std::vector<cv::Rect>&);

private slots:
    void openImage();
    void saveImageAs();
    void saveTextAs();
    void extractText();
    void captureScreen();
    void startCapture();
    void zoomIn();
    void zoomOut();
    void extractDimensions();
    void showCameraInfo();
    void openOCRUSBCamera();
    void openNectaCamera();
    void openOakDCamera();
    void updateFrame(cv::Mat*);
    void updateFrameNecta();
    void aboutDialog();
    //Capture Video int CaptureVideo();

private:
    QMenu *imageMenu;
    QMenu *videoMenu;
    QMenu *videoUSBMenu;
    QMenu *configMenu;
    QMenu *helpMenu;

    QToolBar *fileToolBar;

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;

    QTextEdit *editor;

    QStatusBar *mainStatusBar;
    QLabel *mainStatusLabel;

    QAction *openAction;
    QAction *saveImageAsAction;
    QAction *saveTextAsAction;
    QAction *exitAction;
    QAction *captureAction;
    QAction *ocrAction;
    QCheckBox *detectAreaCheckBox;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *extractDimensionsAction;
    QAction *cameraInfoAction;
    QAction *OCRUSBcamera;
    QAction *calcFPSAction;
    QAction *NectaCamera;
    QAction *OakDCamera;
    QAction *aboutAction;

    QString currentImagePath;
    QGraphicsPixmapItem *currentImage;

    tesseract::TessBaseAPI *tesseractAPI;
    cv::dnn::Net net;
    QCamera *camera;
    QCameraViewfinder *viewfinder;

    cv::Mat currentFrame;

    // for capture thread
    QMutex *data_lock;
    USBCaptureThread *capturer;
    NectaCaptureThread *nectacapturer;
    OakdCaptureThread *oakdcapturer;

};




#endif // MAINWINDOW_H
