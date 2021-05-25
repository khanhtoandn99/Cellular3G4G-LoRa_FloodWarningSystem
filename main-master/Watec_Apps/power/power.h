/*
 * power.h
 *
 *  Created on: May 1, 2021
 *      Author: ASUS
 */

#ifndef POWER_POWER_H_
#define POWER_POWER_H_

#ifdef __cplusplus
extern "C" {
#endif


class power {
public:
	bool rxDone_FLAG = 0 ;

	char previousBuff  = 0;
	char rxBuff[1] ;
	char rxData[100] ;
	int  dataCount = 0 ;

	int status ;

	// Du lieu toan bo :
	char powData[20] ;

	power( UART_HandleTypeDef * _huartx ) ;

	void IRQhandler() ;

	void init() ;

	bool getPeriodData() ;


private:
	// First User config here :
	int rxData_MAX = 100 ;

	UART_HandleTypeDef *huartx ;

	bool sendCommand (const char* ATcommand, const char* expected_answer, int timeout) ;

	bool sendCommand (const char* ATcommand, const char* expected_answer, int timeout, int retryTime ) ;

	int response( const char* response, int timeout ) ;
};


#ifdef __cplusplus
}
#endif

#endif /* POWER_POWER_H_ */
