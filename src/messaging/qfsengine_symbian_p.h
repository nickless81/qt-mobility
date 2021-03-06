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

#ifndef QFSENGINE_SYMBIAN_H
#define QFSENGINE_SYMBIAN_H

#include <qmessageglobal.h>
#include <QObject>
#include <QMap>
#include <QSet>
#include <QList>
#include <QHash>

#include <e32base.h>

#include "qmessagemanager.h"
#include "qmessagestore_symbian_p.h"
#include "qmessagefilter.h"
#include "qmessagefilter_p.h"
#include "qmessagefolderfilter.h"
#include "qmessageservice.h"
#include "qmessageservice_symbian_p.h"
#include "maemohelpers_p.h"
#include <emailinterfacefactory.h>
#include <memailclientapi.h>
#include <memailmessagesearch.h>
#include <memailmessage.h>
#include <emailinterfacefactory.h>
#include <emailsorting.h>
#include <memailmessagesearch.h>
#include <memailfolder.h>
#include <mmailboxsyncobserver.h>
#include <mmailboxcontentobserver.h>

#ifdef FREESTYLEMAILMAPI12USED
#include <memailclientapiobserver.h>
#include <memailrequestobserver.h>
#endif

using namespace EmailInterface;

QTM_BEGIN_NAMESPACE

class CFSMessagesFindOperation;
class CFSContentFetchOperation;
class CFSContentStructureFetchOperation;
class QMessageId;
class QMessageAccount;

struct FSMessageQueryInfo
{
    int operationId;
    bool isQuery;
    QString body;
    QMessageDataComparator::MatchFlags matchFlags;
    QMessageFilter filter;
    QMessageSortOrder sortOrder;
    int offset;
    int limit;
    QMessageServicePrivate* privateService;
    CFSMessagesFindOperation* findOperation;
    int currentFilterListIndex;
    QMessageIdList ids;
    int count;
    bool canceled;    
};

struct FSSearchOperation
{
    MEmailMailbox* m_mailbox;
    TEmailSortCriteria m_emailSortCriteria;
};

class EMailSyncRequest : public MMailboxSyncObserver
{
public:
    EMailSyncRequest(QMessageServicePrivate &observer, QList<EMailSyncRequest*> &requestList, 
            const TMailboxId &mailboxId) : 
        m_mailboxId(mailboxId), m_observer(observer), m_requestList(requestList), m_active(false) {};

public: // from MMailboxSyncObserver
    void MailboxSynchronisedL(TInt aResult);
    
public:
  
    TMailboxId m_mailboxId;
    QMessageServicePrivate& m_observer;
    
    // not own
    QList<EMailSyncRequest*>& m_requestList;
    
    bool m_active;
};

#ifdef FREESTYLEMAILMAPI12USED

/*!
 * Helper class for tracking multiple move requests.
 */
class EMailMoveRequest
{
public:
    EMailMoveRequest( QMessageServicePrivate* observer, TMailboxId mailbox ) :
        m_observer( observer ), m_mailbox( mailbox ) {}
    EMailMoveRequest() : m_observer( NULL ) {}
    
    bool isNull() { 
        return !(m_observer && m_mailbox.iId != KUndefinedEntryId);
    }
    
public:
    /// Not own.
    QMessageServicePrivate*     m_observer;
    /// 
    TMailboxId                  m_mailbox;
};
#endif

#ifdef FREESTYLEMAILMAPI12USED
class CFSEngine : public QObject, public MMailboxContentObserver, public MEmailClientApiObserver, public MEmailRequestObserver
#else
class CFSEngine : public QObject, public MMailboxContentObserver
#endif
{
    Q_OBJECT

public: 
    
    static CFSEngine* instance();

    CFSEngine();
    ~CFSEngine();
#ifdef FREESTYLEMAILMAPI12USED
    void setMessageStorePrivateSingleton(QMessageStorePrivate* privateStore);
#endif
    QMessageAccountIdList queryAccounts(const QMessageAccountFilter &filter, const QMessageAccountSortOrder &sortOrder, uint limit, uint offset) const;
    int countAccounts(const QMessageAccountFilter &filter) const;
    QMessageAccount account(const QMessageAccountId &id) const;
    QMessageAccountId defaultAccount(QMessage::Type type) const;
#ifdef FREESTYLEMAILMAPI12USED
    int removeAccount(const QMessageAccountId &id);
#endif
    QMessageFolderIdList queryFolders(const QMessageFolderFilter &filter, const QMessageFolderSortOrder &sortOrder, uint limit, uint offset) const;
    int countFolders(const QMessageFolderFilter &filter) const;
    QMessageFolder folder(const QMessageFolderId &id) const;
    
    bool addMessage(QMessage *m);
    bool updateMessage(QMessage *m);
    bool removeMessage(const QMessageId &id, QMessageManager::RemovalOption option);
    bool removeMessages(const QMessageFilter &filter, QMessageManager::RemovalOption option);
    bool queryMessages(QMessageServicePrivate& privateService, const QMessageFilter &filter, const QMessageSortOrder &sortOrder, uint limit, uint offset) const;
    bool queryMessages(QMessageServicePrivate& privateService, const QMessageFilter &filter, const QString &body, QMessageDataComparator::MatchFlags matchFlags, const QMessageSortOrder &sortOrder, uint limit, uint offset) const;
    bool countMessages(QMessageServicePrivate& privateService, const QMessageFilter &filter);
    bool showMessage(const QMessageId &id);
    bool composeMessage(const QMessage &message);   
    QMessage message(const QMessageId& id) const;
    bool sendEmail(QMessage &message);
   
    bool retrieve(QMessageServicePrivate& privateService, const QMessageId &messageId, const QMessageContentContainerId& id);
    bool retrieveBody(QMessageServicePrivate& privateService, const QMessageId& id);
    bool retrieveHeader(QMessageServicePrivate& privateService, const QMessageId& id);
    bool synchronize(QMessageServicePrivate& observer, const QMessageAccountId& id);
#ifdef FREESTYLEMAILMAPI12USED
    bool moveMessages(QMessageServicePrivate& observer, const QMessageIdList &messageIds, const QMessageFolderId &toFolderId);
#endif
    QMessageManager::NotificationFilterId registerNotificationFilter(QMessageStorePrivate& aPrivateStore,
                                        const QMessageFilter& filter, QMessageManager::NotificationFilterId aId);
    void unregisterNotificationFilter(QMessageManager::NotificationFilterId notificationFilterId);

    void filterAndOrderMessagesReady(bool success, int operationId, QMessageIdList ids, int numberOfHandledFilters,
                                     bool resultSetOrdered);
    
    void retrieveMessageContentHeaders(QMessage& message) const;
    QString attachmentTextContent(long int messageId, TMessageContentId attachmentContentId, const QByteArray &charset) const;
    QByteArray attachmentContent(long int messageId, TMessageContentId attachmentContentId) const;
    QString bodyContent(long int messageId, TMessageContentId bodyContentId) const;
    
    void cancel(QMessageServicePrivate& privateService);

public:
    void setPluginObserversL();
    
public:
        // from MMailboxContentObserver
    void NewMessageEventL(const TMailboxId& aMailbox, const REmailMessageIdArray aNewMessages, const TFolderId& aParentFolderId);
    void MessageChangedEventL(const TMailboxId& aMailbox, const REmailMessageIdArray aChangedMessages, const TFolderId& aParentFolderId);
    void MessageDeletedEventL(const TMailboxId& aMailbox, const REmailMessageIdArray aDeletedMessages, const TFolderId& aParentFolderId);       

#ifdef FREESTYLEMAILMAPI12USED
public:
    // from MEmailClientApiObserver
    void EmailClientApiEventL(const TEmailClientApiEvent aEvent, const TMailboxId& aId);
    // from MEmailRequestObserver
    void EmailRequestCompleteL(TInt aResult, TUint aRequestId);
#endif
    
public slots:
    void cleanupFSBackend();
    void contentFetched(void* service, bool success);
#ifdef FREESTYLEMAILMAPI12USED
    void contentStructureFetched(void* service, bool success);
#endif

private:

    void queryMessagesL(QMessageServicePrivate& privateService, const QMessageFilter &filter, const QMessageSortOrder &sortOrder, uint limit, uint offset) const;
    void queryMessagesL(QMessageServicePrivate& privateService, const QMessageFilter &filter, const QString &body, QMessageDataComparator::MatchFlags matchFlags, const QMessageSortOrder &sortOrder, uint limit, uint offset) const;
    void countMessagesL(QMessageServicePrivate& privateService, const QMessageFilter &filter);
    void doNextQuery() const;
    
    QMessageAccountIdList accountsByType(QMessage::Type type) const;
    void updateEmailAccountsL() const;
    void updateMessageL(QMessage* message);
    MEmailMessage* createFSMessageL(const QMessage &message, const MEmailMailbox* mailbox);
    QMessageFolderIdList folderIdsByAccountId(const QMessageAccountId& accountId) const;
    QMessageFolderIdList folderIdsByAccountIdL(const QMessageAccountId& accountId) const;
    QMessageFolderIdList filterMessageFoldersL(const QMessageFolderFilter& filter, bool& filterHandled) const;
    QMessageFolderIdList allFolders() const;
    QMessageFolder folderL(const QMessageFolderId &id) const;
    bool fsFolderL(const QMessageFolderId& id, MEmailMailbox*& mailbox, MEmailFolder*& folder) const;
    TFolderType standardFolderId(QMessage::StandardFolder standardFolder);
    
    QMessageFolderIdList filterMessageFolders(const QMessageFolderFilter& filter, bool& filterHandled) const;
    void CreateQMessageL(QMessage* aQMessage, const MEmailMessage& aFSMessage) const;
    void addMessagePartsToQMessage(QMessage& message, MEmailMessage& mEmailMessage) const;
    void addContentToQMessage(QMessage& message, const MEmailMessageContent& content, TMessageId messageId) const;
    void addAttachmentToQMessage(QMessage& message, QMessageContentContainer& attachment) const;
    QString bodyContentFromMessageContent(const MEmailMessageContent& messageContent) const;
    QDateTime symbianTTimetoQDateTime(const TTime& time) const;
    TTime qDateTimeToSymbianTTime(const QDateTime& date) const;
    
    bool message(QMessage* message, const QMessageId& id) const;
    static bool accountLessThan(const QMessageAccountId accountId1, const QMessageAccountId accountId2);
    void orderAccounts(QMessageAccountIdList& accountIds,  const QMessageAccountSortOrder &sortOrder) const;
    void applyOffsetAndLimitToAccountIds(QMessageAccountIdList& idList, int offset, int limit) const;
    
    static bool folderLessThan(const QMessageFolderId folderId1, const QMessageFolderId folderId2);
    void orderFolders(QMessageFolderIdList& folderIds,  const QMessageFolderSortOrder &sortOrder) const;
    void applyOffsetAndLimitToMsgFolderIds(QMessageFolderIdList& idList, int offset, int limit) const;
    void handleNestedFiltersFromFolderFilter(QMessageFolderFilter &filter) const;

    static bool messageLessThan(const QMessage& message1, const QMessage& message2);
    void orderMessages(QMessageIdList& messageIds,  const QMessageSortOrder &sortOrder) const;
    void applyOffsetAndLimitToMsgIds(QMessageIdList& idList, int offset, int limit) const;

    void handleNestedFiltersFromMessageFilter(QMessageFilter &filter) const;
    void synchronizeL(QMessageServicePrivate &observer, const QMessageAccountId &id);
#ifdef FREESTYLEMAILMAPI12USED
    void moveMessagesL(QMessageServicePrivate &observer, const REmailMessageIdArray &messages, const TFolderId &toFolder);
#endif

    MEmailAttachment* attachmentById(TMessageContentId attachmentId) const;
    MEmailTextContent* textContentById(TMessageContentId contentId, MEmailMessageContent* parentContent = NULL) const;
    void deleteContentFetchOperation(QMessageServicePrivate& service);

    static TMessageId fsMessageIdFromQMessageId(QMessageId messageId);
    static QMessageId qMessageIdFromFsMessageId(TMessageId messageId);
    static TFolderId fsFolderIdFromQMessageFolderId(QMessageFolderId folderId);
    static QMessageFolderId qMessageFolderIdFromFsFolderId(TFolderId folderId);
    static TMailboxId fsMailboxIdFromQMessageAccountId(QMessageAccountId accountId);
    static QMessageAccountId qMessageAccountIdFromFsMailboxId(TMailboxId mailboxId);

    static void convertQMessageAddressToFreestyle(const QString& emailAddress,  QString& qaddress, QString& qname,
                                                    TPtrC16& address, TPtrC16& displayname);
    static void convertFreestyleAddressToQString(MEmailAddress* emailAddress, QString& combinedAddress);

    static void cleanup();

    void notificationL(const TMailboxId& aMailbox, const TMessageId& aMessageId, 
                        const TFolderId& aParentFolderId, QMessageStorePrivate::NotificationType aNotificationType);
    
    void addMessageL(QMessage *message);
    friend class QMessageService;
    friend class CMessagesFindOperation;
    friend class CFSContentFetchOperation;
    
private:
    CEmailInterfaceFactory* m_factory;
    mutable MEmailInterface* m_ifPtr;
    mutable MEmailClientApi* m_clientApi;
    mutable QHash<QString, QMessageAccount> m_accounts;
    mutable QHash<TEntryId, QMessage::StandardFolder> m_folderTypes;
    mutable int m_operationIds;
    mutable QList<FSMessageQueryInfo> m_messageQueries;
    mutable QMap<QMessageServicePrivate*, CFSContentFetchOperation*> m_contentFetchOperations;
#ifdef FREESTYLEMAILMAPI12USED
    mutable QMap<QMessageServicePrivate*, CFSContentStructureFetchOperation*> m_contentStructurefetchOperations;
#endif
    mutable bool m_messageQueryActive;
    TMailboxId m_mailboxId;
    QMessageStorePrivate* ipMessageStorePrivate;
    bool iListenForNotifications;
    QMessageManager::NotificationFilterId m_filterId;
    QMap<QMessageManager::NotificationFilterId, QMessageFilter> m_filters;
    QMessageAccount m_account;
    mutable QMap<TEntryId, MEmailMailbox*> m_mailboxes;
    REmailAttachmentArray m_attachments;
    friend class QMessageService;
    friend class CFSMessagesFindOperation;
    
    mutable QMessageAccountSortOrder m_currentAccountOrdering;
    mutable QMessageFolderSortOrder m_currentFolderOrdering;
    mutable QMessageSortOrder m_currentMessageOrdering;

    mutable bool m_cleanedup;

    QList<EMailSyncRequest*> m_syncRequests;
#ifdef FREESTYLEMAILMAPI12USED
    QMap<uint, EMailMoveRequest> m_moveRequests;
    uint m_mailboxMoveRequestId;
    QMessageStorePrivate* m_messageStorePrivateSingleton;
#endif
};

class CFSContentFetchOperation : public QObject, MEmailFetchObserver
{
    Q_OBJECT

public:
    CFSContentFetchOperation(CFSEngine& parentEngine, QMessageServicePrivate& service,
                             MEmailMessageContent* content, MEmailMessage* message = NULL);
    ~CFSContentFetchOperation();

    bool fetch();
    void cancelFetch();

protected: // From MEmailFetchObserver
    void DataFetchedL(const TInt aResult);

private:
    CFSEngine& m_parentEngine;
    QMessageServicePrivate& m_service;
    MEmailMessageContent* m_content;
    MEmailMessage* m_message;

    friend class CFSEngine;
};

#ifdef FREESTYLEMAILMAPI12USED
class CFSContentStructureFetchOperation : public QObject, MEmailMessageObserver
{
    Q_OBJECT

public:
    CFSContentStructureFetchOperation(CFSEngine& parentEngine, QMessageServicePrivate& service,
                             MEmailMessage* message = NULL);
    ~CFSContentStructureFetchOperation();

    bool fetch();
    void cancelFetch();

protected: // From MEmailMessageObserver
    void DataFetchedL(const TInt aResult);

private:
    CFSEngine& m_parentEngine;
    QMessageServicePrivate& m_service;
    MEmailMessage* m_message;

    friend class CFSEngine;
};
#endif

class CFSMessagesFindOperation : public QObject
{
    Q_OBJECT
    
public:
    CFSMessagesFindOperation(CFSEngine& aOwner, int aOperationId); 
    ~CFSMessagesFindOperation();

    int filterAndOrderMessages(const QMessageFilter& filter,
                               const QMessageSortOrder& sortOrder,
                               const QString body = QString(),
                               QMessageDataComparator::MatchFlags matchFlags = 0);
    int filterAndOrderMessages(const QMessageFilterPrivate::SortedMessageFilterList& filters,
                               const QMessageSortOrder& sortOrder,
                               const QString body = QString(),
                               QMessageDataComparator::MatchFlags matchFlags = 0);

    enum Field {None = 0, Sender, Recipients, Subject, Body};

    
public slots:
    void searchAccountFolders();
    void SearchCompleted();

private:
    void filterAndOrderMessagesL(const QMessageFilterPrivate::SortedMessageFilterList& filters,
                                const QMessageSortOrder& sortOrder,
                                const QString body = QString(),
                                QMessageDataComparator::MatchFlags matchFlags = 0);
    
    void getAllMessagesL(TEmailSortCriteria& sortCriteria);
    void getAccountSpecificMessagesL(QMessageAccount& messageAccount, TEmailSortCriteria& sortCriteria);
    void getAccountSpecificMessagesFromAccountFoldersL(FSSearchOperation& searchOperation);
    void getFolderSpecificMessagesL(MEmailFolder& folder, TEmailSortCriteria sortCriteria);
    
    bool filterBody(QMessageId& messageId);

    // TODO: Remove this function as soon as CMessageIterator bug is fixed
    static unsigned int pluginReferenceCount(MEmailFolder* folder);
    // TODO: Remove this function as soon as CMessageIterator bug is fixed
    static void setPluginReferenceCount(MEmailFolder* folder, unsigned int referenceCount);
    
private: // Data
    CFSEngine& m_owner;
    
    int m_numberOfHandledFilters;
    int m_operationId;
    bool m_asynchronousSearchStarted;
    bool m_resultCorrectlyOrdered;
    QMessageIdList m_idList;
    QMessageFilterPrivate::SortedMessageFilterList m_filterList;
    QMessageIdList m_excludeIdList;

    MEmailClientApi* m_clientApi;
    
    CEmailInterfaceFactory* m_factory; 
    MEmailInterface* m_interfacePtr;
    bool m_receiveNewMessages;
    QList<FSSearchOperation> m_searchOperations;
    
    QString m_body;
    QMessageDataComparator::MatchFlags m_matchFlags;
};

// TODO: Remove this class definition as soon as CMessageIterator bug is fixed
class TPluginData
{
public:
    TPluginData(TUid uid) : iUid(uid) {}
    void* iPlugin;
    const TUid iUid;
};

// TODO: Remove this class definition as soon as CMessageIterator bug is fixed
class CPluginData : public CBase
{
public:
    CPluginData() : iData(TPluginData(TUid::Null())), iPluginLoadError(0), iRefCount(0) {}
    TPluginData iData;
    TInt iPluginLoadError;
    TUint iRefCount;
};

// TODO: Remove this class definition as soon as CMessageIterator bug is fixed
class CEmailFolder : public CBase, public MEmailFolder
{
public:
    CEmailFolder(CPluginData &aPluginData) : iPluginData(aPluginData) {}
    CPluginData&    iPluginData;
};

QTM_END_NAMESPACE

#endif // QFSENGINE_SYMBIAN_H
