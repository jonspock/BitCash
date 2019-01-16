// Copyright (c) 2018 The Bitcash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <links.h>
#include <linksdb.h>

#include <string>
#include <boost/algorithm/string/case_conv.hpp> 

std::unique_ptr<LinksDatabase> ldatabase;
std::map<std::string, CLinksBookData> mapLinksBook;

void InitLinksDB()
{
   fs::path path=GetDataDir();
   path /= "Links";
   ldatabase=LinksDatabase::Create(path);
   DBNICKErrors nLoadWalletRet = LinksBatch(*ldatabase,"cr+").LoadWallet();
   (void)nLoadWalletRet; // to silence unused variable warning
}

bool SetLink(const std::string& strlink,const std::string& desc,const std::string& amount,int status,uint64_t timestamp)
{
    mapLinksBook[strlink].description = desc;
    mapLinksBook[strlink].amount = amount;
    mapLinksBook[strlink].status = status;
    mapLinksBook[strlink].timestamp = timestamp;

    bool valid=true;
    if (!LinksBatch(*ldatabase).WriteNameLinkDesc(strlink, desc))valid=false;
    if (!LinksBatch(*ldatabase).WriteNameLinkAmount(strlink, amount))valid=false;
    if (!LinksBatch(*ldatabase).WriteNameLinkStatus(strlink, status))valid=false;
    if (!LinksBatch(*ldatabase).WriteNameLinkTimestamp(strlink, timestamp))valid=false;

    return valid;
}

bool DeleteLink(const std::string& strlink)
{
    mapLinksBook.erase(strlink);

    bool valid=true;
    if (!LinksBatch(*ldatabase).EraseNameLink(strlink))valid=false;

    return valid;
}

