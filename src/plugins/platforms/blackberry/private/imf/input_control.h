/*
 * input_control.h
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



#ifndef INPUT_CONTROL_H_
#define INPUT_CONTROL_H_

#include <imf/events.h>

#if defined(__cplusplus)
namespace InputMethodSystem {
extern "C" {
#endif

/**Dispatches input events into the input system. While processing input method
 * may or may not consume the event. If the event is not consumed then calling
 * component should process it.
 */
int32_t ictrl_dispatch_event(/*in-out*/event_t* event);

/**
 * Gets an array of locales that are supported by the current input method.
 * The number of available locales is returned in the specified output parameter
 * locales_count.
 */
input_locale_t* ictrl_get_available_locales(/*in-out*/int32_t* locales_count);



/**
 * Gets an array of input modes that are supported by the current input method.
 * The number of the input modes is returned in the specified output parameter
 * input_modes_count.
 */
int32_t * ictrl_get_available_input_modes(/*in-out*/int32_t* input_modes_count);

/**
 * Sets the specified input mode to make it the input mode for the current input
 * method.
 * Returns 0 if success, -1 if error.
 */
int32_t ictrl_set_input_mode(int32_t input_mode);

/**
 * Gets the current input locale.
 */
input_locale_t* ictrl_get_locale();

/**
 * Sets the current input locale.
 * Returns 0 if success, -1 if error.
 */
int32_t ictrl_set_locale(input_locale_t* locale);

//}
//
#if defined(__cplusplus)
}
}
#endif
#endif /* INPUT_CONNECTION_H_ */
