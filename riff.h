/* riff.h Mickey Mouse 4935512001 */

#ifndef RIFF_H
#define RIFF_H

#define	WAVE_FORMAT_PCM 	0x0001 /* PCM */
#define	WAVE_FORMAT_IEEE_FLOAT 	0x0003 /* IEEE float */
#define	WAVE_FORMAT_ALAW 	0x0006 /* 8-bit ITU-T G.711 A-law */
#define	WAVE_FORMAT_MULAW 	0x0007 /* 8-bit ITU-T G.711 µ-law */
#define	WAVE_FORMAT_EXTENSIBLE 	0xFFFE /* Determined by SubFormat */

/* under MS-DOS, a C language int was 2 bytes long, and the variable name
   began with "w" for "word"; a long was 4 bytes long, and the variable name
   began with "dw" for "double word"; the name of a character string variable
   began with "s"; in these structures, the Microsoft variable names from
   Visual Studio are used
*/

/* a block or frame has one sample for each channel, at a certain sample time;
   it is followed by a block or frame with the number_of_channels samples for
   the next sample time, and so on
*/

typedef
  struct {
    char          dat_sGroupID [4];   /* "data" */
    uint32_t      dat_dwChunkSize;    /* bytes in the struct, after this field
                                      */
    unsigned char dat_byteArray [0];  /* bytes of audio data */
    float         dat_floatArray [0];
    short         dat_shortArray [0];
  } data_chunk_t; /* data_size bytes, without the first 2 fields */

typedef
  struct {
    char     fct_sGroupID [4];   /* "fact" */
    uint32_t fct_dwChunkSize;    /* bytes in the struct, after this field */
    uint32_t fct_dwSampleLength; /* number of samples per channel */
  } fact_chunk_t; /* minimum of 4+4+4=12 */

typedef
  struct {
    char     fmt_sGroupID [4];        /* four bytes: "fmt " */
    uint32_t fmt_dwChunkSize;         /* bytes in the struct, after this field
                                      */
    uint16_t fmt_wFormatTag;          /* 1 (MS PCM) */
    uint16_t fmt_wChannels;           /* number of channels */
    uint32_t fmt_dwSamplesPerSec;     /* frequency of the audio in Hz... 44100
                                      */
    uint32_t fmt_dwAvgBytesPerSec;    /* for estimating RAM allocation */
    uint16_t fmt_wBlockAlign;         /* sample frame size, in bytes */
    uint16_t fmt_wBitsPerSample;      /* bits per sample: must be a multiple
                                         of 8 */
    uint16_t fmt_wExtensionSize;      /* size of the extension: 0 or 22 */
    uint16_t fmt_wValidBitsPerSample; /* number of valid bits */
    uint32_t fmt_dwChannelMask;       /* speaker position mask */
    char     fmt_sSubFormat [16];     /*  */
  } format_chunk_t; /* 2+2+4+4+2+2=16 bytes, without the first 2 fields */
                    /* 2+2+4+4+2+2+2=18 bytes */
                    /* 2+2+4+4+2+2+2+2+4+16=40 bytes */

typedef
  struct {
    char     hdr_sGroupID [4];  /* "RIFF" */
    uint32_t hdr_dwFileLength;  /* bytes in the struct, after this field, plus
                                   total bytes in all other chunks */
    char     hdr_sRiffType [4]; /* always "WAVE" */
  } header_chunk_t; /*  4 bytes,without the first 2 fields */

#endif
