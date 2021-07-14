/*
 * MyWaveFile.h
 *
 *  Created on: 2 thg 7, 2021
 *      Author: Dell
 */

#ifndef MYWAVEFILE_H_
#define MYWAVEFILE_H_

#include <stdint.h>
#include "inc/hw_types.h"
#include "ff.h"
#include "diskio.h"
//*****************************************************************************
//
// The .wav file header information.
//
//*****************************************************************************
typedef struct
{
    //
    // Sample rate in bytes per second.
    //
    uint32_t ui32SampleRate;

    //
    // The average byte rate for the .wav file.
    //
    uint32_t ui32AvgByteRate;

    //
    // The size of the .wav data in the file.
    //
    uint32_t ui32DataSize;

    //
    // The number of bits per sample.
    //
    uint16_t ui16BitsPerSample;

    //
    // The .wav file format.
    //
    uint16_t ui16Format;

    //
    // The number of audio channels.
    //
    uint16_t ui16NumChannels;
}
tWavHeader;

//*****************************************************************************
//
// The structure used to hold the .wav file state.
//
//*****************************************************************************
typedef struct
{
    //
    // The wav files header information
    //
    tWavHeader sWavHeader;

    //
    // The file information for the current file.
    //
    FIL i16File;

    //
    // Current state flags, a combination of the WAV_FLAG_* values.
    //
    uint32_t ui32Flags;
} tWavFile;

void WavGetFormat(tWavFile *psWavData, tWavHeader *psWaveHeader);
int WavOpen(const char *pcFileName, tWavFile *psWavData);
void WavClose(tWavFile *psWavData);
uint16_t WavRead(tWavFile *psWavData, unsigned char *pucBuffer,
                        uint32_t ui32Size);

#endif /* MYWAVEFILE_H_ */
