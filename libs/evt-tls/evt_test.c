//%LICENSE////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Devchandra M. Leishangthem (dlmeetei at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE)
//
//////////////////////////////////////////////////////////////////////////
//
//%///////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <assert.h>
#include "evt_tls.h"

typedef struct test_tls_s test_tls_t;

struct test_tls_s {
    evt_tls_t *endpt;
};

//Needed for pretty printing
char *role[2] = {
    "Client"
   ,"Server"
};

struct my_data {
    char data[16*1024];
    int sz;
    int stalled;
}test_data;


int test_tls_init(evt_ctx_t *ctx, test_tls_t *tst_tls)
{
    memset( tst_tls, 0, sizeof *tst_tls);
    evt_tls_t *t = evt_ctx_get_tls(ctx);
    assert(t != NULL);
    t->data = tst_tls;
    tst_tls->endpt = t;
    return 0;
}

void cls(evt_tls_t *evt, int status)
{
    printf("[%s]: on_close_cb called ++++++++\n", role[evt_tls_get_role(evt)]);
    evt_tls_free(evt);
}

int test_tls_close(test_tls_t *t, evt_close_cb cls)
{
    return evt_tls_close(t->endpt, cls);
}

void on_client_rd( evt_tls_t *tls, char *buf, int sz)
{
    printf("[%s]: on_client_rd called ++++++++\n", role[evt_tls_get_role(tls)]);
    printf("%s", (char*)buf);

    test_tls_close((test_tls_t *)tls->data, cls);
}

void on_write(evt_tls_t *tls, int status)
{
    assert(status > 0);
    printf("[%s]: On_write called ++++++++\n", role[evt_tls_get_role(tls)]);
    evt_tls_read( tls, on_client_rd);
}

void on_server_wr(evt_tls_t *tls, int status)
{
    assert(status > 0);
    printf("[%s]: on_server_wr called ++++++++\n", role[evt_tls_get_role(tls)]);
    //test_tls_close((test_tls_t *)tls->data, cls);
}

void on_server_rd( evt_tls_t *tls, char *buf, int sz)
{
    test_tls_t *test_tls = (test_tls_t*)tls->data;
    if ( sz <= 0 ) {
        test_tls_close(test_tls, cls);
        return;
    }
    printf("[%s]: on_server_rd called ++++++++\n", role[evt_tls_get_role(tls)]);
    printf("%s", (char*)buf);
    evt_tls_write(tls, buf, sz, on_server_wr);
}


void on_connect(evt_tls_t *tls, int status)
{
    int r = 0;
    printf("[%s]: On_connect called ++++++++\n", role[evt_tls_get_role(tls)]);
    if ( status ) {
	char msg[] = "Hello from event based tls engine\n";
	int str_len = sizeof(msg);
	r =  evt_tls_write(tls, msg, str_len, on_write);
        assert( r = str_len);
    }
    else { //handle ssl_shutdown
        test_tls_close((test_tls_t*)tls, cls);
    }
}

//test net writer for the test code
int test_net_wrtr(evt_tls_t *c, void *buf, int sz)
{
    static int first_time = 1;
    if (test_data.stalled || first_time == 1) {
        memset(&test_data, 0, sizeof(test_data));
        first_time = 0;
        memcpy(test_data.data, buf, sz);
        test_data.sz = sz;
        test_data.stalled = 0;
    }
    return 0;

}

int start_nio(test_tls_t *source, test_tls_t *destination)
{
    for(;;) {
        if ( test_data.stalled ) break;
        test_data.stalled = 1;
        evt_tls_feed_data(destination->endpt, test_data.data, test_data.sz);
        test_tls_t *tmp = destination;
        destination = source;
        source = tmp;
    }
    //make compiler happy
    return 0;
}

int test_tls_connect(test_tls_t *t, evt_handshake_cb on_connect)
{
    return evt_tls_connect(t->endpt, on_connect);
}

void on_accept(evt_tls_t *svc, int status)
{
    printf("[%s]: On_accept called ++++++++\n", role[evt_tls_get_role(svc)]);
    //read data now
    if ( status > 0 ) {
        evt_tls_read(svc, on_server_rd );
    }
    else {
        test_tls_close((test_tls_t*)svc, cls);
    }
}

int test_tls_accept(test_tls_t *tls, evt_handshake_cb on_accept)
{
    return evt_tls_accept(tls->endpt, on_accept);
}

int main()
{
    evt_ctx_t tls;
    memset(&tls, 0, sizeof(tls));
    assert(0 == evt_ctx_init(&tls));


    assert(0 == evt_ctx_is_crtf_set(&tls));
    assert(0 == evt_ctx_is_key_set(&tls));
    
    if (!evt_ctx_is_crtf_set(&tls)) {
	evt_ctx_set_crt_key(&tls, "server-cert.pem", "server-key.pem");
    }

    assert( 1 == evt_ctx_is_crtf_set(&tls));
    assert( 1 == evt_ctx_is_key_set(&tls));

    assert(tls.writer == NULL);
    evt_ctx_set_writer(&tls, test_net_wrtr);
    assert(tls.writer != NULL);

    test_tls_t clnt_hdl;
    test_tls_init( &tls, &clnt_hdl);

    test_tls_t svc_hdl;
    test_tls_init( &tls, &svc_hdl);

    test_tls_connect(&clnt_hdl, on_connect);
    test_tls_accept(&svc_hdl, on_accept);

    start_nio(&clnt_hdl, &svc_hdl);

    evt_ctx_free(&tls);

    return 0;
}
