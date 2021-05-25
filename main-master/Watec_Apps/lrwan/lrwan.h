/*
 * lrwan.h
 *
 *  Created on: May 1, 2021
 *      Author: ASUS
 */

#ifndef LRWAN_LRWAN_H_
#define LRWAN_LRWAN_H_


// Tạo class để sử dụng cho module SIM ( kế thừa class debug để debug )
class lrwan {
public:

	// Cờ ngắt nhận data
	bool rxDone_FLAG = 0 ;

	// Byte đệm
	char rxBuff[1] ;

	// Dữ liệu nhận được từ module SIM
	char rxData[RX_DATA_MAX] ;
	int  dataCount = 0 ;


	lrwan( UART_HandleTypeDef * _huartx ) ;

	void IRQhandler() ;

	void init() ;

	// Cập nhật dữ liệu vào bộ đệm và gửi luôn
	void updateData( const char *_Data, int timeout ) ;
	// chỉ cập nhật dữ liệu vào bufer
//	void updateData( float _Temp, float _Humi, const char* _U_out, const char* _Power, const char* _I_pv, const char* _I_load, int _status);
	void updateData( float _Temp, float _Humi, char _powData[20], int _status) ;

	// Gửi dữ liệu lên Server
	void send() ;

	// Kiểm tra và xử lý sự kiện đến
	void checkingAndProcess() ;

private:

	// UART control module sim
	UART_HandleTypeDef *huartx ;


	bool sendCommand (const char* command, const char* expected_answer, int timeout) ;

	void response (const char* response, int timeout) ;

	//
	char lrData[50];
};

#endif /* LRWAN_LRWAN_H_ */
