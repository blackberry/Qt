/*
 * input_connection_interface.h
 *
 *  Created on: Aug 16, 2011
 *      Author: achapiro
 */
#ifndef INPUT_CONNECTION_INTERFACE_H_
#define INPUT_CONNECTION_INTERFACE_H_

#include <imf/input_connection.h>


#if defined(__cplusplus)
extern "C" {
#endif


typedef int32_t (*ic_begin_batch_edit_t)(input_session_t* ic);
typedef int32_t (*ic_clear_meta_key_states_t)(input_session_t* ic, int32_t states);
typedef int32_t (*ic_commit_text_t)(input_session_t* ic, spannable_string_t* text,
                           int32_t new_cursor_position);
typedef int32_t (*ic_delete_surrounding_text_t)(input_session_t* ic, int32_t left_length,
            int32_t right_length);
typedef int32_t (*ic_end_batch_edit_t)(input_session_t* ic);
typedef int32_t (*ic_finish_composing_text_t)(input_session_t* ic);
typedef int32_t (*ic_get_cursor_caps_mode_t)(input_session_t* ic, int32_t req_modes);
typedef int32_t (*ic_get_cursor_position_t)(input_session_t* ic);
typedef extracted_text_t* (*ic_get_extracted_text_t)(input_session_t* ic,
                 extracted_text_request_t* request, int32_t flags);
typedef spannable_string_t* (*ic_get_selected_text_t)(input_session_t* ic, int32_t flags);
typedef spannable_string_t* (*ic_get_text_after_cursor_t)(input_session_t* ic, int32_t n,
                 int32_t flags);
typedef spannable_string_t* (*ic_get_text_before_cursor_t)(input_session_t* ic, int32_t n,
                 int32_t flags);
typedef int32_t (*ic_perform_editor_action_t)(input_session_t* ic, int32_t editor_action);
typedef int32_t (*ic_report_fullscreen_mode_t)(input_session_t* ic, int32_t enabled);
typedef int32_t (*ic_send_key_event_t)(input_session_t* ic, key_event_t * key_event);
typedef int32_t (*ic_send_event_t)(input_session_t* ic, event_t * event);
typedef int32_t (*ic_send_async_event_t)(input_session_t* ic, event_t * event);
typedef int32_t (*ic_set_composing_region_t)(input_session_t* ic, int32_t start,
         int32_t end);
typedef int32_t (*ic_set_composing_text_t)(input_session_t* ic, spannable_string_t* text,
       int32_t new_cursor_position);
typedef int32_t (*ic_set_selection_t)(input_session_t* ic, int32_t start, int32_t end);
typedef int32_t (* ic_set_candidates_t)(input_session_t* ic, candidates_t* candidates,
        candidates_t* more_candidates);


typedef struct {
    ic_begin_batch_edit_t ic_begin_batch_edit;
    ic_clear_meta_key_states_t ic_clear_meta_key_states;
    ic_commit_text_t ic_commit_text;
    ic_delete_surrounding_text_t ic_delete_surrounding_text;
    ic_end_batch_edit_t ic_end_batch_edit;
    ic_finish_composing_text_t ic_finish_composing_text;
    ic_get_cursor_caps_mode_t ic_get_cursor_caps_mode;
    ic_get_cursor_position_t ic_get_cursor_position;
    ic_get_extracted_text_t ic_get_extracted_text;
    ic_get_selected_text_t ic_get_selected_text;
    ic_get_text_after_cursor_t ic_get_text_after_cursor;
    ic_get_text_before_cursor_t ic_get_text_before_cursor;
    ic_perform_editor_action_t ic_perform_editor_action;
    ic_report_fullscreen_mode_t ic_report_fullscreen_mode;
    ic_send_key_event_t ic_send_key_event;
    ic_send_event_t ic_send_event;
    ic_send_async_event_t ic_send_async_event;
    ic_set_composing_region_t ic_set_composing_region;
    ic_set_composing_text_t ic_set_composing_text;
    ic_set_selection_t ic_set_selection;
    ic_set_candidates_t ic_set_candidates;
} connection_interface_t;


#if defined(__cplusplus)
}
#endif




#endif /* INPUT_CONNECTION_INTERFACE_H_ */
