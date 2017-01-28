#include "openglviewer.h"

#include <vector>

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtGui/qopenglextrafunctions.h>

#include "common.h"
#include "glutils.h"
#include "tiny_obj_loader.h"

static constexpr float cameraFov = 30.0f;
static constexpr float cameraNearClip = 1.0f;
static constexpr float cameraFarClip = 1000.0f;

static const QVector3D eyePos = QVector3D(0.0f, 15.0f, 55.0f);
static const QVector3D eyeTo = QVector3D(0.0f, 0.0f, 0.0f);
static const QVector3D eyeUp = QVector3D(0.0f, 1.0f, 0.0f);

static const QVector3D lightPos = QVector3D(0.0f, 10.0f, 0.0f);

OpenGLViewer::OpenGLViewer(QWidget *parent)
    : QOpenGLWidget(parent) {
    camera = std::make_unique<ArcballCamera>(this);
    timer = new QTimer(this);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(onAnimate()));
}

OpenGLViewer::~OpenGLViewer() {
}

void OpenGLViewer::load(const std::string &filename) {
    QFileInfo fileinfo(filename.c_str());
    std::string dirname = (fileinfo.absoluteDir().absolutePath() + "/").toStdString();

    makeCurrent();
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string errmsg;
    bool success = tinyobj::LoadObj(shapes, materials, errmsg, filename.c_str(), dirname.c_str());
    if (!errmsg.empty()) {
        WarnMsg("%s\n", errmsg.c_str());
    }

    if (!success) {
        ErrorMsg("Failed to load file: %s", filename.c_str());
    }
    
    // Set vertex arrays.
    using Face = std::tuple<int, int, int, int>;
    std::vector<float> tempPos;
    std::vector<float> tempNorm;
    std::vector<float> tempUv;
    std::vector<Face> tempFace;     
    for (const auto &shape : shapes) {
        const auto &mesh = shape.mesh;
        tempPos.insert(tempPos.end(), mesh.positions.begin(), mesh.positions.end());
        tempNorm.insert(tempNorm.end(), mesh.normals.begin(), mesh.normals.end());
        tempUv.insert(tempUv.end(), mesh.texcoords.begin(), mesh.texcoords.end());
        for (int i = 0; i < mesh.material_ids.size(); i++) {
            Face f;
            std::get<0>(f) = mesh.material_ids[i];
            std::get<1>(f) = mesh.indices[i * 3 + 0];
            std::get<2>(f) = mesh.indices[i * 3 + 1];
            std::get<3>(f) = mesh.indices[i * 3 + 2];
            tempFace.push_back(f);
        }
    }
    std::sort(tempFace.begin(), tempFace.end());

    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<uint32_t> indices;
    std::vector<int> border(materials.size(), -1);

    int count = 0;
    int mtrl_count = 0;
    for (int i = 0; i < tempFace.size(); i++) {
        const auto &f = tempFace[i];
        int idx[3] = { std::get<1>(f), std::get<2>(f), std::get<3>(f) };
        for (int j = 0; j < 3; j++) {
            positions.push_back(tempPos[idx[j] * 3 + 0]);    
            positions.push_back(tempPos[idx[j] * 3 + 1]);    
            positions.push_back(tempPos[idx[j] * 3 + 2]);
            normals.push_back(tempNorm[idx[j] * 3 + 0]);
            normals.push_back(tempNorm[idx[j] * 3 + 1]);
            normals.push_back(tempNorm[idx[j] * 3 + 2]);
            texcoords.push_back(tempUv[idx[j] * 2 + 0]);
            texcoords.push_back(tempUv[idx[j] * 2 + 1]);
            indices.push_back(count++);
        }

        int matId = std::get<0>(f);
        while (mtrl_count < materials.size() && border[mtrl_count] == -1 && mtrl_count <= matId) {
            border[mtrl_count] = i * 3;
            mtrl_count += 1;
        }
    }
    border.push_back(indices.size());

    // Initialize VAO (Scene).
    sceneVao = std::make_unique<VertexArrayObject>();
    sceneVao->addVertexAttrib(positions, 0, 3);
    sceneVao->addVertexAttrib(normals, 1, 3);
    sceneVao->addVertexAttrib(texcoords, 2, 2);
    sceneVao->addIndices(indices);

    std::vector<MaterialInfo> matInfo;
    for (int i = 0; i < materials.size(); i++) {
        const auto &m = materials[i];

        MaterialInfo material;        
        material.diffuse = QVector3D(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
        material.specular = QVector3D(m.specular[0], m.specular[1], m.specular[2]);
        material.shininess = m.shininess;
        
        if (!m.diffuse_texname.empty()) {
            QImage img;
            if(!img.load((dirname + m.diffuse_texname).c_str())) {
                WarnMsg("Failed to load image file: %s", m.diffuse_texname.c_str());
            }
            material.diffuse_texture = std::make_shared<ImageTexture>(img);
        }
        
        if (!m.specular_texname.empty()) {
            QImage img;
            if (!img.load((dirname + m.specular_texname).c_str())) {
                WarnMsg("Failed to load image file: %s", m.specular_texname.c_str());
            }
            material.specular_texture = std::make_shared<ImageTexture>(img);
        }

        if (!m.bump_texname.empty()) {
            QImage img;
            if (!img.load((dirname + m.bump_texname).c_str())) {
                WarnMsg("Failed to load image file: %s", m.bump_texname.c_str());
            }
            material.bump_texture = std::make_shared<ImageTexture>(img);
        }

        matInfo.push_back(material);

        SegmentInfo segment;
        segment.start = border[i];
        segment.count = border[i + 1] - border[i];
        segment.material = material;
        sceneVao->addSegment(segment);
    }
    sceneVao->setReady();

    // Initialize VAO for screen rectangle.
    squareVao = std::unique_ptr<VertexArrayObject>(VertexArrayObject::asSquare());

    // Initialize arcball controller
    camera->setLookAt(eyePos, eyeTo, eyeUp);
    camera->setPerspective(cameraFov, (float)width() / (float)height(), cameraNearClip, cameraFarClip);
}

void OpenGLViewer::setAAMethod(int type, int subsample) {
    makeCurrent();

    aaMethod.type = type;
    aaMethod.subsample = subsample;

    updateFboSize();
}

void OpenGLViewer::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    load(std::string(DATA_DIRECTORY) + "sponza.obj");

    shader = std::unique_ptr<QOpenGLShaderProgram>(
        buildGLSLProgram(QString(SHADER_DIRECTORY) + "render"));

    gbufShader = std::unique_ptr<QOpenGLShaderProgram>(
        buildGLSLProgram(QString(SHADER_DIRECTORY) + "gbuffer"));

    csShader = std::unique_ptr<QOpenGLShaderProgram>(
        buildGLSLComputeShader(QString(SHADER_DIRECTORY) + "msaa"));

    displayShader = std::unique_ptr<QOpenGLShaderProgram>(
        buildGLSLProgram(QString(SHADER_DIRECTORY) + "display"));
}

void OpenGLViewer::paintGL() {
    if (!sceneVao) return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (aaMethod.type == 0) {
        drawScene();
    } else {
        drawGbuffer();
        drawSceneCS();
    }
}

void OpenGLViewer::resizeGL(int w, int h) {
    glViewport(0, 0, width(), height());

    updateFboSize();

    renderTargetCS = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
    renderTargetCS->setFormat(QOpenGLTexture::TextureFormat::RGBA8_SNorm);
    renderTargetCS->setSize(width(), height());
    renderTargetCS->allocateStorage(QOpenGLTexture::PixelFormat::RGBA, QOpenGLTexture::PixelType::UInt8);

    camera->setPerspective(cameraFov, (float)width() / (float)height(), cameraNearClip, cameraFarClip);
}

void OpenGLViewer::updateFboSize() {
    QSize bufferSize(width() * aaMethod.subsample, height() * aaMethod.subsample);
    gbufFbo = std::make_unique<QOpenGLFramebufferObject>(
        bufferSize, QOpenGLFramebufferObject::Attachment::Depth,
        QOpenGLTexture::Target2D, QOpenGLTexture::RGBA16F);
    gbufFbo->addColorAttachment(bufferSize, QOpenGLTexture::RGBA16F);
    gbufFbo->addColorAttachment(bufferSize, QOpenGLTexture::RGBA8_SNorm);
    gbufFbo->addColorAttachment(bufferSize, QOpenGLTexture::RGBA8_SNorm);
    gbufFbo->addColorAttachment(bufferSize, QOpenGLTexture::R32F);
}

void OpenGLViewer::mousePressEvent(QMouseEvent* ev) {
    // camera
    camera->mousePressEvent(ev);
}

void OpenGLViewer::mouseMoveEvent(QMouseEvent* ev) {
    // camera
    camera->mouseMoveEvent(ev);
}

void OpenGLViewer::mouseReleaseEvent(QMouseEvent* ev) {
    // camera
    camera->mouseReleaseEvent(ev);
}

void OpenGLViewer::wheelEvent(QWheelEvent* ev) {
    // Camera
    camera->wheelEvent(ev);
}

void OpenGLViewer::onAnimate() {
    update();
}

void OpenGLViewer::drawScene() {
    shader->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->setUniformValue("u_mvMat", camera->mvMat());
    shader->setUniformValue("u_mvpMat", camera->mvpMat());
    shader->setUniformValue("u_normMat", camera->normMat());
    shader->setUniformValue("u_lightPos", lightPos);

    sceneVao->drawAs(GL_TRIANGLES, *shader);

    shader->release();
}

void OpenGLViewer::drawGbuffer() {
    glViewport(0, 0, width() * aaMethod.subsample, height() * aaMethod.subsample);

    gbufShader->bind();
    gbufFbo->bind();

    GLenum bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                      GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
                      GL_COLOR_ATTACHMENT4 };
    auto func = QOpenGLContext::currentContext()->extraFunctions();
    func->glDrawBuffers(5, bufs);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gbufShader->setUniformValue("u_mvpMat", camera->mvpMat());

    sceneVao->drawAs(GL_TRIANGLES, *gbufShader);

    gbufShader->release();
    gbufFbo->release();

    glViewport(0, 0, width(), height());
}

void OpenGLViewer::drawSceneCS() {
    csShader->bind();

    csShader->setUniformValue("u_mvMat", camera->mvMat());
    csShader->setUniformValue("u_normMat", camera->mvMat());
    csShader->setUniformValue("u_lightPos", lightPos);
    csShader->setUniformValue("u_aaType", aaMethod.type);
    csShader->setUniformValue("u_subsample", aaMethod.subsample);

    auto func = QOpenGLContext::currentContext()->extraFunctions();
    func->glBindImageTexture(0, gbufFbo->textures()[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    func->glBindImageTexture(1, gbufFbo->textures()[1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    func->glBindImageTexture(2, gbufFbo->textures()[2], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8_SNORM);
    func->glBindImageTexture(3, gbufFbo->textures()[3], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8_SNORM);
    func->glBindImageTexture(4, gbufFbo->textures()[4], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    func->glBindImageTexture(5, renderTargetCS->textureId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8_SNORM);

    const int localSize = 32;
    func->glDispatchCompute((width() + localSize - 1) / localSize, (height() + localSize - 1) / localSize, 1);

    csShader->release();

    // Draw antialiased scene.
    displayShader->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTargetCS->textureId());

    squareVao->drawAs(GL_TRIANGLES);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    displayShader->release();
}
