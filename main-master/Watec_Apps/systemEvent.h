/*
 * systemEvent.h
 *
 *  Created on: Mar 5, 2021
 *      Author: ASUS
 */

#ifndef SYSTEMEVENT_H_
#define SYSTEMEVENT_H_

#ifdef __cplusplus
extern "C" {
#endif


// Enum khai báo các sự kiện yêu cầu với hệ thống
enum system_event_enum {
	// Đưa hệ thống vào trạng thái sleep mode
	SLEEP = 0 ,

	// Cập nhật dữ liệu lên server định kì qua module SIM, giao thức MQTT
	SIM_MQTT_UPDATE_DATA = 1 ,

	// Cập nhật dữ liệu cho module lorawan
	LORAWAN_UPDATE_DATA = 2 ,

	// Thực hiện cảnh báo theo các level :
	WARNING_LEVEL_1 = 3 ,
	WARNING_LEVEL_2 = 4 ,
	WARNING_LEVEL_3 = 5 ,
	WARNING_LEVEL_4 = 6 ,
	WARNING_LEVEL_5 = 7 ,

	// Tắt cảnh báo
	STOP_WARNING    = 8 ,

	// Thu thập toàn bộ dữ liệu hệ thống theo định kì
	GET_SYSTEM_DATA = 9 ,

	// cập nhật thông tin cho hệ thống từ người dùng, vd : số điện thoại yêu cầu, mã cảnh báo ...
	SYSTEM_UPDATE   = 10 ,

	// Cập nhật firmware từ xa
	SYSTEM_FOTA     = 11 ,

	// khi có bất ổn trong hệ thống, vd như điện áp pin quá thấp
	SYSTEM_ERROR    = 12 ,

	// Trạng thái sau khi đã thực hiện xong các event yêu cầu, chờ chuẩn bị sang trạng thái SLEEP
	IDLE            = 13 ,


};



// Dữ liệu để cập nhật trạng thái cho hệ thống
enum system_status_enum {
	IS_NOT_WARNING = 0 ,
	IS_WARNING_LEVEL_1 = 1,
	IS_WARNING_LEVEL_2 = 2,
	IS_WARNING_LEVEL_3 = 3,
	IS_WARNING_LEVEL_4 = 4,
	IS_WARNING_LEVEL_5 = 5,
};



// Khai báo 2 biến toàn cục để bắt sự kiện cho hệ thống :
extern int warningEvent ;
extern int updateEvent ;


#ifdef __cplusplus
}
#endif




#endif /* SYSTEMEVENT_H_ */
