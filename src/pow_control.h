#ifndef POW_CONTROL_H
#define POW_CONTROL_H

extern bool fTestNet;

static const int P1_End = 43200; // V1 Blockchain PoW end.
static const int P2_Start = 640336; // V2 Blockchain PoW start
static const int P2_End = 10000000; // 0 coin reward PoW mining enabled for about 10 years. Incase of PoS diff drop
static const int P1_End_TestNet = 150;
static const int P2_Start_TestNet = 160;
static const int P2_End_TestNet = 10000000;


#endif // POW_CONTROL_H
