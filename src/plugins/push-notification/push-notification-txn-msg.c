/* Copyright (c) 2015-2016 Dovecot authors, see the included COPYING file */

#include "lib.h"
#include "hash.h"
#include "mail-storage-private.h"

#include "push-notification-drivers.h"
#include "push-notification-events.h"
#include "push-notification-txn-msg.h"


struct push_notification_txn_msg *
push_notification_txn_msg_create(struct push_notification_txn *txn,
                                 struct mail *mail)
{
    struct push_notification_txn_msg *msg = NULL;
    i_debug ("EEEEEEEEE - push_notification_txn_msg_create");

    if (hash_table_is_created(txn->messages)) {
        msg = hash_table_lookup(txn->messages,
                                POINTER_CAST(txn->t->save_count + 1));
    } else {
        hash_table_create_direct(&txn->messages, txn->pool, 4);
    }

    if (msg == NULL) {
    // if (TRUE) {
        msg = p_new(txn->pool, struct push_notification_txn_msg, 1);
        msg->mailbox = mailbox_get_vname(mail->box);
    }

    /* Save sequence number - used to determine UID later. */
    msg->seq = txn->t->save_count;
    msg->uid = mail->uid;
    i_debug("EEEE - uid = %d, seq = %d", msg->uid, msg->seq);
    struct push_notification_txn_msg_uid *uid, *u;
    uid->uid = msg->uid;
    uid->seq = msg->seq;
    array_append(&msg->uids, &uid, 1);
    
    // uint32_t u;
    array_foreach(&msg->uids, u){
        i_debug ("UUU 22:  %d", u->uid);
    }

    hash_table_insert(txn->messages, POINTER_CAST(txn->t->save_count + 1),
                        msg);
    

    return msg;
}

void
push_notification_txn_msg_end(struct push_notification_txn *ptxn,
                              struct mail_transaction_commit_changes *changes)
{
    struct hash_iterate_context *hiter;
    void *key;
    struct push_notification_driver_txn **dtxn;
    struct seq_range_iter siter;
    struct mailbox_status status;
    uint32_t uid;
    struct push_notification_txn_msg *value;

    if (!hash_table_is_created(ptxn->messages)) {
        return;
    }

    hiter = hash_table_iterate_init(ptxn->messages);
    seq_range_array_iter_init(&siter, &changes->saved_uids);
    // array_foreach(&ptxn->events,event){
    //     i_debug ("EVENTTTT : %s", event->event->name);
    // }

    i_debug ("JJJJJJJJJJJJ - push_notification_txn_msg_end");
    // /* uid_validity is only set in changes if message is new. */
    // if (changes->uid_validity == 0) {
    //     mailbox_get_open_status(ptxn->mbox, STATUS_UIDVALIDITY, &status);
    //     uid_validity = status.uidvalidity;
    // } else {
    //     uid_validity = changes->uid_validity;
    // }


    while (hash_table_iterate(hiter, ptxn->messages, &key, &value)) {
        if (value->uid == 0) {
            if (seq_range_array_iter_nth(&siter, value->seq, &uid)) {
                value->uid = uid;
            }
        }
        i_debug ("JJJJJJJJJJJJ - value->uid = %d, uid = %d", value->uid, uid);

        /* uid_validity is only set in changes if message is new. */
        if (changes->uid_validity == 0) {
            mailbox_get_open_status(ptxn->mbox, STATUS_UIDVALIDITY, &status);
            value->uid_validity = status.uidvalidity;
        } else {
            value->uid_validity = changes->uid_validity;
        }

        array_foreach_modifiable(&ptxn->drivers, dtxn) {
            if ((*dtxn)->duser->driver->v.process_msg != NULL) {
                (*dtxn)->duser->driver->v.process_msg(*dtxn, value);
            }
        }

        push_notification_txn_msg_deinit_eventdata(value);
    }

    hash_table_iterate_deinit(&hiter);
    hash_table_destroy(&ptxn->messages);
}

void *
push_notification_txn_msg_get_eventdata(struct push_notification_txn_msg *msg,
                                        const char *event_name)
{
    struct push_notification_txn_event **mevent;

    i_debug ("TOWOWOW - push_notification_txn_msg_get_eventdata - event_name: %s, uid: %d", event_name, msg->uid);
    if (array_is_created(&msg->eventdata)) {
        array_foreach_modifiable(&msg->eventdata, mevent) {
            if (strcmp((*mevent)->event->event->name, event_name) == 0) {
                return (*mevent)->data;
            }
        }
    }

    return NULL;
}

void
push_notification_txn_msg_set_eventdata(struct push_notification_txn *txn,
                                        struct push_notification_txn_msg *msg,
                                        struct push_notification_event_config *event,
                                        void *data)
{
    i_debug ("WWWWWWWWWW - push_notification_txn_msg_set_eventdata");
    struct push_notification_txn_event *mevent;

    if (!array_is_created(&msg->eventdata)) {
        i_debug ("WWWWWWWWWW - push_notification_txn_msg_set_eventdata - array_isn't_created");
        p_array_init(&msg->eventdata, txn->pool, 4);
    }

    mevent = p_new(txn->pool, struct push_notification_txn_event, 1);
    mevent->data = data;
    mevent->event = event;

    array_append(&msg->eventdata, &mevent, 1);
}

void
push_notification_txn_msg_deinit_eventdata(struct push_notification_txn_msg *msg)
{
    struct push_notification_txn_event **mevent;

    if (array_is_created(&msg->eventdata)) {
        array_foreach_modifiable(&msg->eventdata, mevent) {
            if (((*mevent)->data != NULL) &&
                ((*mevent)->event->event->msg.free_msg != NULL)) {
                (*mevent)->event->event->msg.free_msg(*mevent);
            }
        }
    }
}
