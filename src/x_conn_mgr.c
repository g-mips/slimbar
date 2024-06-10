#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>

#include "x_conn_mgr.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

struct x_connection {
    Display          *xlib_conn;
    xcb_connection_t *xcb_conn;
    xcb_screen_t     *screen;
    xcb_window_t      status_bar_id;
    bool              has_valid_status_bar_id;
};

x_connection_t *x_conn_mgr_init(void)
{
    x_connection_t *conn = calloc(1, sizeof(struct x_connection));
    if (conn != NULL)
    {
        conn->xlib_conn = XOpenDisplay(NULL);
        if (conn->xlib_conn == NULL)
        {
            fprintf(stderr, "Could not open a connection to the X display.\n");
        }
        else
        {
            conn->xcb_conn = XGetXCBConnection(conn->xlib_conn);
            if (conn->xcb_conn == NULL)
            {
                fprintf(stderr, "Could not setup the XCB connection.\n");
                x_conn_mgr_fini(&conn);
            }
        }
    }

    return conn;
}

void x_conn_mgr_fini(x_connection_t **conn)
{
    if ((conn != NULL) && (*conn != NULL))
    {
        if ((*conn)->xlib_conn != NULL)
        {
            XCloseDisplay((*conn)->xlib_conn);
            (*conn)->xlib_conn = NULL;
            (*conn)->xcb_conn = NULL;
            free(*conn);
            *conn = NULL;
        }
    }
}

xcb_connection_t *x_conn_mgr_get_connection(x_connection_t *conn)
{
    xcb_connection_t *xcb_conn = NULL;
    if (conn != NULL)
    {
        xcb_conn = conn->xcb_conn;
    }

    return xcb_conn;
}

xcb_screen_t *x_conn_mgr_get_screen(x_connection_t *conn)
{
    xcb_screen_t *screen = NULL;
    if (conn != NULL)
    {
        if (conn->screen != NULL)
        {
            screen = conn->screen;
        }
        else if (conn->xcb_conn != NULL)
        {
            const xcb_setup_t *setup = xcb_get_setup(conn->xcb_conn);
            if (setup != NULL)
            {
                xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
                screen = iter.data;
            }
        }
    }

    return screen;
}

bool x_conn_mgr_get_window(x_connection_t *conn, xcb_window_t *win)
{
    bool set_window = false;
    if (conn != NULL)
    {
        if (!conn->has_valid_status_bar_id)
        {
            conn->status_bar_id = xcb_generate_id(conn->xcb_conn);
            conn->has_valid_status_bar_id = true;
        }

        *win = conn->status_bar_id;
        set_window = conn->has_valid_status_bar_id;
    }

    return set_window;
}

bool x_conn_mgr_setup_main_window(x_connection_t *conn)
{
    bool win_setup = false;

    if (conn != NULL)
    {
        xcb_screen_t *screen = x_conn_mgr_get_screen(conn);
        xcb_window_t window = 0;
        if ((screen != NULL) && x_conn_mgr_get_window(conn, &window))
        {
            uint32_t mask      = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
            uint32_t values[2] = { screen->white_pixel, XCB_EVENT_MASK_EXPOSURE };

            xcb_void_cookie_t win_cookie = xcb_create_window_checked(
                conn->xcb_conn, screen->root_depth, window, screen->root, 0, 0,
                screen->width_in_pixels, 100, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                screen->root_visual, mask, values);

            xcb_void_cookie_t map_cookie = xcb_map_window_checked(conn->xcb_conn, window);

            xcb_generic_error_t *error = xcb_request_check(conn->xcb_conn, win_cookie);
            if (error)
            {
                fprintf(stderr, "Could not create the main window: %d\n", error->error_code);
            }
            else
            {
                error = xcb_request_check(conn->xcb_conn, map_cookie);
                if (error)
                {
                    fprintf(stderr, "Could not map the main window: %d\n", error->error_code);
                }
                else
                {
                    win_setup = true;
                    xcb_flush(conn->xcb_conn);
                }
            }
        }
    }

    return win_setup;
}
