// Copyright (c) 2018 The Bitcash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <payments.h>
#include <paymentsdb.h>

#include <string>
#include <boost/algorithm/string/case_conv.hpp> 

std::unique_ptr<PaymentsDatabase> pdatabase;
std::map<std::string, CPaymentsBookData> mapPaymentsBook;

void InitPaymentsDB()
{
   fs::path path=GetDataDir();
   path /= "Payments";
    pdatabase=PaymentsDatabase::Create(path);
    DBNICKErrors nLoadWalletRet = PaymentsBatch(*pdatabase,"cr+").LoadWallet();
    (void)nLoadWalletRet;
}

bool SetPayment(const std::string& strPayment,const std::string& recipient, const std::string& desc,const std::string& amount,int day,int month)
{
    mapPaymentsBook[strPayment].recipient = recipient;
    mapPaymentsBook[strPayment].description = desc;
    mapPaymentsBook[strPayment].amount = amount;
    mapPaymentsBook[strPayment].day = day;
    mapPaymentsBook[strPayment].month = month;

    bool valid=true;
    if (!PaymentsBatch(*pdatabase).WriteNamePaymentRecipient(strPayment, recipient))valid=false;
    if (!PaymentsBatch(*pdatabase).WriteNamePaymentDesc(strPayment, desc))valid=false;
    if (!PaymentsBatch(*pdatabase).WriteNamePaymentAmount(strPayment, amount))valid=false;
    if (!PaymentsBatch(*pdatabase).WriteNamePaymentDay(strPayment, day))valid=false;
    if (!PaymentsBatch(*pdatabase).WriteNamePaymentMonth(strPayment, month))valid=false;

    return valid;
}

bool DeletePayment(const std::string& strPayment)
{
    mapPaymentsBook.erase(strPayment);

    bool valid=true;
    if (!PaymentsBatch(*pdatabase).EraseNamePayment(strPayment))valid=false;

    return valid;
}

