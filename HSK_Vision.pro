#   Copyright 2022 Javier Alvarez
#   This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
TEMPLATE = app
TARGET = HSK_Vision

QT += core gui multimedia multimediawidgets concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += .

# use your own path in the following config
unix: {
    INCLUDEPATH += /usr/include/tesseract
    LIBS += -L/usr/lib/x86_64-linux-gnu -ltesseract
}

# opencv config
unix: !mac {
    INCLUDEPATH += /usr/include/opencv4
    LIBS += -L/usr/lib/x86_64-linux-gnu -lopencv_core -lopencv_imgproc -lopencv_dnn -lopencv_imgcodecs -lopencv_video -lopencv_videoio
}

# Alkeria config
linux: LIBS += -L/usr/local/lib -lalkusb3-1.1
linux: INCLUDEPATH += /usr/local/include/libalkusb3-1.1
linux: DEPENDPATH += /usr/local/include/libalkusb3-1.1

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += TESSDATA_PREFIX=\\\"/usr/share/tesseract-ocr/4.00/tessdata/\\\"

# Input
HEADERS += mainwindow.h screencapturer.h \
    necta_camera.h \
    oakd_camera.h \
    usb_camera.h \
    utilities.h
SOURCES += main.cpp mainwindow.cpp screencapturer.cpp \
    necta_camera.cpp \
    oakd_camera.cpp \
    usb_camera.cpp \
    utilities.cpp

FORMS += \
    mainwindow.ui
