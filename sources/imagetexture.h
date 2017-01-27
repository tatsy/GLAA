#ifdef _MSC_VER
#pragma once
#endif

#ifndef _IMAGETEXTURE_H_
#define _IMAGETEXTURE_H_

#include <memory>
#include <QtGui/qimage.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qopengltexture.h>

class ImageTexture {
public:
    ImageTexture()
        : image_() {
    }

    ImageTexture(const QImage &image)
        : ImageTexture() {
        setImage(image);
    }

    ImageTexture(const ImageTexture &texture)
        : ImageTexture() {
        this->operator=(texture);
    }

    virtual ~ImageTexture() {
    }

    ImageTexture & operator=(const ImageTexture &texture) {
        setImage(texture.image_);
        return *this;
    }

    QVector3D operator()(float u, float v) const {
        return this->operator()(QVector2D(u, v));
    }

    QVector3D operator()(const QVector2D &uv) const {
        float u = uv.x();
        float v = uv.y();
        while (u > 1.0f) u -= 1.0f;
        while (v > 1.0f) v -= 1.0f;
        while (u < 0.0f) u += 1.0f;
        while (v < 0.0f) v += 1.0f;

        const int x = image_.width() * u;
        const int y = image_.height() * (1.0f - v);
        if (x >= 0 && y >= 0 && x < image_.width() && y < image_.height()) {
            QColor rgb = image_.pixelColor(x, y);
            return QVector3D(rgb.redF(), rgb.greenF(), rgb.blueF());
        }
        return QVector3D(0.0f, 0.0f, 0.0f);
    }

    void setImage(const QImage &image) {
        this->image_ = image;
        this->texture_ = std::make_shared<QOpenGLTexture>(image.mirrored(),
            QOpenGLTexture::MipMapGeneration::DontGenerateMipMaps);
        texture_->setWrapMode(QOpenGLTexture::CoordinateDirection::DirectionS, QOpenGLTexture::WrapMode::Repeat);
        texture_->setWrapMode(QOpenGLTexture::CoordinateDirection::DirectionT, QOpenGLTexture::WrapMode::Repeat);
    }

    void bind() {
        texture_->bind();
    }

    void release() {
        texture_->release();
    }

    GLuint textureId() const {
        return texture_->textureId();
    }

private:
    QImage image_;
    std::shared_ptr<QOpenGLTexture> texture_;
};

#endif  // _IMAGETEXTURE_H_
