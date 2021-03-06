#include "loader.h"

/*
*   Add a column to a GtkTreeView (required as a GtkWidget)
*   
*   @param      tree            a GtkTreeView casted as GtkWidget
*   @param      name            the name of the new column
*   @param      position        the position of the column
*/
void
add_column(GtkWidget *tree, char *name, int position)
{
    /**
     * Creating a renderer and insert a new collumn with set attributes in the GtkTreeView
     */
    GtkCellRenderer *renderer;
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, name, renderer, "text", position, NULL);
}

/*
*   Load the client list in the main tree_view
*   
*   @param      builder         the main builder, who construct the elements
*   @param      store           the tree_view model (for a lcient in the list)
*   @param      client_tree     the main client_tree of the client list
*/
void
load_clients_list(GtkBuilder *builder, GtkListStore *store, GtkWidget *client_tree)
{
    GObject *client_scroll;

    // Get the scrollbar container for clients set in the builder.ui
    client_scroll = gtk_builder_get_object(builder, "clients");

    // Add all the columns of the client_tree
    add_column(client_tree, "NAME", COL_NAME);
    add_column(client_tree, "ID", COL_SOCKET);
    add_column(client_tree, "IP", COL_IPV4);
    add_column(client_tree, "STATUS", COL_STATUS);

    // Set the model of the client tree_view
    gtk_tree_view_set_model(GTK_TREE_VIEW(client_tree), GTK_TREE_MODEL(store));

    // Add the client tree_view to the scroll container
    gtk_container_add(GTK_CONTAINER(client_scroll), client_tree);
}

/*
*   Load VTE terminal to the main window
*   
*   @param      builder         the builder object
*/
void
load_terminal(GtkBuilder *builder)
{
    gchar **envp, **command;
    GObject *terminal;

    // Get the environment of the user, and set the command to a shell
    envp = g_get_environ();
    command = (gchar *[]){g_strdup(g_environ_getenv(envp, "SHELL")), NULL};
    g_strfreev(envp);

    // Get the terminal object from the builder.ui file
    terminal = gtk_builder_get_object(builder, "terminal");

    // Check the version of VTE and start a deprecated function or not
#if VTE_CHECK_VERSION(0, 48, 0)
    vte_terminal_spawn_async(VTE_TERMINAL(terminal), VTE_PTY_DEFAULT,
                             NULL,       /* working directory  */
                             command,    /* command */
                             NULL,       /* environment */
                             0,          /* spawn flags */
                             NULL, NULL, /* child setup */
                             NULL,       /* child pid */
                             30 * 1000,  /* time before timeout */
                             NULL,
                             NULL, NULL);
#else
    vte_terminal_spawn_sync(VTE_TERMINAL(terminal), VTE_PTY_DEFAULT,
                            NULL,       /* working directory  */
                            command,    /* command */
                            NULL,       /* environment */
                            0,          /* spawn flags */
                            NULL, NULL, /* child setup */
                            NULL,       /* child pid */
                            NULL, NULL);
#endif
}

/*
*   Load the logs part in the main window
*   
*   @param      builder         the builder object
*   @param      text_view       the GtkTextView who will receive the logs
*/
void
load_logs(GtkBuilder *builder, GtkTextView *text_view)
{
    GObject *text_view_scroll;

    // Get the scrollbar container from the builder.ui
    text_view_scroll = gtk_builder_get_object(builder, "logs");

    // Add the log text_view to the scrollbar container
    gtk_container_add(GTK_CONTAINER(text_view_scroll), GTK_WIDGET(text_view));

    // Set the text_view to not be editable by the user
    gtk_text_view_set_editable(text_view, 0);
}

/*
*   Load the main interface components
*   
*   @param      builder         the builder object
*   @param      text_view       the GtkTextView who will receive the logs
*   @param      store           the GtkTreeView model, of the client list
*   @param      client_tree     the GtkTreeView of the client list (cast as a GtkWidget)
*/
void
load_interface(GtkBuilder *builder, GtkTextView *text_view, GtkListStore *store, GtkWidget *client_tree)
{
    load_clients_list(builder, store, client_tree);
    load_terminal(builder);
    load_logs(builder, text_view);

    log_add(text_view, "Finished loading", "interface");
}