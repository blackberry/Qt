/*
 * imf_vkb_helper.h
 *
 *  Created on: Jul 5, 2011
 *      Author: achapiro
 */

#ifndef IMF_VKB_HELPER_H_
#define IMF_VKB_HELPER_H_
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#include <wchar.h>
#include "sys/imf.h"
#include <imf/input_data.h>

//#if defined(__cplusplus)
//namespace InputMethodSystem {
//#endif

typedef enum {
	VARIANT_CANCELLED = 0,
	VARIANT_SELECTED = 1,
	VARIANT_HIGHLIGHTED = 2,
} variant_status;

typedef int32_t (* candidate_selection_changed)(int32_t index, int32_t  status);
typedef void (* locale_changed)(imf_handle_t hdl, imf_event_t *event);

typedef int32_t vkb_handle_t;


#if defined(__cplusplus)
extern "C" {
#endif

void set_vkb_callbacks(locale_changed lccb, candidate_selection_changed cshcb);
imf_handle_t set_switch_locale_callback(imf_callbacks_t * pslcb);
void disconnect_switch_locale_callback();
const char * get_current_locale();

/**
 * Next two functions represent global vkb selection  service
 * If this one is set on, all instances of client interface (input connection) could
 * share VKB selection resource. This service and  set_candidate_selection_changed_callback
 * are mutual exclusive.
 * You should initialize this service after initialization of client library.
 * In case of multi-interface application any interface owner can stick with
 * its own implementation of candidate lookup (remember: it shouldn't use VKB lookup
 * service). For such case global interface will be ignored.
 */
int32_t vkb_init_selection_service() __attribute__ ((deprecated ));
void vkb_destroy_selection_service() __attribute__ ((deprecated ));

imf_handle_t set_candidate_selection_changed_callback(candidate_selection_changed cshcb);
void disconnect_candidate_selection_changed_callback();
bool vkb_candidates_service_on();

int32_t vkb_send_list(candidates_t* candidates,  candidates_t* more_candidates) ;
int32_t vkb_clear_list();

int32_t vkb_getindex(const wchar_t * wword);
const wchar_t * vkb_getWord(int32_t index);


int vkb_hide();
int vkb_show();

#if defined(__cplusplus)
}
//};
#endif

#endif /* IMF_VKB_HELPER_H_ */
