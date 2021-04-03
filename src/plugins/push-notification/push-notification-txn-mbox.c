/* Copyright (c) 2015-2016 Dovecot authors, see the included COPYING file */

#include "lib.h"
#include "hash.h"
#include "mail-storage-private.h"

#include "push-notification-drivers.h"
#include "push-notification-events.h"
#include "push-notification-txn-mbox.h"


struct push_notification_txn_mbox *
push_notification_txn_mbox_create(struct push_notification_txn *txn,
                                  struct mailbox *box)
{
    if (txn->mbox_txn == NULL) {
        i_debug("MBBOOOOOXX - push_notification_txn_mbox_create"); 
        
        txn->mbox_txn = p_new(txn->pool, struct push_notification_txn_mbox, 1);
        txn->mbox_txn->mailbox = mailbox_get_vname(box);
        i_debug("trangggg ne 1: %s", txn->mbox_txn->mailbox);
        // i_debug("trangggg ne 2: %s", txn->mbox_txn->mailbox); 
    }

    return txn->mbox_txn;
}

void
push_notification_txn_mbox_end(struct push_notification_txn *ptxn)
{
    struct push_notification_driver_txn **dtxn;
    i_debug("TRANG - push_notification_txn_mbox_end");

    if (ptxn->mbox_txn != NULL) {
        array_foreach_modifiable(&ptxn->drivers, dtxn) {
            if ((*dtxn)->duser->driver->v.process_mbox != NULL) {
                (*dtxn)->duser->driver->v.process_mbox(*dtxn, ptxn->mbox_txn);
            }
        }

        push_notification_txn_mbox_deinit_eventdata(ptxn->mbox_txn);
    }
}

void *
push_notification_txn_mbox_get_eventdata(struct push_notification_txn_mbox *mbox,
                                         const char *event_name)
{
    struct push_notification_txn_event **mevent;
    i_debug ("Eventttt: %s", event_name);
    if (array_is_created(&mbox->eventdata)) {
        array_foreach_modifiable(&mbox->eventdata, mevent) {
            if (strcmp((*mevent)->event->event->name, event_name) == 0) {
                i_debug ("Event_nameeee 1: %s", (*mevent)->event->event->name);
                i_debug ("Event_nameeee 1 mailbox: %s", &mbox->mailbox);
                i_debug ("Event_nameeee 1 data: %s", (*mevent)->data);
                return (*mevent)->data;
            }
        }
    }

    return NULL;
}

void
push_notification_txn_mbox_set_eventdata(struct push_notification_txn *txn,
                                         struct push_notification_txn_mbox *mbox,
                                         struct push_notification_event_config *event,
                                         void *data)
{
    struct push_notification_txn_event *mevent;

    if (!array_is_created(&mbox->eventdata)) {
        p_array_init(&mbox->eventdata, txn->pool, 4);
    }

    mevent = p_new(txn->pool, struct push_notification_txn_event, 1);
    mevent->data = data;
    mevent->event = event;

    array_append(&mbox->eventdata, &mevent, 1);
}

void
push_notification_txn_mbox_deinit_eventdata
(struct push_notification_txn_mbox *mbox)
{
    struct push_notification_txn_event **mevent;
    i_debug ("TRANG - push_notification_txn_mbox_deinit_eventdata");
    int a = array_is_created(&mbox->eventdata);
    print ("Tranggg - array_is_created: %d", a);

    if (array_is_created(&mbox->eventdata)) {
        array_foreach_modifiable(&mbox->eventdata, mevent) {
            if (((*mevent)->data != NULL) &&
                ((*mevent)->event->event->mbox.free_mbox != NULL)) {
                (*mevent)->event->event->mbox.free_mbox(*mevent);
            }
        }
    }
}
