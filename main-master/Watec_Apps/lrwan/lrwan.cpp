/*
 * lrwan.cpp
 *
 *  Created on: May 1, 2021
 *      Author: ASUS
 */

#include "hw.h"
#include "systemEvent.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ===================================== ALL FUNCTION DEFINITIONS =============================================//

debug lrDebug(&DEBUG_UART) ;
unDebug lrUnDebug ;

#ifdef LORAWAN_DEBUG
	#define LR_DEBUG lrDebug
#else
	#undef LR_DEBUG
	#define LR_DEBUG lrUnDebug
#endif

lrwan::lrwan( UART_HandleTypeDef * _huartx ){
	huartx = &*_huartx ;
}


/*
 * Hàm sim7x00::IRQhandler(); sẽ được đặt vào hàm ngắt UART : void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) ;
 */
void lrwan::IRQhandler(){
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



bool lrwan::sendCommand (const char* command, const char* expected_answer, int timeout) {

    int answer=0;

    dataCount = 0 ;
    rxDone_FLAG = 0 ;
    memset(rxData, 0, RX_DATA_MAX );

    LR_DEBUG.println("Requested to LRWAN module : ") ;
    LR_DEBUG.print(command) ;

    // Bắt đầu gửi lệnh cmd ngay tại đây
    HAL_UART_Transmit(huartx, (uint8_t*)command, strlen(command), timeout) ;

    LR_DEBUG.println("SIM7600E responsed : ") ;

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

    // In thông tin mà module lorawan đã phản hồi về :
    LR_DEBUG.println(rxData) ;

    // Nếu kết quả phản hồi không như mong muốn :
    if ( answer == 0 )
    	LR_DEBUG.println("Fail to get responses from SIM7600E! ") ;

    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, (GPIO_PinState) 0 ) ;

    // Reset lại các biến phục vụ trong quá trình lấy dữ liệu từ module sim
    dataCount = 0 ;
    rxDone_FLAG = 0 ;
    //  Do hiện tại, rxData được sử dụng cho các tác vụ kế tiếp sau AT cmd, nên không được reset hàm  memset(rxData, 0, sizeof(rxData) ) ;

    return answer;
}



void lrwan::response( const char* response, int timeout ){
	HAL_UART_Transmit(huartx, (uint8_t*)response, strlen(response), 1000) ;
}



void lrwan::checkingAndProcess(){
	if( strstr(rxData, REQ_WARNING_LV1) != NULL ){
		warningEvent = WARNING_LEVEL_1 ;
		response("OK\r", 1000 );
	}else if( strstr(rxData, REQ_WARNING_LV2) != NULL ){
		warningEvent = WARNING_LEVEL_2 ;
		response("OK\r", 1000 );
	}else if( strstr(rxData, REQ_WARNING_LV3) != NULL ){
		warningEvent = WARNING_LEVEL_3 ;
		response("OK\r", 1000 );
	}else if( strstr(rxData, REQ_WARNING_LV4) != NULL ){
		warningEvent = WARNING_LEVEL_4 ;
		response("OK\r", 1000 );
	}else if( strstr(rxData, REQ_WARNING_LV5) != NULL ){
		warningEvent = WARNING_LEVEL_5 ;
		response("OK\r", 1000 );
	}else if( strstr(rxData, REQ_STOP_WARNING) != NULL ){
		warningEvent = STOP_WARNING ;
		response("OK\r", 1000 );
	}else if( strstr(rxData, REQ_DATA_PERIODLY) != NULL ){
		updateEvent = LORAWAN_UPDATE_DATA ;
		response("OK\r", 1000 );
	}
}



void lrwan::init(){
	// Init uart4 :
	MX_UART4_Init();

	// Khoi dong ngat uart4
	HAL_UART_Receive_IT(&huart4, (uint8_t*)rxBuff, 1) ;
}



//void lrwan::updateData( const char *_Data, int timeout ){
//	LR_DEBUG.println("Sending data to Loranwan module ...  ") ;
//
//	// Đầu tiên, lấy kích thước của dữ liệu vào
//	int dataLength = strlen(_Data) ;
//
//	// Tạo 1 mảng phụ để đính kèm đuôi "\r\n"
//	char Data[dataLength + 2] = ""  ;
//	strcpy(Data, _Data) ;
//	strcat(Data, "\r\n") ;
//
//	// Bắt đầu gửi
//	LR_DEBUG.print(Data) ;
//	HAL_UART_Transmit(huartx, (uint8_t*)Data, strlen(Data), 1000) ;
//	LR_DEBUG.print("Done") ;
//}




void lrwan::send(){
	HAL_UART_Transmit(huartx, (uint8_t*)this->lrData, (int)sizeof(lrData)/sizeof(lrData[0]), 2000) ;
	LR_DEBUG.println("\nLORAWAN_UPDATE_DATA done !") ;
}




void lrwan::updateData( float _Temp, float _Humi, char _powData[20], int _status){

	LR_DEBUG.println("Update data to buffer Data ... ") ;
	char CharTemp[5] ;
	char CharHumi[2] ;
	char CharStatus[1] ;
	sprintf(CharTemp, "%.2f", _Temp);	// Cach chuyen float thanh char[x], Neu muon thanh char* thi ep kieu la duoc
	sprintf(CharHumi, "%.0f", _Humi);	// Cach chuyen float thanh char[x], Neu muon thanh char* thi ep kieu la duoc
	sprintf(CharStatus, "%d", _status);	// Cach chuyen float thanh char[x], Neu muon thanh char* thi ep kieu la duoc

	char CharEndPackage[3] = "\r\n" ;

	char CharPowData[21] ;
	memset( CharPowData, '\0', 21 ) ;

	memcpy(CharPowData, _powData, 20 ) ;

	memset(lrData, 0, 50) ;

	// Cach 1:
	strcat(lrData, CharTemp ) ;    //add temperature value to pakage
	strcat(lrData, CharHumi ) ;	//add humidity value to pakage
	strcat(lrData, CharPowData ) ;	//add power data to package
	strcat(lrData, CharStatus ) ;	//add Status to pakage
	strcat(lrData, CharEndPackage ) ;	//add Status to pakage
	// End Cach 1


	LR_DEBUG.println("Package is : ") ;
	LR_DEBUG.print(lrData) ;



//	// Test :
//	strcpy(this->lrData, "32.329500.0011.1122.2233.335") ;

}


#ifdef __cplusplus
}
#endif
