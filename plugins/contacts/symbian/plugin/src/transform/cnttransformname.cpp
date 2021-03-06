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
#include "cnttransformname.h"

#include <QDebug>

QList<CContactItemField *> CntTransformName::transformDetailL(const QContactDetail &detail)
{
    if(detail.definitionName() != QContactName::DefinitionName)
       User::Leave(KErrArgument);

    QList<CContactItemField *> fieldList;

    //cast to name
    const QContactName &name(static_cast<const QContactName&>(detail));

    //create new fields without contexts
    transformToTextFieldL(name, fieldList, name.prefix(), KUidContactFieldPrefixName, KUidContactFieldVCardMapUnusedN, false);
    transformToTextFieldL(name, fieldList, name.firstName(), KUidContactFieldGivenName, KUidContactFieldVCardMapUnusedN, false);
    transformToTextFieldL(name, fieldList, name.middleName(), KUidContactFieldAdditionalName, KUidContactFieldVCardMapUnusedN, false);
    transformToTextFieldL(name, fieldList, name.lastName(), KUidContactFieldFamilyName, KUidContactFieldVCardMapUnusedN, false);
    transformToTextFieldL(name, fieldList, name.suffix(), KUidContactFieldSuffixName, KUidContactFieldVCardMapUnusedN, false);
    transformToTextFieldL(name, fieldList, name.customLabel(), KUidContactFieldTemplateLabel, KUidContactFieldVCardMapUnusedN, false);

    return fieldList;
}


QContactDetail *CntTransformName::transformItemField(const CContactItemField& field, const QContact &contact)
{
    QContactName *name = new QContactName(contact.detail<QContactName>());

    CContactTextField* storage = field.TextStorage();
    QString nameValue = QString::fromUtf16(storage->Text().Ptr(), storage->Text().Length());

    for (int i = 0; i < field.ContentType().FieldTypeCount(); i++) {
        //Prefix
        if (field.ContentType().FieldType(i) == KUidContactFieldPrefixName) {
            name->setPrefix(nameValue);
        }
        //First name
        else if (field.ContentType().FieldType(i) == KUidContactFieldGivenName) {
            name->setFirstName(nameValue);
        }
        //Middle name
        else if (field.ContentType().FieldType(i) == KUidContactFieldAdditionalName) {
            name->setMiddleName(nameValue);
        }
        //Last name
        else if (field.ContentType().FieldType(i) == KUidContactFieldFamilyName) {
            name->setLastName(nameValue);
        }
        //Suffix
        else if (field.ContentType().FieldType(i) == KUidContactFieldSuffixName) {
            name->setSuffix(nameValue);
        }
        // custom label
        else if (field.ContentType().FieldType(i) == KUidContactFieldTemplateLabel) {
            name->setCustomLabel(nameValue);
        }
    }

    return name;
}

bool CntTransformName::supportsDetail(QString detailName) const
{
    bool ret = false;
    if (detailName == QContactName::DefinitionName) {
        ret = true;
    }
    return ret;
}

QList<TUid> CntTransformName::supportedFields() const
{
    return QList<TUid>()
        << KUidContactFieldPrefixName
        << KUidContactFieldGivenName
        << KUidContactFieldAdditionalName
        << KUidContactFieldFamilyName
        << KUidContactFieldSuffixName
        << KUidContactFieldTemplateLabel;
}

QList<TUid> CntTransformName::supportedSortingFieldTypes(QString detailFieldName) const
{
    QList<TUid> uids;

    if (detailFieldName == QContactName::FieldPrefix)
        return uids << KUidContactFieldPrefixName;

    if (detailFieldName == QContactName::FieldFirstName)
        return uids << KUidContactFieldGivenName;

    if (detailFieldName == QContactName::FieldMiddleName)
        return uids << KUidContactFieldAdditionalName;

    if (detailFieldName == QContactName::FieldLastName)
        return uids << KUidContactFieldFamilyName;

    if (detailFieldName == QContactName::FieldSuffix)
        return uids << KUidContactFieldSuffixName;

    if (detailFieldName == QContactName::FieldCustomLabel)
        return uids << KUidContactFieldTemplateLabel;

    return uids;
}

/*!
 * Checks whether the subtype is supported
 *
 * \a subType The subtype to be checked
 * \return True if this subtype is supported
 */
bool CntTransformName::supportsSubType(const QString& subType) const
{
    Q_UNUSED(subType);
    return false;
}

/*!
 * Returns the filed id corresponding to a field
 *
 * \a fieldName The name of the supported field
 * \return fieldId for the fieldName, 0  if not supported
 */
quint32 CntTransformName::getIdForField(const QString& fieldName) const
{
    if (QContactName::FieldPrefix == fieldName)
        return KUidContactFieldPrefixName.iUid;
    else if (QContactName::FieldFirstName == fieldName)
        return KUidContactFieldGivenName.iUid;
    else if (QContactName::FieldMiddleName == fieldName)
        return KUidContactFieldAdditionalName.iUid;
    else if (QContactName::FieldLastName == fieldName)
        return KUidContactFieldFamilyName.iUid;
    else if (QContactName::FieldSuffix == fieldName)
        return KUidContactFieldSuffixName.iUid;
    else if (QContactName::FieldCustomLabel == fieldName)
        return KUidContactFieldTemplateLabel.iUid;
    else
        return 0;
}

/*!
 * Modifies the detail definitions. The default detail definitions are
 * queried from QContactManagerEngine::schemaDefinitions and then modified
 * with this function in the transform leaf classes.
 *
 * \a definitions The detail definitions to modify.
 * \a contactType The contact type the definitions apply for.
 */
void CntTransformName::detailDefinitions(QMap<QString, QContactDetailDefinition> &definitions, const QString& contactType) const
{
    if(definitions.contains(QContactName::DefinitionName)) {
        QContactDetailDefinition d = definitions.value(QContactName::DefinitionName);
        QMap<QString, QContactDetailFieldDefinition> fields = d.fields();

        // groups support only custom label
        if(contactType == QContactType::TypeGroup) {
            fields.remove(QContactName::FieldPrefix);
            fields.remove(QContactName::FieldFirstName);
            fields.remove(QContactName::FieldMiddleName);
            fields.remove(QContactName::FieldLastName);
            fields.remove(QContactName::FieldSuffix);
        } else {
            // Note: Custom labels cannot be enabled for a contact in pre-10.1
            // platforms because setting custom label for a contact causes
            // issues for S60 Phonebook editor. If custom label support is
            // needed in 10.1 or later, it needs to be variated away from
            // pre-10.1 platforms.
#ifndef SYMBIAN_BACKEND_USE_CNTMODEL_V2        
            fields.remove(QContactName::FieldCustomLabel);
#endif            
        }

        // Context not supported in symbian back-end, remove
        fields.remove(QContactName::FieldContext);

        d.setFields(fields);
        d.setUnique(true);

        // Replace original definitions
        definitions.insert(d.name(), d);
    }
}
