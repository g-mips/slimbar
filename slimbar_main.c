#include <xcb/xcb.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

static xcb_screen_t *get_screen(xcb_connection_t *connection);
static bool setup_main_window(xcb_connection_t *connection, xcb_window_t *win, xcb_screen_t *screen);
static void event_loop(xcb_connection_t *connection, xcb_screen_t *screen, xcb_window_t win);
static bool generate_graphics_context(xcb_connection_t *connection, xcb_window_t win, xcb_screen_t *screen, char const *font_name, xcb_gcontext_t *gc);

int main(int argc, char *argv[])
{
    xcb_connection_t *connection = xcb_connect(NULL, NULL);

    xcb_screen_t *screen = get_screen(connection);

    xcb_window_t win = 0;

    if (setup_main_window(connection, &win, screen))
    {
        xcb_flush(connection);
        event_loop(connection, screen, win);
    }

    xcb_disconnect(connection);

    return EXIT_SUCCESS;
}

static xcb_screen_t *get_screen(xcb_connection_t *connection)
{
    const xcb_setup_t     *setup      = xcb_get_setup(connection);
    xcb_screen_iterator_t  iter       = xcb_setup_roots_iterator(setup);

    return iter.data;
}

static bool setup_main_window(xcb_connection_t *connection, xcb_window_t *win, xcb_screen_t *screen)
{
    bool         win_setup = false;

    *win = xcb_generate_id(connection);

    uint32_t mask      = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2] = { screen->white_pixel, XCB_EVENT_MASK_EXPOSURE };

    xcb_void_cookie_t      win_cookie = xcb_create_window_checked(
        connection, screen->root_depth, *win, screen->root, 0, 0,
        screen->width_in_pixels, 100, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual, mask, values);

    xcb_void_cookie_t map_cookie = xcb_map_window_checked(connection, *win);

    xcb_generic_error_t *error = xcb_request_check(connection, win_cookie);
    if (error)
    {
        fprintf(stderr, "Could not create the main window: %d\n", error->error_code);
    }
    else
    {
        error = xcb_request_check(connection, map_cookie);
        if (error)
        {
            fprintf(stderr, "Could not map the main window: %d\n", error->error_code);
        }
        else
        {
            win_setup = true;
        }
    }

    return win_setup;
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

static void event_loop(xcb_connection_t *connection, xcb_screen_t *screen, xcb_window_t win)
{
    while (1)
    {
        xcb_generic_event_t *event = xcb_poll_for_event(connection);
        if (event)
        {
            switch (event->response_type & ~0x80)
            {
                case XCB_EXPOSE:
                    char *text = "Hello, World!";
                    char *font = "fixed";
                    xcb_gcontext_t gc = 0;
                    if (generate_graphics_context(connection, win, screen, font, &gc))
                    {
                        xcb_void_cookie_t text_cookie = xcb_image_text_8_checked(
                            connection, strlen(text), win, gc, 10, 20, text);
                        xcb_generic_error_t *error = xcb_request_check(connection, text_cookie);
                        if (error)
                        {
                            fprintf(stderr, "Can't draw text: %d\n", error->error_code);
                        }

                        xcb_void_cookie_t gc_cookie = xcb_free_gc(connection, gc);
                        error = xcb_request_check(connection, gc_cookie);
                        if (error)
                        {
                            fprintf(stderr, "Can't free the graphics context: %d\n", error->error_code);
                        }
                    }

                    //text_draw(connection, screen, window, 10, HEIGHT - 10, text);
                    break;
                default:
                    break;
            }

            free(event);
        }
    }
}
