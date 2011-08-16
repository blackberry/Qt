/*
 * c_client.h
 *
 *  Created on: Jun 9, 2011
 *      Author: achapiro
 */

#ifndef C_CLIENT_H_
#define C_CLIENT_H_


#include <imf/input_data.h>
#include <imf/input_connection.h>
#include <imf/input_connection_interface.h>



#if defined(__cplusplus)
extern "C"  {
#endif

/**
 * Initialize input services connection
 * Return: EOK or error code (see errno)
 */
int32_t imf_client_init();

/**
 * Disconnect application from input service
 */
void imf_client_disconnect();


/**
 * Register connection interface. If particular method is not implemented,
 * error code will be returned.
 * Arguments:
 * connection_interface * pconnection_interface - will be used for this session invocations
 *      until  session closing; if argument == 0, default interface will be used
 * Return: unique session id.
 *
 */
const input_session_t *ictrl_open_session(connection_interface_t * pconnection_interface);

/**
 * Close connection session; if session is closed,  input connection invocations
 * with the same session will be ignored.
 */
void ictrl_close_session(input_session_t * p_is);

/**
 * Return number of active sessions
 */
int32_t ictrl_get_num_active_sessions();

/**
 * User can setup some data which will be transmitted in input_connection
 * invocation. Set the first parameter to 0 if you assign data to default
 * interface (you are going to work without explicit session opening).
 */
void ictrl_set_user_data(const input_session_t *p_is, void * pDefaultUserData);

/**
 *  Default implementation of callback function that is invoked from
 *  reader thread. User can replace this function using set_callback
 *  function to provide more sophisticated processing. The example of such
 *  implementation is Cascades demo application which replace simple_process_user_data
 *  with callback function which in order to avoid illegitimate processing
 *  being out of main thread just takes data and posts message contained this
 *  data into the main thread queue. Another function that reads this queue
 *  utilizes simple_process_user_data without any risk of data corruption.
 */
int32_t simple_process_user_data(uint8_t * buffer, int32_t buffer_length);

/**
 * See description of simple_process_user_data above
 */
typedef int32_t (* callback_process_user_data) (uint8_t * buffer, int32_t buffer_length);

/**
 * See description of simple_process_user_data above
 */
void  imf_set_process_user_data_callback(callback_process_user_data user_callback);

/**
 * Return default connection interface
 */
connection_interface_t * get_default_interface();

#if defined(__cplusplus)
}
#endif


#endif /* C_CLIENT_H_ */
