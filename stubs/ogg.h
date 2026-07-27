/* ogg.h - minimal stub for builds without libogg */
#ifndef _ogg_h_
#define _ogg_h_
#include <stdint.h>
typedef struct {
    unsigned char *packet;
    long bytes;
    long b_o_s;
    long e_o_s;
    int64_t granulepos;
    int64_t packetno;
} ogg_packet;
typedef struct {
    unsigned char *header;
    unsigned char *body;
    long header_len;
    long body_len;
} ogg_page;
typedef struct {
    void *internal_state;
} ogg_stream_state;
typedef struct {
    unsigned char *data;
    long storage;
    long fill;
    int returned;
    int unsynced;
    int header_fill;
    int e_o_s;
    int b_o_s;
    int packetno;
    int64_t granulepos;
} ogg_sync_state;
#endif
