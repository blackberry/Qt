/*
 * $QNXLicenseC:
 * Copyright 2010, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable license fees to QNX
 * Software Systems before you may reproduce, modify or distribute this software,
 * or any work that includes all or part of this software. Free development
 * licenses are available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *
 * This file may contain contributions from others. Please review this entire
 * file for other proprietary rights or license notices, as well as the QNX
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/
 * for other information.
 * $
 */


#ifndef VIRTUALKEYBOARD_H_
#define VIRTUALKEYBOARD_H_

#include <QtGui/QPlatformScreen>
#include <QtGui/QWindowSystemInterface>
#include <QThread>

#include <stddef.h>
#include <vector>
#include <string>
#include <sys/pps.h>

QT_BEGIN_NAMESPACE

/* Shamelessly copied from the browser - this should be rewritten once we have a proper PPS wrapper class */
class QBBVirtualKeyboard : QThread
{
public:
    // NOTE:  Not all the following keyboard modes are currently used.
    // Default - Regular Keyboard
    // Url/Email - Enhanced keys for each types.
    // Web - Regular keyboard with two blank keys, currently unused.
    // NumPunc - Numbers & Punctionation, alternate to Symbol
    // Symbol - All symbols, alternate to NumPunc, currently unused.
    // Phone - Phone enhanced keyboard - currently unused as no alternate keyboard available to access a-zA-Z
    // Pin - Keyboard for entering Pins (Hex values) currently unused.
    //
    // SPECIAL NOTE: Usage of NumPunc may have to be removed, ABC button is non-functional.
    //
    enum KeyboardMode { Default, Url, Email, Web, NumPunc, Symbol, Phone, Pin };

    static QBBVirtualKeyboard& instance();
    static void destroy();

    bool showKeyboard();
    bool hideKeyboard();
    int  getHeight() { return mVisible ? mHeight : 0; }
    void setKeyboardMode(KeyboardMode);
    void notifyClientActiveStateChange(bool);
    bool isVisible() const { return mVisible; }
    QString languageId() const { return mLanguageId; }
    QString countryId() const { return mCountryId; }

private:
    QBBVirtualKeyboard();
    virtual ~QBBVirtualKeyboard();

    pps_encoder_t          *mEncoder;
    pps_decoder_t          *mDecoder;
    char                   *mBuffer;
    int                     mHeight;
    int                     mFd;
    KeyboardMode            mKeyboardMode;
    bool                    mVisible;
    QString                 mLanguageId;
    QString                 mCountryId;

    // Path to keyboardManager in PPS.
    static const char  *sPPSPath;
    static const size_t sBufferSize;

    // Will be called internally if needed.
    bool connect();
    void close();
    void ppsDataReady();
    bool queryPPSInfo();
    void handleKeyboardStateChangeMessage(bool visible);
    void handleKeyboardInfoMessage();

    void applyKeyboardModeOptions();
    void addDefaultModeOptions();
    void addUrlModeOptions();
    void addEmailModeOptions();
    void addWebModeOptions();
    void addNumPuncModeOptions();
    void addSymbolModeOptions();
    void addPhoneModeOptions();
    void addPinModeOptions();

    // QThread overrides
    virtual void run();

};

#endif /* VIRTUALKEYBOARD_H_ */
