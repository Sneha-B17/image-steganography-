#include <stdio.h>
#include "decode.h"
#include "types.h"
#include<string.h>

Status read_and_validate_decode_args(char *argv[],DecodeInfo*decInfo)
{
    if(argv[2]==NULL)
    {
        printf("error:no .bmp file");
        return e_failure;
    }

    if(strstr(argv[2],".bmp")==NULL)
    {
        printf("provide a file,of .bmp");
        return e_failure;
    }
    decInfo->stego_fname=argv[2];

    if(argv[3]==NULL)
    {
        strcpy(decInfo->output_fname,"output");
    }
    else
    {
        strcpy(decInfo->output_fname,argv[3]);
    }
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
   if(open_file(decInfo) == e_failure)
   {
        return e_failure;
   }
    fseek(decInfo->fptr_stego,54,SEEK_SET);
    if(decode_magic_string(decInfo) == e_failure)
    {
        return e_failure;
    }
    printf("verification succesfull\n");
    decode_extn_file_size(&decInfo->extn_size, decInfo);
    decode_secret_file_extn(decInfo->extn, decInfo);
    //strcpy(decInfo->output_fname, "output");
    strcat(decInfo->output_fname,decInfo->extn);
    decInfo->fptr_output = fopen(decInfo->output_fname, "w");
    decode_secret_file_size(&decInfo->file_size, decInfo);
    decode_secret_file_data(decInfo);

    fclose(decInfo->fptr_stego);
    fclose(decInfo->fptr_output);

    return e_success;
}

Status open_file(DecodeInfo*decInfo)
{
    decInfo->fptr_stego=fopen(decInfo->stego_fname,"r");
    if(decInfo->fptr_stego==NULL)
    {
        printf("file not opened");
        return e_failure;
    }
    return e_success;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    char string[3];
    char temp[8];
    char magic_string[3];

    for(int i = 0; i < 2; i++)
    {
        fread(temp, 1, 8, decInfo->fptr_stego);
        decInfo->magic_string[i] = decode_lsb_to_byte(temp);
    }
    decInfo->magic_string[2] = '\0';
    //printf("magic string obtained after decoding:%s\n",decInfo->magic_string);
    printf("enter a magic string:");
    scanf("%[^\n]",string);
    if(strcmp(magic_string,string))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

char decode_lsb_to_byte(char * buffer)
{
    char ch=0;
    for(int i=0;i<8;i++)
    {
        ch=ch<<1;
        ch=ch | (buffer[i]&1);
    }
    return ch;
}

int decode_lsb_to_size(char*buffer)
{
    int num=0;
    for(int i=0;i<32;i++)
    {
        num=num<<1;
        num=num| (buffer[i]&1);
    }
    return num;
}

Status decode_extn_file_size(int *extn_size, DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, 1, 32, decInfo->fptr_stego);
    *extn_size = decode_lsb_to_size(buffer);
    //printf("%d\n",*extn_size);
    return e_success;
}

Status decode_secret_file_extn(char *extn, DecodeInfo *decInfo)
{
    char buffer[8];
    for(int i = 0; i < decInfo->extn_size; i++)
    {
        fread(buffer, 1, 8, decInfo->fptr_stego);
        extn[i] = decode_lsb_to_byte(buffer);
    }
    extn[decInfo->extn_size] = '\0';
    //printf("%s\n",extn);

    return e_success;
}

Status decode_secret_file_size(int *file_size, DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, 1, 32, decInfo->fptr_stego);
    *file_size = decode_lsb_to_size(buffer);

    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;
    for(int i = 0; i < decInfo->file_size; i++)
    {
        fread(buffer, 1, 8, decInfo->fptr_stego);
        ch = decode_lsb_to_byte(buffer);
        fputc(ch, decInfo->fptr_output);
    }
    return e_success;
}

