/***************************************************************************
  This is a library for the SH1106 SPI OLED display
  These displays use SPI to communicate
  The implementation needed to go here instead of a .cpp file, because that
  way the configuration #defines would not work!
  See: https://github.com/arduino/Arduino/pull/1808#issuecomment-370210551
  Written by https://github.com/notisrac
  MIT license, all text above must be included in any redistribution
 ***************************************************************************/

#pragma once

#ifndef _SH1106LIB_H
#define _SH1106LIB_H

#include <SPI.h>
#include <Arduino.h>
//#include <TinyPrint.h>

// region display_constants

#define SH1106_LCDWIDTH 128
#define SH1106_LCDHEIGHT 64

#define SH1106_SETCONTRAST 0x81
#define SH1106_DISPLAYALLON_RESUME 0xA4
#define SH1106_DISPLAYALLON 0xA5

#define SH1106_NORMALDISPLAY 0xA6
#define SH1106_INVERTDISPLAY 0xA7
#define SH1106_DISPLAYOFF 0xAE
#define SH1106_DISPLAYON 0xAF

#define SH1106_SETDISPLAYOFFSET 0xD3
#define SH1106_SETCOMPINS 0xDA

#define SH1106_SETVCOMDETECT 0xDB

#define SH1106_SETDISPLAYCLOCKDIV 0xD5
#define SH1106_SETPRECHARGE 0xD9

#define SH1106_SETMULTIPLEX 0xA8

#define SH1106_SETCOLUMNADDRLOW 0x00
#define SH1106_SETCOLUMNADDRHIGH 0x10

#define SH1106_SETSTARTLINE 0x40

#define SH1106_MEMORYMODE 0x20
//#define SH1106_COLUMNADDR 0x21
#define SH1106_PAGEADDR   0xB0

#define SH1106_COMSCANINC 0xC0
#define SH1106_COMSCANDEC 0xC8

#define SH1106_SEGREMAP 0xA0

#define SH1106_CHARGEPUMP 0x8B

#define SH1106_DCDC 0xAD

#define SH1106_EXTERNALVCC 0x1
#define SH1106_SWITCHCAPVCC 0x2
#define SH1106_PUMPVOLTAGE 0x3

#define SH1106_MESSAGETYPE_COMMAND 0x80
#define SH1106_MESSAGETYPE_DATA 0x40

#define SH1106_READMODIFYWRITE_START 0xE0
#define SH1106_READMODIFYWRITE_END 0xEE

#define SH1106_MAXSEGMENTSPERWRITE 16
#define SH1106_ROWHEIGHT 8

#define SH1106_NUMBEROF_SEGMENTS 128
#define SH1106_NUMBEROF_PAGES 8

// endregion display_constants

#define BLACK 0
#define WHITE 1
#define TRANSPARENT 255
#define SOLID 127


// font flags
#define FONT_NUMBERS        1
#define FONT_UPPERCASECHARS 2
#define FONT_LOWERCASECHARS 4
#define FONT_HASSPACE       8
#define FONT_FULL           255


//class SH1106Lib : public TinyPrint
class SH1106Lib : public Print
{
public:
	SH1106Lib(int8_t dc_pin, int8_t rst_pin, int8_t cs_pin, uint32_t bitrate = 8000000UL);
	~SH1106Lib();

	void initialize();
	void sendCommand(uint8_t c);
	void sendData(uint8_t data);

	void clearDisplay(void);
  void fillDisplay(uint8_t pattern);
  void fillPage(uint8_t page, uint8_t pattern);

	void setCursor(uint8_t x, uint8_t y);
	void setFont(const unsigned char *font, uint8_t width, uint8_t height, int8_t offset = 0, uint8_t flags = FONT_FULL);
	void setTextWrap(bool enableWrap);
	void setTextColor(uint8_t color, uint8_t backgroundType);
	
	void drawChar(uint8_t x, uint8_t y, uint8_t character, uint8_t color, uint8_t backgroundType);

  using Print::write;
  virtual size_t write(uint8_t);

	void _setDisplayWritePosition(uint8_t x, uint8_t y);
	/**
		Moves the display write position, and starts the ReadModifyWrite mode
		@param x The x coordinate 0..127 (this will select column)
		@param y The y coordinate 0..63 (this will select the page - will be converted to page number)
		@returns nothing
	*/

	void _drawColumn(uint8_t data);
	/**
		Draws a number of columns (same 8bits vertically) on the current page.
		Note: the ReadModifyWrite must be enabled for this to work
		@param x The x coordinate 0..127 (this will select column)
		@param y The y coordinate 0..63 (this will select the page - will be converted to page number)
		@param data The byte that will be output to the column
		@param count How many columns should be filled with the data
		@param color The color that the 1 pixels is displayed should be black or whit - ANDs or ORs the data to the pixels on the screen
		@returns nothing
	*/
	void _drawColumns(uint8_t x, uint8_t y, uint8_t data, uint8_t count);

  private:
	// text related variables
	uint8_t _cursorX = 0;
	uint8_t _cursorY = 0;
	uint8_t _textColor = WHITE;
	uint8_t _backgroundType = TRANSPARENT;
	uint8_t _fontWidth = 5;
	uint8_t _fontHeight = 8;
	int8_t _fontOffset = 0;
	uint8_t _fontFlags = FONT_FULL;
	bool _wrap = true;
	const unsigned char* _font;

	uint8_t _pixelPosX = 0;
	uint8_t _pixelPosY = 0;

  SPISettings *_spiSetting = nullptr;
  int dcPin,  ///< The Arduino pin connected to D/C (for SPI)
      csPin,  ///< The Arduino pin connected to CS (for SPI)
      rstPin; ///< The Arduino pin connected to reset (-1 if unused)
};


#define intCeil(x,y) (((x) + (y) - 1) / (y))

SH1106Lib::SH1106Lib(int8_t dc_pin, int8_t rst_pin, int8_t cs_pin, uint32_t bitrate)
{
  dcPin = dc_pin;
  csPin = cs_pin;
  rstPin = rst_pin;
  _spiSetting = new SPISettings(bitrate, MSBFIRST, SPI_MODE0);
}

SH1106Lib::~SH1106Lib()
{
}

/*
Initializes the display
*/
void SH1106Lib::initialize()
{
  pinMode(rstPin, OUTPUT);
  pinMode(dcPin, OUTPUT); 
  pinMode(csPin, OUTPUT); 
  digitalWrite(rstPin, HIGH);
  delay(10);                  // VDD goes high at start, pause
  digitalWrite(rstPin, LOW);  // Bring reset low
  delay(10);                  // Wait 10 ms
  digitalWrite(rstPin, HIGH); // Bring out of reset
  delay(10);

  SPI.begin();
  
	sendCommand(SH1106_DISPLAYOFF);                    // 0xAE
	sendCommand(SH1106_SETDISPLAYCLOCKDIV);            // 0xD5
	sendCommand(0x80);                                 // the suggested ratio 0x80
	sendCommand(SH1106_SETMULTIPLEX);                  // 0xA8
	sendCommand(0x3F);
	sendCommand(SH1106_SETDISPLAYOFFSET);              // 0xD3
	sendCommand(0x00);                                 // no offset
	sendCommand(SH1106_SETSTARTLINE | 0x0);            // 0x40, line #0
	sendCommand(SH1106_DCDC);                          // 0xAD  DC/DC on
	sendCommand(0x8B);
	sendCommand(SH1106_SEGREMAP | 0x1);
	sendCommand(SH1106_COMSCANDEC);
	sendCommand(SH1106_SETCOMPINS);                    // 0xDA
	sendCommand(0x12);
	sendCommand(SH1106_SETCONTRAST);                   // 0x81
	sendCommand(0x7F);
	sendCommand(SH1106_SETPRECHARGE);                  // 0xd9
	sendCommand(0x1F);
	sendCommand(SH1106_SETVCOMDETECT);                 // 0xDB
	sendCommand(0x40);
  sendCommand(SH1106_PUMPVOLTAGE | 0x3);             // 0x33
	sendCommand(SH1106_NORMALDISPLAY);                 // 0xA6
  sendCommand(SH1106_DISPLAYALLON_RESUME);           // 0xA4

  delay(100);
  clearDisplay(); 
	sendCommand(SH1106_DISPLAYON); //--turn on oled panel
}

/*
Sends a single command to the display
command: the command (byte) that needs to be sent to the device
*/
void SH1106Lib::sendCommand(uint8_t command)
{
  digitalWrite(dcPin, LOW);  
  digitalWrite(csPin, LOW);
  SPI.beginTransaction(*_spiSetting);
  SPI.transfer(command);
  SPI.endTransaction();
  digitalWrite(csPin, HIGH);
}

/*
Sends a single byte of data to the display
data: the data (byte) that needs to be sent to the device
*/
void SH1106Lib::sendData(uint8_t data)
{
  digitalWrite(csPin, LOW);
  SPI.beginTransaction(*_spiSetting);
  SPI.transfer(data);
  SPI.endTransaction();
  digitalWrite(csPin, HIGH);
}


/*
Clears the display
*/
void SH1106Lib::clearDisplay(void)
{
	uint8_t page, segments;

	// clear the buffer so we can fill the screen with zeroes
	for (page = 0; page < SH1106_NUMBEROF_PAGES; page++)
	{
		// move to the beginning of the next page
		_setDisplayWritePosition(0, page * SH1106_ROWHEIGHT);
    digitalWrite(dcPin, HIGH);    // A0 = 1 (write to display data RAM)
    SPI.beginTransaction(*_spiSetting);
    digitalWrite(csPin, LOW);

		for (segments = 0; segments < SH1106_NUMBEROF_SEGMENTS; segments++)
				SPI.transfer(0x00);	
        
    digitalWrite(csPin, HIGH);    
	}
  SPI.endTransaction();
}


void SH1106Lib::fillDisplay(uint8_t pattern)
{
  uint8_t page, segments, parts;

	// clear the buffer so we can fill the screen with zeroes
	for (page = 0; page < SH1106_NUMBEROF_PAGES; page++)
	{
		// move to the beginning of the next page
		_setDisplayWritePosition(0, page * SH1106_ROWHEIGHT);
    digitalWrite(dcPin, HIGH);    // A0 = 1 (write to display data RAM)
    SPI.beginTransaction(*_spiSetting);
    digitalWrite(csPin, LOW);

		for (segments = 0; segments < SH1106_NUMBEROF_SEGMENTS; segments++)
				SPI.transfer(pattern);

    digitalWrite(csPin, HIGH);    
	}
  SPI.endTransaction();
}

void SH1106Lib::fillPage(uint8_t page, uint8_t pattern)
{
  uint8_t segments;

  // move to the beginning of the next page
  _setDisplayWritePosition(0, page * SH1106_ROWHEIGHT);
  digitalWrite(dcPin, HIGH);    // A0 = 1 (write to display data RAM)
  SPI.beginTransaction(*_spiSetting);
  digitalWrite(csPin, LOW);

  for (segments = 0; segments < SH1106_NUMBEROF_SEGMENTS; segments++)
      SPI.transfer(pattern);

  digitalWrite(csPin, HIGH);    
	
  SPI.endTransaction();  
}


/*
Sets the text cursor to this position
x: the x coordinate
y: the y coordiante
*/
void SH1106Lib::setCursor(uint8_t x, uint8_t y)
{
	_cursorX = x;
	_cursorY = y;
}

/*
Sets the font to write with
font: pinter to the array containing the font
width: the width of the font in pixels
height: the height of the font in pixels
offset: signed value to offset the position the character is found in the font
flags: set of flags describing the properties of the font
*/
void SH1106Lib::setFont(const unsigned char *font, uint8_t width, uint8_t height, int8_t offset/* = 0*/, uint8_t flags/* = FONT_FULL*/)
{
	_font = font;
	_fontWidth = width;
	_fontHeight = height;
	_fontOffset = offset;
	_fontFlags = flags;
}

/*
Sets whether the text should continue on the next row, if it has reached the end of the current one
enableWrap: true to enable, false to disable
*/
void SH1106Lib::setTextWrap(bool enableWrap)
{
	_wrap = enableWrap;
}

/*
Sets whether the text should continue on the next row, if it has reached the end of the current one
color: the color of the text
backColor: the color of the background of the text. Set it to TRANSPARENT to have a transparend background
*/
void SH1106Lib::setTextColor(uint8_t color, uint8_t backgroundType)
{
	_textColor = color;
	_backgroundType = backgroundType;
}

/*
Displays a single character
c: the character to display
*/
size_t SH1106Lib::write(uint8_t c)
{
	if (c == '\n') { // on a linebreak move the cursor down one line, and back to the start
		setCursor(0, _cursorY + _fontHeight);
	}
	else if (c == '\r') {
		// skip carrage return, as we already handle newline
	}
	else {
		// draw the pixels for the actual character
		drawChar(_cursorX, _cursorY, c, _textColor, _backgroundType);
		// advance the cursor
		setCursor(_cursorX + _fontWidth + 1, _cursorY);
		// if the next letter would not fit in the current row, jump to the beginnign of the next one
		if (_wrap && (_cursorX > (SH1106_LCDWIDTH - _fontWidth + 1))) {
			setCursor(0, _cursorY + _fontHeight + 1);
		}
	}
	return 1;
}


/*
Draws a character on the screen from the font
x: x coordinate where the character should be displayed
y: y coordinate where the character should be displayed
character: The character to display
color: the color of the character
backgroundType: SOLID or TRANSPARENT
*/
void SH1106Lib::drawChar(uint8_t x, uint8_t y, uint8_t character, uint8_t color, uint8_t backgroundType)
{
	if ((x + _fontWidth >= SH1106_LCDWIDTH) || // Clip right
		(y + _fontHeight >= SH1106_LCDHEIGHT) || // Clip bottom
		((x + _fontWidth + 1) < 0) || // Clip left
		((y + _fontHeight) < 0))   // Clip top
		return;

	uint8_t i, j, n;
	uint8_t diff, yActual, actualByte;
	uint8_t byteHeight = intCeil(_fontHeight, 8) /*ceil(_fontHeight / 8.0)*/;

	// height / 8, because we will plot in columns with the height of 8
	for (j = 0; j < byteHeight; j++)
  {
		// calculate the start pos
		diff = (y + j * 8) % 8;
		for (n = 0; n < ((0 == diff) ? 1 : 2); n++) // if it starts on the page border, then we can do it in one run
		{
			yActual = y + ((j + n) * 8);
			if (yActual > SH1106_LCDHEIGHT)
			{ // don't try to write outside the display area
				continue;
			}
			//_startRMWMode(x, y + ((j + n) * 8));
      _setDisplayWritePosition(x, y + ((j + n) * 8));
			// loop through the width of the image, and plot the columns
			for (i = 0; i < _fontWidth; i++)
      {
				if ((x + i) > SH1106_LCDWIDTH)
				{ // don't try to write outside the display area
					continue;
				}

				if (i == _fontWidth || (character == ' ' && ((_fontFlags & FONT_HASSPACE) != FONT_HASSPACE)))
        {
					actualByte = 0x00;
				}
				else
        {
					actualByte = pgm_read_byte(_font + ((character - _fontOffset) * _fontWidth + i) * byteHeight + j);
				}
        
				if (0 == n)
				{
					actualByte = actualByte << diff;   // this is the below the page barrier
				}
				else
				{
					actualByte = actualByte >> (8 - diff);   // this is the leftover - this only comes in play when the current part of the image crosses the page boundary
				}
        
				// display the column of pixels
				_drawColumn(actualByte);
      }
		}
	}

}


/*
Draws a character on the screen from the font
x: the x coordinate
y: the y coordinate
*/
void SH1106Lib::_setDisplayWritePosition(uint8_t x, uint8_t y)
{
	if (x == _pixelPosX && ((y >> 3) == _pixelPosY))
	{ // should not try to set on the same position again
		return;
	}

	_pixelPosX = x;
	_pixelPosY = y >> 3;

	// the SH1106 display starts at x = 2! (there are two columns of off screen pixels)
	x += 2;

	//set page address - there are 8 pages (B0H-B7H), each loop sets the next page - THIS IS THE Y COORD IN 8 INCREMENTS, 0->7
	sendCommand(SH1106_PAGEADDR + (y >> 3));
	// set lower column address  (00H - 0FH) => need the upper half only - THIS IS THE X, 0->127
	sendCommand((x & 0x0F));
	// set higher column address (10H - 1FH) => 0x10 | (2 >> 4) = 10
	sendCommand(SH1106_SETCOLUMNADDRHIGH + (x >> 4));
}


void SH1106Lib::_drawColumn(uint8_t data)
{
  digitalWrite(dcPin, HIGH);
	sendData(data);
}

void SH1106Lib::_drawColumns(uint8_t x, uint8_t y, uint8_t data, uint8_t count)
{
  _setDisplayWritePosition(x, y);
	for (uint8_t i = 0; i < count; i++)
	{
		_drawColumn(data);
	}
}


#endif // _SH1106LIB_H