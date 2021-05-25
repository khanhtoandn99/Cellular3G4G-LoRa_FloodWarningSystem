/*
 * power.cpp
 *
 *  Created on: May 1, 2021
 *      Author: ASUS
 */

#include "hw.h"

#ifdef __cplusplus
extern "C" {
#endif

debug pwrDebug(&DEBUG_UART) ;
unDebug pwrUnDebug ;

#ifdef POWER_DEBUG
	#define POW_DEBUG pwrDebug
#else
	#undef POW_DEBUG
	#define POW_DEBUG pwrUnDebug
#endif

#define REQ_PERIOD_DATA_CMD "RPD\r"


power::power( UART_HandleTypeDef * _huartx ){
	huartx = &*_huartx ;
}


void power::IRQhandler(){
	// khi nào nhận được ký tự \r thì sẽ hiểu là đã kết thúc lệnh.
	if(  rxBuff[0] == '\r' ){
		rxDone_FLAG = 1 ;
		memset(rxBuff,0,1) ;
	}else{
		// tạo ra 1 biến dữ liệu  rxData = \nOK\n
		rxData[dataCount] = rxBuff[0] ;
		// Cộng dồn dữ liệu lên nhưng phải đảm bảo nằm trong vùng nhớ của bộ đệm ! Không được để quá, sẽ gây treo VĐK ( nguy hiểm )
		if ( dataCount < rxData_MAX ) dataCount++ ;
		else {}
	}
	HAL_UART_Receive_IT(huartx, (uint8_t*)rxBuff, 1) ;
}



bool power::sendCommand (const char* ATcommand, const char* expected_answer, int timeout) {

    int answer=0;

    dataCount = 0 ;
    rxDone_FLAG = 0 ;
    memset(rxData, 0, rxData_MAX );

    POW_DEBUG.println("Requested to POWER Manager : ") ;
    POW_DEBUG.print(ATcommand) ;

    // Bắt đầu gửi lệnh AT ngay tại đây
    HAL_UART_Transmit(huartx, (uint8_t*)ATcommand, strlen(ATcommand), timeout) ;

    POW_DEBUG.println("POWER Manager responsed : ") ;

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
			HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin) ;

	}

    // In thông tin mà module sim đã phản hồi về :
    POW_DEBUG.println(rxData) ;

    // Nếu kết quả phản hồi không như mong muốn :
    if ( answer == 0 )
    	POW_DEBUG.println("Fail to get responses from POWER Manager! ") ;

    HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, (GPIO_PinState) 0 ) ;

    // Reset lại các biến phục vụ trong quá trình lấy dữ liệu từ module sim
    dataCount = 0 ;
    rxDone_FLAG = 0 ;
    //  Do hiện tại, rxData được sử dụng cho các tác vụ kế tiếp sau AT cmd, nên không được reset hàm  memset(rxData, 0, sizeof(rxData) ) ;

    return answer;
}



int power::response( const char* response, int timeout ){
	HAL_UART_Transmit(huartx, (uint8_t*)response, strlen(response), 1000) ;
	return 1 ;
}



void power::init(){
	  MX_USART3_UART_Init();
	  HAL_UART_Receive_IT(huartx, (uint8_t*)rxBuff, 1) ;

}



bool power::getPeriodData(){
	bool result = false ;
	bool cmdResult = false ;
	POW_DEBUG.println("Now getting Power data ... ") ;

	// First, send the cmd to get Data
	if ( sendCommand(REQ_PERIOD_DATA_CMD, "OK", 3000 ) == false ) cmdResult = false ;
	else cmdResult = true ;

	// Bat dau qua trinh lay du lieu

	// Neu trong du lieu ma co chu ok, thi bat dau nhan:
	memset(powData, 0, 20 ) ;

//		POW_DEBUG.println("check  point 1 ") ;
	if( cmdResult == true ){
		// Lay du lieu tai day :
		memcpy( powData, rxData, 20 ) ;

		// powData = 00.00 11.11 22.22 33.33

		result = true ;
	}


	memset(rxData, 0, rxData_MAX ) ;
	dataCount = 0 ;
	rxDone_FLAG = false ;

	POW_DEBUG.println("Done !") ;

	// Neu qua trinh lay data tu POWER bi loi :
	// => Cho gia tri du lieu = 00.00
	if ( result == false ){
		char failVal[21] ="24.0001.0001.0201.23" ;
		memcpy( powData, failVal, 20 ) ;
	}

	return result ;
}

#ifdef __cplusplus
}
#endif

