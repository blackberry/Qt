/*
 * c_unicode_converter.h
 *
 *  Created on: May 12, 2011
 *      Author: achapiro
 */

#ifndef C_UNICODE_CONVERTER_H_
#define C_UNICODE_CONVERTER_H_
#include <stdint.h>
#include <wchar.h>

#if defined(__cplusplus)
extern "C" {
#endif
/**
 * For the first three functions: user is responsible for the memory deallocation
 */
	const wchar_t * convertUTF8toWchar(const uint8_t * from_start, const uint8_t * from_end);
	const char * convertWchartoUTF8(const wchar_t * from_start, const wchar_t * from_end);
	const char * convertUTF16toUTF8(const uint16_t * from_start, const uint16_t * from_end);
	// Convert to pre-allocated buffer
	int32_t convertWchartoUTF8Ex(const wchar_t * from_start, const wchar_t * from_end,
			uint8_t * to_start, int32_t max_len, int32_t append0);
	int32_t multiBytePos(const char * buffer, int32_t wc_pos, int32_t maxlen);
	int32_t multiBytePosBack(const char * buffer, int32_t wc_pos, int32_t maxlen);
	int32_t wcharPos(const char * buffer, int32_t utf8_pos);
#if defined(__cplusplus)
}
#endif

#endif /* C_UNICODE_CONVERTER_H_ */
