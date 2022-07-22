/*  Copyright 2022 Javier Alvarez
    This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSplitter>
#include <QDebug>
#include <QCameraInfo>
#include <QPixmap>
#include <QKeyEvent>
#include <QDebug>
#include <QGridLayout>
#include <QIcon>
#include <QStandardItem>
#include <QSize>
#include <unistd.h>

#include "opencv2/videoio.hpp"

#include "mainwindow.h"
#include "screencapturer.h"
#include "utilities.h"





MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
    , currentImage(nullptr)
    , tesseractAPI(nullptr)
//    , fileMenu(nullptr)
//    , capturer(nullptr)
{
    initUI();
    data_lock = new QMutex();
}

MainWindow::~MainWindow()
{
    // Destroy used object and release memory
    if(tesseractAPI != nullptr) {
        tesseractAPI->End();
        delete tesseractAPI;
    }
}

void MainWindow::initUI()
{
    this->resize(800, 600);
    // setup menubar
    imageMenu = menuBar()->addMenu("&Image");
    videoMenu = menuBar()->addMenu("&Video");
    configMenu = menuBar()->addMenu("&Config");
    helpMenu = menuBar()->addMenu("&Help");

    // setup toolbar
    fileToolBar = addToolBar("File");


    // main area
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    imageScene = new QGraphicsScene(this);
    imageView = new QGraphicsView(imageScene);
    splitter->addWidget(imageView);

    editor = new QTextEdit(this);
    splitter->addWidget(editor);

    QList<int> sizes = {400, 400};
    splitter->setSizes(sizes);

    setCentralWidget(splitter);

    // setup status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("Application information will be here!");

    createActions();
}

void MainWindow::createActions()
{
    // create actions, add them to menus
    openAction = new QAction("&Open", this);
    imageMenu->addAction(openAction);
    zoomInAction = new QAction("Zoom in", this);
    imageMenu->addAction(zoomInAction);
    zoomOutAction = new QAction("Zoom Out", this);
    imageMenu->addAction(zoomOutAction);
    ocrAction = new QAction("OCR", this);
    imageMenu->addAction(ocrAction);
    extractDimensionsAction = new QAction("Dimensions", this);
    imageMenu->addAction(extractDimensionsAction);
    saveImageAsAction = new QAction("Save &Image as", this);
    imageMenu->addAction(saveImageAsAction);
    saveTextAsAction = new QAction("Save &Text as", this);
    imageMenu->addAction(saveTextAsAction);
    exitAction = new QAction("&Quit", this);
    imageMenu->addAction(exitAction);
    cameraInfoAction = new QAction("&Camera info", this);
    configMenu->addAction(cameraInfoAction);  
    USBcamera = new QAction("&USB Camera", this);
    videoMenu->addAction(USBcamera);
    NectaCamera = new QAction("&Necta Camera", this);
    videoMenu->addAction(NectaCamera);
    OakDCamera = new QAction("&OAK-D Camera", this);
    videoMenu->addAction(OakDCamera);
    aboutAction = new QAction("About", this);
    helpMenu->addAction(aboutAction);

    // add actions to toolbars
    captureAction = new QAction("Screen shot", this);
    fileToolBar->addAction(captureAction);
    detectAreaCheckBox = new QCheckBox("Detect Text Areas", this);
    fileToolBar->addWidget(detectAreaCheckBox);


    // connect the signals and slots
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openImage()));
    connect(saveImageAsAction, SIGNAL(triggered(bool)), this, SLOT(saveImageAs()));
    connect(saveTextAsAction, SIGNAL(triggered(bool)), this, SLOT(saveTextAs()));
    connect(ocrAction, SIGNAL(triggered(bool)), this, SLOT(extractText()));
    connect(extractDimensionsAction, SIGNAL(triggered(bool)), this, SLOT(extractDimensions()));
    connect(captureAction, SIGNAL(triggered(bool)), this, SLOT(captureScreen()));
    connect(zoomInAction, SIGNAL(triggered(bool)), this, SLOT(zoomIn()));
    connect(zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(zoomOut()));
    connect(cameraInfoAction, SIGNAL(triggered(bool)), this, SLOT(showCameraInfo()));
    connect(USBcamera, SIGNAL(triggered(bool)), this, SLOT(openUSBCamera()));
    connect(NectaCamera, SIGNAL(triggered(bool)), this, SLOT(openNectaCamera()));
    connect(OakDCamera, SIGNAL(triggered(bool)), this, SLOT(openOakDCamera()));
    connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(aboutDialog()));
    setupShortcuts();
}

void MainWindow::openImage()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle("Open Image");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
    QStringList filePaths;
    if (dialog.exec()) {
        filePaths = dialog.selectedFiles();
        showImage(filePaths.at(0));
    }
}


void MainWindow::showImage(QPixmap image)
{
    imageScene->clear();
    imageView->resetMatrix();
    currentImage = imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
 }

void MainWindow::showImage(QString path)
{
    QPixmap image(path);
    showImage(image);
    currentImagePath = path;
    QString status = QString("%1, %2x%3, %4 Bytes").arg(path).arg(image.width())
        .arg(image.height()).arg(QFile(path).size());
    mainStatusLabel->setText(status);
}

void MainWindow::showImage(cv::Mat mat)
{
    QImage image(
        mat.data,
        mat.cols,
        mat.rows,
        mat.step,
        QImage::Format_RGB888);

    QPixmap pixmap = QPixmap::fromImage(image);
    imageScene->clear();
    imageView->resetMatrix();
    currentImage = imageScene->addPixmap(pixmap);
    imageScene->update();
    imageView->setSceneRect(pixmap.rect());
}

void MainWindow::saveImageAs()
{
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Nothing to save.");
        return;
    }
    QFileDialog dialog(this);
    dialog.setWindowTitle("Save Image as ...");
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
        if(QRegExp(".+\\.(png|bmp|jpg)").exactMatch(fileNames.at(0))) {
            currentImage->pixmap().save(fileNames.at(0));
        } else {
            QMessageBox::information(this, "Error", "Save error: Bad format or file.");
        }
    }
}

void MainWindow::saveTextAs()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle("Save Text as ...");
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Files of text (*.txt)"));
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
        if(QRegExp(".+\\.(txt)").exactMatch(fileNames.at(0))) {
            QFile file(fileNames.at(0));
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::information(this, "Error", "Test can't be saved.");
                return;
            }
            QTextStream out(&file);
            out << editor->toPlainText() << "\n";
        } else {
            QMessageBox::information(this, "Error", "Save error: Bad format or file.");
        }
    }
}

void MainWindow::setupShortcuts()
{
    QList<QKeySequence> shortcuts;
    shortcuts << (Qt::CTRL + Qt::Key_O);
    openAction->setShortcuts(shortcuts);

    shortcuts.clear();
    shortcuts << (Qt::CTRL + Qt::Key_Q);
    exitAction->setShortcuts(shortcuts);
}

void MainWindow::extractText()
{
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Image not opened.");
        return;
    }

    char *old_ctype = strdup(setlocale(LC_ALL, NULL));
    setlocale(LC_ALL, "C");
    if (tesseractAPI == nullptr) {
        tesseractAPI = new tesseract::TessBaseAPI();
        // Initialize tesseract-ocr with English, with specifying tessdata path
        if (tesseractAPI->Init(TESSDATA_PREFIX, "eng")) {
            QMessageBox::information(this, "Error", "Tesseract could not be initialized.");
            return;
        }
    }

    QPixmap pixmap = currentImage->pixmap();
    QImage image = pixmap.toImage();
    image = image.convertToFormat(QImage::Format_RGB888);

    tesseractAPI->SetImage(image.bits(), image.width(), image.height(),
        3, image.bytesPerLine());

    if (detectAreaCheckBox->checkState() == Qt::Checked) {
        std::vector<cv::Rect> areas;
        cv::Mat newImage = detectTextAreas(image, areas);
        showImage(newImage);
        editor->setPlainText("");
        for(cv::Rect &rect : areas) {
            tesseractAPI->SetRectangle(rect.x, rect.y, rect.width, rect.height);
            char *outText = tesseractAPI->GetUTF8Text();
            editor->setPlainText(editor->toPlainText() + outText);
            delete [] outText;
        }
    } else {
        char *outText = tesseractAPI->GetUTF8Text();
        editor->setPlainText(outText);
        delete [] outText;
    }

    setlocale(LC_ALL, old_ctype);
    free(old_ctype);
}

cv::Mat MainWindow::detectTextAreas(QImage &image, std::vector<cv::Rect> &areas)
{
    float confThreshold = 0.5;
    float nmsThreshold = 0.4;
    int inputWidth = 320;
    int inputHeight = 320;
    std::string model = "./frozen_east_text_detection.pb";
    // Load DNN network.
    if (net.empty()) {
        net = cv::dnn::readNet(model);
    }

    std::vector<cv::Mat> outs;
    std::vector<std::string> layerNames(2);
    layerNames[0] = "feature_fusion/Conv_7/Sigmoid";
    layerNames[1] = "feature_fusion/concat_3";

    cv::Mat frame = cv::Mat(
        image.height(),
        image.width(),
        CV_8UC3,
        image.bits(),
        image.bytesPerLine()).clone();
    cv::Mat blob;

    cv::dnn::blobFromImage(
        frame, blob,
        1.0, cv::Size(inputWidth, inputHeight),
        cv::Scalar(123.68, 116.78, 103.94), true, false
    );
    net.setInput(blob);
    net.forward(outs, layerNames);

    cv::Mat scores = outs[0];
    cv::Mat geometry = outs[1];

    std::vector<cv::RotatedRect> boxes;
    std::vector<float> confidences;
    decode(scores, geometry, confThreshold, boxes, confidences);

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    // Render detections.
    cv::Point2f ratio((float)frame.cols / inputWidth, (float)frame.rows / inputHeight);
    cv::Scalar green = cv::Scalar(0, 255, 0);

    for (size_t i = 0; i < indices.size(); ++i) {
        cv::RotatedRect& box = boxes[indices[i]];
        cv::Rect area = box.boundingRect();
        area.x *= ratio.x;
        area.width *= ratio.x;
        area.y *= ratio.y;
        area.height *= ratio.y;
        areas.push_back(area);
        cv::rectangle(frame, area, green, 1);
        QString index = QString("%1").arg(i);
        cv::putText(
            frame, index.toStdString(), cv::Point2f(area.x, area.y - 2),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, green, 1
        );
    }
    return frame;
}

void MainWindow::decode(const cv::Mat& scores, const cv::Mat& geometry, float scoreThresh,
    std::vector<cv::RotatedRect>& detections, std::vector<float>& confidences)
{
    CV_Assert(scores.dims == 4); CV_Assert(geometry.dims == 4);
    CV_Assert(scores.size[0] == 1); CV_Assert(scores.size[1] == 1);
    CV_Assert(geometry.size[0] == 1);  CV_Assert(geometry.size[1] == 5);
    CV_Assert(scores.size[2] == geometry.size[2]);
    CV_Assert(scores.size[3] == geometry.size[3]);

    detections.clear();
    const int height = scores.size[2];
    const int width = scores.size[3];
    for (int y = 0; y < height; ++y) {
        const float* scoresData = scores.ptr<float>(0, 0, y);
        const float* x0_data = geometry.ptr<float>(0, 0, y);
        const float* x1_data = geometry.ptr<float>(0, 1, y);
        const float* x2_data = geometry.ptr<float>(0, 2, y);
        const float* x3_data = geometry.ptr<float>(0, 3, y);
        const float* anglesData = geometry.ptr<float>(0, 4, y);
        for (int x = 0; x < width; ++x) {
            float score = scoresData[x];
            if (score < scoreThresh)
                continue;

            // Decode a prediction.
            // Multiple by 4 because feature maps are 4 time less than input image.
            float offsetX = x * 4.0f, offsetY = y * 4.0f;
            float angle = anglesData[x];
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);
            float h = x0_data[x] + x2_data[x];
            float w = x1_data[x] + x3_data[x];

            cv::Point2f offset(offsetX + cosA * x1_data[x] + sinA * x2_data[x],
                offsetY - sinA * x1_data[x] + cosA * x2_data[x]);
            cv::Point2f p1 = cv::Point2f(-sinA * h, -cosA * h) + offset;
            cv::Point2f p3 = cv::Point2f(-cosA * w, sinA * w) + offset;
            cv::RotatedRect r(0.5f * (p1 + p3), cv::Size2f(w, h), -angle * 180.0f / (float)CV_PI);
            detections.push_back(r);
            confidences.push_back(score);
        }
    }
}

void MainWindow::captureScreen()
{
    this->setWindowState(this->windowState() | Qt::WindowMinimized);
    QTimer::singleShot(500, this, SLOT(startCapture()));
}

void MainWindow::startCapture()
{
    ScreenCapturer *cap = new ScreenCapturer(this);
    cap->show();
    cap->activateWindow();
}
void MainWindow::zoomIn()
{
    imageView->scale(1.2, 1.2);
}

void MainWindow::zoomOut()
{
    imageView->scale(1/1.2, 1/1.2);
}

void MainWindow::showCameraInfo()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QString info = QString("Available Cameras: \n");

    foreach (const QCameraInfo &cameraInfo, cameras) {
        info += "  - " + cameraInfo.deviceName() +  ": ";
        info += cameraInfo.description() + "\n";
    }
    QMessageBox::information(this, "Cameras", info);
}

void MainWindow::openUSBCamera()
{
    /*if(capturer != nullptr) {
        // if a thread is already running, stop it
        capturer->setRunning(false);
        disconnect(capturer, &USBCaptureThread::frameCaptured, this, &MainWindow::updateFrame);
        connect(capturer, &USBCaptureThread::finished, capturer, &USBCaptureThread::deleteLater);
    }*/
    // I am using my second camera whose Index is 2.  Usually, the
    // Index of the first camera is 0.
    int camID = 2;
    capturer = new USBCaptureThread(camID, data_lock);
    connect(capturer, &USBCaptureThread::frameCaptured, this, &MainWindow::updateFrame);
    capturer->start();
    mainStatusLabel->setText(QString("Capturing Camera %1").arg(camID));
}

void MainWindow::openNectaCamera()
{
    int camID = 0;
    nectacapturer = new NectaCaptureThread(camID, data_lock);
    connect(nectacapturer, &NectaCaptureThread::nectaframeCaptured, this, &MainWindow::updateFrameNecta);
    nectacapturer->start();
    mainStatusLabel->setText(QString("Capturing Necta Camera"));
            //QPixmap image = QPixmap("/home/javi/prueba.bmp");
            //QPixmap image = QPixmap("/home/javi/prueba.bmp", "BMP",Qt::NoFormatConversion);
            //QPixmap image = QPixmap::fromImage(imageq,Qt::MonoOnly);
            //QPixmap image = QPixmap("/home/javi/prueba.bmp", "BMP",Qt::MonoOnly);
            //sleep(10);
            //CAlkUSB3::INectaCamera::Destroy(*myCam);
 }


void MainWindow::openOakDCamera()
{
    int camID = 0;
    oakdcapturer = new OakdCaptureThread(camID, data_lock);
    oakdcapturer->start();
    mainStatusLabel->setText(QString("Capturing OAK-D Camera"));
}

void MainWindow::updateFrame(cv::Mat *mat)
{
    data_lock->lock();
    currentFrame = *mat;
    data_lock->unlock();

    QImage frame(
        currentFrame.data,
        currentFrame.cols,
        currentFrame.rows,
        currentFrame.step,
        QImage::Format_RGB888);
    //3 lines added for OCR Video
    QImage ocrframe;
    ocrframe=extractTextVideo(frame);
    QPixmap image = QPixmap::fromImage(ocrframe);
    imageScene->clear();
    imageView->resetMatrix();
    imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
}

void MainWindow::updateFrameNecta()
{
    data_lock->lock();
    //sleep(0.1);
    QImage imageq = QImage("/home/javi/prueba.bmp");
    data_lock->unlock();
    QPixmap image = QPixmap::fromImage(imageq);
    imageScene->clear();
    imageView->resetMatrix();
    imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
}
QImage MainWindow::extractTextVideo(QImage frame)
{
    QImage image;
    char *old_ctype = strdup(setlocale(LC_ALL, NULL));
    setlocale(LC_ALL, "C");
    if (tesseractAPI == nullptr) {
        tesseractAPI = new tesseract::TessBaseAPI();
        // Initialize tesseract-ocr with English, with specifying tessdata path
        if (tesseractAPI->Init(TESSDATA_PREFIX, "eng")) {
            QMessageBox::information(this, "Error", "Tesseract could not be initialized.");
            return(frame);
        }
    }
    image = frame;

    tesseractAPI->SetImage(image.bits(), image.width(), image.height(),
        3, image.bytesPerLine());

    if (detectAreaCheckBox->checkState() == Qt::Checked) {
        std::vector<cv::Rect> areas;
        cv::Mat newImage = detectTextAreas(image, areas);
        //showImage(newImage);
        editor->setPlainText("");
        for(cv::Rect &rect : areas) {
            tesseractAPI->SetRectangle(rect.x, rect.y, rect.width, rect.height);
            char *outText = tesseractAPI->GetUTF8Text();
            editor->setPlainText(editor->toPlainText() + outText);
            delete [] outText;
        }
        QImage returnedImage(
            newImage.data,
            newImage.cols,
            newImage.rows,
            newImage.step,
            QImage::Format_RGB888);
        setlocale(LC_ALL, old_ctype);
        free(old_ctype);
        return(returnedImage);

    } else {
        char *outText = tesseractAPI->GetUTF8Text();
        editor->setPlainText(outText);
        delete [] outText;
    }

    setlocale(LC_ALL, old_ctype);
    free(old_ctype);
    return(frame);
}

void MainWindow::aboutDialog()
{
    QMessageBox::about(this, "About HSK Vision","HSK Vision 1.1.""Under GPL licence." "Computer vision application developed by HardSoftKoop using QT libraries.");
}
void MainWindow::extractDimensions()
{
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Image not opened.");
        return;
    }
    QPixmap imagePix = currentImage->pixmap();
    QImage imageQIm = imagePix.toImage();
    cv::Mat mat = cv::Mat(
        imageQIm.height(),
        imageQIm.width(),
        CV_8UC3,
        imageQIm.bits(),
        imageQIm.bytesPerLine()).clone();
    cv::Mat mat_gray;
    int thresh = 100;
    cv::RNG rng(12345);
    cvtColor( mat, mat_gray, cv::COLOR_BGR2GRAY );
    //blur( mat_gray, mat_gray, cv::Size(3,3) );
    cv::Mat canny_output;
    Canny( mat_gray, canny_output, thresh, thresh*2 );
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    findContours( canny_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );
    cv::Mat output = cv::Mat::zeros( canny_output.size(), CV_8UC3 );
    for( size_t i = 0; i< contours.size(); i++ )
    {
        cv::Scalar color = cv::Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
        drawContours( output, contours, (int)i, color, 2, cv::LINE_8, hierarchy, 0 );
    }
    showImage(output);

}
