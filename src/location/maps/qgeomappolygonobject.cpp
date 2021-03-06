/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qgeomappolygonobject.h"
#include "qgeomappolygonobject_p.h"

#include "qgeocoordinate.h"

QTM_BEGIN_NAMESPACE

/*!
    \class QGeoMapPolygonObject
    \brief The QGeoMapPolygonObject class is a QGeoMapObject used to draw
    a polygon on a map.

    \inmodule QtLocation
    \since 1.1

    \ingroup maps-mapping-objects

    The polygon is specified by a set of at least 3 valid QGeoCoordinate
    instances listed in the same order in which the coordinates would be
    traversed when traveling around the border of the polygon.
*/

/*!
    Constructs a new polygon object.
*/
QGeoMapPolygonObject::QGeoMapPolygonObject()
    : d_ptr(new QGeoMapPolygonObjectPrivate())
{
    setUnits(QGeoMapObject::RelativeArcSecondUnit);
    setTransformType(QGeoMapObject::ExactTransform);
}

/*!
    Destroys this polygon object.
*/
QGeoMapPolygonObject::~QGeoMapPolygonObject()
{
    delete d_ptr;
}

/*!
    \reimp
    \since 1.1
*/
QGeoMapObject::Type QGeoMapPolygonObject::type() const
{
    return QGeoMapObject::PolygonType;
}

/*!
    \property QGeoMapPolygonObject::path
    \brief the ordered list of coordinates which define the
    polygon to be drawn by this polygon object.

    The default value of this property is an empty list of coordinates.

    The coordinates should be listed in the order in which they would be
    traversed when traveling around the border of the polygon.

    Invalid coordinates in the list will be ignored, and if the list of
    coordinates contains less than 3 valid coordinates then the polygon object
    will not be displayed.

    \since 1.1
*/
void QGeoMapPolygonObject::setPath(const QList<QGeoCoordinate> &path)
{
    if (d_ptr->path != path) {
        d_ptr->path = path;
        if (path.size() != 0)
            setOrigin(path.at(0));
        else
            setOrigin(QGeoCoordinate());
        emit pathChanged(emit d_ptr->path);
    }
}

QList<QGeoCoordinate> QGeoMapPolygonObject::path() const
{
    return d_ptr->path;
}

/*!
    \property QGeoMapPolygonObject::pen
    \brief the pen that will be used to draw this object.

    The pen is used to draw an outline around the polygon. The polygon is
    filled using the QGeoMapPolygonObject::brush property.

    The pen will be treated as a cosmetic pen, which means that the width
    of the pen will be independent of the zoom level of the map.
    \since 1.1
*/
void QGeoMapPolygonObject::setPen(const QPen &pen)
{

    QPen newPen = pen;
    newPen.setCosmetic(true);

    if (d_ptr->pen == newPen)
        return;

    d_ptr->pen = pen;
    emit penChanged(pen);
}

QPen QGeoMapPolygonObject::pen() const
{
    return d_ptr->pen;
}

/*!
    \property QGeoMapPolygonObject::brush
    \brief the brush that will be used to draw this object.

    The brush is used to fill in polygon.

    The outline around the perimeter of the polygon is drawn using the
    QGeoMapPolygonObject::pen property.
    \since 1.1
*/
void QGeoMapPolygonObject::setBrush(const QBrush &brush)
{
    if (d_ptr->brush != brush) {
        d_ptr->brush = brush;
        emit brushChanged(brush);
    }
}

QBrush QGeoMapPolygonObject::brush() const
{
    return d_ptr->brush;
}

/*!
\fn void QGeoMapPolygonObject::pathChanged(const QList<QGeoCoordinate> &path)

    This signal is emitted when the ordered list of coordinates that define
    the polygon to be drawn by this polygon object has changed.

    The new value is \a path.
    \since 1.1
*/

/*!
\fn void QGeoMapPolygonObject::penChanged(const QPen &pen)

    This signal is emitted when the pen used to draw the edge of this
    polygon object has changed.

    The new value is \a pen.
    \since 1.1
*/

/*!
\fn void QGeoMapPolygonObject::brushChanged(const QBrush &brush)

    This signal is emitted when the brush used to fill in the interior of
    this polygon object has changed.

    The new value is \a brush.
    \since 1.1
*/

/*******************************************************************************
*******************************************************************************/

QGeoMapPolygonObjectPrivate::QGeoMapPolygonObjectPrivate()
{
    pen.setCosmetic(true);
}

QGeoMapPolygonObjectPrivate::~QGeoMapPolygonObjectPrivate() {}

#include "moc_qgeomappolygonobject.cpp"

QTM_END_NAMESPACE

