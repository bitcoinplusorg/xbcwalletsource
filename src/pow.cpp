// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "main.h"
#include "primitives/block.h"
#include "uint256.h"

unsigned int static GetNextWorkRequired_legacy(const CBlockIndex* pindexLast, const Consensus::Params& params)
{
    unsigned int nRetarget = 30;
    int64_t nPowTargetTimespan_legacy = params.nPowTargetSpacing * nRetarget;
    int64_t nInterval = nPowTargetTimespan_legacy / params.nPowTargetSpacing;

    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);

    // Genesis block
    if (pindexLast == nullptr)
        return bnPowLimit.GetCompact();

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

    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

 unsigned int GetNextWorkRequired_DGW(const CBlockIndex* pindexLast, bool fProofOfStake, const Consensus::Params& params)
 {
     const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);

     if(pindexLast->nHeight < 200)
         return bnPowLimit.GetCompact();

     const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
     if (pindexPrev->pprev == nullptr)
         return bnPowLimit.GetCompact(); // first block

     const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
     if (pindexPrevPrev->pprev == nullptr)
         return bnPowLimit.GetCompact(); // second block

     if (params.fPowNoRetargeting)
        return pindexLast->nBits;

     int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

     if (nActualSpacing < 0)
         nActualSpacing = params.nPowTargetSpacing;

     arith_uint256 bnNew;
     bnNew.SetCompact(pindexPrev->nBits);
     bnNew *= ((params.DifficultyAdjustmentInterval() - 1) * params.nPowTargetSpacing + nActualSpacing + nActualSpacing);
     bnNew /= ((params.DifficultyAdjustmentInterval() + 1) * params.nPowTargetSpacing);

     if (bnNew <= 0 || bnNew > bnPowLimit)
         bnNew = bnPowLimit;

     return bnNew.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, bool fProofOfStake, const Consensus::Params& params)
{
    if(pindexLast->nHeight < 200)
        return GetNextWorkRequired_legacy(pindexLast, params);

    return GetNextWorkRequired_DGW(pindexLast, fProofOfStake, params);
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
