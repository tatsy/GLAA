#ifdef _MSC_VER
#pragma once
#endif

#ifndef _MAINGUI_H_
#define _MAINGUI_H_

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qgridlayout.h>

#include "openglviewer.h"

class MainGui : public QMainWindow {
    Q_OBJECT

public:
    explicit MainGui(QWidget *parent = nullptr);
    virtual ~MainGui();

private slots:
    void onFrameSwapped();
    void onUpdateButtonClicked();

private:
    QWidget *mainWidget = nullptr;
    QGridLayout *mainLayout = nullptr;

    OpenGLViewer *viewer = nullptr;

    class Ui;
    Ui *ui = nullptr;
};

#endif  // _MAINGUI_H_
