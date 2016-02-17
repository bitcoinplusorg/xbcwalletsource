#ifndef POW_CONTROL_H
#define POW_CONTROL_H

extern bool fTestNet;

static const int RP1_End = 60000; // Release version PoW end
static const int P1_Start = 100602; // Phase 1 PoW start
static const int P2_Start = 114602; // Phase 2 PoW start
static const int P2_End = 400000; // Phase 2 PoW end. 0 Coin reward PoW mining enabled. Incase of PoS diff drop


static const int RP1_End_TestNet = 150;
static const int P1_Start_TestNet = 160;
static const int P2_Start_TestNet = 200; // Phase 2 PoW start
static const int P2_End_TestNet = 300; // Phase 2 PoW end. 0 Coin reward PoW mining enabled. Incase of PoS diff drop


#endif // POW_CONTROL_H
