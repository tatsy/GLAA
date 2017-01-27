#ifdef _MSC_VER
#pragma once
#endif

#ifndef _VERTEXARRAYOBJECT_H_
#define _VERTEXARRAYOBJECT_H_

#include <map>
#include <vector>

#include <QtGui/qopenglshaderprogram.h>
#include <QtGui/qopenglvertexarrayobject.h>
#include <QtGui/qopenglbuffer.h>
#include <QtGui/qopenglfunctions.h>

#include "imagetexture.h"

namespace {

struct AttribInfo {
    AttribInfo() : location(0u), head(0u), size(0u) {}
    AttribInfo(uint32_t l, uint32_t h, uint32_t s)
        : location(l), head(h), size(s) {}

    uint32_t location;
    uint32_t head;
    uint32_t size;
};

}  // anonymouse namespace

struct MaterialInfo {
    QVector3D diffuse = { 1.0f, 1.0f, 1.0f };
    QVector3D specular = { 0.0f, 0.0f, 0.0f };
    QVector3D ambient = { 0.0f, 0.0f, 0.0f };
    float shininess = 1.0f;
    std::shared_ptr<ImageTexture> diffuse_texture = nullptr;
    std::shared_ptr<ImageTexture> specular_texture = nullptr;
    std::shared_ptr<ImageTexture> bump_texture = nullptr;
};

struct SegmentInfo {
    int start;
    int count;
    MaterialInfo material;
};

/**
 * Vertex Array Object
 * @details
 * -- Usage --
 * 1) construct the object.
 * 2) set vertices and indices by addXXX functions.
 * 3) call setReady() to transfer data to GPU.
 **/
class VertexArrayObject : protected QOpenGLFunctions {
public:
    VertexArrayObject() {
        initialize();
    }

    VertexArrayObject(const VertexArrayObject &) = delete;
    VertexArrayObject & operator=(const VertexArrayObject &) = delete;

    VertexArrayObject(VertexArrayObject &&vao) {
        this->operator=(std::move(vao));
    }

    VertexArrayObject & operator=(VertexArrayObject &&vao) {
        this->vao_ = vao.vao_;
        this->vbo_ = vao.vbo_;
        this->ibo_ = vao.ibo_;
        this->vertexData_ = std::move(vao.vertexData_);
        this->indices_ = std::move(vao.indices_);
        this->attribInfo_ = std::move(vao.attribInfo_);
        vao.vao_ = nullptr;
        vao.vbo_ = nullptr;
        vao.ibo_ = nullptr;

        return *this;
    }

    virtual ~VertexArrayObject() {
        if (vbo_) {
            delete vbo_;
            vbo_ = nullptr;
        }

        if (ibo_) {
            delete ibo_;
            ibo_ = nullptr;
        }

        if (vao_) {
            delete vao_;
            vao_ = nullptr;
        }
    }

    void clear() {
        vertexData_.clear();
        indices_.clear();
        attribInfo_.clear();
    }

    int numIndices() const {
        return ibo_->size() / sizeof(unsigned int);
    }

    void addVertexAttrib(const std::vector<float> &data, uint32_t location, uint8_t tupleSize) {
        if (data.empty()) {
            printf("[WARNING] size of input data for is empty.");
        }

        const uint32_t headPos = vertexData_.size() * sizeof(float);
        vertexData_.insert(vertexData_.end(), data.begin(), data.end());
        attribInfo_.emplace_back(location, headPos, tupleSize);
    }

    void addIndices(const std::vector<uint32_t> &indices) {
        indices_.insert(indices_.end(), indices.begin(), indices.end());
    }

    void addSegment(const SegmentInfo &segment) {
        segmentInfo_.push_back(segment);
    }

    //! Allocate memory on GPU and transfer buffers to it.
    void setReady() {
        vao_->bind();

        if (vertexData_.size() == 0) {
            printf("[WARNING]\n"
                   "  Size of VERTEX array is 0.\n"
                   "  You should check your program to properly add the data.\n");
        }

        vbo_->bind();
        vbo_->allocate(&vertexData_[0], vertexData_.size() * sizeof(float));
        for (const auto &info : attribInfo_) {
            glEnableVertexAttribArray(info.location);
            glVertexAttribPointer(info.location, info.size, GL_FLOAT, GL_FALSE, 0, (void*)info.head);
        }

        if (indices_.size() == 0) {
            printf("[WARNING]\n"
                   "  Size of INDEX array is 0.\n"
                   "  You should check your program to properly add the data.\n");
        }

        ibo_->bind();
        ibo_->allocate(&indices_[0], indices_.size() * sizeof(uint32_t));

        vao_->release();
    }

    void drawAs(GLuint drawMode) {
        vao_->bind();
        glDrawElements(drawMode, indices_.size(), GL_UNSIGNED_INT, 0);
        vao_->release();
    }

    void drawAs(GLuint drawMode, QOpenGLShaderProgram &program) {
        vao_->bind();
        for (const auto &seg : segmentInfo_) {
            program.setUniformValue("u_diffColor", seg.material.diffuse);         
            program.setUniformValue("u_specColor", seg.material.specular);
            if (seg.material.diffuse_texture) {
                glActiveTexture(GL_TEXTURE10);
                glBindTexture(GL_TEXTURE_2D, seg.material.diffuse_texture->textureId());
                program.setUniformValue("u_diffuseMap", 10);
                program.setUniformValue("u_hasDiffuseTex", 1);
            } else {
                program.setUniformValue("u_hasDiffuseTex", 0);
            }

            if (seg.material.specular_texture) {
                glActiveTexture(GL_TEXTURE11);
                glBindTexture(GL_TEXTURE_2D, seg.material.specular_texture->textureId());
                program.setUniformValue("u_specularMap", 11);
                program.setUniformValue("u_hasSpecularTex", 1);
            } else {
                program.setUniformValue("u_hasSpecularTex", 0);
            }
            program.setUniformValue("u_shininess", seg.material.shininess);

            if (seg.material.bump_texture) {
                glActiveTexture(GL_TEXTURE12);
                glBindTexture(GL_TEXTURE_2D, seg.material.diffuse_texture->textureId());
                program.setUniformValue("u_bumpMap", 12);
                program.setUniformValue("u_hasBumpTex", 1);
            } else {
                program.setUniformValue("u_hasBumpTex", 0);
            }

            glDrawElements(drawMode, seg.count, GL_UNSIGNED_INT, (void*)(seg.start * sizeof(uint32_t)));

            for (int i = 0; i < 5; i++) {
                glActiveTexture(GL_TEXTURE10 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }   
        vao_->release();
    }

    static VertexArrayObject *asSquare() {
        VertexArrayObject *vao = new VertexArrayObject();

        std::vector<float> positions = { 
            -1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 1.0f, 0.0f
        };

        std::vector<float> normals = {
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
        };

        std::vector<float> texcoors = {
            0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 1.0f
        };

        std::vector<uint32_t> indices = {
            0, 3, 1, 0, 2, 3
        };

        vao->addVertexAttrib(positions, 0, 3);
        vao->addVertexAttrib(normals, 1, 3);
        vao->addVertexAttrib(texcoors, 2, 2);
        vao->addIndices(indices);
        vao->setReady();

        return vao;
    }

    template <class T>
    void update(const std::vector<T> &objArray) {
        vao_->bind();
        vbo_->bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(T) * objArray.size(), &objArray[0]);
        vao_->release();
    }

    template <class T>
    static VertexArrayObject *fromStructArray(const std::vector<T> &objArray, const std::vector<int> &tupleSizes) {
        VertexArrayObject *vao = new VertexArrayObject();

        vao->vao_->bind();

        // Write vertex array.
        vao->vbo_->bind();
        vao->vbo_->allocate(&objArray[0], objArray.size() * sizeof(T));

        uint32_t offset = 0u;
        for (int i = 0; i < tupleSizes.size(); i++) {
            vao->glEnableVertexAttribArray(i);
            vao->glVertexAttribPointer(i, tupleSizes[i], GL_FLOAT, GL_FALSE, sizeof(T), (void*)(offset * sizeof(float)));
            offset += tupleSizes[i];
        }

        // Write index array.
        vao->indices_.resize(objArray.size());
        for (int i = 0; i < objArray.size(); i++) {
            vao->indices_[i] = i;
        }
        vao->ibo_->bind();
        vao->ibo_->allocate(&vao->indices_[0], vao->indices_.size() * sizeof(uint32_t));

        vao->vao_->release();

        return vao;
    }

private:
    void initialize() {
        initializeOpenGLFunctions();

        vao_ = new QOpenGLVertexArrayObject();
        vao_->create();
        vao_->bind();

        vbo_ = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        vbo_->create();
        vbo_->setUsagePattern(QOpenGLBuffer::DynamicDraw);
        vbo_->bind();
        vbo_->release();

        ibo_ = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        ibo_->create();
        ibo_->setUsagePattern(QOpenGLBuffer::StaticDraw);
        ibo_->bind();
        ibo_->release();

        vao_->release();
    }

    QOpenGLVertexArrayObject *vao_ = nullptr;
    QOpenGLBuffer *vbo_ = nullptr;
    QOpenGLBuffer *ibo_ = nullptr;

    std::vector<float> vertexData_;
    std::vector<uint32_t> indices_;
    std::vector<AttribInfo> attribInfo_;
    std::vector<SegmentInfo> segmentInfo_;
};

#endif  // _VERTEXARRAYOBJECT_H_
