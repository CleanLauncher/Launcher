// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher: Minecraft Launcher
 *  Copyright (c) 2024 Trial97 <alexandru.tripon97@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ui/dialogs/skins/draw/SkinOpenGLWindow.h"

#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QProcessEnvironment>
#include <QVector2D>
#include <QVector3D>
#include <QtMath>
#include <functional>

#include "BuildConfig.h"
#include "minecraft/skins/SkinModel.h"
#include "rainbow.h"
#include "ui/dialogs/skins/draw/BoxGeometry.h"
#include "ui/dialogs/skins/draw/Scene.h"

SkinOpenGLWindow::SkinOpenGLWindow(SkinProvider* parent, QColor color)
    : QOpenGLWindow(), QOpenGLFunctions(), m_baseColor(color), m_parent(parent)
{
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setDepthBufferSize(24);
    setFormat(format);
}

SkinOpenGLWindow::~SkinOpenGLWindow()
{
    makeCurrent();

    if (m_scene) {
        delete m_scene;
    }
    if (m_background) {
        delete m_background;
    }
    if (m_backgroundTexture) {
        if (m_backgroundTexture->isCreated()) {
            m_backgroundTexture->destroy();
        }
        delete m_backgroundTexture;
    }
    if (m_modelProgram) {
        if (m_modelProgram->isLinked()) {
            m_modelProgram->release();
        }
        m_modelProgram->removeAllShaders();
        delete m_modelProgram;
    }
    if (m_backgroundProgram) {
        if (m_backgroundProgram->isLinked()) {
            m_backgroundProgram->release();
        }
        m_backgroundProgram->removeAllShaders();
        delete m_backgroundProgram;
    }
    doneCurrent();
}

void SkinOpenGLWindow::mousePressEvent(QMouseEvent* e)
{
    m_mousePosition = QVector2D(e->pos());
    m_isMousePressed = true;
}

void SkinOpenGLWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::MouseButton::LeftButton)) {
        m_isMousePressed = false;
        return;
    }

    if (m_isMousePressed) {
        int dx = event->position().x() - m_mousePosition.x();
        int dy = event->position().y() - m_mousePosition.y();

        m_yaw += dx * 0.5f;
        m_pitch += dy * 0.5f;

        if (m_yaw > 360.0f)
            m_yaw -= 360.0f;
        else if (m_yaw < 0.0f)
            m_yaw += 360.0f;

        m_mousePosition = QVector2D(event->pos());
        update();
    }
}

void SkinOpenGLWindow::mouseReleaseEvent([[maybe_unused]] QMouseEvent* e)
{
    m_isMousePressed = false;
}

void SkinOpenGLWindow::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 1, 1);

    initShaders();

    generateBackgroundTexture(32, 32, 1);

    QImage skin, cape;
    bool slim = false;
    if (m_parent) {
        if (auto s = m_parent->getSelectedSkin()) {
            skin = s->getTexture();
            slim = s->getModel() == SkinModel::SLIM;
            cape = m_parent->capes().value(s->getCapeId(), {});
        }
    }

    m_scene = new opengl::Scene(skin, slim, cape);
    m_background = opengl::BoxGeometry::Plane();
    glEnable(GL_TEXTURE_2D);
}

void SkinOpenGLWindow::initShaders()
{
    m_modelProgram = new QOpenGLShaderProgram(this);

    if (!m_modelProgram->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vshader_skin_model.glsl"))
        close();

    if (!m_modelProgram->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fshader.glsl"))
        close();

    if (!m_modelProgram->link())
        close();

    if (!m_modelProgram->bind())
        close();

    m_backgroundProgram = new QOpenGLShaderProgram(this);

    if (!m_backgroundProgram->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vshader_skin_background.glsl"))
        close();

    if (!m_backgroundProgram->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fshader.glsl"))
        close();

    if (!m_backgroundProgram->link())
        close();

    if (!m_backgroundProgram->bind())
        close();
}

void SkinOpenGLWindow::resizeGL(int w, int h)
{
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    const qreal zNear = 15., fov = 45;

    m_projection.setToIdentity();

    double radians = qDegreesToRadians(fov / 2.);
    double sine = std::sin(radians);
    if (sine == 0)
        return;
    double cotan = std::cos(radians) / sine;

    m_projection(0, 0) = cotan / aspect;
    m_projection(1, 1) = cotan;
    m_projection(2, 2) = 0.;
    m_projection(3, 2) = -1.;
    m_projection(2, 3) = zNear;
    m_projection(3, 3) = 0.;
}

void SkinOpenGLWindow::paintGL()
{
    qreal dpr = devicePixelRatio();
    if (dpr != 1.f) {
        QSize scaledSize = size() * dpr;
        glViewport(0, 0, scaledSize.width(), scaledSize.height());
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_backgroundProgram->bind();
    renderBackground();
    m_backgroundProgram->release();

    QMatrix4x4 matrix;
    float yawRad = qDegreesToRadians(m_yaw);
    float pitchRad = qDegreesToRadians(m_pitch);
    matrix.lookAt(QVector3D(

                      m_distance * qCos(pitchRad) * qCos(yawRad),

                      m_distance * qSin(pitchRad) - 8,

                      m_distance * qCos(pitchRad) * qSin(yawRad)),
                  QVector3D(0, -8, 0), QVector3D(0, 1, 0));

    m_modelProgram->bind();
    m_modelProgram->setUniformValue("mvp_matrix", m_projection * matrix);

    m_scene->draw(m_modelProgram);
    m_modelProgram->release();

    if (m_isFirstFrame) {
        m_isFirstFrame = false;
        update();
    }
}

void SkinOpenGLWindow::updateScene(SkinModel* skin)
{
    if (skin && m_scene) {
        m_scene->setMode(skin->getModel() == SkinModel::SLIM);
        m_scene->setSkin(skin->getTexture());
        update();
    }
}
void SkinOpenGLWindow::updateCape(const QImage& cape)
{
    if (m_scene) {
        m_scene->setCapeVisible(!cape.isNull());
        m_scene->setCape(cape);
        update();
    }
}

QColor calculateContrastingColor(const QColor& color)
{
    auto luma = Rainbow::luma(color);
    if (luma < 0.5) {
        constexpr float contrast = 0.05f;
        return Rainbow::lighten(color, contrast);
    } else {
        constexpr float contrast = 0.2f;
        return Rainbow::darken(color, contrast);
    }
}

QImage generateChessboardImage(int width, int height, int tileSize, QColor baseColor)
{
    QImage image(width, height, QImage::Format_RGB888);
    bool isDarkBase = Rainbow::luma(baseColor) < 0.5;
    float contrast = isDarkBase ? 0.05 : 0.45;
    auto contrastFunc = std::bind(isDarkBase ? Rainbow::lighten : Rainbow::darken, std::placeholders::_1, contrast, 1.0);
    auto white = contrastFunc(baseColor);
    auto black = contrastFunc(calculateContrastingColor(baseColor));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            bool isWhite = ((x / tileSize) + (y / tileSize)) % 2 == 0;
            image.setPixelColor(x, y, isWhite ? white : black);
        }
    }
    return image;
}

void SkinOpenGLWindow::generateBackgroundTexture(int width, int height, int tileSize)
{
    m_backgroundTexture = new QOpenGLTexture(generateChessboardImage(width, height, tileSize, m_baseColor));
    m_backgroundTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_backgroundTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
}

void SkinOpenGLWindow::renderBackground()
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    m_backgroundTexture->bind();
    m_backgroundProgram->setUniformValue("texture", 0);
    m_background->draw(m_backgroundProgram);
    m_backgroundTexture->release();
    glDepthMask(GL_TRUE);

    glEnable(GL_DEPTH_TEST);
}

void SkinOpenGLWindow::wheelEvent(QWheelEvent* event)
{
    int delta = event->angleDelta().y();

    m_distance -= delta * 0.01f;

    m_distance = qMax(16.f, m_distance);

    update();
}
void SkinOpenGLWindow::setElytraVisible(bool visible)
{
    if (m_scene)
        m_scene->setElytraVisible(visible);
}

bool SkinOpenGLWindow::hasOpenGL()
{
    if (!QProcessEnvironment::systemEnvironment()
             .value(QStringLiteral("%1_DISABLE_GLVULKAN").arg(BuildConfig.LAUNCHER_ENVNAME))
             .isEmpty()) {
        return false;
    }

    QOpenGLContext ctx;
    return ctx.create();
}
