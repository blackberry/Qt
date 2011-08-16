/*
 * events.h
 *
 *  Created on: Mar 2, 2011
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



#ifndef EVENTS_H_
#define EVENTS_H_

#include <stdint.h>
#include <imf/input_data.h>
#include <pthread.h>
#include <sys/keycodes.h>

typedef enum {
   EVENT_NOTHING = 0,
   EVENT_KEY = 1,
   EVENT_CARET = 2,
   EVENT_NAVIGATION,
   EVENT_FOCUS,
   EVENT_ACTION,
   EVENT_USER_ACTION,
   EVENT_SPELL_CHECK,
   EVENT_STROKE,
   EVENT_INVOKE_LATER,
   USER_EVENT_START=100,
} EventType ;


#define    IS_CONSUMED(event_flags)  ( event_flags  & 0x01 )
#define    SET_CONSUMED(event_flags) ( event_flags  |= 0x01)
#define    UNSET_CONSUMED(event_flags) ( event_flags  &= 0xfffffffe )

#define    IS_REPORT_CONSUMED_STATUS(event_flags)  (  event_flags & 0x02 )
#define    SET_REPORT_CONSUMED_STATUS(event_flags)  (  event_flags  |=  0x02 )
#define    UNSET_REPORT_CONSUMED_STATUS(event_flags)  (  event_flags  &=  0xfffffffd )

#define   IS_REPORT_SYNCHRONOUS(event_flags)  (  event_flags & 0x04 )
#define   SET_REPORT_SYNCHRONOUS(event_flags)  (  event_flags  |=  0x04 )
#define   SET_REPORT_ASYNCHRONOUS(event_flags)  (  event_flags  &=  0xfffffffb )

typedef struct {
  int32_t event_id;
  EventType event_type;
  int32_t pid;
  int32_t component_id;
  int32_t transaction_id;
  int32_t event_flags;
  int32_t event_cause;
} event_t;

/*******************************************************
* Key Event
********************************************************/

typedef enum {
	IMF_KEY_DOWN = 1,
	IMF_KEY_UP,
	IMF_KEY_REPEAT,
} KeyEventId;

typedef enum {
	SPECIAL_KEY_DELETE = 1,
	SPECIAL_KEY_BACKSPACE,
	SPECIAL_KEY_SPACE,
	SPECIAL_KEY_ESCAPE,
	SPECIAL_KEY_ENTER,
} SpecialKey;

typedef enum {
	META_ALT_LEFT_ON = 0x01,
	META_ALT_RIGHT_ON = 0x02,
	META_ALT_ON = (META_ALT_LEFT_ON | META_ALT_RIGHT_ON ),
	META_SHIFT_LEFT_ON = 0x04,
	META_SHIFT_RIGHT_ON = 0x08,
	META_SHIFT_ON = (META_SHIFT_LEFT_ON | META_SHIFT_RIGHT_ON ),
	META_CTRL_LEFT_ON = 0x10,
	META_CTRL_RIGHT_ON = 0x20,
	META_CTRL_ON = (META_CTRL_LEFT_ON | META_CTRL_RIGHT_ON ),
	META_CAPS_LOCK_ON = 0x40,
	META_SYM_ON = 0x80,
} MetaKeyState;

typedef struct {
  event_t event;
  imf_char_t character;
  int32_t key_code;
  uint32_t meta_key_state;
} key_event_t;

/*******************************************************
* Caret Event
********************************************************/

typedef enum {
	CARET_POS_CHANGED = 1,
	CHECK_SPELLING
} CaretEventId;

typedef struct {
	  event_t event;
	  int32_t old_pos;
	  int32_t new_pos;
} caret_event_t;

/*******************************************************
* Navigate Event
********************************************************/

typedef enum {
	NAVIGATE_UP = 1,
	NAVIGATE_DOWN,
	NAVIGATE_LEFT,
	NAVIGATE_RIGHT,
} NavigateKeyEventId;

typedef struct {
  event_t event;
  int32_t magnitude;
} navigation_event_t;

/*******************************************************
* Focus Event
********************************************************/

typedef enum {
	FOCUS_GAINED = 1,
	FOCUS_LOST,
} FocusEventId;

typedef enum {
    DEFAULT_STYLE       =        0,   // default style with full support
    NO_COMPLEX_INPUT    =   1 << 0,   // Typed characters will be directly printed with no conversion or prediction.
                                      // This implicitly means that there will be no autotext as well.
    NO_AUTO_TEXT        =   1 << 1,   // no autocap, autoperiod, and word substitution
    NO_PREDICTION       =   1 << 2,   // no predictions will be provided
    NO_AUTO_CORRECTION  =   1 << 3,   // do not correct what the user typed
} InputStyle;

typedef struct {
  event_t event;
  int64_t style; // contains a set of InputStyle masks
  // I think the next two fields are redundant, leave them for a while
  int32_t other_pid;
  int32_t other_component_id;
} focus_event_t;

/*******************************************************
* Action Event
********************************************************/

typedef enum {
	ACTION_END_COMPOSITION = 1,
	ACTION_TEXT_CHANGED,
	ACTION_ADD_WORD,
	ACTION_REMOVE_WORD,
	// SpellCheck Group start
	ACTION_CHECK_MISSPELLINGS,
	ACTION_CHECK_MISSPELLINGS_SYNC,
	ACTION_GET_SUGGESTIONS,
	ACTION_GET_SUGGESTIONS_SYNC,
	// SpellCheck Group end
	ACTION_SET_OPTION,
	ACTION_OPTIONS_CHANGED,
	// Service actions
	ACTION_SET_LOCALE = 100,
} ActionEventId;

typedef struct {
  event_t event;
  int32_t length_data;
  void* event_data;
} action_event_t;

/*******************************************************
* User Action Event
********************************************************/

typedef enum {
	CANDIDATE_HIGHLIGHTED = 1,
	CANDIDATE_SELECTED,
	HORIZONTAL_SWIPE,
	ERASE_LEARNING_DICTIONARIES
} UserActionEventId;

typedef struct {
  event_t event;
  char* event_data;
  int32_t length_data;
  int32_t candidates_id;
  int32_t index;
} user_action_event_t;

/*******************************************************
* Spell Check Event
********************************************************/

typedef enum {
	NOTIFY_SP_GET_SUGGESTIONS = 1,
	NOTIFY_SP_CHECK_MISSPELLINGS,
} SpellCheckEventId;

typedef struct {
  event_t event;
  int32_t data_length;
  spannable_string_t** data;
} spell_check_event_t;

/*******************************************************
* Stroke Event
********************************************************/

typedef enum {
	STROKE_STARTED = 1,
	STROKE_DRAWN,
} StrokeEventId;

typedef struct {
	event_t event;
	int32_t path_length;
	int16_t* path;
	int32_t width;
	int32_t height;
} stroke_event_t;

#endif /* EVENTS_H_ */
