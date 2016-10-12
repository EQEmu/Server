//%LICENSE////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Devchandra M. Leishangthem (dlmeetei at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE)
//
//////////////////////////////////////////////////////////////////////////
//
//%///////////////////////////////////////////////////////////////////////////

#include "uv_tls.h"
#include <assert.h>

static void alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf)
{
    buf->base = (char*)malloc(size);
    memset(buf->base, 0, size);
    buf->len = size;
    assert(buf->base != NULL && "Memory allocation failed");
}

int uv_tls_writer(evt_tls_t *t, void *bfr, int sz) {
    int rv = 0;
    uv_buf_t b;
    b.base = bfr;
    b.len = sz;
    uv_tls_t *uvt = t->data;
    if(uv_is_writable((uv_stream_t*)&(uvt->skt)) ) {
        rv = uv_try_write((uv_stream_t*)&(uvt->skt), &b, 1);
    }
    return rv;
}

int uv_tls_init(uv_loop_t *loop, evt_ctx_t *ctx, uv_tls_t *endpt)
{
    int r = 0;
    memset( endpt, 0, sizeof *endpt);

    r = uv_tcp_init(loop, &(endpt->skt));

    evt_tls_t *t = evt_ctx_get_tls(ctx);
    assert( t != NULL );

    t->data = endpt;

    endpt->tls = t;
    endpt->tls_rd_cb = NULL;
    endpt->tls_cls_cb = NULL;
    endpt->tls_hsk_cb = NULL;
    endpt->tls_wr_cb = NULL;
    return r;
}

void on_tcp_read(uv_stream_t *stream, ssize_t nrd, const uv_buf_t *data)
{
    uv_tls_t *parent = CONTAINER_OF(stream, uv_tls_t, skt);
    assert( parent != NULL);
    if ( nrd <= 0 ) {
        if( nrd == UV_EOF) {
            uv_tls_close((uv_handle_t*)stream, (uv_close_cb)free);
        }
        free(data->base);
        return;
    }

    static int is_tls = 0;
    static int is_first = 1;
    if (is_first) {
        if (is_tls_stream(data->base, nrd)) {
            evt_tls_feed_data(parent->tls, data->base, nrd);
            is_tls = 1;
        }
        is_first = 0;
    }
    else {
        if(is_tls) {
            evt_tls_feed_data(parent->tls, data->base, nrd);
        }
    }
    free(data->base);
}

static void on_hd_complete( evt_tls_t *t, int status)
{
    uv_tls_t *ut = (uv_tls_t*)t->data;
    assert( ut != NULL && ut->tls_hsk_cb != NULL);
    ut->tls_hsk_cb(ut, status -1);
}


int uv_tls_accept(uv_tls_t *t, uv_handshake_cb cb)
{
    int rv = 0;
    assert( t != NULL);
    t->tls_hsk_cb = cb;
    evt_tls_t *tls = t->tls;
    rv = evt_tls_accept(tls, on_hd_complete);
    uv_read_start((uv_stream_t*)&(t->skt), alloc_cb, on_tcp_read);
    return rv;
}

static void evt_on_rd(evt_tls_t *t, char *bfr, int sz)
{
    uv_buf_t data;
    uv_tls_t *tls = (uv_tls_t*)t->data;

    data.base = bfr;
    data.len = sz;

    assert(tls->tls_rd_cb != NULL);
    tls->tls_rd_cb((uv_stream_t*)tls, sz, &data);
}

void on_close(evt_tls_t *tls, int status)
{
    assert(1 == status);
    uv_tls_t *ut = (uv_tls_t*)tls->data;
    assert( ut->tls_cls_cb != NULL);

    evt_tls_free(tls);
    if ( !uv_is_closing((uv_handle_t*)&(ut->skt)))
        uv_close( (uv_handle_t*)&(ut->skt), ut->tls_cls_cb);
}

int uv_tls_close(uv_handle_t *strm,  uv_close_cb cb)
{
    uv_tls_t *t = (uv_tls_t*)strm;
    t->tls_cls_cb = cb;

    return evt_tls_close(t->tls, on_close);
}

//uv_alloc_cb is unused. This is here to mimick API similarity with libuv
// XXX remove?
int uv_tls_read(uv_stream_t *tls, uv_alloc_cb alloc_cb, uv_read_cb cb)
{
    uv_tls_t *ptr = (uv_tls_t*)tls;
    ptr->tls_rd_cb = cb;
    return evt_tls_read(ptr->tls, evt_on_rd);
}

static void on_hshake(evt_tls_t *etls, int status)
{
    assert(etls != NULL);
    uv_tls_t *ut = (uv_tls_t*)etls->data;
    assert(ut != NULL && ut->tls_hsk_cb != NULL);
    ut->tls_hsk_cb(ut, status - 1);
}

int uv_tls_connect(uv_tls_t *t, uv_handshake_cb cb)
{
    assert( t != NULL);
    t->tls_hsk_cb = cb;
    evt_tls_t *evt = t->tls;
    assert( evt != NULL);

    evt_tls_connect(evt, on_hshake);
    return uv_read_start((uv_stream_t*)&(t->skt), alloc_cb, on_tcp_read);
}

void on_evt_write(evt_tls_t *tls, int status) {
    assert( tls != NULL);
    uv_tls_t *ut = (uv_tls_t*)tls->data;
    assert( ut != NULL && ut->tls_wr_cb != NULL);
    ut->tls_wr_cb(ut, status);
}

int uv_tls_write(uv_tls_t *stream, uv_buf_t *buf, uv_tls_write_cb cb)
{
    assert( stream != NULL);
    stream->tls_wr_cb = cb;
    evt_tls_t *evt = stream->tls;
    assert( evt != NULL);

    return evt_tls_write(evt, buf->base, buf->len, on_evt_write);
}
