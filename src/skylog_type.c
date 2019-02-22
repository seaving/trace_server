#include "common.h"
#include "skylog_type.h"

/*
	顺序必须和枚举顺序一致
*/
static char _skylog_name[][64] = {
	"Crash", 
	"Error", 
	"Log", 
	"Debug", 
	"Flow", 
	"DNS",
	"UnkownType"
};

const char *get_skylog_type_name(skylog_type_t type)
{
	if (E_SKY_LOG_TYPE_CRASH <= type 
		&& type <= E_SKY_LOG_TYPE_UNKOWN)
	{
		return _skylog_name[type];
	}

	return NULL;
}


