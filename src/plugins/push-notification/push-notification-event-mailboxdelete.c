/* Copyright (c) 2015-2016 Dovecot authors, see the included COPYING file */

#include "lib.h"

#include "push-notification-drivers.h"
#include "push-notification-events.h"
#include "push-notification-event-mailboxdelete.h"
#include "push-notification-txn-mbox.h"


#define EVENT_NAME "MailboxDelete"


static void push_notification_event_mailboxdelete_debug_mbox
(struct push_notification_txn_event *event ATTR_UNUSED)
{
    i_debug("%s: Mailbox was deleted", EVENT_NAME);
}

static void push_notification_event_mailboxdelete_event(
    struct push_notification_txn *ptxn,
    struct push_notification_event_config *ec,
    struct push_notification_txn_mbox *mbox)
{
    struct push_notification_event_mailboxdelete_data *data;
    struct mailbox_status status;

    if (mailbox_get_status(ptxn->mbox, STATUS_UIDVALIDITY, &status) < 0) {
        i_error(EVENT_NAME "Failed to get created mailbox '%s' uidvalidity: %s",
                mailbox_get_vname(ptxn->mbox),
                mailbox_get_last_error(ptxn->mbox, NULL));
        status.uidvalidity = 0;
    }

    data = p_new(ptxn->pool,
                 struct push_notification_event_mailboxdelete_data, 1);
    data->uid_validity = status.uidvalidity;
    data->deleted = TRUE;

    push_notification_txn_mbox_set_eventdata(ptxn, mbox, ec, data);
}


/* Event definition */

extern struct push_notification_event push_notification_event_mailboxdelete;

struct push_notification_event push_notification_event_mailboxdelete = {
    .name = EVENT_NAME,
    .mbox = {
        .debug_mbox = push_notification_event_mailboxdelete_debug_mbox
    },
    .mbox_triggers = {
        .delete = push_notification_event_mailboxdelete_event
    }
};
