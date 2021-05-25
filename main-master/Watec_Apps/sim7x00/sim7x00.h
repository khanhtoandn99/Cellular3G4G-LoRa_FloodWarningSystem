/*
 * sim7600x.h
 *
 *  Created on: May 1, 2021
 *      Author: ASUS
 */

#ifndef SIM7X00_SIM7X00_H_
#define SIM7X00_SIM7X00_H_


#define RX_DATA_MAX 2000


// Tạo class để sử dụng cho module SIM ( kế thừa class debug để debug )
class sim7x00 {
public:

	// Cờ ngắt nhận data
	bool rxDone_FLAG = 0 ;

	// Byte đệm
	char rxBuff[1] ;

	// Dữ liệu nhận được từ module SIM
	char rxData[RX_DATA_MAX] ;
	int  dataCount = 0 ;

	// Dữ liệu nội dung tin nhắn SMS :
	char SMSContent[RX_DATA_MAX] ;


	sim7x00( UART_HandleTypeDef * _huartx ) ;

	void IRQhandler() ;

	// Hàm khởi tạo setup ban đầu cho module sim
	void init() ;

	/**
	 * Hàm bắt đầu phát cảnh báo theo các mức :
	 * @param _level : mức cảnh báo, theo system_event_enum
	 * @param _times : số lần cảnh báo, từ 0 - 255
	 */
	bool warningStart(int _level, int _times) ;

	// Hàm stop cảnh báo
	bool warningStop() ;

	// Hàm setup các số điện thoại quản trị viên :
	void setAllowedPhoneNumber( const char *list[], int _quantity ) ;

	// Hàm hẹn giờ cảnh báo


	// Kiểm tra sự kiện đến và xử lý yêu cầu :
	void checkingAndProcess() ;

	int checkIncommingEvent() ;

	// Hàm Mở và đọc nội dung tin nhắn :
	void readSMS() ;

	// Hàm xác minh số điện thoại có phải số được hệ thống cấp phép điều hành hay không
	bool isAuthorizePhoneNumber( const char *_SMScontent ) ;

	// Hàm xác thực nội dung yêu cầu từ tin nhắn
	int identifySMSRequest( const char *_SMScontent ) ;

	// kích hoạt MQTT và kết nối đến server
	bool MQTTStart ( const char* _deviceName, const char* _MQTTWillTopic, const char* _MQTTWillMsg, const char* _BrokerAddress ) ;

	// Đẩy dữ lệu lên Broker Topic
	void MQTTPublishData ( const char* _topic, const char* _data ) ;
	void MQTTPublishData () ;

	// Cập nhật dữ liệu vào bộ đệm  :
	void updateData( float _Temp, float _Humi, const char* _U_out, const char* _Power, const char* _I_pv, const char* _I_load, int _status);

private:

	// UART control module sim
	UART_HandleTypeDef *huartx ;


	bool sendATcommand (const char* ATcommand, const char* expected_answer, int timeout) ;

	bool sendATcommand (const char* ATcommand, const char* expected_answer, int timeout, int retryTime ) ;

	int response (const char* ATcommand, int timeout) ;

	char simData[30] ;

};


#endif /* SIM7X00_SIM7X00_H_ */
