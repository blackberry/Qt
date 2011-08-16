/*
 * spannable_string.h
 *
 *  Created on: Mar 3, 2011
 *  Author: achapiro
 *  Research In Motion Ltd., 2010-
 *  295 Phillip Street Waterloo, ON N2L 3W8
 *  Canada
 *  This software is the confidential and proprietary information of
 *  Research In Motion Ltd. ("Confidential Information").  You shall not
 *  disclose such Confidential Information and shall use it only in
 *  accordance with the terms of the license agreement you entered into
 *  with Research In Motion.
 */



#ifndef SPANNABLE_STRING_H_
#define SPANNABLE_STRING_H_

#include <stdint.h>
#include <stddef.h>

#ifndef SIZEOF_WCHAR
#if __WCHAR_MAX__ > 0x10000
#define SIZEOF_WCHAR 4
#else
#define SIZEOF_WCHAR 2
#endif
#endif


typedef wchar_t imf_char_t;

typedef enum {
	COMPOSED_TEXT_ATTRIB = 0x01,
	ACTIVE_REGION_ATTRIB = 0x02,
	MISSPELLED_WORD_ATTRIB = 0x04,
	VERBATIM_ATTRIB = 0x08,
	PREDICTION_ATTRIB = 0x10,
	SPELLCHECK_ATTRIB = 0x20,
} SpannableStringAttribute;

typedef struct _span {
  uint64_t attributes_mask; // Combination of attributes
  size_t start; // start index of the span in the string
  size_t end; // end index of the span in the string
} span_t;


typedef struct {
  imf_char_t* str;
  size_t length;
  span_t* spans;
  size_t spans_count;
} spannable_string_t;




#endif /* SPANNABLE_STRING_H_ */
