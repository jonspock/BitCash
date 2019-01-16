// Copyright (c) 2018 The Bitcash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <nicknames.h>
#include <nicknamedb.h>

#include <string>
#include <boost/algorithm/string/case_conv.hpp> 

std::unique_ptr<NicknameDatabase> database;
std::map<std::string, CNicknameBookData> mapNicknameBook;
std::map<CPubKey, CNicknameAddrBookData> mapAddressForNicknameBook;

void InitNicknameDB()
{
   fs::path path=GetDataDir();
   path /= "nicknames";
   database=NicknameDatabase::Create(path);
   DBNICKErrors nLoadWalletRet = NicknameBatch(*database,"cr+").LoadWallet();
   (void)nLoadWalletRet; // to silence unused variable warning
}

bool SetNickname(const std::string& strName,const CPubKey address, uint256 hash, bool storeblockhash)
{
    bool valid=true;
    std::string oldnick=GetNicknameForAddress(address);
    if (oldnick.size()>0)
    {
        oldnick=boost::to_upper_copy(oldnick); 
        mapNicknameBook[oldnick].invalid = true;
        if (!NicknameBatch(*database).WriteInvalidForNameNick(oldnick, true))valid=false;
    }

    std::string nick=boost::to_upper_copy(strName); 
    mapNicknameBook[nick].pubkey = address;
    mapNicknameBook[nick].hash=hash;
    mapNicknameBook[nick].invalid = false;
    mapAddressForNicknameBook[address].name=strName;

    if (storeblockhash) mapAddressForNicknameBook[address].hash=hash;

    if (!NicknameBatch(*database).WriteNameNick(nick, address))valid=false;
    if (!NicknameBatch(*database).WriteNameNickAddr(address, strName))valid=false;
    if (!NicknameBatch(*database).WriteHashForNameNick(nick, hash))valid=false;
    if (!NicknameBatch(*database).WriteInvalidForNameNick(nick, false))valid=false;
    if (storeblockhash){
        if (!NicknameBatch(*database).WriteHashForNameNickAddr(address, hash))valid=false;
    }

    return valid;
}

bool DeleteNickname(const std::string& strName,CPubKey address)
{

    std::string nick=boost::to_upper_copy(strName); 

    mapNicknameBook[nick].pubkey = CPubKey();
    mapNicknameBook[nick].hash = uint256S("0x0");
    mapAddressForNicknameBook[address].name = "";
    mapAddressForNicknameBook[address].hash = uint256S("0x0");

    bool valid=true;
    if (!NicknameBatch(*database).WriteNameNick(nick, CPubKey()))valid=false;
    if (!NicknameBatch(*database).WriteNameNickAddr(address, ""))valid=false;
    if (!NicknameBatch(*database).WriteHashForNameNick(nick, uint256S("0x0")))valid=false;
    if (!NicknameBatch(*database).WriteHashForNameNickAddr(address, uint256S("0x0")))valid=false;

    return valid;
}

void DeleteAllNicknames()
{
        std::map<std::string, CNicknameBookData>::iterator it = mapNicknameBook.begin();
        while(it != mapNicknameBook.end())
        {
            DeleteNickname(it->first,it->second.pubkey);
            it++;
        }
}

bool DoesNicknameExist(std::string nick) 
{
    if (nick.length()>0 && nick[0]=='@') nick.erase(0,1);
    boost::to_upper(nick); 
    return mapNicknameBook[nick].pubkey.size()>0;
}


CPubKey GetAddressForNickname(std::string nick) 
{
    if (nick.length()>0 && nick[0]=='@') nick.erase(0,1);
    boost::to_upper(nick); 
    if (mapNicknameBook[nick].invalid) return CPubKey();else
    return mapNicknameBook[nick].pubkey;
}

std::string GetNicknameForAddress(CPubKey address) 
{
    return mapAddressForNicknameBook[address].name;
}

uint256 GetHashForNickname(std::string nick) 
{
    boost::to_upper(nick); 
    return mapNicknameBook[nick].hash;
}


uint256 GetHashForAddress(CPubKey address) 
{

    return mapAddressForNicknameBook[address].hash;
}




