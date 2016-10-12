# evt-tls
evt-tls is an abstraction layer of OpenSSL using bio pair to expose callback based asynchronous API and should integrate easily with any event based networking library like libuv, libevent and libev or any other network library which want to
use OpenSSL as an state machine.

The evt-tls will evaluate and try to support other TLS library like libtls, mbedtls etc.

`Keep Watching for More Actions on This Space`

# How the evt-tls work
evt-tls uses the BIO-pair from OpenSSL, which is the suggested way, for using TLS engine for handling network I/O(nio) independently. Hence, user is responsible for nio and feed TLS engine with whatever data we receive from network. Evt will unwrap the data and give you application data via a callback. It also wraps data and write to network.

# How to work with evt-tls
Sample integrations and usage can be found in `sample/libuv-tls` for integration with libuv. `Integrations with other libraries are most welcome for contributions`. Sample usage can also be seen at `evt_test.c`. These are the sources of
tutorials until a better document comes. `If anybody want to contribute doc, Most welcome.`
```C
#include "uv_tls.h"

void on_write(uv_tls_t *tls, int status) {
    uv_tls_close((uv_handle_t*)&tls->skt, (uv_close_cb)free);
}
void uv_rd_cb( uv_stream_t *strm, ssize_t nrd, const uv_buf_t *bfr) {
    if ( nrd <= 0 ) return;
    uv_tls_write((uv_tls_t*)strm, (uv_buf_t*)bfr, on_write);
}
void on_uv_handshake(uv_tls_t *ut, int status) {
    if ( 0 == status )
        uv_tls_read((uv_stream_t*)ut, NULL, uv_rd_cb);
    else
        uv_tls_close((uv_handle_t*)ut, (uv_close_cb)free);
}
void on_connect_cb(uv_stream_t *server, int status) {
    if( status ) return;
    uv_tls_t *sclient = malloc(sizeof(*sclient)); //freed on uv_close callback
    if( uv_tls_init(server->loop, (evt_ctx_t*)server->data, sclient) < 0 ) {
        free(sclient);
        return;
    }
    if (!uv_accept(server, (uv_stream_t*)&(sclient->skt))) {
        uv_tls_accept(sclient, on_uv_handshake);
    }
}
int main() {
    uv_loop_t *loop = uv_default_loop();
    int port = 8000, r = 0;
    evt_ctx_t ctx;
    struct sockaddr_in bind_addr;

    evt_ctx_init_ex(&ctx, "server-cert.pem", "server-key.pem");
    evt_ctx_set_nio(&ctx, NULL, uv_tls_writer);
    uv_tcp_t listener;
    uv_tcp_init(loop, &listener);
    listener.data = &ctx;

    uv_ip4_addr("0.0.0.0", port, &bind_addr);
    if ((r = uv_tcp_bind(&listener, (struct sockaddr*)&bind_addr, 0)))
        fprintf( stderr, "bind: %s\n", uv_strerror(r));
    if ((r = uv_listen((uv_stream_t*)&listener, 128, on_connect_cb)))
        fprintf( stderr, "listen: %s\n", uv_strerror(r));
    printf("Listening on %d\n", port);
    uv_run(loop, UV_RUN_DEFAULT);
    return 0;
}
```
# BUILD AND TEST
To join the actions, download the code and to build and test

`make`
