#pragma once

typedef enum {
	SUCCESS,
	DONE,
	ERR_NULL_PTR,
	ERR_MEMORY,
	ERR_COMMU,
	ERR_FILE,
	ERR_THREAD,
	SERVER_FULL,
	WAIT = -999
}Result;