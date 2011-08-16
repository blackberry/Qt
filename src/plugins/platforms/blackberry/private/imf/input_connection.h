/*
 * input_connection.h
 *
 *  Created on: Mar 2, 2011
 *  Author: achapiro
 *  Research In Motion Ltd., 2010-295 Phillip Street
 *  Waterloo, ON N2L 3W8 Canada
 *  This software is the confidential and proprietary information of
 *  Research In Motion Ltd. ("Confidential Information").  You shall not
 *  disclose such Confidential Information and shall use it only in
 *  accordance with the terms of the license agreement you entered into
 *  with Research In Motion.
 */



#ifndef INPUT_CONNECTION_H_
#define INPUT_CONNECTION_H_


#if defined(__cplusplus)
extern "C" {
#endif


#include <stdint.h>
#include <imf/input_data.h>
#include <imf/spannable_string.h>
#include <imf/events.h>


typedef enum {
	IME_ACTION_CLEAR_DRAWING_AREA = 1,
	IME_ACTION_DONE,
	IME_ACTION_GO,
	IME_ACTION_NEXT,
	IME_ACTION_SEARCH,
} EditorAction;

typedef enum {
	CAP_MODE_CHARACTERS = 0x01,
	CAP_MODE_SENTENCES	= 0x02,
	CAP_MODE_WORDS		= 0x04,
} CapsMode;

typedef enum {
    // next flag (the only one for now) indicate you would like
	// to receive updates when the extracted text changes. When the
	// text has changed, then the application platform
	// must send back a TEXT_CHANGED action_event_t
	// to the input system
	GET_EXTRACTED_TEXT_MONITOR = 1,
} ExtractedFlags;

/**
 * Tell the editor that you are starting a batch of editor operations. The editor
 * will try to avoid sending you updates about its state until ic_end_batch_edit()
 * is called. Returns 0 if success, -1 if the input connection is no longer valid.
 */
int32_t ic_begin_batch_edit(input_session_t* ic);

/**
 * Clear the given meta key pressed states in the given input connection.
 */
int32_t ic_clear_meta_key_states(input_session_t* ic, int32_t states);

/**
 * Commit text to the text box and set the new cursor position. Any composing
 * text set previously will be removed automatically.
 */
int32_t ic_commit_text(input_session_t* ic, spannable_string_t* text,
                       int32_t new_cursor_position);

/**
 * Delete left_length characters of text before the current cursor position, and
 * delete right_length characters of text after the current cursor position,
 * excluding composing text.
 * left_length	the number of characters to be deleted before the current cursor
 * 		position
 * right_length 	the number of characters to be deleted after the current
 * 		cursor position
 * Returns: 0 if success, -1 if the input connection is no longer valid.
 */
int32_t ic_delete_surrounding_text(input_session_t* ic, int32_t left_length,
        int32_t right_length);

/**
 * Tell the editor that you are done with a batch edit previously initiated
 * with ic_begin_batch_edit().
 * Returns: 0 if success, -1 if the input connection is no longer valid.
 */
int32_t ic_end_batch_edit(input_session_t* ic);

/**
 * Have the text editor finish whatever composing text is currently active.
 * This simply leaves the text as-is, removing any special composing styling or
 * other state that was around it. The cursor position remains unchanged.
 * Returns: 0 if success, -1 if the input connection is no longer valid.
 */
int32_t ic_finish_composing_text(input_session_t* ic);

/**
 * Retrieve the current capitalization mode in effect at the current cursor
 * position in the text. This method may fail either if the input connection
 * has become invalid (such as its process crashing) or the client is taking
 * too long to respond with the text (it is given a couple seconds to return).
 * In either case, a 0 is returned.
 * Parameters:
 * req_modes	The desired modes to retrieve.
 * Returns: the caps mode flags that are in effect.
 */
int32_t ic_get_cursor_caps_mode(input_session_t* ic, int32_t req_modes);

/**
 * Gets the cursor position in the component.
 * Returns -1 if the input connection is no longer valid.
 */
int32_t ic_get_cursor_position(input_session_t* ic);



/**
 * Retrieve the current text in the input connection's editor, and monitor for
 * any changes to it. This function returns with the current text, and optionally
 * the input connection can send updates to the input method when its text changes.
 * Parameters:
 * 	request	Description of how the text should be returned.
 *  flags	Additional options to control the client, either 0 or GET_EXTRACTED_TEXT_MONITOR
 * Returns: an extracted_text_t describing the state of the text view and containing
 * 	the extracted text itself.
 */
extracted_text_t* ic_get_extracted_text(input_session_t* ic,
             extracted_text_request_t* request,
             int32_t flags);

/**
 * Gets the selected text, if any.
 * Parameters:
 *	flags	Supplies additional options controlling how the text is returned.
 *		May be either 0 or GET_TEXT_WITH_STYLES.
 * Returns: the text that is currently selected, if any, or null.
 */
spannable_string_t* ic_get_selected_text(input_session_t* ic, int32_t flags);

/**
 * Get n characters of text after the current cursor position.
 * Parameters:
 *	n	the expected length of the text.
 *	flags	Supplies additional options controlling how the text is returned.
 *  	May be either 0 or GET_TEXT_WITH_STYLES which will have style information
 *  	returned along with the text.
 * Returns: the text after the cursor position; the length of the returned text
 * 		might be less than n.
 */
spannable_string_t* ic_get_text_after_cursor(input_session_t* ic, int32_t n,
                  int32_t flags);

/**
 * Get n characters of text before the current cursor position.
 *	n	the expected length of the text.
 *	flags	Supplies additional options controlling how the text is returned.
 *  	May be either 0 or GET_TEXT_WITH_STYLES which will have style information
 *  	returned along with the text.
 * Returns: the text before the cursor position; the length of the returned text
 * 		might be less than n.
 */
spannable_string_t* ic_get_text_before_cursor(input_session_t* ic, int32_t n,
                   int32_t flags);

/**
 * Have the editor perform an action.
 * Parameters:
 *	editor_action	One of the editor action ids.
 * Returns: 0 on success, -1 if the input connection is no longer valid.
 */
int32_t ic_perform_editor_action(input_session_t* ic, int32_t editor_action);


/**
 * Called by the IME to tell the client when it switches between fullscreen and
 * 	normal modes.
 * Returns 0 on success, -1 if the input connection is no longer valid.
 */
int32_t ic_report_fullscreen_mode(input_session_t* ic, int32_t enabled);

/**
Used by the input system to asynchronously send back an event to the application platform. This function doesn’t block after the event is sent to the application platform. For now, only spell_check_event_t are sent in this function. For more information on spell check, please refer to section 5.
Parameters:
	event	the event
Returns:
Returns 0 on success, -1 if the input connection is no longer valid.
*/
int32_t ic_send_async_event(input_session_t* ic, event_t* event);
/**
 * Send a key event to the process that is currently attached through this input
 * connection. The event will be dispatched like a normal key event, to the
 * currently focused component.
 * Deprecated: use ic_send_event instead of ic_send_key_event
 * Parameters:
 *	event	the key event
 * Returns: 0 on success, -1 if the input connection is no longer valid.
 */
int32_t ic_send_key_event(input_session_t* ic, key_event_t * key_event)
	__attribute__ ((deprecated ));

/**
 * Send an event to the process that is currently attached through this input
 * connection. The event will be dispatched like a normal event, to the
 * currently focused component.
 * Parameters:
 *	event	the event
 * Returns: 0 on success, -1 if the input connection is no longer valid.
 */
int32_t ic_send_event(input_session_t* ic, event_t * event);

/**
 * Mark a certain region of text as composing text. Any composing text set
 * previously will be removed automatically. The default style for composing
 * text is used.
 * Parameters:
 *	start	the position in the text at which the composing region begins
 *	end	the position in the text at which the composing region ends
 * Returns: 0 if success, -1 if the input connection is no longer valid.
 */
int32_t ic_set_composing_region(input_session_t* ic, int32_t start,
     int32_t end);

/**
 * Set composing text around the current cursor position with the given text,
 * and set the new cursor position. Any composing text set previously will be
 * removed automatically.
 * Parameters:
 *	text	The composing text with styles if necessary. If no style object
 *		attached to the text, the default style for composing text is used.
 * 	newCursorPosition	The new cursor position around the text. If > 0, this
 * 		is relative to the end of the text - 1; if <= 0, this is relative to
 * 		the start of the text.
 * Returns: 0 if success, -1 if the input connection is no longer valid.
 */
int32_t ic_set_composing_text(input_session_t* ic, spannable_string_t* text,
   int32_t new_cursor_position);

/**
 * Sets the selection of the text component. To set the cursor position, start
 * and end should have the same value.
 * Returns: 0 if success, -1 if the input connection is no longer valid.
 */
int32_t ic_set_selection(input_session_t* ic, int32_t start, int32_t end);

/**
 * Sets that candidates that should be shown to the user.
 * Parameters:
 *	candidates		The candidates to show.
 *  more_candidates	The additional candidates to show. These should be shown
 *  	close to the selected candidate of the primary candidates.
 * Returns: 0 if success, -1 if the input connection is no longer valid.
 */
int32_t ic_set_candidates(input_session_t* ic, candidates_t* candidates,
	candidates_t* more_candidates);

#if defined(__cplusplus)
}
#endif

#endif /* INPUT_CONNECTION_H_ */
