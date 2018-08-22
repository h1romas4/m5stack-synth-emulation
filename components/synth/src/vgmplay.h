// Header file for VGM file handling

typedef struct _vgm_file_header
{
	u_int32_t fccVGM;
	u_int32_t lngEOFOffset;
	u_int32_t lngVersion;
	u_int32_t lngHzPSG;
	u_int32_t lngHzYM2413;
	u_int32_t lngGD3Offset;
	u_int32_t lngTotalSamples;
	u_int32_t lngLoopOffset;
	u_int32_t lngLoopSamples;
	u_int32_t lngRate;
	u_int16_t shtPSG_Feedback;
	u_int8_t bytPSG_SRWidth;
	u_int8_t bytPSG_Flags;
	u_int32_t lngHzYM2612;
	u_int32_t lngHzYM2151;
	u_int32_t lngDataOffset;
} VGM_HEADER;
