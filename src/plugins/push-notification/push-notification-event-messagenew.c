/* Copyright (c) 2015-2016 Dovecot authors, see the included COPYING file */

#include "lib.h"
#include "array.h"
#include "hash.h"
#include "iso8601-date.h"
#include "istream.h"
#include "mail-storage.h"

#include <time.h>

#include "push-notification-drivers.h"
#include "push-notification-events.h"
#include "push-notification-event-message-common.h"
#include "push-notification-event-messagenew.h"
#include "push-notification-txn-msg.h"


#define EVENT_NAME "MessageNew"

static struct push_notification_event_messagenew_config default_config;


static void *push_notification_event_messagenew_default_config(void)
{
    memset(&default_config, 0, sizeof(default_config));

    return &default_config;
}

static void push_notification_event_messagenew_debug_msg
(struct push_notification_txn_event *event)
{
    struct push_notification_event_messagenew_data *data = event->data;
    struct tm *tm;

    if (data->date != -1) {
        tm = gmtime(&data->date);
        i_debug("%s: Date [%s]", EVENT_NAME,
                iso8601_date_create_tm(tm, data->date_tz));
    }

    if (data->from != NULL) {
        i_debug("%s: From [%s]", EVENT_NAME, data->from);
    }

    if (data->snippet != NULL) {
        i_debug("%s: Snippet [%s]", EVENT_NAME, data->snippet);
    }

    if (data->subject != NULL) {
        i_debug("%s: Subject [%s]", EVENT_NAME, data->subject);
    }

    if (data->to != NULL) {
        i_debug("%s: To [%s]", EVENT_NAME, data->to);
    }
}

static void
push_notification_event_messagenew_event(struct push_notification_txn *ptxn,
                                         struct push_notification_event_config *ec,
                                         struct push_notification_txn_msg *msg,
                                         struct mail *mail)
{
    i_debug ("0000000 - push_notification_event_messagenew_event");
    struct push_notification_event_messagenew_config *config =
        (struct push_notification_event_messagenew_config *)ec->config;
    struct push_notification_event_messagenew_data *data;
    time_t date;
    int tz;
    const char *value;

    if (!config->flags) {
        return;
    }

    data = push_notification_txn_msg_get_eventdata(msg, EVENT_NAME);
    if (data == NULL) {
        data = p_new(ptxn->pool,
                     struct push_notification_event_messagenew_data, 1);
        data->date = -1;

        push_notification_txn_msg_set_eventdata(ptxn, msg, ec, data);
    }

    if ((data->msgid == NULL) && 
        (config->flags & PUSH_NOTIFICATION_MESSAGE_HDR_MSGID) &&
        (mail_get_first_header(mail, "To", &value) >= 0)) {
        data->msgid = p_strdup(ptxn->pool, value);
    }

    i_debug ("TRAAAAAAAAA - data->msgid: %c", data->msgid);
    mail_get_first_header(mail, "Message-ID", &value);
    i_debug ("TRAAAAAAAAA - value: %c", value);
    data->msgid = p_strdup(ptxn->pool, value);
    i_debug ("TRAAAAAAAAA - data->msgid 222222: %c", data->msgid);

    if ((data->to == NULL) &&
        (config->flags & PUSH_NOTIFICATION_MESSAGE_HDR_TO) &&
        (mail_get_first_header(mail, "To", &value) >= 0)) {
        data->to = p_strdup(ptxn->pool, value);
    }

    if ((data->from == NULL) &&
        (config->flags & PUSH_NOTIFICATION_MESSAGE_HDR_FROM) &&
        (mail_get_first_header(mail, "From", &value) >= 0)) {
        data->from = p_strdup(ptxn->pool, value);
    }

    if ((data->subject == NULL) &&
        (config->flags & PUSH_NOTIFICATION_MESSAGE_HDR_SUBJECT) &&
        (mail_get_first_header(mail, "Subject", &value) >= 0)) {
        data->subject = p_strdup(ptxn->pool, value);
    }

    if ((data->date == -1) &&
        (config->flags & PUSH_NOTIFICATION_MESSAGE_HDR_DATE) &&
        (mail_get_date(mail, &date, &tz) >= 0)) {
        data->date = date;
        data->date_tz = tz;
    }

    if ((data->snippet == NULL) &&
        (config->flags & PUSH_NOTIFICATION_MESSAGE_BODY_SNIPPET) &&
        (mail_get_special(mail, MAIL_FETCH_BODY_SNIPPET, &value) >= 0)) {
        /* [0] contains the snippet algorithm, skip over it */
        i_assert(value[0] != '\0');
        data->snippet = p_strdup(ptxn->pool, value + 1);
    }
}


/* Event definition */

extern struct push_notification_event push_notification_event_messagenew;

struct push_notification_event push_notification_event_messagenew = {
    .name = EVENT_NAME,
    .init = {
        .default_config = push_notification_event_messagenew_default_config
    },
    .msg = {
        .debug_msg = push_notification_event_messagenew_debug_msg
    },
    .msg_triggers = {
        .save = push_notification_event_messagenew_event
    }
};
