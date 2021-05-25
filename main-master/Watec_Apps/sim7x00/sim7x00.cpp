/*
 * sim7600.c
 *
 *  Created on: Jan 23, 2021
 *      Author: ASUS
 */


#include "hw.h"
#include "systemEvent.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ===================================== ALL FUNCTION DEFINITIONS =============================================//

debug simDebug(&DEBUG_UART) ;
unDebug simUnDebug ;

#ifdef SIM7600_DEBUG
	#define SIM_DEBUG simDebug
#else
	#undef SIM_DEBUG
	#define SIM_DEBUG simUnDebug
#endif



//===================ĐIỀU CHỈNH SỐ ĐIỆN THOẠI CHO PHÉP CẢNH BÁO TẠI ĐÂY==============================//

#define MAX_PHONE_NUMBER_ALLOWED           4  // số điện thoại tối đa cho phép

const char* phoneNumbersAllowed[MAX_PHONE_NUMBER_ALLOWED] = { "+84334293870",     // SĐT của Toàn
													   "+84914486786",		// SĐT thầy Thanh
													   "+84386599045", 	    // SDT nut nhan vat ly
													   "+84914486786"       // SDT thầy Thanh

};


//====================ĐIỀU CHỈNH MÃ CẢNH BÁO TẠI ĐÂY ==========================//

#define MAX_WARNING_CODES                   5  					// số mã cảnh báo tối đa, lưu ý, số mã phải đủ trong mảng dưới


const char* warningCodesAllowed[MAX_WARNING_CODES] = { "canhbao1",	// sắp xếp theo thứ tự ưu tiên từ trên xuống
								 	 	 	     "canhbao2",
											     "canhbao3",
											     "canhbao4",
											     "canhbao5"
};

const char* stopWarningCodesAllowed = 				 "stop" ; 	     // mã dừng cảnh báo



/*
 * Hàm kiểm tra và xác định yêu cầu từ thông tin đến
 */
enum incomming_event {
	IS_NOTHING = 0,
	IS_SMS = 1,
	IS_INCOMMING_CALL = 2,

};



sim7x00::sim7x00( UART_HandleTypeDef * _huartx ){
	huartx = &*_huartx ;
}


/*
 * Hàm sim7x00::IRQhandler(); sẽ được đặt vào hàm ngắt UART : void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) ;
 */
void sim7x00::IRQhandler(){
	// khi nào nhận được ký tự \r thì sẽ hiểu là đã kết thúc lệnh.
	if(  rxBuff[0] == '\r' ){
		rxDone_FLAG = 1 ;
		memset(rxBuff,0,1) ;
	}else{
		// tạo ra 1 biến dữ liệu  rxData = \nOK\n
		rxData[dataCount] = rxBuff[0] ;
		// Cộng dồn dữ liệu lên nhưng phải đảm bảo nằm trong vùng nhớ của bộ đệm ! Không được để quá, sẽ gây treo VĐK ( nguy hiểm )
		if ( dataCount < RX_DATA_MAX ) dataCount++ ;
		else {}
	}
	HAL_UART_Receive_IT(huartx, (uint8_t*)rxBuff, 1) ;
}



bool sim7x00::sendATcommand (const char* ATcommand, const char* expected_answer, int timeout) {

    int answer=0;

    dataCount = 0 ;
    rxDone_FLAG = 0 ;
    memset(rxData, 0, RX_DATA_MAX );

    SIM_DEBUG.println("Requested to SIM7600 : ") ;
    SIM_DEBUG.print(ATcommand) ;

    // Bắt đầu gửi lệnh AT ngay tại đây
    HAL_UART_Transmit(huartx, (uint8_t*)ATcommand, strlen(ATcommand), timeout) ;

    SIM_DEBUG.println("SIM7600E responsed : ") ;

    // lệnh này để bắt đầu lấy mốc t.g để check Timeout
    uint32_t tickStart = HAL_GetTick() ;


    while( answer == 0 ) {

    	// Kiểm tra thời gian phản hồi, nếu lâu quá timeout thì break, và báo lỗi
		if ( (int)( HAL_GetTick() - tickStart )  >=  timeout ) {
			break ;
		}

	    /*
	     * Kiểm tra module sim có phản hồi về đúng expected_answer hay khong
	     * Note : Hàm strstr(X, x) sẽ tìm 1 chuỗi con x trong chuỗi lớn hơn X
	     *  	  Nếu x xuất hiện trong X, thì hàm strstr sẽ trả về 1 chuỗi bắt đầu từ x và chuỗi
	     *  	  kéo dài còn lại phía sau từ x trong X
	     */
		if ( strstr(rxData, expected_answer) != NULL ){
			// Nếu có thi la OK, answer == 1, thoát khỏi vòng while
			answer = 1;
		}

		// Led nhấp nháy báo hiệu đang tương tác với module sim
		if ( ( (int)( HAL_GetTick() - tickStart )/(int)100 % 2 ) == 0 )
			HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin) ;

	}

    // In thông tin mà module sim đã phản hồi về :
    SIM_DEBUG.println(rxData) ;

    // Nếu kết quả phản hồi không như mong muốn :
    if ( answer == 0 )
    	SIM_DEBUG.println("Fail to get responses from SIM7600E! ") ;

    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, (GPIO_PinState) 0 ) ;

    // Reset lại các biến phục vụ trong quá trình lấy dữ liệu từ module sim
    dataCount = 0 ;
    rxDone_FLAG = 0 ;
    //  Do hiện tại, rxData được sử dụng cho các tác vụ kế tiếp sau AT cmd, nên không được reset hàm  memset(rxData, 0, sizeof(rxData) ) ;

    return answer;
}



bool sim7x00::sendATcommand (const char* ATcommand, const char* expected_answer, int timeout, int retryTime ) {

	int retryTimeCounter = 0 ;

    int answer=0;

    while ( answer == 0 && retryTimeCounter < retryTime ) {

		dataCount = 0 ;
		rxDone_FLAG = 0 ;
		memset(rxData, 0, RX_DATA_MAX );

		SIM_DEBUG.println("Requested to SIM7600 : ") ;
		SIM_DEBUG.print(ATcommand) ;

		// Bắt đầu gửi lệnh AT ngay tại đây
		HAL_UART_Transmit(huartx, (uint8_t*)ATcommand, strlen(ATcommand), timeout) ;

		SIM_DEBUG.println("SIM7600E responsed : ") ;

		// lệnh này để bắt đầu lấy mốc t.g để check Timeout
		uint32_t tickStart = HAL_GetTick() ;


		while( answer == 0 ) {

			// Kiểm tra thời gian phản hồi, nếu lâu quá timeout thì break, và báo lỗi
			if ( (int)( HAL_GetTick() - tickStart )  >=  timeout ) {
				break ;
			}

			/*
			 * Kiểm tra module sim có phản hồi về đúng expected_answer hay khong
			 * Note : Hàm strstr(X, x) sẽ tìm 1 chuỗi con x trong chuỗi lớn hơn X
			 *  	  Nếu x xuất hiện trong X, thì hàm strstr sẽ trả về 1 chuỗi bắt đầu từ x và chuỗi
			 *  	  kéo dài còn lại phía sau từ x trong X
			 */
			if ( strstr(rxData, expected_answer) != NULL ){
				// Nếu có thi la OK, answer == 1, thoát khỏi vòng while
				answer = 1;
			}

			// Led nhấp nháy báo hiệu đang tương tác với module sim
			if ( ( (int)( HAL_GetTick() - tickStart )/(int)100 % 2 ) == 0 )
				HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin) ;

		}

		// In thông tin mà module sim đã phản hồi về :
		SIM_DEBUG.println(rxData) ;

		// Nếu kết quả phản hồi không như mong muốn :
		if ( answer == 0 ){
			SIM_DEBUG.println("Fail to get responses from SIM7600E! ") ;
			if ( retryTimeCounter < retryTime ) {
				retryTimeCounter++ ;
			}
		}

		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, (GPIO_PinState) 0 ) ;
    }

    // Reset lại các biến phục vụ trong quá trình lấy dữ liệu từ module sim
    dataCount = 0 ;
    rxDone_FLAG = 0 ;
    //  Do hiện tại, rxData được sử dụng cho các tác vụ kế tiếp sau AT cmd, nên không được reset hàm  memset(rxData, 0, sizeof(rxData) ) ;

    return answer;
}


void sim7x00::init(){
	// Khởi tạo UART 1 và kích hoạt ngắt :
	MX_USART1_UART_Init();
	HAL_UART_Receive_IT(&huart1, (uint8_t*)rxBuff, 1) ;

	// Khởi động module sim bằng cách set chân PWRKEY mức 0 trên module sim
	SIM_DEBUG.println("SIM7600E booting ... " ) ;
	HAL_GPIO_WritePin( SIM7600_PWRKEY_GPIO_Port, SIM7600_PWRKEY_Pin, (GPIO_PinState) 1) ;
	HAL_GPIO_WritePin( SIM7600_PWRKEY_GPIO_Port, SIM7600_PWRKEY_Pin, (GPIO_PinState) 0) ;

	// Chờ ~ > 10s để quá trình khởi động module sim hoàn tất
//	HAL_Delay(20000) ;

	int step = 0 ;
	bool checkStep[20] ;
	bool checkInit = false ;

	while( checkInit == false ){
		switch (step) {
			case 0:
				checkStep[step] = sendATcommand((char*)"AT\r", "OK", 2000) ;  // check AT response
				if (checkStep[step] == true) step++ ;
				break ;
			case 1:
				checkStep[step] = sendATcommand((char*)"AT+CREG?\r", "OK", 2000) ;
				if (checkStep[step] == true) step++ ;
				break ;
			case 2:
				checkStep[step] = sendATcommand((char*)"AT+CNMP=2\r", "OK", 2000) ;
				if (checkStep[step] == true) step++ ;
				break ;
			case 3:
				checkStep[step] = sendATcommand((char*)"ATI\r", "OK", 2000) ;
				if (checkStep[step] == true) step++ ;
				break ;
			case 4:
				checkStep[step] = sendATcommand("AT+CNMI=2,1,0,0,0\r", "OK", 2000 ) ;
				if (checkStep[step] == true) step++ ;
				break ;
			case 5:
				SIM_DEBUG.println("Setting up SMS... " ) ;
				checkStep[step] = sendATcommand((char*)"AT+CNMP=2\r", "OK", 2000) ;
				if (checkStep[step] == true) step++ ;
				break ;
			case 6:
				SIM_DEBUG.println("Delete all old SMS... " ) ;
				checkStep[step] = sendATcommand("AT+CMGD=,4\r", "OK", 1000);    // xóa hết toàn bộ tin nhắn
				if (checkStep[step] == true) step++ ;
				break ;
			case 7:
				SIM_DEBUG.println("Set SMS mode to text... ");
				checkStep[step] = sendATcommand("AT+CMGF=1\r", "OK", 1000);    // sets the SMS mode to text
				if (checkStep[step] == true) step++ ;
				break ;
			case 8:
				SIM_DEBUG.println("Checking audio file : " ) ;
				checkStep[step] = sendATcommand("AT+FSCD=E:\r", "OK", 2000) ;
				if (checkStep[step] == true) step++ ;
				break ;
			case 9:
				checkStep[step] = sendATcommand("AT+FSLS\r", "OK", 2000) ;
				if (checkStep[step] == true) step++ ;
				break ;
			case 10:
				SIM_DEBUG.println("Sound test start :" ) ;
				checkStep[step] = warningStart(WARNING_LEVEL_4, 100) ;
				if( checkStep[step] == false ){
					warningStop() ;
					checkStep[step] = warningStart(WARNING_LEVEL_4, 100) ;
				}
				if (checkStep[step] == true) step++ ;
				break ;
			case 11:
				HAL_Delay(1000) ;
				checkStep[step] = warningStop() ;
				if (checkStep[step] == true) step++ ;
				break ;
			case 12:
				checkStep[step] = sendATcommand("AT+CGMR\r", "OK", 2000) ;
				if (checkStep[step] == true) step++ ;
				break ;
			case 13:
				checkInit = true ;
				break;
		}
	}

	rxDone_FLAG = 0 ;
	SIM_DEBUG.println("SIM7600E init completed ! ") ;


}



void sim7x00::setAllowedPhoneNumber( const char *list[], int _quantity ){

}



void sim7x00::checkingAndProcess(){

	// Kiểm tra sự kiện đến từ module sim :
	switch ( checkIncommingEvent() ) {
		/*------------------------------------------------------------------------
		 * TIN NHẮN :
		 */
		case IS_SMS:
			// Thực hiện theo quy trình sau :

			// Đầu tiên phải mở SMS ra đọc :
			readSMS() ;

			// Tiến hành xác thực Số điện thoại :
			if( isAuthorizePhoneNumber(SMSContent) ){
				// Nếu đúng số điện thoại cho phép, thì kiểm tra nội dung yêu cầu :
				int result = IDLE ;
				result = identifySMSRequest(SMSContent) ;

				switch (result) {
					case WARNING_LEVEL_1:
						warningEvent = WARNING_LEVEL_1 ;
						SIM_DEBUG.print("WARNING_LEVEL_1" ) ;
						break;
					case WARNING_LEVEL_2:
						warningEvent = WARNING_LEVEL_2 ;
						SIM_DEBUG.print("WARNING_LEVEL_2" ) ;
						break;
					case WARNING_LEVEL_3:
						warningEvent = WARNING_LEVEL_3 ;
						SIM_DEBUG.print("WARNING_LEVEL_3" ) ;
						break;
					case WARNING_LEVEL_4:
						warningEvent = WARNING_LEVEL_4 ;
						SIM_DEBUG.print("WARNING_LEVEL_4" ) ;
						break;
					case WARNING_LEVEL_5:
						warningEvent = WARNING_LEVEL_5 ;
						SIM_DEBUG.print("WARNING_LEVEL_5" ) ;
						break;
					case STOP_WARNING:
						warningEvent = STOP_WARNING ;
						SIM_DEBUG.print("STOP_WARNING" ) ;
						break;


//					case

					// Ngoài ra còn nhiều chức năng ngoài cảnh báo, như là add thêm số điện thoại, hẹn giờ cảnh báo ... to be continue
					default:
						break;
				}

			}else{} // Nếu không phải số điện thoại được cấp phép, thi bỏ qua, đỡ tốn thời gian !

			break ;


		/*------------------------------------------------------------------------
		 * CUỘC GỌI ĐẾN :
		 */
		case IS_INCOMMING_CALL:
			// Bỏ qua
			break;


		default:
			break;
	}

	// reset lại nội dung tin nhắn
	memset(SMSContent, 0, RX_DATA_MAX) ;

}



int sim7x00::checkIncommingEvent(){
	int result = IS_NOTHING ;

	// Kiểm tra có phải SMS đến ?
	char* isSMS = strstr(rxData, "SM") ;

	// Kiểm tra có phải cuộc gọi đến ?
	char* isPhoneCall = strstr(rxData, "RING") ;

	if( isSMS != NULL){
		SIM_DEBUG.println("Received SMS !") ;
		result = IS_SMS ;
	}
	if( isPhoneCall != NULL){
		SIM_DEBUG.println("Incomming call !") ;
		result = IS_INCOMMING_CALL ;
	}

	return result ;
}



void sim7x00::readSMS(){

	// Reset lại bộ nhớ SMS
	memset(SMSContent, 0, RX_DATA_MAX) ;

	// Selects the memory
	sendATcommand("AT+CPMS=\"SM\",\"SM\",\"SM\"\r", "OK", 3000);

	// Gửi lệnh đọc SMS mới nhất đó
	sendATcommand("AT+CMGR=0\r", "OK", 5000);

	// Copy nội dung lệnh từ bộ đệm đọc được sang biến SMSContent để xử lý
	strcpy(SMSContent, rxData) ;

	// debug nội dung SMS để kiểm tra
	SIM_DEBUG.println(SMSContent) ;

	// check xong rồi thì xóa tin nhắn đó đi
	sendATcommand("AT+CMGD=,4\r", "OK", 1000);

}



/*
 * Hàm xác thực số điện thoại được phép hay không
 */
bool sim7x00::isAuthorizePhoneNumber( const char *_SMScontent ){
	SIM_DEBUG.println("Authorizing phone number ... ") ;

	bool result = 0 ;

	// Bắt đầu kiểm tra số điện thoại cho phép
	for(int i = 0 ; i < MAX_PHONE_NUMBER_ALLOWED ; i++){
		if(  strstr(_SMScontent, phoneNumbersAllowed[i]) != NULL  ){
			result = 1 ;
		}
	}

	if(result == 1)
		SIM_DEBUG.println("Phone number is accepted !" ) ;
	else
		SIM_DEBUG.println("Phone number is invalid !" ) ;

	return result ;
}



/*
 * Hàm xác thực nội dung yêu cầu từ SMS
 */
int sim7x00::identifySMSRequest( const char *_SMScontent ){
	SIM_DEBUG.println("Identifying SMS request content ... ") ;
	int result = IDLE ;

	for(int i = 0 ; i < MAX_WARNING_CODES ; i++){

		// kiểm tra xem đã đúng cú pháp chưa
		if( strstr(_SMScontent, warningCodesAllowed[i]) != NULL ){
			if     (i == 0 ) result = WARNING_LEVEL_1 ;
			else if(i == 1 ) result = WARNING_LEVEL_2 ;
			else if(i == 2 ) result = WARNING_LEVEL_3 ;
			else if(i == 3 ) result = WARNING_LEVEL_4 ;
			else if(i == 4 ) result = WARNING_LEVEL_5 ;

		// kiểm tra xem có phải là stop không
		}else if( strstr(_SMScontent, stopWarningCodesAllowed) != NULL ){
			result = STOP_WARNING ;
		}else{
			// do nothing
			// Ngoài ra còn nhiều chức năng ngoài cảnh báo, như là add thêm số điện thoại, hẹn giờ cảnh báo ... to be continue
		}
	}

	if(result != STOP_WARNING  )
		SIM_DEBUG.println("WARNING was requested : " ) ;
	else if (result == STOP_WARNING  )
		SIM_DEBUG.println("STOP WARNING was requested !!!\n" ) ;

	// Ngoài ra còn nhiều chức năng ngoài cảnh báo, như là add thêm số điện thoại, hẹn giờ cảnh báo ... to be continue

	return result ;
}




/**
 * Hàm bắt đầu thực hiện cảnh báo theo các mức :
 */
bool sim7x00::warningStart(int _level, int _times){
	bool result = 0 ;

	sim7x00::sendATcommand("AT+FSCD=E:\r", "OK", 2000) ;
	switch (_level) {
		case WARNING_LEVEL_1:
			result = sim7x00::sendATcommand(ALARM_1, "OK", 2000) ;
			break;
		case WARNING_LEVEL_2:
			result = sim7x00::sendATcommand(ALARM_2, "OK", 2000) ;
			break;
		case WARNING_LEVEL_3:
			result = sim7x00::sendATcommand(ALARM_3, "OK", 2000) ;
			break;
		case WARNING_LEVEL_4:
			result = sim7x00::sendATcommand(ALARM_4, "OK", 2000) ;
			break;
		case WARNING_LEVEL_5:
			result = sim7x00::sendATcommand(ALARM_5, "OK", 2000) ;
			break;
		default:
			result = sim7x00::sendATcommand(ALARM_5, "OK", 2000) ;
			break;
	}

	return result ;
}




bool sim7x00::warningStop(){
	bool result = 0 ;

	result = sim7x00::sendATcommand("AT+CCMXSTOP\r", "stop", 10000) ;

	return result ;
}



bool sim7x00::MQTTStart( const char* _deviceName, const char* _MQTTWillTopic, const char* _MQTTWillMsg, const char* _BrokerAddress ){
	bool checkMQTTStart = false ;

	while( checkMQTTStart == false ) {

		// Bước 0 : gửi lệnh start MQTT :
		if ( sendATcommand("AT+CMQTTSTART\r", "OK", 2000) == false) {
		// Nếu start mà thất bại thì thử logout, Stop, rồi start lại
		sendATcommand("AT+CMQTTSTOP\r", "OK", 2000) ;
		sendATcommand("AT+CMQTTSTART\r", "OK", 2000) ;
		}

		// Bước 1 : set MQTT client name của device
		SIM_DEBUG.println("Setting Client name ...") ;
		char setClientNameCmd[100] = "AT+CMQTTACCQ=0,\"" ;
		strcat(setClientNameCmd, _deviceName) ;
		strcat(setClientNameCmd, "\"\r") ;

		if ( sendATcommand(setClientNameCmd, "OK", 2000, 5) == false ) break ;

		// Bước 2 : set MQTT will topic
		SIM_DEBUG.println("Setting MQTT Will Topic ...") ;
		// Đầu tiên, lấy kích thước dữ liệu của _MQTTWillTopic
		float iMQTTWillTopicSize = (float)strlen(_MQTTWillTopic) ;
		char cMQTTWillTopicSize[2] ;
		sprintf( cMQTTWillTopicSize, "%.0f", iMQTTWillTopicSize ) ;

		char setWillTopicCmd[50] = "AT+CMQTTWILLTOPIC=0," ;
		strcat( setWillTopicCmd, cMQTTWillTopicSize ) ;
		strcat( setWillTopicCmd, "\r") ;

		if ( sendATcommand(setWillTopicCmd, ">", 2000, 5) == false )  break ;
		if ( sendATcommand( _MQTTWillTopic, "OK", 2000, 5) == false )  break ;

		// Bước 3 : set MQTT Will message :
		SIM_DEBUG.println("Setting MQTT Will Message ...") ;
		// Đầu tiên, lấy kích thước dữ liệu của _MQTTWillTopic
		float iMQTTWillMsgSize = (float)strlen(_MQTTWillMsg) ;
		char  cMQTTWillMsgSize[2] ;
		sprintf( cMQTTWillMsgSize, "%.0f", iMQTTWillMsgSize ) ;

		char setWillMsgCmd[100] = "AT+CMQTTWILLMSG=0," ;
		strcat( setWillMsgCmd, cMQTTWillMsgSize ) ;
		strcat( setWillMsgCmd, ",2\r") ;

		if ( sendATcommand(setWillMsgCmd, ">", 2000, 5 ) == false )  break ;

		if ( sendATcommand(_MQTTWillMsg, "OK", 2000, 5 ) == false )  break ;

		// Bước 4 :
		SIM_DEBUG.println("Now connecting to MQTT Broker ...") ;
		char brokerConnectCmd[100] = "AT+CMQTTCONNECT=0,\"" ;
		strcat(brokerConnectCmd, mqttBrokerIP) ;
		strcat(brokerConnectCmd, "\",60,1\r") ;

		if ( sendATcommand(brokerConnectCmd, "OK", 2000, 5 ) == false )  break ;

		checkMQTTStart = true ;
	}

	// END
	rxDone_FLAG = 0 ;
	if ( checkMQTTStart == true ) SIM_DEBUG.println("MQTT start successful ! ") ;
	else SIM_DEBUG.println("MQTT start failed ! ") ;

	return checkMQTTStart ;
}



void sim7x00::MQTTPublishData ( const char* _topic, const char* _data ) {

	SIM_DEBUG.println("MQTT Publish data : ") ;

	// name topic :
	SIM_DEBUG.println("Setting MQTT Topic... ") ;
	char setTopicCmd[50] = "" ;
	strcat( setTopicCmd, _topic ) ;
	strcat( setTopicCmd, "\r" ) ;
	sendATcommand( setTopicCmd , "OK", 2000) ;

	// prepare payload to publish :
	SIM_DEBUG.println("Preparing payload ... ") ;
	float size = (float)strlen(_data) ;
	char cSize[3] ;
	sprintf( cSize, "%.0f", size ) ;

	char setPayloadCmd[100] = "AT+CMQTTPAYLOAD=0," ;
	strcat( setPayloadCmd, cSize ) ;
	strcat( setPayloadCmd, "\r" ) ;

	sendATcommand(setPayloadCmd, ">", 2000) ;

	// set payload :
	char payload[100] = "" ;
	strcat( payload, _data ) ;
	strcat( payload, "\r" ) ;
	sendATcommand(payload, "OK", 2000) ;

	// Now publish the payload
	SIM_DEBUG.println("Now publishing ... ") ;
	sendATcommand("AT+CMQTTPUB=0,1,60\r", "OK", 2000) ;
	SIM_DEBUG.print("Done !") ;
}



void sim7x00::MQTTPublishData () {

	sendATcommand("AT+CMQTTSTART\r", "OK", 2000) ;

	sendATcommand("AT+CMQTTACCQ=0,\"datalogger_2\"\r", "OK", 2000) ;

	sendATcommand("AT+CMQTTWILLTOPIC=0,7\r", ">", 2000) ;

	sendATcommand("OFFLINE\r", "OK", 2000) ;

	sendATcommand("AT+CMQTTWILLMSG=0,7,2\r", ">", 2000) ;

	sendATcommand("SIM_OFF\r", "OK", 2000) ;

	sendATcommand("AT+CMQTTCONNECT=0,\"tcp://52.230.105.94:1883\",60,1\r", "OK", 2000) ;

	// set mode publish topic :
	sendATcommand("AT+CMQTTTOPIC=0,6\r", ">", 2000) ;

	// name topic :
	sendATcommand("status\r", "OK", 2000) ;


	// prepare payload to publish :
	SIM_DEBUG.println("Preparing payload ... ") ;
	float size = (float)strlen(this->simData) ;
	char cSize[3] ;
	sprintf( cSize, "%.0f", size ) ;

	char setPayloadCmd[100] = "AT+CMQTTPAYLOAD=0," ;
	strcat( setPayloadCmd, cSize ) ;
	strcat( setPayloadCmd, "\r" ) ;

	sendATcommand(setPayloadCmd, ">", 2000) ;

	// set payload :
	sendATcommand(this->simData, "OK", 2000) ;

	// Now publish the payload
	SIM_DEBUG.println("Now publishing ... ") ;
	sendATcommand("AT+CMQTTPUB=0,1,60\r", "OK", 2000) ;
	SIM_DEBUG.print("Done !") ;
	// set payload :
	sendATcommand("AT+CMQTTSTOP\r", "OK", 2000) ;

}



void sim7x00::updateData( float _Temp, float _Humi, const char* _U_out, const char* _Power, const char* _I_pv, const char* _I_load, int _status){
	char CharTemp[5] ;
	char CharHumi[2] ;
	char CharStatus[1] ;
	sprintf(CharTemp, "%.2f", _Temp);	// Cach chuyen float thanh char[x], Neu muon thanh char* thi ep kieu la duoc
	sprintf(CharHumi, "%.0f", _Humi);	// Cach chuyen float thanh char[x], Neu muon thanh char* thi ep kieu la duoc
	sprintf(CharStatus, "%d", _status);	// Cach chuyen float thanh char[x], Neu muon thanh char* thi ep kieu la duoc

	char CharUout[5] ;
	char CharPower[5] ;
	char CharIpv[5] ;
	char CharIload[5] ;
	strcpy(CharUout, _U_out ) ;
	strcpy(CharPower, _Power ) ;
	strcpy(CharIpv, _I_pv ) ;
	strcpy(CharIload, _I_load ) ;

	memset(this->simData, 0, 30) ;
	strcat(this->simData, CharTemp ) ;    //add temperature value to pakage
	strcat(this->simData, CharHumi ) ;	//add humidity value to pakage
	//	strcat(this->lrData, CharUout ) ;	//add Output Voltage value to pakage
	//	strcat(this->lrData, CharPower ) ;	//add power value to pakage
	//	strcat(this->lrData, CharIpv ) ;	//add Ipv value to pakage
	//	strcat(this->lrData, CharIload ) ;	//add I load value to pakage
	strcat(this->simData, CharStatus ) ;	//add Status to pakage
	strcat(this->simData, "\r\n" ) ;	//add Status to pakage
}


#ifdef __cplusplus
}
#endif
