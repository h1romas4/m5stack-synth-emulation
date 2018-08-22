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
#include "sn76496.h"
}
#include "vgmplay.h"

#define SAMPLING_RATE 44100
#define FRAME_SIZE_MAX 512

#define STEREO 2
#define MONO 0

VGM_HEADER *vgmheader;
u_int8_t *vgm;
u_int32_t vgmpos = 0x40;
bool vgmend = false;

u_int32_t clock_sn76489;
u_int32_t clock_ym2612;

void vgm_load(void) {
    vgm = (unsigned char *) malloc(3000000);
    int fd = open("../../vgm/03.vgm", O_RDONLY);
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
    vgmpos = vgmheader->lngDataOffset;

    printf("vgmpos %x\n", vgmheader->lngDataOffset);
    printf("clock_sn76489 %d\n", clock_sn76489);
    printf("clock_ym2612 %d\n", clock_ym2612);
}

u_int8_t get_vgm_ui8()
{
    u_int8_t ret = vgm[vgmpos++];
    return ret;
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
            SN76496Write(dat);
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
            printf("PCM not implement\n");
            printf("pcm %x\n", get_vgm_ui8()); // 0x66
            printf("pcm %x\n", get_vgm_ui8()); // data type
            vgmpos += get_vgm_ui32(); // size of data, in bytes
            break;
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
            wait = (command & 0x0f) + 1;
            break;
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
        case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
            printf("PCM not implement1\n");
            wait = (command & 0x0f);
            break;
        case 0xe0:
            printf("PCM not implement2\n");
            get_vgm_ui32();
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
    SN76496_init(clock_sn76489, SAMPLING_RATE);
    YM2612_Init(clock_ym2612, SAMPLING_RATE, 0);

    // malloc sound buffer
    int **buflr;
    short *bufmn;
    int **bufdummy;

    buflr = (int **)malloc(sizeof(int) * STEREO);
    assert(buflr != NULL);
    buflr[0] = (int *)malloc(sizeof(int) * FRAME_SIZE_MAX);
    assert(buflr[0] != NULL);
    buflr[1] = (int *)malloc(sizeof(int) * FRAME_SIZE_MAX);
    assert(buflr[1] != NULL);
    bufmn = (short *)malloc(sizeof(short) * FRAME_SIZE_MAX);
    assert(bufmn != NULL);

    size_t bytes_written = 0;
    u_int16_t frame_size = 0;
    u_int16_t frame_size_count = 0;
    u_int32_t frame_all = 0;

    int fd = open("../../vgm/s16le.pcm", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    assert(fd != -1);

    do {
        frame_size = parse_vgm();
        if(frame_size == 0) {
            YM2612_Update((int **)bufdummy, 0);
            SN76496Update((short *)bufdummy, 0, 0);
        } else {
            frame_size_count += frame_size;
        }
        while(frame_size_count >= FRAME_SIZE_MAX || vgmend) {
            int16_t frame_update_count;
            if(frame_size_count - FRAME_SIZE_MAX > 0) {
                frame_update_count = FRAME_SIZE_MAX;
            } else {
                frame_update_count = frame_size_count;
            }
            YM2612_ClearBuffer((int **)buflr, frame_update_count);
            YM2612_Update((int **)buflr, frame_update_count);
            memset((short *)bufmn, 0, sizeof(short) * frame_update_count);
            SN76496Update((short *)bufmn, frame_update_count, 0);
            for(int i = 0; i < frame_update_count; i++) {
                short d[2];
                d[0] = audio_write_sound_stereo(buflr[0][i] + bufmn[i]);
                d[1] = audio_write_sound_stereo(buflr[1][i] + bufmn[i]);
                write(fd, d, sizeof(short) * STEREO);
            }
            frame_size_count -= frame_update_count;
            if(frame_size_count <= 0) break;
        }
        frame_all += frame_size;
    } while(!vgmend);

    close(fd);

    free(buflr[0]);
    free(buflr[1]);
    free(buflr);
    free(bufmn);

    YM2612_End();

    printf("end!\n");

    return 0;
}
