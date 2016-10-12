#include "uv_tls.h"

void on_write()
{

}

void on_tls_handshake(uv_tls_t *tls, int status)
{
    uv_write_t *rq = 0;
    uv_buf_t dcrypted;
    dcrypted.base = "Hello from lib-tls";
    dcrypted.len = strlen(dcrypted.base);

    if ( 0 == status ) // TLS connection not failed
    {
	rq = (uv_write_t*)malloc(sizeof(*rq));
        uv_tls_write(tls, &dcrypted, on_write);
    }
    else {
	uv_tls_close((uv_handle_t*)tls, (uv_close_cb)free);
    }
}

void on_connect(uv_connect_t *req, int status)
{
    fprintf( stderr, "Entering tls_connect callback\n");
    if( status ) {
        fprintf( stderr, "TCP connection error\n");
        return;
    }
    fprintf( stderr, "TCP connection established\n");

    evt_ctx_t *ctx = req->data;

    uv_tls_t *sclient = malloc(sizeof(*sclient));
    if( uv_tls_init(req->handle->loop, ctx, sclient) < 0 ) {
        free(sclient);
        return;
    }
    uv_tls_connect(sclient, on_tls_handshake);
}


int main()
{
    uv_loop_t *loop = uv_default_loop();
    uv_tcp_t client;
    uv_tcp_init(loop, &client);
    int port = 8000;
 
    evt_ctx_t ctx;
    evt_ctx_init_ex(&ctx, "server-cert.pem", "server-key.pem");
    evt_ctx_set_nio(&ctx, NULL, uv_tls_writer);


    struct sockaddr_in conn_addr;
    uv_ip4_addr("127.0.0.1", port, &conn_addr);

    uv_connect_t req;
    req.data = &ctx;
    uv_tcp_connect(&req, &client, (const struct sockaddr*)&conn_addr, on_connect);

    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}
