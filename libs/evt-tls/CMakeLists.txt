cmake_minimum_required(VERSION 2.8)
project(evt_tls)

set(evt_tls_sources
	evt_tls.c
)

set(evt_tls_headers
	evt_tls.h
	queue.h
)

add_library(evt_tls ${evt_tls_sources} ${evt_tls_headers})
