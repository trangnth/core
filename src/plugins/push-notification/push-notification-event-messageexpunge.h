/* Copyright (c) 2015-2016 Dovecot authors, see the included COPYING file */

#ifndef PUSH_NOTIFICATION_EVENT_MESSAGEEXPUNGE_H
#define PUSH_NOTIFICATION_EVENT_MESSAGEEXPUNGE_H


struct push_notification_event_messageexpunge_data {
    /* Can only be true. */
    bool expunge;
    /* PUSH_NOTIFICATION_MESSAGE_HDR_MESSAGEID */
    const char *msgid;
};


#endif /* PUSH_NOTIFICATION_EVENT_MESSAGEEXPUNGE_H */

