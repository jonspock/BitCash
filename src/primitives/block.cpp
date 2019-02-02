// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>
#include <utilstrencodings.h>
#include <crypto/common.h>

uint256 CBlockHeader::GetHash() const
{
    const bool x16ractive = (this->nVersion & ((uint32_t)1) << 3) != 0;
    if ( x16ractive) {
        return HashX16R(BEGIN(nVersion), END(nNonce), hashPrevBlock);
    } else {
        return SerializeHash(*this);    
    }
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, nEdgeBits=%d, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        nEdgeBits,
        vtx.size());

    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }

    return s.str();
}
