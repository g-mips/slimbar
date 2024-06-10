#ifndef INC__X_CONN_MGR_H
#define INC__X_CONN_MGR_H

#include <xcb/xcb.h>

#include <stdbool.h>

typedef struct x_connection x_connection_t;

x_connection_t   *x_conn_mgr_init(void);
void              x_conn_mgr_fini(x_connection_t **conn);
xcb_connection_t *x_conn_mgr_get_connection(x_connection_t *conn);
xcb_screen_t     *x_conn_mgr_get_screen(x_connection_t *conn);
bool              x_conn_mgr_get_window(x_connection_t *conn, xcb_window_t *win);
bool              x_conn_mgr_setup_main_window(x_connection_t *conn);

#endif // INC__X_CONN_MGR_H
