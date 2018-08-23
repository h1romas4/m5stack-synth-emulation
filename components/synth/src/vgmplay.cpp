// for vgm testing
// make clean && make && ./vgmplay && ffplay -f s16le -ar 44100 -ac 2 ../../vgm/s16le.pcm
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "ym2612.hpp"
extern "C" {
#include "sn76489.h"
}
#include "vgmplay.h"

#define SAMPLING_RATE 44100
#define FRAME_SIZE_MAX 4096

#define STEREO 2
#define MONO 0

VGM_HEADER *vgmheader;
u_int8_t *vgm;
u_int32_t vgmpos = 0x40;
u_int32_t datpos;
u_int32_t pcmpos;
u_int32_t pcmoffset;

bool vgmend = false;

u_int32_t clock_sn76489;
u_int32_t clock_ym2612;

SN76489_Context *sn76489;

void vgm_load(void) {
    vgm = (unsigned char *) malloc(3000000);
    int fd = open("../../vgm/02.vgm", O_RDONLY);
    assert(fd != -1);
    read(fd, vgm, 3000000);
    close(fd);

    vgmheader = (VGM_HEADER *)vgm;
    printf("version %x\n", vgmheader->lngVersion);
    if(vgmheader->lngVersion >= 0x150) {
        vgmheader->lngDataOffset += 0x00000034;
    }

    clock_sn76489 = vgmheader->lngHzPSG;
    clock_ym2612 = vgmheader->lngHzYM2612;
    if(clock_ym2612 == 0) clock_ym2612 = 7670453;
    if(clock_sn76489 == 0) clock_ym2612 = 3579545;

    vgmpos = vgmheader->lngDataOffset;

    printf("vgmpos %x\n", vgmheader->lngDataOffset);
    printf("clock_sn76489 %d\n", clock_sn76489);
    printf("clock_ym2612 %d\n", clock_ym2612);
}

u_int8_t get_vgm_ui8()
{
    return vgm[vgmpos++];
}

u_int16_t get_vgm_ui16()
{
    return get_vgm_ui8() + (get_vgm_ui8() << 8);
}

u_int32_t get_vgm_ui32()
{
    return get_vgm_ui8() + (get_vgm_ui8() << 8) + (get_vgm_ui8() << 16) + (get_vgm_ui8() << 24);
}

u_int16_t parse_vgm()
{
    u_int8_t command;
    u_int16_t wait = 0;
    u_int8_t reg;
    u_int8_t dat;

    command = get_vgm_ui8();

    switch (command) {
        case 0x50:
            dat = get_vgm_ui8();
            SN76489_Write(sn76489, dat);
            break;
        case 0x52:
        case 0x53:
            reg = get_vgm_ui8();
            dat = get_vgm_ui8();
            YM2612_Write(0 + ((command & 1) << 1), reg);
            YM2612_Write(1 + ((command & 1) << 1), dat);
            break;
        case 0x61:
            wait = get_vgm_ui16();
            break;
        case 0x62:
            wait = 735;
            break;
        case 0x63:
            wait = 882;
            break;
        case 0x66:
            vgmend = true;
            break;
        case 0x67:
            get_vgm_ui8(); // 0x66
            get_vgm_ui8(); // 0x00 data type
            datpos = vgmpos + 4;
            vgmpos += get_vgm_ui32(); // size of data, in bytes
            break;
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
            wait = (command & 0x0f) + 1;
            break;
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
        case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
            wait = (command & 0x0f);
            YM2612_Write(0, 0x2a);
            YM2612_Write(1, vgm[datpos + pcmpos + pcmoffset]);
            pcmoffset++;
            break;
        case 0xe0:
            pcmpos = get_vgm_ui32();
            pcmoffset = 0;
            break;
        default:
            printf("unknown cmd at 0x%x: 0x%x\n", vgmpos, vgm[vgmpos]);
            vgmpos++;
            break;
    }

	return wait;
}

short audio_write_sound_stereo(int sample32)
{
    short sample16;

    if (sample32 < -0x7FFF)
        sample16 = -0x7FFF;
    else if (sample32 > 0x7FFF)
        sample16 = 0x7FFF;
    else
        sample16 = (short)(sample32);

    return sample16;
}

// The loop routine runs over and over again forever
int main(void)
{
    vgm_load();

    // // Reset for NTSC Genesis/Megadrive
    sn76489 = SN76489_Init(clock_sn76489, SAMPLING_RATE);
    SN76489_Reset(sn76489);
    YM2612_Init(clock_ym2612, SAMPLING_RATE, 0);

    // malloc sound buffer
    int **buflr;

    buflr = (int **)malloc(sizeof(int) * STEREO);
    assert(buflr != NULL);
    buflr[0] = (int *)malloc(sizeof(int) * FRAME_SIZE_MAX);
    assert(buflr[0] != NULL);
    buflr[1] = (int *)malloc(sizeof(int) * FRAME_SIZE_MAX);
    assert(buflr[1] != NULL);

    size_t bytes_written = 0;
    u_int16_t frame_size = 0;
    u_int16_t frame_size_count = 0;
    u_int32_t frame_all = 0;

    int fd = open("../../vgm/s16le.pcm", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    assert(fd != -1);

    int32_t last_frame_size;
    int32_t update_frame_size;
    do {
        frame_size = parse_vgm();
        last_frame_size = frame_size;
        do {
            if(last_frame_size > FRAME_SIZE_MAX) {
                update_frame_size = FRAME_SIZE_MAX;
            } else {
                update_frame_size = last_frame_size;
            }
            // get sampling
            SN76489_Update(sn76489, (int **)buflr, update_frame_size);
            YM2612_Update((int **)buflr, update_frame_size);
            YM2612_DacAndTimers_Update((int **)buflr, update_frame_size);
            for(uint32_t i = 0; i < update_frame_size; i++) {
                short d[STEREO];
                d[0] = audio_write_sound_stereo(buflr[0][i]);
                d[1] = audio_write_sound_stereo(buflr[1][i]);
                write(fd, d, sizeof(short) * STEREO);
            }
            last_frame_size -= FRAME_SIZE_MAX;
        } while(last_frame_size > 0);
        frame_all += frame_size;
    } while(!vgmend);

    close(fd);

    free(buflr[0]);
    free(buflr[1]);
    free(buflr);

    YM2612_End();
    SN76489_Shutdown(sn76489);

    printf("end!\n");

    return 0;
}
