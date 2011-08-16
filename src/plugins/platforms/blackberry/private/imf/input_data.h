/*
 * input_data.h
 *
 *  Created on: Mar 19, 2011
 *      Author: achapiro
 */

#ifndef INPUT_DATA_H_
#define INPUT_DATA_H_

#include <stdint.h>
#include <imf/spannable_string.h>


typedef struct {
  int32_t pid;
  int32_t component_id;
  void * p_user_data;
} input_session_t;

typedef struct {
  int32_t flags;
  int32_t hint_max_char;
  int32_t hint_max_lines;
  int32_t token;
} extracted_text_request_t;



typedef struct {
  int32_t flags;
  int32_t partial_end_offset;
  int32_t partial_start_offset;
  int32_t selection_end;
  int32_t selection_start;
  int32_t start_offset;
  spannable_string_t* text;
} extracted_text_t;

typedef struct {
  int32_t id;
  int32_t selected_index;
  int32_t verbatim_index;
  spannable_string_t** acandidates;
  int32_t candidates_count;
} candidates_t;

typedef struct {
	uint16_t language;
	uint16_t country;
	char * variant;
} input_locale_t;




#endif /* INPUT_DATA_H_ */
