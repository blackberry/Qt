/*
 * spell_check_services.h
 *
 *  Created on: Jul 27, 2011
 *      Author: achapiro
 */

#ifndef SPELL_CHECK_SERVICES_H_
#define SPELL_CHECK_SERVICES_H_

#include "imf/events.h"
#include "imf/spannable_string.h"
#include "sys/imf.h"

typedef int32_t (* ic_send_async_event_t)(input_session_t * ic ,event_t * evt);
typedef spannable_string_t ** ap_suggestions;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize spell checker "synchronous" interface
 */
int32_t sp_init();


/**
 * Check words for misspelling. Caller is also responsible for releasing memory for misspellings
 * Arguments:
 * imf_char_t * p_text_to_check
 * size_t text_length
 * imf_sc_misspellings_t ** misspellings
 * Return: EOK in case of success, otherwise
 */
int32_t sp_check_misspelling(/* in */ const imf_char_t * p_text_to_check,
		/* in */ size_t text_length, /* out */ imf_sc_misspellings_t ** misspellings );

/**
 * Request for correction suggestions. Caller is also responsible for releasing memory for p_word_suggestions
 * Arguments:
 * imf_char_t * p_misspelled_word
 * size_t word_length
 * size_t max_num_suggestions
 * imf_sc_suggestions_t ** p_word_suggestions
 * Return: EOK in case of success, otherwise
 */
int32_t sp_get_suggestions(/* in */ const imf_char_t * p_misspelled_word,
		/* in */ size_t word_length, /* in */ size_t max_num_suggestions,
		/* out */ imf_sc_suggestions_t ** p_word_suggestions );

#ifdef __cplusplus
}
#endif

#endif /* SPELL_CHECK_SERVICES_H_ */
