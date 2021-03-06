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

#ifndef QDECLARATIVECONTACTTYPE_H
#define QDECLARATIVECONTACTTYPE_H

#include "qdeclarativecontactdetail_p.h"
#include "qcontacttype.h"

class QDeclarativeContactType : public QDeclarativeContactDetail
{
    Q_OBJECT
    Q_PROPERTY(ContactType type READ type WRITE setType NOTIFY fieldsChanged)
    Q_ENUMS(FieldType)
    Q_ENUMS(ContactType)
    Q_CLASSINFO("DefaultProperty", "type")
public:
    enum FieldType {
        TypeField = 0
    };



    ContactDetailType detailType() const
    {
        return QDeclarativeContactDetail::ContactType;
    }
    static QString fieldNameFromFieldType(int fieldType)
    {
        switch (fieldType) {
        case TypeField:
            return QContactType::FieldType;
        default:
            break;
        }
        return "";
    }
    QDeclarativeContactType(QObject* parent = 0)
        :QDeclarativeContactDetail(parent)
    {
        setDetail(QContactType());
        connect(this, SIGNAL(valueChanged()), SIGNAL(fieldsChanged()));
    }

    void setType(ContactType type)
    {
         if (!readOnly())  {
            if (type == Contact)
                detail().setValue(QContactType::FieldType, QContactType::TypeContact);
            else if (type == Group)
                detail().setValue(QContactType::FieldType, QContactType::TypeGroup);
         }
    }
    ContactType type() const
    {
        QString typeString = detail().value(QContactType::FieldType);
        if (typeString == QContactType::TypeContact)
            return Contact;
        return Group;
    }
signals:
    void fieldsChanged();
};
QML_DECLARE_TYPE(QDeclarativeContactType)
#endif

