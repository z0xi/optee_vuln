#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <toctou.h>
#include <unistd.h>
#include <string.h>

struct command {
	int size;
	char content[256];
};

TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");
	void* ptrs[2] = {0x1020304050607080,0x0102030405060708};
	DMSG("pointer value: %p", (void*)*((void**)(((char*)ptrs)+1)));
	return TEE_SUCCESS;
}


void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;
	IMSG("Hello World!\n");

	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}

static TEE_Result create_share_memory(uint32_t param_types,
	TEE_Param params[4])
{
	char arr[64];
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT ,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	DMSG("===========================");
	DMSG("has been called");
	struct command *target;

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;
	struct command *payload = (struct command*)(params[0].memref.buffer);
	DMSG("Prev size: %d", payload->size);

	if(payload->size <= 64){
		DMSG("Prev content: %s", payload->content);
		for( int i =0;i<10;i++){
			DMSG("This is procedure after check before use");
		}
		DMSG("After size: %d", payload->size);
		DMSG("After buffer: %s", payload->content);
		memcpy(&arr[0], payload->content, payload->size);
	}
	DMSG("TEE array content: %s", arr);
	DMSG("===========================");

	return TEE_SUCCESS;
}


TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id) {
	case TA_TOCTOU_CMD_CREATE_MEMORY:
		return create_share_memory(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
