#include <M5Stack.h>
#include "nvs_flash.h"
#include "esp_partition.h"
#include <esp_heap_caps.h>
#include "driver/i2s.h"
#include "ym2612.hpp"
extern "C" {
#include "sn76496.h"
}

#define SAMPLING_RATE 44100
#define STEREO 2
#define MONO 0

#ifdef YM2612
#define VGM_DATA_POS 0x80;
#else
#define VGM_DATA_POS 0x40;
#endif

uint8_t *vgm;
uint32_t vgmpos;
bool vgmend = false;
bool play = false;

uint8_t *get_vgmdata()
{
    uint8_t* data;
    const esp_partition_t* part;
    spi_flash_mmap_handle_t hrom;
    esp_err_t err;

    nvs_flash_init();

    part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_PHY, NULL);
    if (part == 0) {
        printf("Couldn't find vgm part!\n");
    }

    err = esp_partition_mmap(part, 0, 0x1EF000, SPI_FLASH_MMAP_DATA, (const void**)&data, &hrom);
    if (err != ESP_OK) {
        printf("Couldn't map vgm part!\n");
    }
    printf("read vgm data @%p\n", data);

    return (uint8_t *)data;
}

uint8_t get_vgm_ui8()
{
    uint8_t ret = vgm[vgmpos];
    vgmpos++;
    return ret;
}

uint16_t get_vgm_ui16()
{
    return get_vgm_ui8() + (get_vgm_ui8() << 8);
}

uint16_t parse_vgm()
{
    uint8_t command;
    uint16_t wait = 0;
    uint8_t reg;
    uint8_t dat;

    command = get_vgm_ui8();
    switch (command) {
        case 0x50:
            dat = get_vgm_ui8();
            if(play) SN76496Write(dat);
            break;
        case 0x52:
        case 0x53:
            reg = get_vgm_ui8();
            dat = get_vgm_ui8();
            if(play) {
                YM2612_Write(0 + ((command & 1) << 1), reg);
                YM2612_Write(1 + ((command & 1) << 1), dat);
            }
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
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
            wait = (command & 0x0f) + 1;
            break;
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
        case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
            printf("PCM not implement");
            break;
        case 0xe0:
            printf("PCM not implement");
            get_vgm_ui8(); get_vgm_ui8(); get_vgm_ui8(); get_vgm_ui8();
            break;
        default:
            printf("unknown cmd at 0x%x: 0x%x\n", vgmpos, vgm[vgmpos]);
            vgmpos++;
            break;
    }

	return wait;
}

uint16_t parse_max_frame()
{
    uint16_t frame_size;
    uint16_t frame_max_size = 0;

    play = false;
    vgmpos = VGM_DATA_POS;

    do {
        frame_size = parse_vgm();
        if(frame_max_size < frame_size) {
            frame_max_size = frame_size;
        }
    } while(!vgmend);

    play = true;
    vgmend = false;
    vgmpos = VGM_DATA_POS;

    return frame_max_size;
}

static const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number

void init_dac(void)
{
    i2s_config_t i2s_config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = SAMPLING_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = 0,
        .dma_buf_count = 16,
        .dma_buf_len = 512,
        .use_apll = false,
        .fixed_mclk = 0
    };

    i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
    i2s_set_pin(i2s_num, NULL);
}

// The setup routine runs once when M5Stack starts up
void setup()
{
    // Initialize the M5Stack object
    M5.begin();

    // Initialize
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.print("MEGADRIVE/GENESIS sound emulation by SN76496/YM2612.\n\n");

    // Load vgm data
    vgm = get_vgmdata();

    // Reset for NTSC Genesis/Megadrive
    SN76496_init(3579540, SAMPLING_RATE);
    #ifdef YM2612
    YM2612_Init(8000000, SAMPLING_RATE, 0);
    #endif

    // // Init DAC
    init_dac();
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
void loop()
{
    size_t bytes_written = 0;

    uint16_t frame_size;
    uint32_t frame_all = 0;

    uint16_t buffer_max_size;
    short *bufmn;
    #ifdef YM2612
    int **buflr;
    #endif

    // parse max frame
    buffer_max_size = parse_max_frame();
    printf("buffer_max_size %d\n", buffer_max_size);

    // malloc sound buffer
    #ifdef YM2612
    buflr = (int **)malloc(sizeof(int *) * STEREO);
    buflr[0] = (int *)heap_caps_malloc(buffer_max_size * sizeof(int), MALLOC_CAP_32BIT);
    if(buflr[0] == NULL) printf("pcm buffer0 alloc fail.\n");
    buflr[1] = (int *)heap_caps_malloc(buffer_max_size * sizeof(int), MALLOC_CAP_32BIT);
    if(buflr[1] == NULL) printf("pcm buffer1 alloc fail.\n");
    #else
    bufmn = (short *)heap_caps_malloc(buffer_max_size * sizeof(short), MALLOC_CAP_8BIT);
    if(bufmn == NULL) printf("pcm buffer3 alloc fail.\n");
    #endif

    printf("last free memory8 %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    printf("last free memory32 %d\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));

    // free memory
    M5.Lcd.printf("frame max size: %d\n", buffer_max_size);
    M5.Lcd.printf("free memory: %d byte\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

    do {
        frame_size = parse_vgm();
        #ifdef YM2612
        memset(buflr[0], 0x00, frame_size * sizeof(int));
        memset(buflr[1], 0x00, frame_size * sizeof(int));
        YM2612_Update(buflr, frame_size);
        #else
        memset(bufmn, 0x00, frame_size * sizeof(short));
        SN76496Update((short *)bufmn, frame_size, MONO);
        #endif
        if(frame_size != 0) {
            for(int16_t i = 0; i < frame_size; i++) {
                short d[STEREO];
                #ifdef YM2612
                d[0] = audio_write_sound_stereo(buflr[0][i]);
                d[1] = 0x00;
                #else
                d[0] = bufmn[i];
                d[1] = 0x00;
                #endif
                i2s_write((i2s_port_t)i2s_num, d, sizeof(short) * STEREO, &bytes_written, portMAX_DELAY);
                // printf("bytes_written %d\n", bytes_written);
            }
        }
        frame_all += frame_size;
    } while(!vgmend);

    #ifdef YM2612
    free(buflr[0]);
    free(buflr[1]);
    free(buflr);

    YM2612_End();
    #else
    free(bufmn);
    #endif

    M5.Lcd.printf("\ntotal frame: %d %d\n", frame_all, frame_all / SAMPLING_RATE);

    i2s_driver_uninstall((i2s_port_t)i2s_num); //stop & destroy i2s driver

    M5.update();

    while(true) {
        asm volatile("nop\n\t nop\n\t nop\n\t nop\n\t");
    }
}
