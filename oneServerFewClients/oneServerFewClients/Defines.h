#pragma once
#define DEFAULT_BUFLEN 512
#define MAX_CLIENTS 500

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "27015"

#define DATA_PATH "hb.txt"
#define DATA_NAME "data/heartbeats"
#define DATA_SUFFIX ".txt"


#define SEC_IN_MILLI 1000

#define SEP " "
#define L_SEP "\n"

#define SAMPLES 20

#define PRNT_ERR(err,file,line) do{ \
		printf("Error: %s(%d) has accured in %s in line: %d\n", #err, err, file, line); \
	}while(0)