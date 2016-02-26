// ASIO Driver
// Copyright (c) Eugene Gavrilov, Max Mikhailov, 2001-2016.
// All rights reserved

// Author: Max Mikhailov

/** \file
    Utility functions. The file provides functions for translating between different representations.
*/

#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

// ----------------------------------------------------------------------------
// conversion

struct {int num; ceapi_sr_e ceapi; int avca; int avcb; const TCHAR* str;} 
const rate_trans[] = 
{
    // num,  ceapi,         avca, avcb, str,
    {32000,  CEAPI_SR_32000,   2,  0,   TEXT("32000 Hz")},
    {44100,  CEAPI_SR_44100,   3,  1,   TEXT("44100 Hz")},
    {48000,  CEAPI_SR_48000,   4,  2,   TEXT("48000 Hz")},
    {64000,  CEAPI_SR_64000,  -1, -1,   TEXT("64000 Hz")},
    {88200,  CEAPI_SR_88200,  10,  3,   TEXT("88200 Hz")},
    {96000,  CEAPI_SR_96000,   5,  4,   TEXT("96000 Hz")}, 
    {176400, CEAPI_SR_176400,  6,  5,   TEXT("176400 Hz")},
    {192000, CEAPI_SR_192000,  7,  6,   TEXT("192000 Hz")},
    // avca rate is as in AVC_STREAM_FORMAT
    // avcb rate is as in 61883-6
};

/// converts sample rate into CEAPI_SR_ constant
inline ceapi_sr_e convRateNum2Ce(int sample_rate)
{
    int n = sizeof(rate_trans)/sizeof(rate_trans[0]);
    for (int i = 0; i < n; i++) 
        if (rate_trans[i].num == sample_rate) 
            return rate_trans[i].ceapi;
    return CEAPI_SR_NONE;
}

/// converts sample rate into CEAPI_SR_ constant
inline ceapi_sr_e convRateNum2Ce(double sample_rate)
{
    return convRateNum2Ce((int) (sample_rate + .5) /* round */);
}

/// converts CEAPI_SR_ constant into sample rate in Hz
inline int convRateCe2Num(ceapi_sr_e sample_rate)
{
    int n = sizeof(rate_trans)/sizeof(rate_trans[0]);
    for (int i = 0; i < n; i++) 
        if (rate_trans[i].ceapi == sample_rate) 
            return rate_trans[i].num;
    return 0;
}

/// converts CEAPI_SR_ constant into a text string
inline const TCHAR* convRateCe2Str(ceapi_sr_e sample_rate)
{
    int n = sizeof(rate_trans)/sizeof(rate_trans[0]);
    for (int i = 0; i < n; i++) 
        if (rate_trans[i].ceapi == sample_rate) 
            return rate_trans[i].str;
    return TEXT("Unknown");
}

/// converts sample rate in Hz into a text string
inline const TCHAR* convRateNum2Str(int sample_rate)
{
    int n = sizeof(rate_trans)/sizeof(rate_trans[0]);
    for (int i = 0; i < n; i++) 
        if (rate_trans[i].num == sample_rate) 
            return rate_trans[i].str;
    return TEXT("Unknown");
}

// ASIO SDK headers included?
#ifdef __ASIO_H

struct {ceapi_bps_e ceapi; ASIOSampleType asio; int bytes; int valid_bytes; const TCHAR* str;}
const bps_trans[] = 
{
    {CEAPI_BPS_LSB_16,       ASIOSTInt16LSB,   16/8, 16/8,	TEXT("16")},
    {CEAPI_BPS_LSB_24,       ASIOSTInt24LSB,   24/8, 24/8,	TEXT("24")},
    {CEAPI_BPS_LSB_24PD,     ASIOSTInt32LSB,   32/8, 24/8,	TEXT("24/32")},    // left-aligned 24-bit data in 32-bit container
    {CEAPI_BPS_LSB_32,       ASIOSTInt32LSB,   32/8, 32/8,	TEXT("32")},
};

/// converts CEAPI_BPS_ into number of bytes
inline int convBpsCe2Bytes(ceapi_bps_e bps)
{
    int n = sizeof(bps_trans)/sizeof(bps_trans[0]);
    for (int i = 0; i < n; i++) 
        if (bps_trans[i].ceapi == bps)
            return bps_trans[i].bytes;
    return 0;
}

/// converts CEAPI_BPS_ into number of bytes
inline int convBpsCe2ValidBytes(ceapi_bps_e bps)
{
    int n = sizeof(bps_trans)/sizeof(bps_trans[0]);
    for (int i = 0; i < n; i++) 
        if (bps_trans[i].ceapi == bps)
            return bps_trans[i].valid_bytes;
    return 0;
}

/// converts number of bytes into ASIO format constant
inline ASIOSampleType convBpsCe2Asio(ceapi_bps_e bps)
{
    int n = sizeof(bps_trans)/sizeof(bps_trans[0]);
    for (int i = 0; i < n; i++) 
        if (bps_trans[i].ceapi == bps) 
            return bps_trans[i].asio;
    return 0;
}

/// converts CEAPI_BPS_ constant into a text string
inline const TCHAR* convBpsCe2Str(ceapi_bps_e bps)
{
    int n = sizeof(bps_trans)/sizeof(bps_trans[0]);
    for (int i = 0; i < n; i++) 
        if (bps_trans[i].ceapi == bps) 
            return bps_trans[i].str;
    return TEXT("Unknown");
}

#endif // __ASIO_H

// ----------------------------------------------------------------------------

#endif // AUDIO_UTILS_H
