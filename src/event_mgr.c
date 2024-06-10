#include "event_mgr.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool generate_graphics_context(xcb_connection_t *connection, xcb_window_t win, xcb_screen_t *screen, char const *font_name, xcb_gcontext_t *gc);

bool event_mgr_loop(x_connection_t *conn)
{
    xcb_connection_t *xcb_conn = x_conn_mgr_get_connection(conn);
    xcb_screen_t     *screen   = x_conn_mgr_get_screen(conn);
    xcb_window_t      window   = 0;

    bool no_error = false;
    bool running = (bool)(
        (conn != NULL) && (xcb_conn != NULL) &&
        (screen != NULL) && x_conn_mgr_get_window(conn, &window));
    while (running)
    {
        no_error = true;
        xcb_generic_event_t *event = xcb_poll_for_event(xcb_conn);
        if (event)
        {
            switch (event->response_type & ~0x80)
            {
                case XCB_EXPOSE:
                    char *text = "Hello, World!";
                    char *font = "fixed";
                    xcb_gcontext_t gc = 0;
                    if (generate_graphics_context(xcb_conn, window, screen, font, &gc))
                    {
                        xcb_void_cookie_t text_cookie = xcb_image_text_8_checked(
                            xcb_conn, strlen(text), window, gc, 10, 20, text);
                        xcb_generic_error_t *error = xcb_request_check(xcb_conn, text_cookie);
                        if (error)
                        {
                            fprintf(stderr, "Can't draw text: %d\n", error->error_code);
                        }

                        xcb_void_cookie_t gc_cookie = xcb_free_gc(xcb_conn, gc);
                        error = xcb_request_check(xcb_conn, gc_cookie);
                        if (error)
                        {
                            fprintf(stderr, "Can't free the graphics context: %d\n", error->error_code);
                        }
                    }

                    //text_draw(xcb_conn, screen, window, 10, HEIGHT - 10, text);
                    break;
                default:
                    break;
            }

            free(event);
        }
    }

    return no_error;
}

static bool generate_graphics_context(xcb_connection_t *connection, xcb_window_t win, xcb_screen_t *screen, char const *font_name, xcb_gcontext_t *gc)
{
    bool gc_generated = false;
    xcb_font_t font = xcb_generate_id(connection);
    xcb_void_cookie_t font_cookie = xcb_open_font_checked(
            connection, font, strlen(font_name), font_name);

    xcb_generic_error_t *error = xcb_request_check(connection, font_cookie);
    if (error)
    {
        fprintf(stderr, "Unable to open font: %d\n", error->error_code);
    }
    else
    {
        uint32_t mask      = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
        uint32_t values[3] = { screen->black_pixel, screen->white_pixel, font };

        *gc = xcb_generate_id(connection);
        xcb_void_cookie_t gc_cookie = xcb_create_gc_checked(connection, *gc, win, mask, values);
        error = xcb_request_check(connection, gc_cookie);
        if (error)
        {
            fprintf (stderr, "Unable to create graphics context: %d\n", error->error_code);
        }
        else
        {
            gc_generated = true;
        }

        font_cookie = xcb_close_font_checked (connection, font);
        error = xcb_request_check(connection, font_cookie);
        if (error)
        {
            fprintf (stderr, "Unable to close font: %d\n", error->error_code);
        }
    }

    return gc_generated;
}
