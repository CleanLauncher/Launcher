// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
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

#include "BoxGeometry.h"

#include <QList>
#include <QMatrix4x4>
#include <QVector2D>
#include <QVector3D>

struct VertexData {
    QVector4D position;
    QVector2D texCoord;
    VertexData(const QVector4D& pos, const QVector2D& tex) : position(pos), texCoord(tex) {}
};

static const QList<QVector4D> vertices = {

    QVector4D(-0.5f, -0.5f, 0.5f, 1.0f),

    QVector4D(0.5f, -0.5f, 0.5f, 1.0f),

    QVector4D(-0.5f, 0.5f, 0.5f, 1.0f),

    QVector4D(0.5f, 0.5f, 0.5f, 1.0f),

    QVector4D(0.5f, -0.5f, 0.5f, 1.0f),

    QVector4D(0.5f, -0.5f, -0.5f, 1.0f),

    QVector4D(0.5f, 0.5f, 0.5f, 1.0f),

    QVector4D(0.5f, 0.5f, -0.5f, 1.0f),

    QVector4D(0.5f, -0.5f, -0.5f, 1.0f),

    QVector4D(-0.5f, -0.5f, -0.5f, 1.0f),

    QVector4D(0.5f, 0.5f, -0.5f, 1.0f),

    QVector4D(-0.5f, 0.5f, -0.5f, 1.0f),

    QVector4D(-0.5f, -0.5f, -0.5f, 1.0f),

    QVector4D(-0.5f, -0.5f, 0.5f, 1.0f),

    QVector4D(-0.5f, 0.5f, -0.5f, 1.0f),

    QVector4D(-0.5f, 0.5f, 0.5f, 1.0f),

    QVector4D(-0.5f, -0.5f, -0.5f, 1.0f),

    QVector4D(0.5f, -0.5f, -0.5f, 1.0f),

    QVector4D(-0.5f, -0.5f, 0.5f, 1.0f),

    QVector4D(0.5f, -0.5f, 0.5f, 1.0f),

    QVector4D(-0.5f, 0.5f, 0.5f, 1.0f),

    QVector4D(0.5f, 0.5f, 0.5f, 1.0f),

    QVector4D(-0.5f, 0.5f, -0.5f, 1.0f),

    QVector4D(0.5f, 0.5f, -0.5f, 1.0f),

};

static const QList<GLushort> indices = {
    0,  1,  2,  3,  3,

    4,  4,  5,  6,  7,  7,

    8,  8,  9,  10, 11, 11,

    12, 12, 13, 14, 15, 15,

    16, 16, 17, 18, 19, 19,

    20, 20, 21, 22, 23

};

static const QList<VertexData> planeVertices = {
    { QVector4D(-1.0f, -1.0f, -0.5f, 1.0f), QVector2D(0.0f, 0.0f) },

    { QVector4D(1.0f, -1.0f, -0.5f, 1.0f), QVector2D(1.0f, 0.0f) },

    { QVector4D(-1.0f, 1.0f, -0.5f, 1.0f), QVector2D(0.0f, 1.0f) },

    { QVector4D(1.0f, 1.0f, -0.5f, 1.0f), QVector2D(1.0f, 1.0f) },

};
static const QList<GLushort> planeIndices = {
    0, 1, 2, 3, 3

};

QList<QVector4D> transformVectors(const QMatrix4x4& matrix, const QList<QVector4D>& vectors)
{
    QList<QVector4D> transformedVectors;
    transformedVectors.reserve(vectors.size());

    for (const QVector4D& vec : vectors) {
        if (!matrix.isIdentity()) {
            transformedVectors.append(matrix * vec);
        } else {
            transformedVectors.append(vec);
        }
    }

    return transformedVectors;
}

QList<QVector2D> getCubeUVs(float u, float v, float width, float height, float depth, float textureWidth, float textureHeight)
{
    auto toFaceVertices = [textureHeight, textureWidth](float x1, float y1, float x2, float y2) -> QList<QVector2D> {
        return {
            QVector2D(x1 / textureWidth, 1.0 - y2 / textureHeight),
            QVector2D(x2 / textureWidth, 1.0 - y2 / textureHeight),
            QVector2D(x2 / textureWidth, 1.0 - y1 / textureHeight),
            QVector2D(x1 / textureWidth, 1.0 - y1 / textureHeight),
        };
    };

    auto top = toFaceVertices(u + depth, v, u + width + depth, v + depth);
    auto bottom = toFaceVertices(u + width + depth, v, u + width * 2 + depth, v + depth);
    auto left = toFaceVertices(u, v + depth, u + depth, v + depth + height);
    auto front = toFaceVertices(u + depth, v + depth, u + width + depth, v + depth + height);
    auto right = toFaceVertices(u + width + depth, v + depth, u + width + depth * 2, v + height + depth);
    auto back = toFaceVertices(u + width + depth * 2, v + depth, u + width * 2 + depth * 2, v + height + depth);

    auto uvRight = {
        right[0],
        right[1],
        right[3],
        right[2],
    };
    auto uvLeft = {
        left[0],
        left[1],
        left[3],
        left[2],
    };
    auto uvTop = {
        top[0],
        top[1],
        top[3],
        top[2],
    };
    auto uvBottom = {
        bottom[3],
        bottom[2],
        bottom[0],
        bottom[1],
    };
    auto uvFront = {
        front[0],
        front[1],
        front[3],
        front[2],
    };
    auto uvBack = {
        back[0],
        back[1],
        back[3],
        back[2],
    };

    QList<QVector2D> uvData;
    uvData.reserve(24);

    for (const auto& uvArray : { uvFront, uvRight, uvBack, uvLeft, uvBottom, uvTop }) {
        uvData.append(uvArray);
    }

    return uvData;
}

namespace opengl {
BoxGeometry::BoxGeometry(QVector3D size, QVector3D position)
    : QOpenGLFunctions(), m_indexBuf(QOpenGLBuffer::IndexBuffer), m_size(size), m_position(position)
{
    initializeOpenGLFunctions();

    m_vertexBuf.create();
    m_indexBuf.create();
}

BoxGeometry::BoxGeometry(QVector3D size, QVector3D position, QPoint uv, QVector3D textureDim, QSize textureSize)
    : BoxGeometry(size, position)
{
    initGeometry(uv.x(), uv.y(), textureDim.x(), textureDim.y(), textureDim.z(), textureSize.width(), textureSize.height());
}

BoxGeometry::~BoxGeometry()
{
    m_vertexBuf.destroy();
    m_indexBuf.destroy();
}

void BoxGeometry::draw(QOpenGLShaderProgram* program)
{

    program->setUniformValue("model_matrix", m_matrix);
    m_vertexBuf.bind();
    m_indexBuf.bind();

    quintptr offset = 0;

    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 4, sizeof(VertexData));

    offset += sizeof(QVector4D);

    int texcoordLocation = program->attributeLocation("a_texcoord");
    program->enableAttributeArray(texcoordLocation);
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    glDrawElements(GL_TRIANGLE_STRIP, m_indecesCount, GL_UNSIGNED_SHORT, nullptr);
}

void BoxGeometry::initGeometry(float u, float v, float width, float height, float depth, float textureWidth, float textureHeight)
{
    auto textureCord = getCubeUVs(u, v, width, height, depth, textureWidth, textureHeight);

    QMatrix4x4 transformation;
    transformation.translate(m_position);
    transformation.scale(m_size);
    auto positions = transformVectors(transformation, vertices);

    QList<VertexData> verticesData;
    verticesData.reserve(positions.size());

    for (int i = 0; i < positions.size(); ++i) {
        verticesData.append(VertexData(positions[i], textureCord[i]));
    }

    m_vertexBuf.bind();
    m_vertexBuf.allocate(verticesData.constData(), static_cast<int>(verticesData.size() * sizeof(VertexData)));

    m_indexBuf.bind();
    m_indexBuf.allocate(indices.constData(), static_cast<int>(indices.size() * sizeof(GLushort)));
    m_indecesCount = indices.size();
}

void BoxGeometry::rotate(float angle, const QVector3D& vector)
{
    m_matrix.rotate(angle, vector);
}

BoxGeometry* BoxGeometry::Plane()
{
    auto b = new BoxGeometry(QVector3D(), QVector3D());

    b->m_vertexBuf.bind();
    b->m_vertexBuf.allocate(planeVertices.constData(), static_cast<int>(planeVertices.size() * sizeof(VertexData)));

    b->m_indexBuf.bind();
    b->m_indexBuf.allocate(planeIndices.constData(), static_cast<int>(planeIndices.size() * sizeof(GLushort)));
    b->m_indecesCount = planeIndices.size();

    return b;
}

void BoxGeometry::scale(const QVector3D& vector)
{
    m_matrix.scale(vector);
}
}
