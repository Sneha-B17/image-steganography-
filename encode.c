#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>


/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    rewind(fptr_image);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

OperationType check_operation_type(char *argv[])
{
    if(argv[1]==NULL)
    {
        return e_unsupported;
    }

    if(!strcmp(argv[1],"-e"))
    {
        return e_encode;
    }

    if(!strcmp(argv[1],"-d"))
    {
        return e_decode;
    }

    return e_unsupported;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if(argv[2]==NULL)
    {
        printf("Error:no command\n");
        return e_failure;
    }

    if((strstr(argv[2],".bmp"))==NULL)
    {
        printf("Error: .bmp is not there\n");
        return e_failure;
    }

    encInfo->src_image_fname = argv[2];

    if(argv[3]==NULL)
    {
        printf("Error:no command\n");
        return e_failure;
    }

    if(strchr(argv[3],'.')==NULL)
    {
        printf("Error: no . files\n");
        return e_failure;
    }

    encInfo->secret_fname = argv[3];

    if(argv[4]==NULL)
    {
        encInfo->stego_image_fname="stego.bmp";
    }
    else
    {
        if(strstr(argv[4],".bmp"))
        {
            encInfo->stego_image_fname=argv[4];
        }
        else
        {
            return e_failure;
        }
    }

    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    Status ret = check_capacity(encInfo);
    if (ret == e_failure)
    {
        printf("Error: Insufficient capacity\n");
        return e_failure;
    }
    copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image);
    encode_magic_string(MAGIC_STRING, encInfo);
    encode_extn_file_size(encInfo->extn_size, encInfo);
    char *extn = strstr(encInfo->secret_fname,".");
    encode_secret_file_extn(extn, encInfo);
    int secret_size = get_file_size(encInfo->fptr_secret);
    encode_secret_file_size(secret_size, encInfo);
    encode_secret_file_data(encInfo);
    copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image);

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_stego_image)
{
    printf("at start:%ld %ld\n",ftell(fptr_src_image),ftell(fptr_stego_image));
    char ch;
    for(int i=0;i<54;i++)
    {
        fread(&ch,1,1,fptr_src_image);
        fwrite(&ch,1,1,fptr_stego_image);
    }
    printf("at end:%ld %ld\n",ftell(fptr_src_image),ftell(fptr_stego_image));
}

uint get_file_size(FILE*fptr)
{
    fseek(fptr,0,SEEK_END);
    int size=ftell(fptr);
    rewind(fptr);
    return size;
}

Status check_capacity(EncodeInfo *encInfo)
{
    int sec_file_size;
    int capacity;
    if(open_files(encInfo) == e_failure)
    {
        return e_failure;
    }
    encInfo->image_size = get_image_size_for_bmp(encInfo->fptr_src_image);
    sec_file_size = get_file_size(encInfo->fptr_secret);
    encInfo->extn_size = strlen(strstr(encInfo->secret_fname, "."));
    capacity = 2 + 4 + encInfo->extn_size + 4 + sec_file_size + 54;
    if(encInfo->image_size < capacity)
    {
        printf("Error: Insufficient image capacity\n");
        return e_failure;
    }

    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    //image buffer size is 8 bytes

    char n = 7;
    for(int i = 0; i < 8; i++)
    {
        //clear lsb of image_buffer[i]
        image_buffer[i] = image_buffer[i] & 0xfe;

        //get bit from data
        char res = data & (1 << n);

        //take res bit to lsb
        res = res >> n;

        //write res bit to lsb of buffer[i]
        image_buffer[i] = image_buffer[i] | res;

        n--;
    }
}

Status encode_size_to_lsb(int data, char *image_buffer)
{
    //image buffer size is 32 bytes

    int n = 31;
    for(int i = 0; i < 32; i++)
    {
        //clear lsb of image_buffer[i]
        image_buffer[i] = image_buffer[i] & 0xfffffffe;

        //get bit from data
        int res = data & (1 << n);

        //take res bit to lsb
        res = res >> n;

        //write res bit to lsb of buffer[i]
        image_buffer[i] = image_buffer[i] | res;

        n--;
    }
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    printf("at start:%ld %ld\n",ftell(encInfo->fptr_src_image),ftell(encInfo->fptr_stego_image));
    char temp[8];

    for(int i=0;i<2;i++)
    {
        fread(temp,1,8,encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i],temp);
        fwrite(temp,1,8,encInfo->fptr_stego_image);
    }
    printf("at end:%ld %ld\n",ftell(encInfo->fptr_src_image),ftell(encInfo->fptr_stego_image));
    return e_success;
}

Status encode_extn_file_size(int extn_size, EncodeInfo *encInfo )
{
    printf("at start:%ld %ld\n",ftell(encInfo->fptr_src_image),ftell(encInfo->fptr_stego_image));
    char temp[32];

    fread(temp,1,32,encInfo->fptr_src_image);
    encode_size_to_lsb(extn_size,temp);
    fwrite(temp,1,32,encInfo->fptr_stego_image);

    printf("at end:%ld %ld\n",ftell(encInfo->fptr_src_image),ftell(encInfo->fptr_stego_image));
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    printf("at start:%ld %ld\n",ftell(encInfo->fptr_src_image),ftell(encInfo->fptr_stego_image));
    char temp[8];
    for(int i=0;file_extn[i];i++)
    {
        fread(temp,8,1,encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i],temp);
        fwrite(temp,8,1,encInfo->fptr_stego_image);
    }
    printf("at last:%ld %ld\n",ftell(encInfo->fptr_src_image),ftell(encInfo->fptr_stego_image));
    return e_success;
}

Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{
    printf("at start:%ld %ld\n",ftell(encInfo->fptr_src_image),ftell(encInfo->fptr_stego_image));
    char temp[32];

    fread(temp,1,32,encInfo->fptr_src_image);
    encode_size_to_lsb(file_size,temp);
    fwrite(temp,1,32,encInfo->fptr_stego_image);

    printf("at end:%ld %ld\n",ftell(encInfo->fptr_src_image),ftell(encInfo->fptr_stego_image));
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    printf("at start:%ld %ld\n",ftell(encInfo->fptr_src_image),ftell(encInfo->fptr_stego_image));
    char ch;
    char temp[8];
    while (!feof(encInfo->fptr_secret))
    {
        ch = fgetc(encInfo->fptr_secret);
        if (feof(encInfo->fptr_secret))
        {
            break;
        }
        fread(temp,1,8,encInfo->fptr_src_image);
        encode_byte_to_lsb(ch,temp);
        fwrite(temp,1,8,encInfo->fptr_stego_image);
    }
    printf("at last:%ld %ld\n",ftell(encInfo->fptr_src_image),ftell(encInfo->fptr_stego_image));
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_stego)
{
    printf("at start:%ld %ld\n",ftell(fptr_src),ftell(fptr_stego));
    char ch;
    while (!feof(fptr_src))
    {
        ch = fgetc(fptr_src);

        if (feof(fptr_src))
        {
            break;
        }

        fputc(ch, fptr_stego);
    }
   printf("at last:%ld %ld\n",ftell(fptr_src),ftell(fptr_stego));
    return e_success;
}





