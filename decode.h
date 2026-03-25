
#ifndef DECODE_H
#define DECODE_H

#include "types.h"


typedef struct _DecodeInfo
{
    /*stego image info*/
    char *stego_fname;
    FILE * fptr_stego;

    /*output file info*/
    char output_fname[50];
    FILE * fptr_output;

    char magic_string[10];

    int extn_size;
    char extn[10];
    int file_size;

}DecodeInfo;


//DECODING finction prototypes


/*read and validate*/
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/*open the stego file*/
Status open_file(DecodeInfo *decInfo);

/*decod magic string*/
Status decode_magic_string(DecodeInfo *decInfo);

/*decode extension file size*/
Status decode_extn_file_size(int *extn_size, DecodeInfo *decInfo);

/*decode secret file extension*/
Status decode_secret_file_extn(char *file_extn, DecodeInfo *decInfo);

/*decode secret file size*/
Status decode_secret_file_size(int *file_size, DecodeInfo *decInfo);

/*decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

/*decode lsb to byte*/
char decode_lsb_to_byte(char *buffer);
/* decode lsb to size*/
int decode_lsb_to_size(char *buffer);

/*start decoding*/
Status do_decoding(DecodeInfo *decInfo);



#endif