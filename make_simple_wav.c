/* make_simple_wav.c Jatupat Pannoi 5735512002 */

#include <errno.h> /* errno */
#include <limits.h> /* INT_MAX INT_MIN SHRT_MAX SHRT_MIN UINT_MAX USHRT_MAX */
#include <math.h> /* cos() M_PI sin() */
#include <stdbool.h> /* bool true */
#include <stddef.h> /* offsetof() */
#include <stdint.h> /* uint16_t uint32_t */
#include <stdio.h> /* fclose() feof() FILE fopen() fprintf() fseek() fwrite()
                      printf() SEEK_SET size_t stderr */
#include <stdlib.h> /* exit() EXIT_FAILURE EXIT_SUCCESS free() malloc()
                       RAND_MAX rand() size_t srand() strtod() strtol() */
#include <string.h> /* strcmp() strncpy() strerror() */
#include <time.h> /* time() */

#include "riff.h" /* data_chunk_t format_chunk_t header_chunk_t */

extern int errno;

int make_header_structure(FILE *arg_file_out) {
  header_chunk_t *header_p;
  size_t          length_to_write;
  size_t          length_written;

  header_p=(header_chunk_t *)malloc((size_t)sizeof(header_chunk_t));
  if( header_p==NULL ) {
    fprintf(stderr,"no room for a a %lu byte header structure\n",
      sizeof(header_chunk_t));
    exit(EXIT_FAILURE);
  }

  strncpy(header_p->hdr_sGroupID, "RIFF",4);
  header_p->hdr_dwFileLength=0; /* correct this later */
  strncpy(header_p->hdr_sRiffType,"WAVE",4);

  length_to_write=sizeof(header_chunk_t);
  length_written=fwrite(header_p,(size_t)1,length_to_write,arg_file_out);
  if( length_written!=length_to_write ) {
    fprintf(stderr,"error writing %ld of %ld bytes of header\n",
      length_written,length_to_write);
    exit(EXIT_FAILURE);
  }

  free(header_p);

  /* skip hdr_sGroupID and hdr_dwFileLength */
  return sizeof(header_chunk_t)-offsetof(header_chunk_t,hdr_sRiffType);

}

int make_format_structure(
  int              arg_bit_depth,
  int              arg_number_of_channels,
  int              arg_sample_rate,
  FILE            *arg_file_out
  ) {
  int             bytes_per_frame;
  format_chunk_t *format_p;
  size_t          length_to_write;
  size_t          length_written;

  format_p=(format_chunk_t *)malloc(
    (size_t)offsetof(format_chunk_t,fmt_wExtensionSize)
    );
  if( format_p==NULL ) {
    fprintf(stderr,"no room for a %lu byte format structure\n",
      offsetof(format_chunk_t,fmt_wExtensionSize));
    exit(EXIT_FAILURE);
  }

  strncpy(format_p->fmt_sGroupID,"fmt ",4);
  format_p->fmt_dwChunkSize=offsetof(format_chunk_t,fmt_wExtensionSize)-
    offsetof(format_chunk_t,fmt_wFormatTag); /* 16 */
  format_p->fmt_wFormatTag=WAVE_FORMAT_PCM;
  format_p->fmt_wChannels=arg_number_of_channels;
  format_p->fmt_dwSamplesPerSec=arg_sample_rate;
  bytes_per_frame=format_p->fmt_wChannels*(arg_bit_depth/8);
  format_p->fmt_dwAvgBytesPerSec=
    format_p->fmt_dwSamplesPerSec*bytes_per_frame;
  format_p->fmt_wBlockAlign=(uint16_t)(bytes_per_frame);
  format_p->fmt_wBitsPerSample=arg_bit_depth;

  length_to_write=offsetof(format_chunk_t,fmt_wExtensionSize);
  length_written=fwrite(format_p,(size_t)1,length_to_write,arg_file_out);
  if( length_written!=length_to_write ) {
    fprintf(stderr,"error writing %ld of %ld bytes of format\n",
      length_written,length_to_write);
    exit(EXIT_FAILURE);
  }

  free(format_p);

  return offsetof(format_chunk_t,fmt_wExtensionSize);

}

int make_data_structure(
  int            arg_data_size,
  int            arg_number_of_channels,
  int            arg_number_of_samples,
  int            arg_pad_size,
  int            arg_samples_per_cycle,
  char          *arg_waveform_type,
  FILE          *arg_file_out
  ) {
  double        angle_increment;
  data_chunk_t *data_p;
  bool          go_up;
  int           height_increment;
  int           i;
  int           j;
  size_t        length_to_write;
  size_t        length_written;
  short        *next_sample_p;
  int           position_in_cycle;
  double        y;

  data_p=(data_chunk_t *)malloc(
    (size_t)(sizeof(data_chunk_t)+arg_data_size+arg_pad_size)
    );
  if( data_p==NULL ) {
    fprintf(stderr,"no room for a %lu byte data structure\n",
      sizeof(data_chunk_t)+arg_data_size+arg_pad_size);
    exit(EXIT_FAILURE);
  }

  strncpy(data_p->dat_sGroupID,"data",4);
  data_p->dat_dwChunkSize=arg_data_size;

  next_sample_p=&(data_p->dat_shortArray[0]);
  position_in_cycle=0;

  if( strcmp(arg_waveform_type,"cosine")==0 ) {
    angle_increment=2.0*M_PI/arg_samples_per_cycle;
    for( i=0; i<arg_number_of_samples; ++i ) {
      y=SHRT_MAX*cos(angle_increment*position_in_cycle);
      for( j=0; j<arg_number_of_channels; ++j ) {
        *next_sample_p=(short)y;
        ++next_sample_p;
      }

      ++position_in_cycle;
      if( position_in_cycle>=arg_samples_per_cycle )
        position_in_cycle=0;
    }
  } else if( strcmp(arg_waveform_type,"noise")==0 ) {
    srand((unsigned int)time(NULL));
    for( i=0; i<arg_number_of_samples; ++i ) {
      /* ((float)rand())/RAND_MAX is between 0 and 1, inclusive
         ((float)rand())/RAND_MAX*2.0 is between 0 and 2, inclusive
         -1.0+((float)rand())/RAND_MAX*2.0 is between -1 and 1, inclusive
      */
      y=(int)(SHRT_MAX*(-1.0+((float)rand())/RAND_MAX*2.0));
      for( j=0; j<arg_number_of_channels; ++j ) {
        *next_sample_p=(short)y;
        ++next_sample_p;
      }

      ++position_in_cycle;
      if( position_in_cycle>=arg_samples_per_cycle )
        position_in_cycle=0;
    }
  } else if( strcmp(arg_waveform_type,"sawtooth")==0 ) {
    height_increment=(SHRT_MAX-SHRT_MIN)/arg_samples_per_cycle;
    for( i=0; i<arg_number_of_samples; ++i ) {
      y=SHRT_MIN+position_in_cycle*height_increment;
      for( j=0; j<arg_number_of_channels; ++j ) {
        *next_sample_p=(short)y;
        ++next_sample_p;
      }

      ++position_in_cycle;
      if( position_in_cycle>=arg_samples_per_cycle )
        position_in_cycle=0;
    }
  } else if( strcmp(arg_waveform_type,"sine")==0 ) {
    angle_increment=2.0*M_PI/arg_samples_per_cycle;
    for( i=0; i<arg_number_of_samples; ++i ) {
      y=SHRT_MAX*sin(angle_increment*position_in_cycle);
      for( j=0; j<arg_number_of_channels; ++j ) {
        *next_sample_p=(short)y;
        ++next_sample_p;
      }

      ++position_in_cycle;
      if( position_in_cycle>=arg_samples_per_cycle )
        position_in_cycle=0;
    }
  } else if( strcmp(arg_waveform_type,"square")==0 ) {
    for( i=0; i<arg_number_of_samples; ++i ) {
      if( position_in_cycle<arg_samples_per_cycle/2 )
        y=SHRT_MIN;
      else
        y=SHRT_MAX;
      for( j=0; j<arg_number_of_channels; ++j ) {
        *next_sample_p=(short)y;
        ++next_sample_p;
      }

      ++position_in_cycle;
      if( position_in_cycle>=arg_samples_per_cycle )
        position_in_cycle=0;
    }
  } else if( strcmp(arg_waveform_type,"toothsaw")==0 ) {
    height_increment=(SHRT_MAX-SHRT_MIN)/arg_samples_per_cycle;
    for( i=0; i<arg_number_of_samples; ++i ) {
      y=SHRT_MAX-position_in_cycle*height_increment;
      for( j=0; j<arg_number_of_channels; ++j ) {
        *next_sample_p=(short)y;
        ++next_sample_p;
      }

      ++position_in_cycle;
      if( position_in_cycle>=arg_samples_per_cycle )
        position_in_cycle=0;
    }
  } else { /* triangle */
    go_up=true;
    height_increment=(SHRT_MAX-SHRT_MIN)/arg_samples_per_cycle;
    for( i=0; i<arg_number_of_samples; ++i ) {
      if( go_up )
        y=SHRT_MIN+position_in_cycle*height_increment;
      else
        y=SHRT_MAX-position_in_cycle*height_increment;
      for( j=0; j<arg_number_of_channels; ++j ) {
        *next_sample_p=(short)y;
        ++next_sample_p;
      }

      ++position_in_cycle;
      if( position_in_cycle>=arg_samples_per_cycle ) {
        position_in_cycle=0;
        go_up=!go_up;
      }
    }
  }

  length_to_write=(sizeof(data_chunk_t)+arg_data_size+arg_pad_size);
  length_written=fwrite(data_p,(size_t)1,length_to_write,arg_file_out);
  if( length_written!=length_to_write ) {
    fprintf(stderr,"error writing %ld of %ld bytes of data\n",
      length_written,length_to_write);
    exit(EXIT_FAILURE);
  }

  free(data_p);

  return sizeof(data_chunk_t)+arg_data_size;

}

float string_to_float(char *arg_text) {
  char   *end;
  double  float_value;

  if( *arg_text=='\0' ) {
    fprintf(stderr,"empty string argument to string_to_float()\n");
    exit(EXIT_FAILURE);
  }
  errno=0;
  float_value=strtod(arg_text,&end);
  if( errno!=0 ) {
    fprintf(stderr,"error converting \"%s\" to float: %s\n",arg_text,
      strerror(errno));
    exit(EXIT_FAILURE);
  }
  if( *end!='\0' ) {
    fprintf(stderr,"trailing garbage converting \"%s\" to float\n",arg_text);
    exit(EXIT_FAILURE);
  }

  return float_value;

}

int string_to_int(char *arg_text) {
  char *end;
  long  long_value;

  if( *arg_text=='\0' ) {
    fprintf(stderr,"empty string argument to string_to_int()\n");
    exit(EXIT_FAILURE);
  }
  errno=0;
  long_value=strtol(arg_text,&end,0);
  if( errno!=0 ) {
    fprintf(stderr,"error converting \"%s\" to long: %s\n",arg_text,
      strerror(errno));
    exit(EXIT_FAILURE);
  }
  if( *end!='\0' ) {
    fprintf(stderr,"trailing garbage converting \"%s\" to long\n",arg_text);
    exit(EXIT_FAILURE);
  }
  if( (long_value<INT_MIN) || (long_value>INT_MAX) ) {
    fprintf(stderr,"value out of integer range: \"%ld\"\n",long_value);
    exit(EXIT_FAILURE);
  }

  return (int)long_value;

}

int main(int argc,char **argv,char **envp) {
  int       bit_depth;
  int       data_size;
  FILE     *file_out;
  char     *filename;
  double    frequency;
  int       i;
  int       number_of_channels;
  int       number_of_samples;
  int       pad_size;
  int       sample_rate;
  int       samples_per_cycle;
  int       seconds_of_audio;
  uint32_t  total_bytes_for_header_length;
  char     *waveform_type;

  if( (argc!=8                             ) ||
      ( (strcmp(argv[3],"byte" )!=0) &&
        (strcmp(argv[3],"float")!=0) &&
        (strcmp(argv[3],"short")!=0)       ) ||
      ( (strcmp(argv[4],"mono"  )!=0) &&
        (strcmp(argv[4],"stereo")!=0)      ) ||
      ( (strcmp(argv[6],"cosine"  )!=0) &&
        (strcmp(argv[6],"noise"   )!=0) &&
        (strcmp(argv[6],"sawtooth")!=0) &&
        (strcmp(argv[6],"sine"    )!=0) &&
        (strcmp(argv[6],"square"  )!=0) &&
        (strcmp(argv[6],"toothsaw")!=0) &&
        (strcmp(argv[6],"triangle")!=0)    )
    ) {
    fprintf(stderr,
      "usage: %s filename sample_rate {byte|float|short} {mono|stereo} "
      "seconds_of_audio {cosine|noise|sawtooth|sine|square|toothsaw|triangle} "
      "frequency\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  filename=argv[1];

  sample_rate=string_to_int(argv[2]);
  if( sample_rate<1 ) {
    fprintf(stderr,"sample rate must be at least 1\n");
    exit(EXIT_FAILURE);
  }

  if( strcmp(argv[3],"byte")==0 )
    bit_depth=8;
  else if( strcmp(argv[3],"float")==0 )
    bit_depth=32;
  else /* short */
    bit_depth=16;
if( bit_depth!=16 ) {
fprintf(stderr,"bit depth must be short, for now\n");
exit(EXIT_FAILURE);
}

  if( strcmp(argv[4],"mono")==0 )
    number_of_channels=1;
  else
    number_of_channels=2; /* stereo */

  seconds_of_audio=string_to_int(argv[5]);
  if( seconds_of_audio<1 ) {
    fprintf(stderr,"seconds of audio must be at least 1\n");
    exit(EXIT_FAILURE);
  }

  waveform_type=argv[6];

  frequency=string_to_float(argv[7]);
  if( frequency<1 ) {
    fprintf(stderr,"frequency must be at least 1\n");
    exit(EXIT_FAILURE);
  }

  /* if the frequency is F cycles/second, then the period 1/F second is the
     length of 1 cycle; if there are N samples per second, then the period 1/N
     seconds is the length of 1 sample; F should be much larger than N; 1
     cycle of frequency lasts for (1/F)/(1/N) or N/F samples
  */
  samples_per_cycle=sample_rate/frequency;

  number_of_samples=sample_rate*seconds_of_audio;
  if( bit_depth==8 )
    data_size=number_of_samples*number_of_channels*sizeof(unsigned char);
  else if( bit_depth==16 )
    data_size=number_of_samples*number_of_channels*sizeof(uint16_t);
  else /* bit_depth==32 */
    data_size=number_of_samples*number_of_channels*sizeof(float);
  pad_size=data_size%2==1 ? 1 : 0;

  if( (file_out=fopen(filename,"wb"))==NULL ) {
    fprintf(stderr,"error opening file \"%s\": %s\n",filename,strerror(errno));
    exit(EXIT_FAILURE);
  }

  total_bytes_for_header_length=0;

  total_bytes_for_header_length+=make_header_structure(
    file_out
    );

  total_bytes_for_header_length+=make_format_structure(
    bit_depth,
    number_of_channels,
    sample_rate,
    file_out
    );

  total_bytes_for_header_length+=make_data_structure(
    data_size,
    number_of_channels,
    number_of_samples,
    pad_size,
    samples_per_cycle,
    waveform_type,
    file_out
    );

  i=fseek(file_out,(long)offsetof(header_chunk_t,hdr_dwFileLength),SEEK_SET);
  if( i==-1 ) {
    fprintf(stderr,"cannot go back and fix header length: %s\n",
      strerror(errno));
    exit(EXIT_FAILURE);
  }
  i=fwrite(
    &total_bytes_for_header_length,
    (size_t)1,
    (size_t)sizeof(total_bytes_for_header_length),
    file_out
    );
  if( i!=sizeof(total_bytes_for_header_length) ) {
    fprintf(stderr,"error writing %d of %ld bytes of data to \"%s\"\n",i,
      sizeof(total_bytes_for_header_length),filename);
    exit(EXIT_FAILURE);
  }

  if( fclose(file_out)==EOF ) {
    fprintf(stderr,"error closing file \"%s\": %s\n",filename,strerror(errno));
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);

}
