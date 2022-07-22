/*  Copyright 2022 Javier Alvarez
    This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef OAKD_CAMERA_H
#define OAKD_CAMERA_H

#include <QString>
#include <QThread>
#include <QMutex>
#include <unistd.h>

#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/video/background_segm.hpp"

using namespace std;

class OakdCaptureThread : public QThread
{
    Q_OBJECT
public:
    OakdCaptureThread(int camera, QMutex *lock);
    OakdCaptureThread(QString videoPath, QMutex *lock);
    ~OakdCaptureThread();
    void setRunning(bool run) {running = run; };
    void startCalcFPS() {fps_calculating = true; };
    enum VideoSavingStatus {
                            STARTING,
                            STARTED,
                            STOPPING,
                            STOPPED
    };

    void setVideoSavingStatus(VideoSavingStatus status) {video_saving_status = status; };
    void setMotionDetectingStatus(bool status) {
        motion_detecting_status = status;
        motion_detected = false;
        if(video_saving_status != STOPPED) video_saving_status = STOPPING;
    };

protected:
    void run() override;

signals:
    void fpsChanged(float fps);
    void videoSaved(QString name);

private:
    void calculateFPS(cv::VideoCapture &cap);
    void startSavingVideo(cv::Mat &firstFrame);
    void stopSavingVideo();
    void motionDetect(cv::Mat &frame);

private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock;
    cv::Mat frame;

    // FPS calculating
    bool fps_calculating;
    float fps;

    // video saving
    int frame_width, frame_height;
    VideoSavingStatus video_saving_status;
    QString saved_video_name;
    cv::VideoWriter *video_writer;

    // motion analysis
    bool motion_detecting_status;
    bool motion_detected;
    cv::Ptr<cv::BackgroundSubtractorMOG2> segmentor;
};
#endif // OAKD_CAMERA_H
