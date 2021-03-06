/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCore module of the Qt Toolkit.
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
#include <cntdef.h>
#include "cntdbinfo.h"
#include "cntfilterdetail.h"
#include <QStringList>

CntDbInfo::CntDbInfo(QContactManagerEngine* engine):
    m_engine(engine)
{
    QString id("%1.%2");
    contactsTableIdColumNameMapping.insert(id.arg(QContactName::DefinitionName, QContactName::FieldFirstName), "first_name");
    contactsTableIdColumNameMapping.insert(id.arg(QContactName::DefinitionName, QContactName::FieldLastName), "last_name");
    contactsTableIdColumNameMapping.insert(id.arg(QContactOrganization::DefinitionName, QContactOrganization::FieldName), "company_name");
    contactsTableIdColumNameMapping.insert(id.arg(QContactName::DefinitionName, QContactName::FieldCustomLabel), "text_fields");
    contactsTableIdColumNameMapping.insert(id.arg(QContactFavorite::DefinitionName, QContactFavorite::FieldFavorite), "favorite_index");
    
    commAddrTableIdColumNameMapping.insert(id.arg(QContactOnlineAccount::DefinitionName, QContactOnlineAccount::FieldAccountUri), QPair<int,bool>(ESipAddress,false));
    commAddrTableIdColumNameMapping.insert(id.arg(QContactEmailAddress::DefinitionName, QContactEmailAddress::FieldEmailAddress), QPair<int,bool>(EEmailAddress,false));
    commAddrTableIdColumNameMapping.insert(id.arg(QContactPhoneNumber::DefinitionName, QContactPhoneNumber::FieldSubTypes), QPair<int,bool>(EPhoneNumber,true));
    commAddrExtraInfoMap.insert(id.arg(QContactPhoneNumber::DefinitionName, QContactPhoneNumber::FieldSubTypes), EMobileNumber);

}

CntDbInfo::~CntDbInfo()
{
    contactsTableIdColumNameMapping.clear();
    commAddrTableIdColumNameMapping.clear();
    commAddrExtraInfoMap.clear();
}

bool CntDbInfo::SupportsDetail(QString definitionName, QString fieldName)
{
    QString fieldId = QString("%1.%2").arg(definitionName, fieldName);
    if (contactsTableIdColumNameMapping.contains(fieldId))
        return true;
    else if (commAddrTableIdColumNameMapping.contains(fieldId))
        return true;
    else if(commAddrExtraInfoMap.contains(fieldId))
        return true;
    else
        return false;
}

/*!
 * Converts filed id to column name of the database table.
 * QContactManager::contacts function.
 *
 * \a fieldId field id representing the detail field name
 * \a sqlDbTableColumnName On return,contains the column name in the database
 */
void CntDbInfo::getDbTableAndColumnName( const QString definitionName,
                                         const QString fieldName,
                                         QString& tableName,
                                         QString& columnName,
                                         bool& isSubType) const
{
    //check contacts table
    QString fieldId = definitionName + "." + fieldName;

    columnName = "";
    tableName = "";
    isSubType = false;

    if (contactsTableIdColumNameMapping.contains(fieldId)){
         columnName = contactsTableIdColumNameMapping.value(fieldId);
         tableName = "contact";
     }

    if (columnName.isEmpty() || tableName.isEmpty()) {
        //Search comm Addr table
        if (commAddrExtraInfoMap.contains(fieldId)){
            int extraTypeInfo = commAddrExtraInfoMap.value(fieldId);
            columnName +=  QString(" EXTRA_TYPE_INFO = %1").arg(extraTypeInfo);
            columnName += " and" ;
            tableName = "comm_addr";
        }
        if (commAddrTableIdColumNameMapping.contains(fieldId)) {
            // communication address table has slightly differnt format, so we make the column name as
            //  "type = <type> and value "
            QPair<int,bool> val = commAddrTableIdColumNameMapping.value(fieldId);
            int typeval = val.first;
            isSubType = val.second;
            columnName +=  QString(" TYPE = %1").arg(typeval);
            columnName += " and value " ;
            tableName = "comm_addr";
        }
    }
}

QString CntDbInfo::getSortQuery( const QList<QContactSortOrder> &sortOrders,
                                 const QString& selectQuery,
                                 QContactManager::Error* error)
{
    // Set to initial select query
    QString sortQuery =  selectQuery;
    
    if(*error == QContactManager::NoError)
    {
        QStringList list;
        QString tempColumn;
        QString concatString;
        bool blanksLast = true;
        QContactSortOrder tempOrder;
        foreach(const QContactSortOrder& s, sortOrders ) {
            QString tableName;
            QString columnName;
            bool isSubType;

            // Get column names for sort order
            getDbTableAndColumnName(s.detailDefinitionName(), s.detailFieldName(), tableName, columnName, isSubType);
            
            if (tableName.compare("contact") != 0 || columnName.isEmpty()) {
                // Skip invalid sort clause
                continue;
            } else {
                //
                // ORDER BY str_column_1 || str_column_2 || str_column_N COLLATE (comparison function);
                //
                // The || operator concatenates the strings to avoid wrong orders in cases where 
                // one of the rows in the colums has a NULL value
                //
                if (isStringFieldType(s.detailDefinitionName())) {
                    if ( tempColumn.isEmpty() ) {
                        // This is the first string. Store sortOrder and 
                        // columnName in temp variable to be used later
                        tempColumn = columnName;
                        tempOrder = s;
                        continue;
                    } else {
                        // Check if the previous sort order has the same direction and
                        // case sensitivity as the current one. If true store the temp
                        // variables to be used later
                        if (s.direction() == tempOrder.direction() && 
                                s.caseSensitivity() == tempOrder.caseSensitivity() 
                                && s.blankPolicy() == tempOrder.blankPolicy()) {
                            tempColumn += " || " + columnName;
                            tempOrder = s;
                            continue;
                        } else {
                            // Its time to break the loop and use the tempColumn string
                            concatString = tempColumn;
                            setDirAndCaseSensitivity(tempOrder, tempColumn);
                            list.append(tempColumn);
                            tempColumn.clear();
                        }
                    }                   
                }
                QString col = ' ' + columnName + ' ';
                setDirAndCaseSensitivity(s, col);
                list.append(col);
            }
        }
        
        if (!tempColumn.isEmpty()) {
            concatString = tempColumn;
            setDirAndCaseSensitivity(tempOrder, tempColumn);
            list.append(tempColumn);
            tempColumn.clear();
        }
        
        if (tempOrder.isValid() && tempOrder.blankPolicy() == QContactSortOrder::BlanksFirst)
            blanksLast = false;
        
        if (list.count() > 0) {
            // Recreate query
            // SELECT DISTINCT contact_id FROM contact WHERE contact_id in (
            //      SELECT ..
            // )  
            sortQuery = "SELECT DISTINCT contact_id FROM contact WHERE contact_id in (";
            sortQuery += selectQuery;
			sortQuery += ')';
			
            QString clause = list.join(" ,");
            
            // ORDER BY CASE <concatenated string fields> WHEN "" THEN 1 ELSE 0 END, <concatenated string fields & other fields> <order>
            if (blanksLast) {
                // If concatString is blank, do not evaluate ORDER BY. All unevaluated rows are pushed to the end
                sortQuery += " ORDER BY CASE WHEN " + concatString + " = \"\" THEN 1 " + " WHEN " + concatString + " IS NULL THEN 1 ELSE 0 END, " + clause;
            } else {
                sortQuery += " ORDER BY CASE WHEN " + concatString + " = \"\" THEN 0 " + " WHEN " + concatString + " IS NULL THEN 0 ELSE 1 END, " + clause;
            }
        }
    }
    
    return sortQuery;
}

bool CntDbInfo::isStringFieldType( const QString definitionName )
{
    if ( definitionName == QContactOrganization::DefinitionName
        || definitionName == QContactName::DefinitionName )
        return true;
    else
        return false;
}

void CntDbInfo::setDirAndCaseSensitivity(QContactSortOrder s, QString& query)
{
    if (s.direction() == Qt::DescendingOrder) {
        if (s.caseSensitivity() == Qt::CaseInsensitive)
            query += " COLLATE NOCASE DESC";
        else
            query += " DESC";
    } else {
        // Default sort order
        if(s.caseSensitivity() == Qt::CaseInsensitive)
            query += " COLLATE CompareC3";
        else
            query += " ASC";
    }
}
