#include "OV7670.h"
#include "digitalWriteFast.h"

/*
https://github.com/arndtjenssen/ov7670/blob/master/ov7670_test/Ov7670.cpp
*/


// camera register metadata
#if _DEBUG

struct strRegNames
{
	uint8_t reg;
	const char *name;
};

#define tbl(x) { OV_REG_##x , "" #x "" }
#define tbl2(x,y) { y , "" #x "" }

const strRegNames arr_reg_names[] =
{
	tbl(AEB), //25 // AGC lower limit
	tbl(AECH), //10 // More bits of AEC value
	tbl(AECHH), //07 // AEC MS 5 bits
	tbl(AEW), //24 // AGC upper limit
	tbl(AWBCTR0), //6F
	tbl(BAVE), //05 // U/B Average level
	tbl(BD50MAX), //a5 // 50hz banding step limit
	tbl(BD60MAX), //ab // 60hz banding step limit
	tbl(BLUE), //01 // blue gain
	tbl(BRIGHT), //55 // Brightness
	tbl(CLKRC), //11 // Clocl control
	tbl(CMATRIX(1)),
	tbl(CMATRIX(2)),
	tbl(CMATRIX(3)),
	tbl(CMATRIX(4)),
	tbl(CMATRIX(5)),
	tbl(CMATRIX(6)),
	tbl(CMATRIX_SIGN), //58
	tbl(COM1), //04 // Control 1
	tbl(COM10), //15 // Control 10
	tbl(COM11), //3b // Control 11
	tbl(COM12), //3c // Control 12
	tbl(COM13), //3d // Control 13
	tbl(COM14), //3e // Control 14
	tbl(COM15), //40 // Control 15
	tbl(COM16), //41 // Control 16
	tbl(COM17), //42 // Control 17
	tbl(COM2), //09 // Control 2
	tbl(COM3), //0c // Control 3
	tbl(COM4), //0d // Control 4
	tbl(COM5), //0e // All "reserved"
	tbl(COM6), //0f // Control 6
	tbl(COM7), //12 // Control 7
	tbl(COM8), //13 // Control 8
	tbl(COM9), //14 // Control 9 - gain ceiling
	tbl(CONTRAST), //56 // Contrast control
	tbl(DBLV), //6b // PLL control
	tbl(DENOISE_STRENGTH), //4c // De-noise strength
	tbl(EDGE), //3f // Edge enhancement factor
	tbl(GAIN) ,
	tbl(GAIN), //00 // Gain lower 8 bits (rest in vref),
	tbl(GFIX), //69 // Fix gain control
	tbl(GGAIN), //6a // G channel AWB gain
	tbl(GbAVE), //06 // Y/Gb Average level
	tbl(HAECC1), //9f // Hist AEC/AGC control 1
	tbl(HAECC2), //a0 // Hist AEC/AGC control 2
	tbl(HAECC3), //a6 // Hist AEC/AGC control 3
	tbl(HAECC4), //a7 // Hist AEC/AGC control 4
	tbl(HAECC5), //a8 // Hist AEC/AGC control 5
	tbl(HAECC6), //a9 // Hist AEC/AGC control 6
	tbl(HAECC7), //aa // Hist AEC/AGC control 7
	tbl(HREF), //32 // HREF pieces
	tbl(HSTART), //17 // Horiz start high bits
	tbl(HSTOP), //18 // Horiz stop high bits
	tbl(HSYEN), //31 // HSYNC falling edge delay
	tbl(HSYST), //30 // HSYNC rising edge delay
	tbl(MANU), //67 // special effects register 1
	tbl(MANV), //68 // special effects register 2
	tbl(MIDH), //1c // Manuf. ID high
	tbl(MIDL), //1d // Manuf. ID low
	tbl(MVFP), //1e // Mirror / vflip
	tbl(PID), //0a // Product ID MSB
	tbl(PSHFT), //1b // Pixel delay after HREF
	tbl(RAVE), //08 // V/R Average level
	tbl(RED), //02 // red gain
	tbl(REG76), //76 // OV's name
	tbl(RGB444), //8c // RGB 444 control
	tbl(SCALING_DCWCTR), //72
	tbl(SCALING_DCWCTR), //72
	tbl(SCALING_PCLK_DELAY), //A2
	tbl(SCALING_PCLK_DELAY), //a2
	tbl(SCALING_PCLK_DIV), //73
	tbl(SCALING_PCLK_DIV), //73
	tbl(SCALING_XSC), //70
	tbl(SCALING_XSC), //70
	tbl(SCALING_YSC), //71
	tbl(SCALING_YSC), //71
	tbl(TSLB), //3a // lots of stuff
	tbl(VER), //0b // Product ID LSB
	tbl(VPT), //26 // AGC/AEC fast mode op region
	tbl(VREF), //03 // Pieces of GAIN, VSTART, VSTOP
	tbl(VSTART), //19 // Vert start high bits
	tbl(VSTOP), //1a // Vert stop high bits

	tbl2(SATCTR, 0xc9),
	tbl2(RSVD, 0x16),
	tbl2(RSVD, 0x29),
	tbl2(RSVD, 0x35),
	tbl2(RSVD, 0x36),
	tbl2(RSVD, 0x4d),
	tbl2(RSVD, 0x4e),
	tbl2(ADCCTR1, 0x21),
	tbl2(ADCCTR2, 0x22),
	tbl2(ADCCTR3, 0x23),
	tbl2(CHLF, 0x33),
	tbl2(ADC, 0x37),
	tbl2(OFEN, 0x39),
	tbl2(REG74, 0x74),
	tbl2(RSVD, 0x8d),
	tbl2(RSVD, 0x8e),
	tbl2(RSVD, 0x8f),
	tbl2(RSVD, 0x90),
	tbl2(RSVD, 0x91),
	tbl2(RSVD, 0x96),
	tbl2(RSVD, 0x97),
	tbl2(RSVD, 0x98),
	tbl2(RSVD, 0x99),
	tbl2(RSVD, 0x9a),
	tbl2(RSVD, 0x9b),
	tbl2(RSVD, 0x9c),

	tbl2(COLORSWAP, 0xb0),
	tbl2(RSVD, 0xb1),
	tbl2(RSVD, 0xb2),

	tbl2(THL_ST, 0xb3),

	tbl2(RSVD, 0xb8),

	{ 0, NULL }
};


uint8_t all_default[256];
char _tname[30];

#endif

void cOV7670::regsDump() {
#if _DEBUG
	Serial.print("Reg  Val  Name\n");
	Serial.print("---  ---  ------------------------------\n");

	for (int i = 0; arr_reg_names[i].name; i++)
	{
		uint8_t val = SCCBReadReg(arr_reg_names[i].reg);

		char buff[100];
		sprintf(buff, " %02x   %02x  %s", arr_reg_names[i].reg, val, arr_reg_names[i].name);
		Serial.println(buff);
	}
#endif
}


#if _DEBUG

char * cOV7670::regsGetName(uint8_t reg, bool retblank)  // return hex value in string if name not found
{

	for (int i = 0; arr_reg_names[i].name; i++)
	{
		if (reg == arr_reg_names[i].reg)
		{
			return (char *)arr_reg_names[i].name;
		}
	}

	// not found

	if (retblank)
	{
		return "";
	}

	sprintf(_tname, "0x%02x", reg);

	return _tname;
}
#endif

/*
TSLB
Line Buffer Test Option
Bit[64,128]: Reserved
Bit[32] : Negative image enable 0 : Normal image 1 : Negative image
Bit[16] : (enable SFX) UV output value 0 : Use normal UV output 1 : Use fixed UV value set in registers MANU and MANV as UV output instead of chip output
Bit[8] : Output sequence(use with register COM13[1](0x3D)) TSLB[3], COM13[1] : 00 : Y U Y V 01 : Y V Y U 10 : U Y V Y 11 : V Y U Y
Bit[1,2,4] : Reserved
  */

#define OV_TSLB_DEFT 0x0c
#define OV_TSLB_RESERVED ((OV_TSLB_DEFT & 7)|(OV_TSLB_DEFT & 0xc0))
#define OV_TSLB_YLAST 4 // UYVY or VYUY - see com13
#define OV_TSLB_UV 16 // enable special effects
#define OV_TSLB_NEGATIVE 32 // enable special effects

const uint8_t arr_effects[][8] =
{
	{OV_REG_TSLB, OV_TSLB_YLAST, OV_REG_MANU, 0x80, OV_REG_MANV, 0x80, OV_EM, OV_EM}, // NORMAL
	{OV_REG_TSLB, OV_TSLB_YLAST | OV_TSLB_UV, OV_REG_MANU, 0xa0, OV_REG_MANV, 0x40, OV_EM, OV_EM}, // Antique
	{OV_REG_TSLB, OV_TSLB_YLAST | OV_TSLB_UV, OV_REG_MANU, 0x80, OV_REG_MANV, 0xc0, OV_EM, OV_EM}, // Bluish
	{OV_REG_TSLB, OV_TSLB_YLAST | OV_TSLB_UV, OV_REG_MANU, 0x40, OV_REG_MANV, 0x40, OV_EM, OV_EM}, // Greenish
	{OV_REG_TSLB, OV_TSLB_YLAST | OV_TSLB_UV, OV_REG_MANU, 0xc0, OV_REG_MANV, 0x80, OV_EM, OV_EM}, // Redish
	{OV_REG_TSLB, OV_TSLB_YLAST | OV_TSLB_UV, OV_REG_MANU, 0x80, OV_REG_MANV, 0x80, OV_EM, OV_EM}, // B&W
	{OV_REG_TSLB, OV_TSLB_YLAST | OV_TSLB_NEGATIVE, OV_REG_MANU, 0x80, OV_REG_MANV, 0x80, OV_EM, OV_EM}, // Negative
	{OV_REG_TSLB, OV_TSLB_YLAST | OV_TSLB_UV | OV_TSLB_NEGATIVE, OV_REG_MANU, 0x80, OV_REG_MANV, 0x80, OV_EM, OV_EM} // B&W negative
};


// 0 - Normal, 1 - Antique, 2 - Bluish, 3 - Greenish
// 4 - Redish, 5 - B&W, 6 - Negative, 7 - B&W negative

void cOV7670::setFX(OV_FX fx) {
	SCCBWriteRegArr("FX", arr_effects[fx], true);
}

cOV7670* ov_me;

void ov_int() {
	ov_me->_DoInt();
}

void cOV7670::WantFrame() {
	if (m_waiting) { return; }

	ov_me = this;
	m_captured = false;
	attachInterrupt(digitalPinToInterrupt(m_vsync.pin), ov_int, CHANGE);

	pulseLowPin(m_fifo_wrst, 3 + 3); // reset write ptr .. also serves as a delay

	m_waiting = 1;
}

// https://github.com/desaster/ov7670fifotest/blob/master/ov7670.c

void cOV7670::_DoInt() {
	if (m_waiting) {
		if (m_waiting == 1 && !fastDigitalRead(m_vsync)) // just went LOW... grab it!!!! ... too late needed to be write enabled etc already
		{
			fastDigitalWrite(m_fifo_wr, HIGH); // write enable
			m_waiting = 2;
		} else if (m_waiting == 2) {
			fastDigitalWrite(m_fifo_wr, LOW); // write disable

			m_waiting = 0;
			detachInterrupt(digitalPinToInterrupt(m_vsync.pin));
			m_captured = true;

			// reset read pointer
			fastDigitalWrite(m_fifo_rrst, LOW);
			pulsePin(m_fifo_rck, 1);
			pulsePin(m_fifo_rck, 1);
			pulsePin(m_fifo_rck, 1);
			fastDigitalWrite(m_fifo_rrst, HIGH);
		}
	}
}


// -2 (low contrast ) to +2 (high contrast)

void cOV7670::setContrast(OV_CONTRAST contrast) {
	static const uint8_t values[] = {0x60, 0x50, 0x40, 0x38, 0x30};

	SCCBWriteReg(OV_REG_CONTRAST, values[contrast]);

	m_contrast = contrast;
}


void cOV7670::setBrightness(OV_BRIGHTNESS brightness) {
	static const uint8_t values[] = {0xb0, 0x98, 0x00, 0x18, 0x30};

	SCCBWriteReg(OV_REG_BRIGHT, values[brightness]);

	m_brightness = brightness;
}

#define OV_COM11_NIGHT 0x80 // Night mode enable
#define OV_COM11_NIGHT_FR2 0x20 // Night mode 1/2 of normal framerate
#define OV_COM11_NIGHT_FR4 0x40 // Night mode 1/4 of normal framerate
#define OV_COM11_NIGHT_FR8 0x60 // Night mode 1/8 of normal framerate
#define OV_COM11_HZAUTO 0x10 // Auto detect 50/60 Hz
#define OV_COM11_50HZ 0x08 // Manual 50Hz select
#define OV_COM11_EXP 0x02 // Exposure timing can be less than limit


#define DBLV_VALUE_30FPS   0x0A
#define EXHCH_VALUE_30FPS  0x00
#define EXHCL_VALUE_30FPS  0x00
#define DM_LNL_VALUE_30FPS   0x00
#define DM_LNH_VALUE_30FPS   0x00
#define COM11_VALUE_30FPS    0x0A

/*
CLKRC
Bit[128]: Digital PLL option
        0: Disable double clock option, meaning the maximum PCLK can be as high as half input clock
		1: Enable double clock option, meaning the maximum PCLK can be as high as input clock
Bit[64]: Use external clock directly (no clock pre-scale available)
Bit[1,2,4,8,18,32]: Internal clock pre-scalar F(internal clock) = F(input clock)/(Bit[5:0]+1) • Range: [0 0000] to [1 1111]
*/


#define CLKRC_VALUE_30FPS  (128)
#define CLKRC_VALUE_NIGHTMODE_AUTO    64 // Auto Frame Rate Adjust

void cOV7670::setNightMode(bool enable) {
	if (enable) {
		//SCCBWriteReg(OV_REG_COM11, OV_COM11_EXP | OV_COM11_HZAUTO);
		SCCBWriteReg(OV_REG_COM11, OV_COM11_EXP | OV_COM11_50HZ | OV_COM11_NIGHT | OV_COM11_NIGHT_FR8);
		SCCBWriteReg(OV_REG_CLKRC, CLKRC_VALUE_NIGHTMODE_AUTO);
	} else {
		//SCCBWriteReg(OV_REG_COM11, OV_COM11_EXP | OV_COM11_HZAUTO | OV_COM11_NIGHT | OV_COM11_NIGHT_FR8);

		SCCBWriteReg(OV_REG_CLKRC, CLKRC_VALUE_30FPS);

		SCCBWriteReg(OV_REG_DBLV, DBLV_VALUE_30FPS);
		SCCBWriteReg(OV_REG_EXHCH, EXHCH_VALUE_30FPS);
		SCCBWriteReg(OV_REG_EXHCL, EXHCL_VALUE_30FPS);

		SCCBWriteReg(OV_REG_DM_LNL, DM_LNL_VALUE_30FPS);
		SCCBWriteReg(OV_REG_DM_LNH, DM_LNH_VALUE_30FPS);

		SCCBWriteReg(OV_REG_COM11, COM11_VALUE_30FPS); // | OV_COM11_HZAUTO); // COM11_VALUE_30FPS); // OV_COM11_HZAUTO);
	}
}

/*
Common Control 8
Bit[128]: Enable fast AGC/AEC algorithm
Bit[64]: AEC - Step size limit 0: Step size is limited to vertical blank 1: Unlimited step size
Bit[32]: Banding filter ON/OFF - In order to turn ON the banding filter, BD50ST (0x9D) or BD60ST (0x9E) must be set to a non-zero value. 0: ON 1: OFF
Bit[8,16]: Reserved
Bit[4]: AGC Enable
Bit[2]: AWB Enable
Bit[1]: AEC Enable
*/

#define COM8_OFF (0)
#define COM8_ON (128|64|2)

const uint8_t arr_wb_options[][10] =
{
	{OV_REG_COM8, COM8_OFF, OV_REG_BLUE, 0x80, OV_REG_RED, 0x80, OV_EM, OV_EM}, // none
	{OV_REG_COM8, COM8_ON, OV_REG_AWBCTR0, 0x9A, OV_REG_BLUE, 0x80, OV_REG_RED, 0x80, OV_EM, OV_EM}, // simple
	{OV_REG_COM8, COM8_OFF, OV_REG_BLUE, 0x5A, OV_REG_RED, 0x5C, OV_EM, OV_EM}, // sunny
	{OV_REG_COM8, COM8_OFF, OV_REG_BLUE, 0x58, OV_REG_RED, 0x60, OV_EM, OV_EM}, // cloudy
	{OV_REG_COM8, COM8_OFF, OV_REG_BLUE, 0x84, OV_REG_RED, 0x4C, OV_EM, OV_EM}, // office
	{OV_REG_COM8, COM8_OFF, OV_REG_BLUE, 0x96, OV_REG_RED, 0x40, OV_EM, OV_EM} // home
};

void cOV7670::setWhiteBalance(OV_WB option) {
	switch (option) {
	case OV_WB_NONE:
	case OV_WB_AUTO_SIMPLE:
	case OV_WB_SUNNY:
	case OV_WB_CLOUDY:
	case OV_WB_OFFICE:
	case OV_WB_HOME: SCCBWriteRegArr("White Balance", arr_wb_options[option], true);
		break;
	}

	SCCBWriteReg(OV_REG_AWBCTR0, 0x9A); // default
}

#define OV_MVFP_MIRROR 0x20 // Mirror image
#define OV_MVFP_FLIP 0x10 // Vertical flip
#define OV_MVFP_BLACK_SUN 0x04 // Vertical flip

void cOV7670::setMirrorFlip(bool mirror, bool flip, bool black_sun) {
	uint8_t val = SCCBReadReg(OV_REG_MVFP);

	val &= ~(OV_MVFP_MIRROR | OV_MVFP_FLIP | OV_MVFP_BLACK_SUN);

	if (black_sun) { val |= OV_MVFP_BLACK_SUN; } // whatever this is :S
	if (mirror) { val |= OV_MVFP_MIRROR; }
	if (flip) { val |= OV_MVFP_FLIP; }

	SCCBWriteReg(OV_REG_MVFP, val);
}

const uint8_t arr_qqvga[] =
{
	// OV_REG_CLKRC, 1,

	OV_REG_COM3, 0x04,
	OV_REG_COM14, 0x1a, // divide by 4
	OV_REG_SCALING_XSC, 0x3A,
	OV_REG_SCALING_YSC, 0x35,
	OV_REG_SCALING_DCWCTR, 0x22, // downsample by 4
	OV_REG_SCALING_PCLK_DIV, 0xf2, // divide by 4
	OV_REG_SCALING_PCLK_DELAY, 0x02,

	OV_REG_HSTART, 0x16,
	OV_REG_HSTOP, 0x04,
	OV_REG_HREF, 0xA4,
	OV_REG_VSTART, 0x02,
	OV_REG_VSTOP, 0x7A,
	OV_REG_VREF, 0x0A,

	OV_EM, OV_EM
};

const uint8_t arr_qvga[] =
{
	//OV_REG_CLKRC, 1,

	OV_REG_COM3, 0x04,
	OV_REG_COM14, 0x19,
	OV_REG_SCALING_XSC, 0x3A,
	OV_REG_SCALING_YSC, 0x35,
	OV_REG_SCALING_DCWCTR, 0x11,
	OV_REG_SCALING_PCLK_DIV, 0xF1,
	OV_REG_SCALING_PCLK_DELAY, 0x02,

	OV_REG_HSTART, 0x16,
	OV_REG_HSTOP, 0x04,
	OV_REG_HREF, 0x24,
	OV_REG_VSTART, 0x02,
	OV_REG_VSTOP, 0x7A,
	OV_REG_VREF, 0x0A,

	OV_EM, OV_EM
};


const uint8_t arr_vga_raw_bayer[] =
{
	//OV_REG_CLKRC, 1,
	OV_REG_COM7, 0x01, // canned proc bayer 320 * 240
	OV_REG_COM3, 0x00,
	OV_REG_COM14, 0x00,

	OV_REG_SCALING_XSC, 0x3A,
	OV_REG_SCALING_YSC, 0x35,
	OV_REG_SCALING_DCWCTR, 0x11,
	OV_REG_SCALING_PCLK_DIV, 0xF0,
	OV_REG_SCALING_PCLK_DELAY, 0x02,

	//OV_REG_HSTART, 0x16,
	//OV_REG_HSTOP, 0x04,
	//OV_REG_HREF, 0x24,
	//OV_REG_VSTART, 0x02,
	//OV_REG_VSTOP, 0x7A,
	//OV_REG_VREF, 0x0A,

	OV_EM, OV_EM
};


const uint8_t arr_qvga_proc_bayer[] =
{
	//OV_REG_CLKRC, 1,
	OV_REG_COM7, 0x11, // canned proc bayer 320 * 240
	OV_REG_COM3, 0x04,
	OV_REG_COM14, 0x1a,

	OV_REG_SCALING_XSC, 0x3A,
	OV_REG_SCALING_YSC, 0x35,
	OV_REG_SCALING_DCWCTR, 0x11,
	OV_REG_SCALING_PCLK_DIV, 0xF9,
	OV_REG_SCALING_PCLK_DELAY, 0x02,

	//OV_REG_HSTART, 0x16,
	//OV_REG_HSTOP, 0x04,
	//OV_REG_HREF, 0x24,
	//OV_REG_VSTART, 0x02,
	//OV_REG_VSTOP, 0x7A,
	//OV_REG_VREF, 0x0A,

	OV_EM, OV_EM
};


/*
Ok, been there, done that! For past few hours I was struggling to get an image from YUV output but keep failing (planning to use this format for a better colour segmentation). Anyway, managed to get it done once read the manual (several times). Is actually pretty simple once you get the idea.

First, need to be sure that the proper register are set the way are supposed to be set:

{TSLB,     0x14}
{COM13,    0x88}
{COM7,     0x00}
*/


#define OV_COM13_GAMMA 0x80 // Gamma enable
#define OV_COM13_UVSAT 0x40 // UV saturation auto adjustment
#define OV_COM13_UVSWAP 0x01 // V before U - w/TSLB
#define OV_COM13_RESERVED_BITS 0x11 // default reserved bits


/*
COM15
Common Control 15
Bit[128,64]: Data format - output full range enable 0x: Output range: [10] to [F0] 10: Output range: [01] to [FE] 11: Output range: [00] to [FF]
Bit[32:16]: RGB 555/565 option (must set COM7[2] = 1 and COM7[0] = 0) x0: Normal RGB output 01: RGB 565 11: RGB 555
Bit[1,2,4,8]: Reserved
*/

#define OV_COM15_R10F0 (0) // Data range 10 to F0
#define OV_COM15_R01FE (128) // 01 to FE
#define OV_COM15_R00FF (128|64) // 00 to FF
#define OV_COM15_RGB565 (16) // RGB565 output
#define OV_COM15_RGB555 (32|16) // RGB555 output
//#define OV_COM15_RGB444 0x10 // RGB444 output

const uint8_t arr_yuv422[] =
{
	OV_REG_RGB444, 0,
	OV_REG_COM15, OV_COM15_R10F0,
	OV_REG_COM1, 0x00,
	OV_REG_CMATRIX(1), 0x80,
	OV_REG_CMATRIX(2), 0x80,
	OV_REG_CMATRIX(3), 0x00,
	OV_REG_CMATRIX(4), 0x22,
	OV_REG_CMATRIX(5), 0x5e,
	OV_REG_CMATRIX(6), 0x80,
	OV_REG_COM13, OV_COM13_GAMMA | OV_COM13_UVSAT | OV_COM13_UVSWAP | OV_COM13_RESERVED_BITS,
	OV_EM, OV_EM
};

const uint8_t arr_rgb565[] =
{
	OV_REG_RGB444, 0,
	OV_REG_COM15, OV_COM15_RGB565 | OV_COM15_R10F0,
	OV_REG_COM1, 0, // OV_COM1_CCIR656 ,
	OV_REG_CMATRIX(1), 0xb3,
	OV_REG_CMATRIX(2), 0xb3,
	OV_REG_CMATRIX(3), 0x00,
	OV_REG_CMATRIX(4), 0x3d,
	OV_REG_CMATRIX(5), 0xa7,
	OV_REG_CMATRIX(6), 0xe4,
	OV_REG_COM13, OV_COM13_GAMMA | OV_COM13_UVSAT | OV_COM13_RESERVED_BITS, // 0x11 is default reserved bits
	OV_EM, OV_EM
};

const uint8_t arr_rgb555[] = {
	OV_REG_RGB444, 0,
	OV_REG_COM15, OV_COM15_RGB555 | OV_COM15_R10F0,
	OV_REG_COM1, 0x00,
	OV_REG_CMATRIX(1), 0xb3,
	OV_REG_CMATRIX(2), 0xb3,
	OV_REG_CMATRIX(3), 0x00,
	OV_REG_CMATRIX(4), 0x3d,
	OV_REG_CMATRIX(5), 0xa7,
	OV_REG_CMATRIX(6), 0xe4,
	OV_REG_COM13, OV_COM13_GAMMA | OV_COM13_UVSAT | OV_COM13_RESERVED_BITS,
	OV_EM, OV_EM
};

const uint8_t arr_init_defaults[] = // don't double up on ones set elsewhere ... in theory if we reset... should all go back to defaults????
{
	OV_REG_SATCTR, 0x60,
	OV_EM, OV_EM,


	/* Almost all of these are magic “reserved” values. */
	OV_REG_COM5, 0x61,
	OV_REG_COM6, 0x4b,
	0x16, 0x02, // RSVD
	OV_REG_MVFP, 0x07,
	0x21, 0x02, // ADCCTR1
	0x22, 0x91, // ADCCTR2
	0x29, 0x07, // rsvd
	0x33, 0x0b,
	0x35, 0x0b,
	0x37, 0x1d, //adc
	0x38, 0x71,
	0x39, 0x2a, // ofen
	OV_REG_COM12, 0x78,
	0x4d, 0x40, // rsvd
	0x4e, 0x20, // rsvd
	OV_REG_GFIX, 0,
	// OV_REG_DBLV, 0x4a ,	// increase clock???
	0x74, 0x10, // reg74
	0x8d, 0x4f, //rsvd
	0x8e, 0, //rsvd
	0x8f, 0, //rsvd
	0x90, 0, //rsvd
	0x91, 0, //rsvd
	0x96, 0, //rsvd
	0x9a, 0, //rsvd
	0xb0, 0x84, // color swap red & green
	0xb1, 0x0c, //rsvd
	0xb2, 0x0e, //rsvd
	0xb3, 0x82, // thl_st
	0xb8, 0x0a, // rsvd
	OV_EM, OV_EM
};

uint8_t arr_windows[] = // note ... this is changed
{
	OV_REG_HSTART, 0, // low
	OV_REG_HSTOP, 0, // low
	OV_REG_HREF, 0, // high for both above

	OV_REG_VSTART, 0, // low
	OV_REG_VSTOP, 0, // low
	OV_REG_VREF, 0, // high for both above
	OV_EM, OV_EM
};


/*
Common Control 7
Bit[7]: SCCB register reset 0 : No change 1 : Resets all registers to default values
Bit[6] : Reserved
Bit[5] : Output format - CIF selection
Bit[4] : Output format - QVGA selection
Bit[3] : Output format - QCIF selection
Bit[2] : Output format - RGB selection(see below) Bit[1] : Color bar 0 : Disable 1 : Enable Bit[0] : Output format - Raw RGB(see below)
COM7[2] COM7[0] YUV 0 0 RGB 1 0 Raw Bayer RGB 0 1 Processed Bayer RGB 1 1
 */

bool cOV7670::setMode(OV_SCREEN_SIZE screen_size, OV_PIXEL_MODE pixel_mode) {
	const uint8_t* script_scr = 0;
	const uint8_t* script_pm = 0;

	m_width = 0;
	m_height = 0;
	m_bytes_per_pixel = 0;

	cameraResetRegisters();
	SCCBWriteRegArr("reset_defaults", arr_init_defaults, true);

	switch (pixel_mode) {
	case OV_PM_RAW_BAYER:
#if _DEBUG
		Serial.println("RAW BAYER");
#endif
		script_scr = arr_vga_raw_bayer;
		m_bytes_per_pixel = 1;
		screen_size = OV_SCR_VGA;
		m_width = 640;
		m_height = 480;
		goto skip_screen;
		break;

	case OV_PM_PROC_BAYER:
#if _DEBUG
		Serial.println("PROC BAYER");
#endif

		script_scr = arr_qvga_proc_bayer;

		m_bytes_per_pixel = 1;
		screen_size = OV_SCR_QVGA;
		m_width = 320;
		m_height = 240;
		goto skip_screen;
		break;

	case OV_PM_YUV422:
#if _DEBUG
		Serial.println("YUV422");
#endif
		script_pm = arr_yuv422;
		SCCBWriteReg(OV_REG_COM7, 0); // OV_COM7_YUV;
		m_bytes_per_pixel = 2;
		break;

	case OV_PM_RGB565:
#if _DEBUG
		Serial.println("RGB565");
#endif
		script_pm = arr_rgb565;
		SCCBWriteReg(OV_REG_COM7, 4); // OV_COM7_RGB;
		m_bytes_per_pixel = 2;
		break;

	case OV_PM_RGB555:
#if _DEBUG
		Serial.println("RGB555");
#endif
		script_pm = arr_rgb555;
		SCCBWriteReg(OV_REG_COM7, 4); // OV_COM7_RGB;
		m_bytes_per_pixel = 2;
		break;
	}


	switch (screen_size) {
	case OV_SCR_QVGA: //
#if _DEBUG
		Serial.println("QVGA");
#endif
		script_scr = arr_qvga;
		m_width = 320;
		m_height = 240;
		break;

	case OV_SCR_QQVGA:
#if _DEBUG
		Serial.println("QQVGA");
#endif
		script_scr = arr_qqvga;
		m_width = 160;
		m_height = 120;
		break;
	}

skip_screen:

	if (m_width == 0) { return false; }


	if (!m_bytes_per_pixel) { return false; }

	m_pixel_mode = pixel_mode;
	m_screen_size = screen_size;

#if _DEBUG
	Serial.println("Valid Size and Mode!");
#endif

	SCCBWriteRegArr("screen_mode", script_scr, true);
	SCCBWriteRegArr("pixel_mode", script_pm, true);

	// set window!!!!!

	// if know width * height then window is inside 640 * 480 area

	/*
	OK need to calulate the following from m_width and m_height


	OV_REG_HSTART // low
	OV_REG_HSTOP  // low
	OV_REG_HREF  // high for both above

	OV_REG_VSTART // low
	OV_REG_VSTOP  // low
	OV_REG_VREF // high for both above

	Horizontal Frame(HREF Column) Start HSTART[7:0], HREF[2:0] 0x17, 0x32
	Horizontal Frame(HREF Column) Stop HSTOP[7:0], HREF[5:3] 0x18, 0x32
	Vertical Frame(Row) Start VSTRT[7:0], VREF[2:0] 0x19. 0x03
	Vertical Frame(Row) Stop VSTOP[7:0], VREF[5:3] 0x1A, 0x03


	for start make
	0 -> width - 1
	0 -> height - 1

	.hstart         = 168,
	726.hstop = 24,
		727.vstart = 12,
		728.vstop = 492,
	*/

	uint16_t hstart = 0;
	uint16_t hstop = m_width - 1;

	uint16_t vstart = 0;
	uint16_t vstop = m_height - 1;

	/*
	http://lxr.free-electrons.com/source/drivers/media/i2c/ov7670.c
	*/

	/*
	879  * Horizontal: 11 bits, top 8 live in hstart and hstop.  Bottom 3 of
	880  * hstart are in href[2:0], bottom 3 of hstop in href[5:3].  There is
	881  * a mystery "edge offset" value in the top two bits of href.
	882  */


	// horizontals

	uint8_t href_curr = SCCBReadReg(OV_REG_HREF) & 0xC0;
	uint8_t val_hstart = (hstart >> 3) & 255;
	uint8_t val_hstop = (hstop >> 3) & 255;
	uint8_t val_href = href_curr | (hstart & 7) | ((hstop & 7) << 3);

	// verticals

	uint8_t vref_curr = SCCBReadReg(OV_REG_VREF) & 0xF0;
	uint8_t val_vstart = (vstart >> 2) & 255;
	uint8_t val_vstop = (vstop >> 2) & 255;
	uint8_t val_vref = vref_curr | (vstart & 3) | ((vstop & 3) << 2);

	// load into array for setting

	arr_windows[1] = val_hstart;
	arr_windows[3] = val_hstop;
	arr_windows[5] = val_href;

	arr_windows[7] = val_vstart;
	arr_windows[9] = val_vstop;
	arr_windows[11] = val_vref;

	//SCCBWriteRegArr("Windowing Registers", arr_windows, true);

	// SCCBWriteReg(OV_REG_DBLV, 0x4a); // increase clock?

	// set all our defaults here

	setNightMode();
	setWhiteBalance();
	setBrightness();
	setContrast();
	setFX();
	setDenoise();
	setEdgeEnhancement();
	setMirrorFlip();

	// new
	setAEC();

	if (1) // pixel_mode == OV_PM_RGB565)
	{
		//return true;

		//	Serial.print("Rewrite CLKRC for this mode :0x");	  // should be 0x80
		//uint8_t val = (SCCBReadReg(OV_REG_CLKRC) & ~3);
		//	Serial.println(val, HEX);

		//SCCBWriteReg(OV_REG_CLKRC, 0x80); // (val | 1));
	}

	return true;
}

// 0 to disable, > 0 enable and set edge enhancement factor

/*
Common Control 16
Bit[64,128]: Reserved
Bit[32]: Enable edge enhancement threshold auto-adjustment for YUV output (result is saved in register EDGE[4:0] (0x3F) and range is controlled by registers REG75[4:0] (0x75) and REG76[4:0] (0x76)) 0: Disable 1: Enable
Bit[16]: De-noise threshold auto-adjustment (result is saved in register DNSTH (0x4C) and range is controlled by register REG77[7:0] (0x77)) 0: Disable 1: Enable
Bit[8]: AWB gain enable
Bit[4]: Reserved
Bit[2]: Color matrix coefficient double option 0: Original matrix 1: Double of original matrix
Bit[1]: Reserved
*/


#define OV_COM16_AWBGAIN 8 // AWB gain enable
#define OV_COM16_DENOISE 16 // Enable de-noise auto adjustment
#define OV_COM16_EDGE 32 // Enable edge enhancement

void cOV7670::setEdgeEnhancement(uint8_t value) // must be 0-255
{
	uint8_t v = OV_COM16_AWBGAIN | (m_denoise ? OV_COM16_DENOISE : 0);

	//return; // needto check!!!!!

	if (!value) {
		SCCBWriteReg(OV_REG_COM16, v);
		SCCBWriteReg(OV_REG_EDGE, 0);
	} else {
		SCCBWriteReg(OV_REG_COM16, v | OV_COM16_EDGE);
		SCCBWriteReg(OV_REG_EDGE, value);
	}
	m_edge_enhancement = value;
}

// 0 to disable, > 0 enable and set denoise factor

void cOV7670::setDenoise(uint8_t value) // must be 0 - 255
{
	uint8_t v = OV_COM16_AWBGAIN | (m_edge_enhancement ? OV_COM16_EDGE : 0);

	if (value == 0) {
		SCCBWriteReg(OV_REG_COM16, v);
		SCCBWriteReg(OV_REG_EDGE, 0);
	} else {
		SCCBWriteReg(OV_REG_COM16, v | OV_COM16_DENOISE);
		SCCBWriteReg(OV_REG_DENOISE_STRENGTH, value);
	}
	m_denoise = value;
}

// AEC/AGC - Automatic Exposure/Gain Control

#define GAIN_VALUE	0x00
#define AEW_VALUE	0x95
#define AEB_VALUE	0x33
#define VPT_VALUE	0xe3

// AEC/AGC Control- Histogram
#define HAECC1_VALUE	0x78
#define HAECC2_VALUE	0x68
#define HAECC3_VALUE	0xd8
#define HAECC4_VALUE	0xd8
#define HAECC5_VALUE	0xf0
#define HAECC6_VALUE	0x90

#define HAECC7_VALUE_HISTOGRAM_AEC_ON	0x94
#define HAECC7_VALUE_AVERAGE_AEC_ON     0x00

void cOV7670::setAEC(bool average) {
	if (average) {
		Serial.println(F("-------------- Setting Camera Average Based AEC/AGC Registers ---------------"));

		SCCBWriteReg(OV_REG_AEW, AEW_VALUE);
		SCCBWriteReg(OV_REG_AEB, AEB_VALUE);
		SCCBWriteReg(OV_REG_VPT, VPT_VALUE);
		SCCBWriteReg(OV_REG_HAECC7, HAECC7_VALUE_AVERAGE_AEC_ON);
	} else {
		Serial.println(F("-------------- Setting Camera Histogram Based AEC/AGC Registers ---------------"));

		SCCBWriteReg(OV_REG_AEW, AEW_VALUE);
		SCCBWriteReg(OV_REG_AEB, AEB_VALUE);
		SCCBWriteReg(OV_REG_HAECC1, HAECC1_VALUE);
		SCCBWriteReg(OV_REG_HAECC2, HAECC2_VALUE);
		SCCBWriteReg(OV_REG_HAECC3, HAECC3_VALUE);
		SCCBWriteReg(OV_REG_HAECC4, HAECC4_VALUE);
		SCCBWriteReg(OV_REG_HAECC5, HAECC5_VALUE);
		SCCBWriteReg(OV_REG_HAECC6, HAECC6_VALUE);
		SCCBWriteReg(OV_REG_HAECC7, HAECC7_VALUE_HISTOGRAM_AEC_ON);
	}
}

void cOV7670::begin(uint8_t sda, uint8_t scl, uint8_t vsync, uint8_t fifo_wr, uint8_t fifo_wrst, uint8_t fifo_rrst, uint8_t fifo_rck) {
	delay(100);

	m_wire.begin(sda, scl, 25); // 1us = 1,000,000Hz 2us = 500,000 ... 3us = 333,333 4us = 250,000 ... 10us = 100,000hz

	pinInit(m_vsync, vsync, INPUT);
	pinInit(m_fifo_wr, fifo_wr, OUTPUT, LOW);
	pinInit(m_fifo_wrst, fifo_wrst, OUTPUT, LOW);
	pinInit(m_fifo_rrst, fifo_rrst, OUTPUT, LOW);
	pinInit(m_fifo_rck, fifo_rck, OUTPUT, LOW);

	//pinInit(m_fifo_oe, 53, OUTPUT, LOW);

	pinMode8_1_INPUT(); // enable parallel data lines
}


// https://developer.mbed.org/users/ms523/notebook/ov7670-camera/
// http://stackoverflow.com/questions/21220738/arduino-ov7670-without-fifo-reading-snapshot

//http ://www.rpg.fi/desaster/blog/2012/10/20/ov7670-fifo-msp430-launchpad/

#if _DEBUG
static int filled = 0;
#endif

#define OV_COM7_RESET 0x80 // Register reset

void cOV7670::cameraResetRegisters() {
	SCCBReadReg(OV_REG_COM7); // Reading needed to prevent error

	SCCBWriteReg(OV_REG_COM7, OV_COM7_RESET);
	delay(5); // software reset, wait 1 ms for the next register access (there is no limitation for other register

	//delay(500); // software reset, wait 1 ms for the next register access (there is no limitation for other register

#if _DEBUG
	if (!filled)
	{
		filled++;

		for (int i = 0; i < 256; i++)
		{
			all_default[i] = SCCBReadReg(i);
		}
	}
#endif

	//regsDump(); // written=pending change, changed = different value to default

	m_pixel_mode = OV_PM_UNKNOWN;
	m_screen_size = OV_SCR_UNKNOWN;
	m_width = 0;
	m_height = 0;
	m_bytes_per_pixel = 0;
}

// double 0 indicates end

void cOV7670::SCCBWriteRegArr(const char* name, const uint8_t* arr, bool show_all) // sends multiple commands
{
	int cmd = 0;

	if (!arr) {
		return;
	}

#if _DEBUG
	Serial.print("OV7670 Run Script <");
	Serial.print(name);
	Serial.print("> {\n");
#endif

	while (!(arr[cmd] == OV_EM && arr[cmd + 1] == OV_EM)) {
#if _DEBUG
		char buff[30];

		if (!show_all)
		{
			sprintf(buff, "%s\n", regsGetName(arr[cmd], false));
		}
		else
		{
			if (arr[cmd + 1] == all_default[arr[cmd]])
			{
				sprintf(buff, " %s=%02x\n", regsGetName(arr[cmd], false), arr[cmd + 1]);
			}
			else
			{
				sprintf(buff, "*%s=%02x\n", regsGetName(arr[cmd], false), arr[cmd + 1]); // changing to non-default
			}
		}

		Serial.print(buff);
#endif

		SCCBWriteReg(arr[cmd], arr[cmd + 1]);
		cmd += 2;
	}

#if _DEBUG
	Serial.println("}");
#endif

	return;
}


/*
void cOV7670::setArrayControl()
{
	Serial.println(F("$Setting Camera Array Control  "));

	const uint8_t cmds[] =
	{
		OV_CHLF, OV_CHLF_VALUE,
		OV_ARBLM, OV_ARBLM_VALUE,
		OV_EM, OV_EM
	};

	SCCBWriteRegArr(cmds);
  }
  */


/*
void cOV7670::setADCControl()
{
	Serial.println(F("$Setting Camera ADC Control  "));
	SCCBWriteReg(OV_ADCCTR1, OV_ADCCTR1_VALUE);
	SCCBWriteReg(OV_ADCCTR2, OV_ADCCTR2_VALUE);
	SCCBWriteReg(OV_ADC, OV_ADC_VALUE);
	SCCBWriteReg(OV_ACOM, OV_ACOM_VALUE);
	SCCBWriteReg(OV_OFON, OV_OFON_VALUE);
}
  */


/*
As control sensor pin defined as follows:

3V3 ----- input supply voltage (recommended 3.3,5 V can also be, but is not recommended)
The GDN ----- ground point
m_pin_scl --- SCCB interface control clock (Note: some of the low-level microcontroller needs pull-up control, and the I2C interface similar)
m_pin_sda --- SCCB interface serial data input (output) end (Note: some of the low-level microcontroller needs pull-up control, and the I2C interface similar)
VSYNC --- frame synchronizing signal (output signal)
OV_HREF ---- line synchronizing signal (the output signal, can generally not applicable to use of special cases)
D0-D7 --- data port (output signal)
RESTE --- reset port (normal use pulled)
PWDN ---- power selection mode (normal use pull down)
The STROBE-photographed flash control port (normal use may not be required)
FIFO_RCK --- FIFO memory read clock control terminal
FIFO_WR_CTR ---- FIFO write control terminal (1 allows the CMOS is written to the FIFO, to prohibit 0)
FIFO_OE ---- FIFO off control
FIFO_WRST-FIFO write pointer reset terminal
FIFO_RRST-FIFO read pointer reset terminal
*/
