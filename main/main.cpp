#include <M5Stack.h>
#include "nvs_flash.h"
#include "esp_partition.h"
#include <esp_heap_caps.h>
#include "driver/i2s.h"
extern "C" {
#include "sn76496.h"
}

#define SAMPLING_RATE 44100

byte *vgm;
int vgmpos = 0x40;
bool vgmend = false;

byte *get_vgmdata() {
    byte* data;
    const esp_partition_t* part;
    spi_flash_mmap_handle_t hrom;
    esp_err_t err;
 
    nvs_flash_init();

    part=esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_PHY, NULL);
    if (part==0) {
        M5.Lcd.print("Couldn't find vgm part!\n");
    }

    err=esp_partition_mmap(part, 0, 0x1EF000, SPI_FLASH_MMAP_DATA, (const void**)&data, &hrom);
    if (err!=ESP_OK) {
        M5.Lcd.print("Couldn't map vgm part!\n");
    }
    M5.Lcd.printf("read vgm data @%p\n", data);

    return (byte*)data;
}

uint16_t parse_vgm()
{
    int wait = 0;
    switch (vgm[vgmpos]) {
        case 0x50:
            vgmpos++;
            SN76496Write(vgm[vgmpos]);
            vgmpos++;
            break;
        case 0x61:
            vgmpos++;
            wait = (vgm[vgmpos+1] << 8) + vgm[vgmpos];
            vgmpos += 2;
            break;
        case 0x62:
            vgmpos++;
            wait = 735;
            break;
        case 0x63:
            vgmpos++;
            wait = 882;
            break;
        case 0x66:
            vgmpos++;
            vgmend = true;
            break;
        case 0x4f:
            vgmpos += 2;
            break;
        case 0x51:
            vgmpos += 3;
            break;
        case 0x70:
            vgmpos++;
            wait = 1;
            break;
        case 0x71:
            vgmpos++;
            wait = 2;
            break;
        case 0x72:
            vgmpos++;
            wait = 3;
            break;
        case 0x73:
            vgmpos++;
            wait = 4;
            break;
        case 0x74:
            vgmpos++;
            wait = 5;
            break;
        case 0x75:
            vgmpos++;
            wait = 6;
            break;
        case 0x76:
            vgmpos++;
            wait = 7;
            break;
        case 0x77:
            vgmpos++;
            wait = 8;
            break;
        case 0x78:
            vgmpos++;
            wait = 9;
            break;
        case 0x79:
            vgmpos++;
            wait = 10;
            break;
        case 0x7A:
            vgmpos++;
            wait = 11;
            break;
        case 0x7B:
            vgmpos++;
            wait = 12;
            break;
        case 0x7C:
            vgmpos++;
            wait = 13;
            break;
        case 0x7D:
            vgmpos++;
            wait = 14;
            break;
        case 0x7E:
            vgmpos++;
            wait = 15;
            break;
        case 0x7F:
            vgmpos++;
            wait = 16;
            break;
        default:
            printf("unknown cmd at 0x%x: 0x%x\n", vgmpos, vgm[vgmpos]);
            vgmpos++;
            break;
    }

	return wait;
}

static const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number

void init_dac(void) {
    i2s_config_t i2s_config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = SAMPLING_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = 0,
        .dma_buf_count = 16,
        .dma_buf_len = 1024,
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
    M5.Lcd.print("PSG emulation by SN76496\n\n");

    // Load vgm data
    vgm = get_vgmdata();

    // Reset for NTSC Genesis/Megadrive
    SN76496_init(3579540, SAMPLING_RATE);

    // free memory
    M5.Lcd.printf("free memory: %d byte\n\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

    // // Init DAC
    init_dac();

    // LCD display
}

// The loop routine runs over and over again forever
void loop()
{
    TickType_t delay = 50 / portTICK_PERIOD_MS;
    size_t bytes_written = 0;

    int frame_size;
    int frame_all = 0;
    short csize = 0;
    short bsize = 0;
    short *buf;
    do {
        frame_size = parse_vgm();
        buf = (short *)heap_caps_malloc(frame_size * sizeof(short) * 2, MALLOC_CAP_8BIT);
        memset(buf, 0x00, frame_size * sizeof(short) * 2);
        SN76496Update(buf, frame_size, 1);
        if(frame_size != 0) {
            i2s_write((i2s_port_t)i2s_num, buf, frame_size * sizeof(short) * 2, &bytes_written, delay);
            csize = (unsigned short)buf[0] / 128;
            M5.Lcd.fillCircle(320 / 2, 240 / 2, bsize, BLACK);
            M5.Lcd.fillCircle(320 / 2, 240 / 2, csize, GREENYELLOW);
            bsize = csize;
            // printf("framesize %d, bytes_written %d\n", frame_size * sizeof(short) * 2, bytes_written);
        }
        free(buf);
        frame_all += frame_size;
    } while(!vgmend);

    M5.Lcd.printf("total frame: %d %d\n", frame_all, frame_all / SAMPLING_RATE);

    i2s_driver_uninstall((i2s_port_t)i2s_num); //stop & destroy i2s driver

    M5.update();

    while(true) {
        asm volatile("nop\n\t nop\n\t nop\n\t nop\n\t");
    }
}
