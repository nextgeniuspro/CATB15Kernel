#ifndef _CFG_HW_FILE_H
#define _CFG_HW_FILE_H

typedef struct
{
	char HW[64];
}File_HW_Struct;

#define CFG_FILE_HW_REC_SIZE    sizeof(File_HW_Struct)
#define CFG_FILE_HW_REC_TOTAL   1

#endif
