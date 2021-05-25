/*
 * debug.h
 *
 *  Created on: May 1, 2021
 *      Author: ASUS
 */

#ifndef DEBUG_DEBUG_H_
#define DEBUG_DEBUG_H_


// User change uart here :
#define DEBUG_UART huart5


class debug {
public:
	debug( UART_HandleTypeDef * _huartx );

	// Hàm in
	void print( const char *_info ) ;

	void println( const char *_info ) ;

private:
	UART_HandleTypeDef *huartx ;
};


class unDebug {
public:
	unDebug();

	// Hàm in
	void print( const char *_info ) ;

	void println( const char *_info ) ;
};


#endif /* DEBUG_DEBUG_H_ */
