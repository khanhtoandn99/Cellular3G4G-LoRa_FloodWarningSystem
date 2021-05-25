/*
 * debug.cpp
 *
 *  Created on: May 1, 2021
 *      Author: ASUS
 */

#include "hw.h"


debug::debug( UART_HandleTypeDef * _huartx ){
	huartx = &*_huartx ;
}


void debug::print( const char *_info ){
	HAL_UART_Transmit(huartx, (uint8_t*)_info, (int)strlen(_info), 1000) ;
}


void debug::println( const char *_info ){
	HAL_UART_Transmit(huartx, (uint8_t*)"\n", 1, 1000) ;
	HAL_UART_Transmit(huartx, (uint8_t*)_info, (int)strlen(_info), 1000) ;
}



unDebug::unDebug(){

}


void unDebug::print( const char *_info ){
	// do nothing
}


void unDebug::println( const char *_info ){
	// do nothing
}

