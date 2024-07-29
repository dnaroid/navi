#pragma once

#include <PinMap.h>

#include "sccb.h"

enum OV_FX {
	OV_FX_MIN = 0,
	OV_FX_NORMAL = 0,
	OV_FX_ANTIQUE = 1,
	OV_FX_MORE_BLUE = 2,
	OV_FX_MORE_GREEN = 3,
	OV_FX_MORE_RED = 4,
	OV_FX_B_AND_W = 5,
	OV_FX_B_AND_W_NEGATIVE = 6,
	OV_FX_MAX = 6,
};

enum OV_BRIGHTNESS {
	OV_BRIGHTNESS_MIN = 0,
	OV_BRIGHTNESS_MINUS_2 = 0,
	OV_BRIGHTNESS_MINUS_1 = 1,
	OV_BRIGHTNESS_NORMAL = 2,
	OV_BRIGHTNESS_PLUS_1 = 3,
	OV_BRIGHTNESS_PLUS_2 = 4,
	OV_BRIGHTNESS_MAX = 4,
};


enum OV_CONTRAST {
	OV_CONTRAST_MIN = 0,
	OV_CONTRAST_MINUS_2 = 0,
	OV_CONTRAST_MINUS_1 = 1,
	OV_CONTRAST_NORMAL = 2,
	OV_CONTRAST_PLUS_1 = 3,
	OV_CONTRAST_PLUS_2 = 4,
	OV_CONTRAST_MAX = 4,
};

enum OV_SCREEN_SIZE {
	OV_SCR_VGA = 0, // 640 * 480
	OV_SCR_QVGA = 1, // 320 * 240
	OV_SCR_QQVGA = 2, // 160 * 120
	OV_SCR_UNKNOWN = 255
};

enum OV_PIXEL_MODE {
	OV_PM_RAW_BAYER = 0,
	OV_PM_PROC_BAYER = 1,
	OV_PM_YUV422 = 2,
	OV_PM_RGB565 = 3,
	OV_PM_RGB555 = 4,
	OV_PM_UNKNOWN = 255
};

enum OV_WB {
	OV_WB_MIN = 0,
	OV_WB_NONE = 0,
	OV_WB_AUTO_SIMPLE = 1,
	OV_WB_SUNNY = 2,
	OV_WB_CLOUDY = 3,
	OV_WB_OFFICE = 4,
	OV_WB_HOME = 5,
	OV_WB_MAX = 5
};

#define OV_I2C_ADDR 0x42

#define OV_REG_AWBCTR0 0x6F

#define OV_REG_GAIN 0x00 // Gain lower 8 bits (rest in vref)
#define OV_REG_BLUE 0x01 // blue gain
#define OV_REG_RED 0x02 // red gain
#define OV_REG_VREF 0x03 // Pieces of GAIN, VSTART, VSTOP
#define OV_REG_COM1 0x04 // Control 1

#define OV_REG_BAVE 0x05 // U/B Average level
#define OV_REG_GbAVE 0x06 // Y/Gb Average level
#define OV_REG_AECHH 0x07 // AEC MS 5 bits
#define OV_REG_RAVE 0x08 // V/R Average level
#define OV_REG_COM2 0x09 // Control 2

#define OV_REG_PID 0x0a // Product ID MSB
#define OV_REG_VER 0x0b // Product ID LSB
#define OV_REG_COM3 0x0c // Control 3

#define OV_REG_COM4 0x0d // Control 4
#define OV_REG_COM5 0x0e // All "reserved"
#define OV_REG_COM6 0x0f // Control 6
#define OV_REG_AECH 0x10 // More bits of AEC value
#define OV_REG_CLKRC 0x11 // Clock control

#define OV_REG_COM7 0x12 // Control 7

#define OV_REG_COM8 0x13 // Control 8
#define OV_REG_COM9 0x14 // Control 9 - gain ceiling
#define OV_REG_COM10 0x15 // Control 10

#define OV_REG_HSTART 0x17 // Horiz start high bits
#define OV_REG_HSTOP 0x18 // Horiz stop high bits
#define OV_REG_VSTART 0x19 // Vert start high bits
#define OV_REG_VSTOP 0x1a // Vert stop high bits
#define OV_REG_PSHFT 0x1b // Pixel delay after HREF
#define OV_REG_MIDH 0x1c // Manuf. ID high
#define OV_REG_MIDL 0x1d // Manuf. ID low
#define OV_REG_MVFP 0x1e // Mirror / vflip

#define OV_REG_AEW 0x24 // AGC upper limit
#define OV_REG_AEB 0x25 // AGC lower limit
#define OV_REG_VPT 0x26 // AGC/AEC fast mode op region

#define OV_REG_EXHCH  0x2A
#define OV_REG_EXHCL  0x2B
#define OV_REG_DM_LNL 0x92
#define OV_REG_DM_LNH 0x93

#define OV_REG_HSYST 0x30 // HSYNC rising edge delay
#define OV_REG_HSYEN 0x31 // HSYNC falling edge delay
#define OV_REG_HREF 0x32 // HREF pieces
#define OV_REG_TSLB 0x3a // lots of stuff

#define OV_REG_COM11 0x3b // Control 11

#define OV_REG_COM12 0x3c // Control 12
#define OV_COM12_HREF 0x80 // HREF always
#define OV_REG_COM13 0x3d // Control 13

#define OV_REG_COM14 0x3e // Control 14

#define OV_REG_EDGE 0x3f // Edge enhancement factor
#define OV_REG_COM15 0x40 // Control 15

#define OV_REG_COM16 0x41 // Control 16

#define OV_REG_COM17 0x42 // Control 17

#define OV_REG_DENOISE_STRENGTH 0x4c // De-noise strength


//--------------------------------------------------------------------------------------
// This matrix defines how the colors are generated, must be
// tweaked to adjust hue and saturation.
//
// Order: v-red, v-green, v-blue, u-red, u-green, u-blue
//
// They are nine-bit signed quantities, with the sign bit
// stored in 0x58. Sign for v-red is bit 0, and up from there.
//--------------------------------------------------------------------------------------
#define OV_REG_CMATRIX_BASE 0x4f
#define OV_REG_CMATRIX(x) (OV_REG_CMATRIX_BASE + x - 1)
#define OV_CMATRIX_LEN 6

#define OV_REG_CMATRIX_SIGN 0x58
#define OV_REG_BRIGHT 0x55 // Brightness
#define OV_REG_CONTRAST 0x56 // Contrast control

#define OV_REG_MANU 0x67 // special effects register 1
#define OV_REG_MANV 0x68 // special effects register 2

#define OV_REG_GFIX 0x69 // Fix gain control
#define OV_REG_GGAIN 0x6a // G channel AWB gain
#define OV_REG_DBLV 0x6b // PLL control

// size registers

#define OV_REG_SCALING_XSC 0x70
#define OV_REG_SCALING_YSC 0x71
#define OV_REG_SCALING_DCWCTR 0x72
#define OV_REG_SCALING_PCLK_DIV 0x73
#define OV_REG_SCALING_PCLK_DELAY 0xa2

#define OV_REG_REG76 0x76 // OV's name

#define OV_REG_RGB444 0x8c // RGB 444 control

#define OV_REG_HAECC1 0x9f // Hist AEC/AGC control 1
#define OV_REG_HAECC2 0xa0 // Hist AEC/AGC control 2
#define OV_REG_BD50MAX 0xa5 // 50hz banding step limit */
#define OV_REG_HAECC3 0xa6 // Hist AEC/AGC control 3 */
#define OV_REG_HAECC4 0xa7 // Hist AEC/AGC control 4 */
#define OV_REG_HAECC5 0xa8 // Hist AEC/AGC control 5 */
#define OV_REG_HAECC6 0xa9 // Hist AEC/AGC control 6 */
#define OV_REG_HAECC7 0xaa // Hist AEC/AGC control 7 */
#define OV_REG_BD60MAX 0xab // 60hz banding step limit */

// scaling

#define OV_REG_SCALING_XSC 0x70
#define OV_REG_SCALING_YSC 0x71
#define OV_REG_SCALING_DCWCTR 0x72
#define OV_REG_SCALING_PCLK_DIV 0x73
#define OV_REG_SCALING_PCLK_DELAY 0xA2

#define OV_REG_SATCTR 0xc9


#define OV_EM 0xf0


/*
Bayer filler pattern

if 640 * 480 raw bayer thn have a 640*480*8 bits NOT 640*480*RGB

Bayer matrix is

BG BG BG ..
GR GR GR ..

BG BG BG ..
GR GR GR ..

etc
*/


/*
Y′UV422 to RGB888 conversion
Input: Read 4 bytes of Y′UV (u, y1, v, y2 )
Output: Writes 6 bytes of RGB (R, G, B, R, G, B)
u  = yuv[0];
y1 = yuv[1];
v  = yuv[2];
y2 = yuv[3];
*/


// Command and Parameter related Strings

class cOV7670 {
private :
	uint8_t m_waiting = 0; // internal
	volatile uint8_t m_captured = 0;
	int16_t m_width; // width in pixels
	int16_t m_height; // height in pixels
	uint8_t m_bytes_per_pixel;
	OV_PIXEL_MODE m_pixel_mode;
	OV_SCREEN_SIZE m_screen_size;
	OV_BRIGHTNESS m_brightness = OV_BRIGHTNESS_NORMAL;
	OV_CONTRAST m_contrast = OV_CONTRAST_NORMAL;
	uint8_t m_denoise = 0;
	uint8_t m_edge_enhancement = 0;

	cSCCB m_wire; // communicate commands with camera (dedicated small lib) ... remember the pullups on SCK and SDA

	strFastPin m_vsync;

	// fifo interface

	strFastPin m_fifo_wr;
	strFastPin m_fifo_rck;
	strFastPin m_fifo_wrst;
	strFastPin m_fifo_rrst;

public:
	void WantFrame(); // call this when want a frame

	uint8_t isFrameReady() {
		if (!m_captured && !m_waiting) {
			WantFrame();
		}
		return m_captured;
	}

	// http://athulyasimon.github.io/project_portfolio/projects/a_camera_pic/#Image Retrieval


	inline void FIFODiscardByte() {
		fastDigitalWrite(m_fifo_rck, HIGH);
		fastDigitalWrite(m_fifo_rck, LOW);
	} // pops 1 byte off the FIFO buffer and discards

	inline void FIFODiscardWord() {
		fastDigitalWrite(m_fifo_rck, HIGH);
		fastDigitalWrite(m_fifo_rck, LOW);
		fastDigitalWrite(m_fifo_rck, HIGH);
		fastDigitalWrite(m_fifo_rck, LOW);
	} // pops 1 word off the FIFO buffer and discards

	inline uint8_t FIFOReadByte() {
		fastDigitalWrite(m_fifo_rck, HIGH);
		fastDigitalWrite(m_fifo_rck, LOW);

		return fastDigitalRead8_1();
	}

	inline uint16_t FIFOReadWord() {
		fastDigitalWrite(m_fifo_rck, HIGH);
		fastDigitalWrite(m_fifo_rck, LOW);

		uint16_t ret = fastDigitalRead8_1() << 8;

		fastDigitalWrite(m_fifo_rck, HIGH);
		fastDigitalWrite(m_fifo_rck, LOW);

		return ret | fastDigitalRead8_1();
	}

	inline uint16_t FIFOReadWordRev() {
		fastDigitalWrite(m_fifo_rck, HIGH);
		fastDigitalWrite(m_fifo_rck, LOW);

		uint16_t ret = fastDigitalRead8_1();

		fastDigitalWrite(m_fifo_rck, HIGH);
		fastDigitalWrite(m_fifo_rck, LOW);

		return ret | (fastDigitalRead8_1() << 8);
	}

	inline int FIFOGetClkPin() { return m_fifo_rck.pin; }

	void begin(uint8_t sda, uint8_t scl, uint8_t vsync, uint8_t fifo_wr, uint8_t fifo_wrst, uint8_t fifo_rrst, uint8_t fifo_rck);

	int getVersion() { return (int)m_wire.readSlaveRegister(OV_I2C_ADDR, OV_REG_PID) << 8 | (int)m_wire.readSlaveRegister(OV_I2C_ADDR, OV_REG_VER); }
	OV_PIXEL_MODE getPixelMode() const { return m_pixel_mode; }
	int16_t getWidth() const { return m_width; }
	int16_t getHeight() const { return m_height; }
	uint8_t getBytesPerPixel() const { return m_bytes_per_pixel; }

	void cameraResetRegisters();

	bool setMode(OV_SCREEN_SIZE screen_size, OV_PIXEL_MODE pixel_mode);

	void setAEC(bool average = true);
	void setBrightness(OV_BRIGHTNESS brightness = OV_BRIGHTNESS_NORMAL);
	void setContrast(OV_CONTRAST contrast = OV_CONTRAST_NORMAL);
	void setNightMode(bool enable = false);
	void setFX(OV_FX fx = OV_FX_NORMAL);
	void setEdgeEnhancement(uint8_t value = 0);
	void setDenoise(uint8_t value = 0);
	void setWhiteBalance(OV_WB option = OV_WB_NONE);
	void setMirrorFlip(bool mirror = false, bool flip = false, bool black_sun = false);

	void _DoInt(); // when wanting a frame to be sent to FIFO this function gets called and determines if just entered or exited active region via vsync

private:
	void regsDump();
	char* regsGetName(uint8_t reg, bool blankval); // return hex value in string if name not found

	void SCCBWriteRegArr(const char* name, const uint8_t* arr, bool show_all = false); // sends multiple commands
	uint8_t SCCBWriteReg(uint8_t reg, uint8_t data) { return m_wire.writeSlaveRegister(OV_I2C_ADDR, reg, data); }
	uint8_t SCCBReadReg(uint8_t reg) { return m_wire.readSlaveRegister(OV_I2C_ADDR, reg); }
};
