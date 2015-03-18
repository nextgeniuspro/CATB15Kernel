#ifndef _CFG_P_SENSOR_THRESHOLD_FILE_H
#define _CFG_P_SENSOR_THRESHOLD_FILE_H

typedef struct
{
        int data[20];
        unsigned int P_SENSOR_CALIBRATION;
        unsigned int PPCOUNT;
        unsigned int HIGHT_THRESHOLD;
        unsigned int LOW_THRESHOLD;
        unsigned int P_OFFSET;
}File_P_SENSOR_THRESHOLD_Struct;

#define CFG_P_SENSOR_THRESHOLD_RESULT_REC_SIZE    sizeof(File_P_SENSOR_THRESHOLD_Struct)
#define CFG_P_SENSOR_THRESHOLD_REC_TOTAL   1

#endif

