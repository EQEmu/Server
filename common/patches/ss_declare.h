

#define E(x) static void Encode_##x(EQApplicationPacket **p, EQStream *dest, bool ack_req);
#define D(x) static void Decode_##x(EQApplicationPacket *p);


