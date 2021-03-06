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

#ifndef QDECLARATIVEQGEOSERVICEPROVIDER_H
#define QDECLARATIVEQGEOSERVICEPROVIDER_H

#include <qgeocoordinate.h>
#include <QtDeclarative/qdeclarativeitem.h>

#include <QMap>
#include <QString>
#include <QVariant>

QTM_BEGIN_NAMESPACE

class QDeclarativeGeoServiceProviderParameter : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

public:
    QDeclarativeGeoServiceProviderParameter(QObject *parent = 0);
    ~QDeclarativeGeoServiceProviderParameter();

    void setName(const QString &name);
    QString name() const;

    void setValue(const QVariant &value);
    QVariant value() const;

Q_SIGNALS:
    void nameChanged(const QString &name);
    void valueChanged(const QVariant &value);

private:
    QString name_;
    QVariant value_;
};

class QDeclarativeGeoServiceProvider : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeGeoServiceProviderParameter> parameters READ parameters)

    Q_CLASSINFO("DefaultProperty", "parameters")

public:
    QDeclarativeGeoServiceProvider(QObject *parent = 0);
    ~QDeclarativeGeoServiceProvider();

    void setName(const QString &name);
    QString name() const;

    QDeclarativeListProperty<QDeclarativeGeoServiceProviderParameter> parameters();

    QMap<QString, QVariant> parameterMap() const;

Q_SIGNALS:
    void nameChanged(const QString &name);

private:
    static void parameter_append(QDeclarativeListProperty<QDeclarativeGeoServiceProviderParameter> *prop, QDeclarativeGeoServiceProviderParameter *mapObject);
    static int parameter_count(QDeclarativeListProperty<QDeclarativeGeoServiceProviderParameter> *prop);
    static QDeclarativeGeoServiceProviderParameter* parameter_at(QDeclarativeListProperty<QDeclarativeGeoServiceProviderParameter> *prop, int index);
    static void parameter_clear(QDeclarativeListProperty<QDeclarativeGeoServiceProviderParameter> *prop);

    QString name_;
    QList<QDeclarativeGeoServiceProviderParameter*> parameters_;

    Q_DISABLE_COPY(QDeclarativeGeoServiceProvider)
};

QTM_END_NAMESPACE

QML_DECLARE_TYPE(QTM_PREPEND_NAMESPACE(QDeclarativeGeoServiceProviderParameter));
QML_DECLARE_TYPE(QTM_PREPEND_NAMESPACE(QDeclarativeGeoServiceProvider));

#endif
