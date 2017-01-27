#ifdef _MSC_VER
#pragma once
#endif

#ifndef _OPENGLVIEWER_H_
#define _OPENGLVIEWER_H_

#include <string>
#include <memory>

#include <QtCore/qtimer.h>
#include <QtWidgets/qopenglwidget.h>
#include <QtGui/qevent.h>
#include <QtGui/qopenglfunctions.h>
#include <QtGui/qopenglshaderprogram.h>
#include <QtGui/qopengltexture.h>
#include <QtGui/qopenglframebufferobject.h>

#include "vertexarrayobject.h"
#include "arcballcamera.h"

struct AAMethod {
    int type = 0;
    int subsample = 2;
};

class OpenGLViewer : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit OpenGLViewer(QWidget *parent = nullptr);
    virtual ~OpenGLViewer();

    void load(const std::string &filename);
    void setAAMethod(int type, int subsample);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;

private slots:
    void onAnimate();

private:
    void drawScene();
    void drawGbuffer();
    void drawSceneCS();
    void updateFboSize();

    std::unique_ptr<QOpenGLShaderProgram> shader = nullptr;
    std::unique_ptr<QOpenGLShaderProgram> gbufShader = nullptr;
    std::unique_ptr<QOpenGLShaderProgram> csShader = nullptr;
    std::unique_ptr<QOpenGLShaderProgram> displayShader = nullptr;

    std::unique_ptr<VertexArrayObject> sceneVao = nullptr;
    std::unique_ptr<VertexArrayObject> squareVao = nullptr;
    std::unique_ptr<QOpenGLFramebufferObject> gbufFbo = nullptr;
    std::unique_ptr<QOpenGLTexture> renderTargetCS = nullptr;
    std::unique_ptr<ArcballCamera> camera = nullptr;

    AAMethod aaMethod;

    QTimer *timer = nullptr;
};

#endif  // _OPENGLVIEWER_H_
