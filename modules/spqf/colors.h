/**
 *	COPYRIGHT 2014 (C) Jason Volk
 *	COPYRIGHT 2014 (C) Svetlana Tkachenko
 *
 *	DISTRIBUTED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) (see: LICENSE)
 */


enum Mode : unsigned char
{
	OFF           = 0x0f,
	BOLD          = 0x02,
	COLOR         = 0x03,
	ITALIC        = 0x09,
	STRIKE        = 0x13,
	UNDER         = 0x15,
	UNDER2        = 0x1f,
	REVERSE       = 0x16,
};

enum class FG
{
	WHITE,    BLACK,      BLUE,      GREEN,
	LRED,     RED,        MAGENTA,   ORANGE,
	YELLOW,   LGREEN,     CYAN,      LCYAN,
	LBLUE,    LMAGENTA,   GRAY,      LGRAY
};

enum class BG
{
	LGRAY_BLINK,     BLACK,           BLUE,          GREEN,
	RED_BLINK,       RED,             MAGENTA,       ORANGE,
	ORANGE_BLINK,    GREEN_BLINK,     CYAN,          CYAN_BLINK,
	BLUE_BLINK,      MAGENTA_BLINK,   BLACK_BLINK,   LGRAY,
};
