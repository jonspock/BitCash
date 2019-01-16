// Copyright (c) 2011-2014 The Bitcash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCASH_QT_BITCASHADDRESSVALIDATOR_H
#define BITCASH_QT_BITCASHADDRESSVALIDATOR_H

#include <interfaces/wallet.h>
#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class BitcashAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit BitcashAddressEntryValidator(QObject *parent = 0);
    void fixup(QString & input) const override;
    QLocale locale() const;
    void setLocale(const QLocale & locale);

    State validate(QString &input, int &pos) const override;
};

/** Bitcash address widget validator, checks for a valid bitcash address.
 */

class BitcashAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit BitcashAddressCheckValidator(QObject * parent = 0);
    void fixup(QString & input) const override;
    QLocale locale() const;
    void setLocale(const QLocale & locale);
    State validate(QString &input, int &pos) const override;
};

class BitcashAddressCheckValidator2 : public QValidator
{
    Q_OBJECT

public:
    explicit BitcashAddressCheckValidator2(QObject * parent = 0);
    void fixup(QString & input) const override;
    QLocale locale() const;
    void setLocale(const QLocale & locale);
    State validate(QString &input, int &pos) const override;
};

#endif // BITCASH_QT_BITCASHADDRESSVALIDATOR_H

//convert comma to point etc.
class BitcashAmountEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit BitcashAmountEntryValidator(QObject *parent = 0);
    void fixup(QString & input) const override;
    QLocale locale() const;
    void setLocale(const QLocale & locale);

    State validate(QString &input, int &pos) const override;
};
