/*
 * config.h
 *
 *  Created on: May 1, 2021
 *      Author: ASUS
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Chỉnh sửa các thông số cấu hình ngày tại đây :
 * Hỗ trợ :
 * 	- Thêm số điện thoại được phép kích hoạt hệ thống
 * 	- Chỉnh lại mã yêu cầu cảnh báo
 */


#define  deviceName      "datalogger_1"


////===================ĐIỀU CHỈNH SỐ ĐIỆN THOẠI CHO PHÉP CẢNH BÁO TẠI ĐÂY==============================//
//
//#define MAX_PHONE_NUMBER_ALLOWED           4  // số điện thoại tối đa cho phép
//
//const char* phoneNumbersAllowed[MAX_PHONE_NUMBER_ALLOWED] = { "+84334293870",     // SĐT của Toàn
//													   "+84337477332",      // SĐT a Dũng
//													   "+84969146385",		// SĐT a Cảnh
//													   "+84914486786"		// SĐT thầy Thanh
//
//};
//
//
////====================ĐIỀU CHỈNH MÃ CẢNH BÁO TẠI ĐÂY ==========================//
//
//#define MAX_WARNING_CODES                   5  					// số mã cảnh báo tối đa, lưu ý, số mã phải đủ trong mảng dưới
//
//
//const char* warningCodesAllowed[MAX_WARNING_CODES] = { "canhbao1",	// sắp xếp theo thứ tự ưu tiên từ trên xuống
//								 	 	 	     "canhbao2",
//											     "canhbao3",
//											     "canhbao4",
//											     "canhbao5"
//};
//
//const char* stopWarningCodesAllowed = 				 "stop" ; 	     // mã dừng cảnh báo


//====================( NÂNG CAO ) ĐIỀU CHỈNH ĐƯỜNG DẪN ĐẾN FILE CẢNH BÁO TRÊN MODULE SIM =======================//

/*
 * note : khi nhập đường link vd : "103.97.125.251",21,"nguyendinhdung@lhnam.net","nguyendinhdung",0
 * thì dấu ngoặc kép bên trong nội dung phải được viết thành \" để tránh bị ngắt mảng, chương trình báo lỗi
 */

#define   ALARM_1   "AT+CCMXPLAY=\"E:/alarm1.mp3\",0,255\r"

#define   ALARM_2   "AT+CCMXPLAY=\"E:/alarm1_1.mp3\",0,255\r"

#define   ALARM_3   "AT+CCMXPLAY=\"E:/alarm2.mp3\",0,255\r"

#define   ALARM_4   "AT+CCMXPLAY=\"E:/alarm2_1.mp3\",0,255\r"

#define   ALARM_5   "AT+CCMXPLAY=\"E:/alarm3.mp3\",0,255\r"

//====================( NÂNG CAO ) CẤU HÌNH CHO FTP SERVER =======================//

#define ftpUser  "nguyendinhdung@lhnam.net"
#define ftpPass  "nguyendinhdung"
#define ftpLinkAlarm1  "\"103.97.125.251\",21,\"nguyendinhdung@lhnam.net\",\"nguyendinhdung\",0\r"



//====================( NÂNG CAO ) CẤU HÌNH CHO MQTT =====================//
#define mqttWillTopic  "OFFLINE\r"
#define mqttWillMsg    "SIM_OFF\r"
#define mqttBrokerIP   "tcp://52.230.105.94:1883"


/**
 * ------------------------------------------------------------------------------------------------------------------
 * LORAWAN CONFIG
 * ------------------------------------------------------------------------------------------------------------------
 */

//#define LORA_STATUS_LED_Port           LED_LORA_STATUS_GPIO_Port
//#define LORA_STATUS_LED_Pin			   LED_LORA_STATUS_Pin
//
//
//// application router ID (LSBF)  < ------- IMPORTANT
//static const u1_t APPEUI[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
////static const u1_t APPEUI[8]  = { 0xC7, 0x64, 0x03, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
//
//// unique device ID (LSBF)       < ------- IMPORTANT f2 8d 26 d6 af ca d7 90
//static const u1_t DEVEUI[8]  = { 0xf2, 0x8d, 0x26, 0xd6, 0xaf, 0xca, 0xd7, 0x90 };
////static const u1_t DEVEUI[8]  = { 0x24, 0x3A, 0xDB, 0x97, 0xC1, 0xA2, 0x9A, 0x00 };
//
//// device-specific AES key (derived from device EUI) 22 f6 90 88 19 d5 35 83 50 71 11 33 5c ce 47 2d
//static const u1_t DEVKEY[16] = { 0x22, 0xf6, 0x90, 0x88, 0x19, 0xd5, 0x35, 0x83, 0x50, 0x71, 0x11, 0x33, 0x5c, 0xce, 0x47, 0x2d };
////static const u1_t DEVKEY[16] = { 0xA0, 0xF3, 0x42, 0x59, 0xB4, 0x12, 0x70, 0xB4, 0xF2, 0x55, 0xA4, 0x44, 0x68, 0xA7, 0x70, 0x7A };


// Tập lệnh thống nhất chung để trao đổi thông tin dữ liệu giữ Master và Slaver :

// Từ Slaver sang Master :

// Lệnh cập nhật dữ liệu định kì
#ifndef REQ_DATA_PERIODLY
#define   REQ_DATA_PERIODLY "RDP"
#endif

// Lệnh yêu cầu phát cảnh báo theo các mức :
#ifndef REQ_WARNING_LV1
#define   REQ_WARNING_LV1 "RWL1"
#endif

#ifndef REQ_WARNING_LV2
#define   REQ_WARNING_LV2 "RWL2"
#endif

#ifndef REQ_WARNING_LV3
#define   REQ_WARNING_LV3 "RWL3"
#endif

#ifndef REQ_WARNING_LV4
#define   REQ_WARNING_LV4 "RWL4"
#endif

#ifndef REQ_WARNING_LV5
#define   REQ_WARNING_LV5 "RWL5"
#endif


// Lệnh dừng cảnh báo :
#ifndef REQ_STOP_WARNING
#define   REQ_STOP_WARNING "RSW"
#endif



#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H_ */
