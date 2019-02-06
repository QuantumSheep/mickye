#include "tcp.h"

GuiEnv *env = NULL;
TcpClientChain *clients = NULL;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int server_socket;

TcpClient *
tcp_get_client(int id)
{
    TcpClientChain *client = clients;

    /**
     * Select the right client
     */
    while (client != NULL && client->client->socket != id)
    {
        client = client->next;
    }

    if (client == NULL)
    {
        return NULL;
    }
    else
    {
        return client->client;
    }
}

void
tcp_annihilate_socket(int socket)
{
    shutdown(socket, SHUT_RDWR);
    close(socket);
}

void
tcp_create_connection(pthread_t *thread, int socket, struct sockaddr_storage *storage)
{
    TcpClient *client;

    struct sockaddr_in *ipv4;
    struct sockaddr_in6 *ipv6;

    /**
     * Construct the client's structure
     */
    client = (TcpClient *)malloc(sizeof(TcpClient));
    client->socket = socket;

    ipv4 = (struct sockaddr_in *)storage;
    ipv6 = (struct sockaddr_in6 *)storage;

    memset(client->ipv4, 0x00, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(ipv4->sin_addr), client->ipv4, INET_ADDRSTRLEN);

    memset(client->ipv6, 0x00, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, &(ipv6->sin6_addr), client->ipv6, INET6_ADDRSTRLEN);

    /**
     * Add a client to the global clients
     */
    TcpClientChain *new = (TcpClientChain *)malloc(sizeof(TcpClientChain));
    new->client = client;
    new->next = clients;
    clients = new;

    /**
     * Add the client in the clients list
     */
    client_add(env->store, client, CLIENT_CONNECTED);
}

int
tcp_bind(int socket, struct sockaddr_in *addr)
{
    int val = 1;

    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));

    return bind(socket, (struct sockaddr *)addr, sizeof(*addr));
}

void *
tcp_init()
{
    /**
     * Server informations
     */
    struct sockaddr_in addr;
    struct sockaddr_storage storage;
    socklen_t storage_size;

    /**
     * Client socket file descriptors
     */
    int client_socket = 0;

    /**
     * Number of connected clients
     */
    int connections = 0;

    pthread_t tid[64];
    int clients[64];

    // Create the socket.
    server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    /**
     * Configure server settings
     *
     * sin_family is Internet
     * sin_port use htons to use the proper byte order
     * sin_addr is the IP address (127.0.0.1 for localhost)
     */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TCP_SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr(TCP_SERVER_HOST);

    // Set all bits of the padding field to 0
    memset(addr.sin_zero, 0x00, sizeof(addr.sin_zero));

    // Bind the address struct to the socket
    tcp_bind(server_socket, &addr);

    // Listen on the socket, with 64 max connection requests queued
    listen(server_socket, 64);

    /**
     * Listening loop
     */
    while (1)
    {
        storage_size = sizeof storage;

        client_socket = accept(server_socket, (struct sockaddr *)&storage, &storage_size);

        if (client_socket == -1)
        {
            break;
        }

        tcp_create_connection(&tid[connections], client_socket, &storage);

        clients[connections] = client_socket;

        if (++connections >= 64)
        {
            pthread_join(tid[connections - 1], NULL);
        }
    }

    /**
     * Annhilating the sockets
     */
    tcp_annihilate_socket(server_socket);
    while (--connections >= 0)
        tcp_annihilate_socket(clients[connections]);

    pthread_exit(NULL);
}

void
start_server(GtkWidget *widget, GtkBuilder *builder, GuiEnv *data)
{
    pthread_t server_thread;
    GObject *stopButton;

    if (env == NULL)
    {
        env = data;
    }

    gtk_widget_set_sensitive(widget, 0);

    stopButton = gtk_builder_get_object(builder, "stop");
    gtk_widget_set_sensitive(GTK_WIDGET(stopButton), 1);

    pthread_create(&server_thread, NULL, tcp_init, NULL);

    log_add(data->text_view, "Started", "Server");
}

void
tcp_stop()
{
    tcp_annihilate_socket(server_socket);
}

void
stop_server(GtkWidget *widget, GtkBuilder *builder, GuiEnv *data)
{
    GObject *startButton;

    gtk_widget_set_sensitive(widget, 0);

    startButton = gtk_builder_get_object(builder, "start");
    gtk_widget_set_sensitive(GTK_WIDGET(startButton), 1);

    tcp_stop();

    log_add(data->text_view, "Stopped", "Server");
}
