#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <type_confusion.h>
#include <string.h>//memcpy
#include <stdio.h>//sprintf

const char* SECRET = "09 F9 11 02 9D 74 E3 5B D8 41 56 C5 63 56 88 C0";
bool PRINTSECRET = false;
 static inline uint64_t reg_pair_to_64(uint32_t reg0, uint32_t reg1)
{
	return (uint64_t)reg0 << 32 | reg1;
}
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

	DMSG("OpenSession has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Hello World!\n");

	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}


char* fibufnacci(const char* textprev, size_t textprevlen)
{
	char tmp[128]; 
	memcpy(&tmp[0], textprev, textprevlen); /*vuln*/
	DMSG("Return pointer: %p", tmp);
	DMSG("Return values: %s", tmp);
	return strdup(tmp);
}

static TEE_Result call_fibufnacci(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES
        (
            TEE_PARAM_TYPE_MEMREF_INOUT, 
            TEE_PARAM_TYPE_VALUE_INOUT, 
            TEE_PARAM_TYPE_MEMREF_INPUT, 
            TEE_PARAM_TYPE_VALUE_INPUT 
        );
	DMSG("=====================================================");

	if (param_types != exp_param_types)
    {
		DMSG("No check!\n");
	}
	DMSG("Hello, I'm VULN!\n");
	
	size_t size0 = reg_pair_to_64(params[1].value.a,params[1].value.b);
	size_t size2 = params[2].memref.size;
	DMSG("Size: %d", size2);
	if(size0 > params[0].memref.size)
	{		
		DMSG("refusing short input buffer a with size %u!", params[0].memref.size);
		return TEE_ERROR_SHORT_BUFFER;
	}
	
	char* result = fibufnacci
	(
		(char*)params[2].memref.buffer,size2
	);
	
	if(strlen(result)+1>params[0].memref.size)
	{
		DMSG("refusing short output buffer!");
		TEE_Free(result);
		return TEE_ERROR_SHORT_BUFFER;
	}
	strcpy((char*)params[0].memref.buffer, result);
	TEE_Free(result);
	if (PRINTSECRET)
	{
		DMSG("%s",SECRET);
	}
	DMSG("=====================================================");
	return TEE_SUCCESS;
}


static TEE_Result show_pointer(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES
        (
            TEE_PARAM_TYPE_VALUE_INOUT,
            TEE_PARAM_TYPE_NONE,
            TEE_PARAM_TYPE_NONE,
            TEE_PARAM_TYPE_NONE
        );

	DMSG("=====================================================");
	DMSG("Hello, I'm backdoor!\n");
	if (param_types != exp_param_types)
    {
		DMSG("No check!\n");
	} // return TEE_ERROR_BAD_PARAMETERS;
	DMSG("SECRET is %s", SECRET);
	DMSG("SECRET is at %p", (void*)SECRET);
	params[0].value.a = (int)SECRET;
	DMSG("Return buffer is %p", params[0].memref.buffer);
	if (PRINTSECRET)
	{
		DMSG("%s",SECRET);
	}
	return TEE_SUCCESS;
	DMSG("=====================================================");

}

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id) {
	case TA_TYPE_CONFUSION_CMD_INVOKE:
		return call_fibufnacci(param_types, params);
	case TA_BACKDOOR_CMD_INVOKE:
		return show_pointer(param_types, params);
		
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
