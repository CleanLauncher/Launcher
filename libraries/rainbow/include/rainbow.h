/* This was part of the KDE project: see KGuiAddons
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright (C) 2007 Olaf Schmidt <ojschmidt@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Zack Rusin <zack@kde.org>
 * Copyright (C) 2015 Petr Mrazek <peterix@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include <QPainter>
class QColor;

namespace Rainbow
{

qreal luma(const QColor&);

void getHcy(const QColor&, qreal* hue, qreal* chroma, qreal* luma, qreal* alpha = 0);

qreal contrastRatio(const QColor&, const QColor&);

QColor lighten(const QColor&, qreal amount = 0.5, qreal chromaInverseGain = 1.0);

QColor darken(const QColor&, qreal amount = 0.5, qreal chromaGain = 1.0);

QColor shade(const QColor&, qreal lumaAmount, qreal chromaAmount = 0.0);

QColor tint(const QColor& base, const QColor& color, qreal amount = 0.3);

QColor mix(const QColor& c1, const QColor& c2, qreal bias = 0.5);

QColor overlayColors(const QColor& base, const QColor& paint, QPainter::CompositionMode comp = QPainter::CompositionMode_SourceOver);
}  // namespace Rainbow
