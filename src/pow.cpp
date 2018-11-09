// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "chainparams.h"
#include "primitives/block.h"
#include "uint256.h"
#include "key.h"
#include "main.h"

arith_uint256 bnProofOfStakeLimit(~arith_uint256() >> 20);
arith_uint256 bnProofOfWorkFirstBlock(~arith_uint256() >> 30);

unsigned int static GetNextWorkRequired_legacy(const CBlockIndex* pindexLast)
{
    CBigNum bnTargetLimit = CBigNum(ArithToUint256(bnProofOfStakeLimit));
    unsigned int nRetarget = 30;
    int64_t nPowTargetTimespan_legacy = Params().GetConsensus().nPowTargetSpacing * nRetarget;
    int64_t nInterval = nPowTargetTimespan_legacy / Params().GetConsensus().nPowTargetSpacing;

    // Genesis block
    if (pindexLast == NULL)
        return bnProofOfWorkFirstBlock.GetCompact();

    // Only change once per interval
    if ((pindexLast->nHeight+1) % nInterval != 0)
    {
        // Special difficulty rule for testnet:
        return pindexLast->nBits;
    }

    // This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = nInterval-1;
    if ((pindexLast->nHeight+1) != nInterval)
        blockstogoback = nInterval;

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;
    assert(pindexFirst);

    if (Params().GetConsensus().fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    if (nActualTimespan < Params().GetConsensus().nPowTargetTimespan/4)
        nActualTimespan = Params().GetConsensus().nPowTargetTimespan/4;
    if (nActualTimespan > Params().GetConsensus().nPowTargetTimespan*4)
        nActualTimespan = Params().GetConsensus().nPowTargetTimespan*4;

    // Retarget
    CBigNum bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= Params().GetConsensus().nPowTargetTimespan;

    if (bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;

    return bnNew.GetCompact();
}

 unsigned int GetNextWorkRequired_DGW(const CBlockIndex* pindexLast, bool fProofOfStake)
 {
     CBigNum bnTargetLimit = CBigNum(ArithToUint256(bnProofOfStakeLimit));

     if (pindexLast == NULL)
         return bnTargetLimit.GetCompact(); // genesis block

     const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
     if (pindexPrev->pprev == NULL)
         return bnTargetLimit.GetCompact(); // first block
     const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
     if (pindexPrevPrev->pprev == NULL)
         return bnTargetLimit.GetCompact(); // second block

     if (Params().GetConsensus().fPowNoRetargeting)
        return pindexLast->nBits;

     int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

     if (nActualSpacing < 0)
         nActualSpacing = Params().GetConsensus().nPowTargetSpacing;

     // target change every block
     // retarget with exponential moving toward target spacing
     // Includes fix for wrong retargeting difficulty by Mammix2

     CBigNum bnNew;
     bnNew.SetCompact(pindexPrev->nBits);
     CBigNum nInterval = (Params().GetConsensus().nPowTargetTimespan) / (Params().GetConsensus().nPowTargetSpacing);
     bnNew *= ((nInterval - 1) * Params().GetConsensus().nPowTargetSpacing + nActualSpacing + nActualSpacing);
     bnNew /= ((nInterval + 1) * Params().GetConsensus().nPowTargetSpacing);

     if (bnNew <= 0 || bnNew > bnTargetLimit)
         bnNew = bnTargetLimit;

     return bnNew.GetCompact();
 }

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    if(pindexLast->nHeight + 1 > 200)
        return GetNextWorkRequired_DGW(pindexLast, fProofOfStake);
    else
        return GetNextWorkRequired_legacy(pindexLast);
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
