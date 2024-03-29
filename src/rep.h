#ifndef REP_H
#define REP_H

#define REP2(S)     S;            S
#define REP4(S)     REP2(S);      REP2(S)
#define REP8(S)     REP4(S);      REP4(S)
#define REP16(S)    REP8(S);      REP8(S)
#define REP32(S)    REP16(S);     REP16(S)
#define REP64(S)    REP32(S);     REP32(S)
#define REP128(S)   REP64(S);     REP64(S)
#define REP256(S)   REP128(S);    REP128(S)
#define REP512(S)   REP256(S);    REP256(S)

#define REP3(S)     REP2(S);      S
#define REP5(S)     REP4(S);      S
#define REP6(S)     REP5(S);      S
#define REP7(S)     REP6(S);      S
#define REP9(S)     REP8(S);      S
#define REP10(S)     REP9(S);      S

#endif
