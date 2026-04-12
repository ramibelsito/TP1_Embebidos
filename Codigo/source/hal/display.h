#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdbool.h>
#include <stdint.h>

#define DIGITS 4

void initDisplay();
uint8_t getDutyPercentage();
void setDutyPercentage(uint8_t percentage);

/**
 * @param segments
 * 8-bit mask representing the state of each segment.
 * Each bit corresponds to one segment in the display:
 *
 *   Bit 0 (LSB) → segment 'a'
 *   Bit 1       → segment 'b'
 *   Bit 2       → segment 'c'
 *   Bit 3       → segment 'd'
 *   Bit 4       → segment 'e'
 *   Bit 5       → segment 'f'
 *   Bit 6       → segment 'g'
 *   Bit 7 (MSB) → decimal point (dp)
 *
 * A bit value of 1 turns the corresponding segment ON, while 0 turns it OFF.
 *
 * @param digit
 * Index of the digit to update
 *
 * @param dutyPercentage
 * Brightness control expressed as a percentage (0–100).
 *
 * @return true if the segments were successfully written,
 *         false otherwise (e.g., invalid digit index or hardware error).
 */
bool writeSegments(uint8_t segments, uint8_t digit);

/**
 * @brief Writes one character to the specified digit.
 *
 * @param character Character to write to display
 * Valid characters are:
 *    A-Z (uppercase)
 *    0-9
 *    ' ' (white space)
 *    '-' (score)
 *    '.' (decimal point)
 *
 * @param digit Index of the display digit to write the character to.
 *
 * @param blink Defines whether the character will blink or no.
 */
bool writeCharacter(char character, uint8_t digit, bool blink);

/**
 * @brief Writes a string of upto 50 characters to the display and cycles the text to fit
 * in the 4 digit display.
 *
 * @param string String to write to display
 * Valid string characters are:
 *    A-Z (uppercase)
 *    0-9
 *    ' ' (white space)
 *    '-' (score)
 *    '.' (decimal point)
 *
 *  The string will overrite a character written through `writeCharacter`, to make individual
 *  characters stick you must `cleanDisplay`.
 */
void writeString(const char* string);

void cleanDisplay();

#endif // _DISPLAY_H_
