#ifdef _MSC_VER
#pragma once
#endif

#ifndef _GL_UTILS_H_
#define _GL_UTILS_H_

#include <iostream>
#include <string>

#include <QtGui/qopenglshaderprogram.h>

inline QOpenGLShaderProgram *buildGLSLComputeShader(const QString &progCS) {
    auto shader = new QOpenGLShaderProgram();
    shader->addShaderFromSourceFile(QOpenGLShader::Compute, progCS + ".cs");
    shader->link();
    if (!shader->isLinked()) {
        std::cerr << "[ERROR] failed to compile or link shader: " << std::endl;
        std::cerr << "   Compute: " << progCS.toStdString() << std::endl;
        return nullptr;
    }

    return shader;
}

inline QOpenGLShaderProgram *buildGLSLProgram(const QString &progVS,
    const QString &progFS) {

    auto shader = new QOpenGLShaderProgram();
    shader->addShaderFromSourceFile(QOpenGLShader::Vertex, progVS);
    shader->addShaderFromSourceFile(QOpenGLShader::Fragment, progFS);
    shader->link();
    if (!shader->isLinked()) {
        std::cerr << "[ERROR] failed to compile or link shader: " << std::endl;
        std::cerr << "    Vertex: " << progVS.toStdString() << std::endl;
        std::cerr << "  Fragment: " << progFS.toStdString() << std::endl;
        return nullptr;
    }

    return shader;    
}

inline QOpenGLShaderProgram *buildGLSLProgram(const QString &progVS,
    const QString &progGS, const QString &progFS) {

    auto shader = new QOpenGLShaderProgram();
    shader->addShaderFromSourceFile(QOpenGLShader::Vertex, progVS);
    shader->addShaderFromSourceFile(QOpenGLShader::Geometry, progGS);
    shader->addShaderFromSourceFile(QOpenGLShader::Fragment, progFS);
    shader->link();
    if (!shader->isLinked()) {
        std::cerr << "[ERROR] failed to compile or link shader: " << std::endl;
        std::cerr << "    Vertex: " << progVS.toStdString() << std::endl;
        std::cerr << "  Geometry: " << progGS.toStdString() << std::endl;
        std::cerr << "  Fragment: " << progFS.toStdString() << std::endl;
        return nullptr;
    }

    return shader; 
}

inline QOpenGLShaderProgram *buildGLSLProgram(const QString& basename, bool useGeom = false) {
    auto shader = new QOpenGLShaderProgram();

    const QString progVS = basename + ".vs";
    const QString progGS = basename + ".gs";
    const QString progFS = basename + ".fs";

    if (useGeom) {
        return buildGLSLProgram(progVS, progGS, progFS);
    }
    return buildGLSLProgram(progVS, progFS);
}

#endif // _GL_UTILS_H_
