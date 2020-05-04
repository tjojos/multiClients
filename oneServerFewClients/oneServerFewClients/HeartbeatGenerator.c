#include "HeartbeatGenerator.h"

#define HEART_BEATS 50 //per packet
#define MAX_HB 200
#define MIN_HB 50


Result generateHeartbeats(const char* path,int heartbeatsPerSample, int samples, int low, int high) {
    /*
        @param: 0 or NULL = DEFAULT VAL
    */

    if (!path)
        path = DATA_PATH;
    if (heartbeatsPerSample == 0)
        heartbeatsPerSample = HEART_BEATS;
    if (samples == 0)
        samples = SAMPLES;
    if (low == 0)
        low = MIN_HB;
    if (high == 0)
        high = MAX_HB;


    FILE* f = NULL;
    if (fopen_s(&f, path, "w") == EINVAL || f == NULL) {
        printf("File error: errno is %d\n", errno);
        return ERR_FILE;
    }
        

    srand(time(NULL));
    for (int min = 0; min < samples; min++) {
        for (int i = 0; i < heartbeatsPerSample; i++) {
            int hb = (rand() % (high - low + 1)) + low;

            fprintf(f, "%d", hb);
            fprintf(f, SEP);
        }
        fprintf(f, L_SEP);
    }

    fclose(f);

    return SUCCESS;
}