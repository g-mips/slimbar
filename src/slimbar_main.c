#include "x_conn_mgr.h"
#include "event_mgr.h"

#include <stdlib.h>

int main(int argc, char *argv[])
{
    int             ret_code   = EXIT_FAILURE;
    x_connection_t *connection = x_conn_mgr_init();
    if (connection != NULL)
    {
        if (x_conn_mgr_setup_main_window(connection))
        {
            ret_code = event_mgr_loop(connection) ? EXIT_SUCCESS : EXIT_FAILURE;
        }

        x_conn_mgr_fini(&connection);
    }

    return ret_code;
}
