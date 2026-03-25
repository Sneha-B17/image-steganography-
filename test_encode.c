#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "decode.h"
#include<string.h>

int main(int argc,char*argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    int ret=check_operation_type(argv);
    if(ret==e_encode)
    {
        int ret1=read_and_validate_encode_args(argv,&encInfo);
        if(ret1==e_failure)
        {
            return e_failure;
        }
        if(do_encoding(&encInfo)==e_failure)
        {
            return e_failure;
        }
        printf("encoded successfully\n");
    }
    else if(ret==e_decode)
    {
        int ret2=read_and_validate_decode_args(argv,&decInfo);
        if(ret2==e_failure)
        {
            return e_failure;
        }
        if(do_decoding(&decInfo)==e_failure)
        {
            return e_failure;
        }
        printf("decoded successfully\n");
    }
    else
    {
        printf("Unsupported\n");
    }
    

}
