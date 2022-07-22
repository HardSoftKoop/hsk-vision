/*  Copyright 2022 Javier Alvarez
    This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include <QWidget>
#include <QPixmap>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPoint>

#include "mainwindow.h"

class ScreenCapturer : public QWidget {
    Q_OBJECT

public:
    explicit ScreenCapturer(MainWindow *w);
    ~ScreenCapturer();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void closeMe();
    void confirmCapture();

private:
    void initShortcuts();
    QPixmap captureDesktop();

private:
    MainWindow *window;
    QPixmap screen;
    QPoint p1, p2;
    bool mouseDown;
};
