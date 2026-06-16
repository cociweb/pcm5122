#pragma once

namespace esphome
{
  namespace pcm5122
  {

    enum ControlState : uint8_t
    {
      CTRL_PWDN = 0x01,       // Power down
      CTRL_STBY = 0x10,       // Stand by
      CTRL_PLAY = 0x00,       // Play
    };

    struct Pcm5122Configuration
    {
      uint8_t offset;
      uint8_t value;
    } __attribute__((packed));

    #define PCM51XX_REG_00 0x00
    #define PCM51XX_REG_STATE 0x02
    #define PCM51XX_REG_MUTE 0x03
    #define PCM51XX_REG_VALUE_MUTE 0x11
    #define PCM51XX_REG_VALUE_UNMUTE 0x00

    #define PCM51XX_REG_VOL_L 0X3D
    #define PCM51XX_REG_VOL_R 0X3E
    
    #define PCM51XX_PAGE_ZERO 0x00
    #define PCM51XX_PAGE_ONE 0x01
    #define PCM51XX_REG_AGAIN 0x02
    #define PCM51XX_REG_AGAIN_0DB 0x00
    #define PCM51XX_REG_AGAIN_MINUS6DB 0x11

    #define PCM51XX_REG_MIXER 0x2a
    #define PCM51XX_REG_MIXER_VAL_MUTE 0b00000000
    #define PCM51XX_REG_MIXER_VAL_STEREO 0b00010001
    #define PCM51XX_REG_MIXER_VAL_STEREO_INV 0b00100010
    #define PCM51XX_REG_MIXER_VAL_RIGHT 0b00100001
    #define PCM51XX_REG_MIXER_VAL_LEFT 0b00010010

    #define PCM5122_REG_PAGE_SET 0x00
    #define PCM5122_CFG_META_DELAY 0xFF

    static const uint8_t PCM5122_REG_RESET = 0x01;
    static const uint8_t PCM5122_REG_PLL_REF = 0x0D;
    static const uint8_t PCM5122_REG_ERROR_DETECT = 0x25;
    static const uint8_t PCM5122_REG_AUDIO_FORMAT = 0x28;
    static const uint8_t PCM5122_RESET_MODULES = 0x10;
    static const uint8_t PCM5122_AUDIO_FORMAT_I2S = 0x00;
    static const uint8_t PCM5122_AUDIO_FORMAT_ALEN_16BIT = 0x00;
    static const uint8_t PCM5122_AUDIO_FORMAT_ALEN_24BIT = 0x02;
    static const uint8_t PCM5122_AUDIO_FORMAT_ALEN_32BIT = 0x03;
    static const uint8_t PCM5122_ERROR_DETECT_IGNORE_CLKHALT = (1 << 3);
    static const uint8_t PCM5122_ERROR_DETECT_DISABLE_DIV_AUTOSET = (1 << 1);
    static const uint8_t PCM5122_PLL_REF_MASK = (7 << 4);
    static const uint8_t PCM5122_PLL_REF_SOURCE_BCK = (1 << 4);

    typedef struct
    {
      uint8_t offset;
      uint8_t value;
    } pcm51xx_cfg_reg_t;

    enum MixerMode : uint8_t {
      STEREO = 0,
      STEREO_INVERSE,
      RIGHT,
      LEFT,
    };

    enum Pcm5122ClockMode : uint8_t {
      CLOCK_MODE_AUTO = 0,
      CLOCK_MODE_BCK,
    };

    enum Pcm5122BitsPerSample : uint8_t {
      PCM5122_BITS_PER_SAMPLE_16 = 16,
      PCM5122_BITS_PER_SAMPLE_24 = 24,
      PCM5122_BITS_PER_SAMPLE_32 = 32,
    };

    static const pcm51xx_cfg_reg_t pcm51xx_init_seq[] = {

        // EXIT SHUTDOWN STATE
        {PCM5122_REG_PAGE_SET, 0x00}, // SELECT PAGE 0
        {PCM51XX_REG_MUTE, 0x00}, // UNMUTE
        {PCM51XX_REG_MIXER, 0x11}, // DAC DATA PATH L->ch1, R->ch2
        {PCM51XX_REG_STATE, 0x00}, // ENTER PLAY MODE
        {0x0d, 0x10}, // BCK as SRC for PLL
        {0x25, 0x08}, // IGNORE MISSING MCLK
    };

  } // namespace pcm5122
} // namespace esphome