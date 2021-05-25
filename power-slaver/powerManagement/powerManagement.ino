#include <STM32ADC.h> //
STM32ADC myADC(ADC1);
#include <String.h>
#include <stdio.h>
int Sample;

#define Ipv PA2
#define Upv PA3
#define Uout PA1   //new
#define Iout PA0
#define PWMpin PB0 // PWM-capable pin, pwmtimer(3)

// Toan code :
void truyenLowVoltage( float _U_out ) ;
void sendAllData2Master( float _U_out, float _Power , float _I_pv , float _I_load ) ;

bool masterDataPeriod_FLAG = false ; // đây là 1 cái cờ nhận yêu cầu lấy dữ liệu từ master, khi nào nhận được hoàn thành gói tin từ master,
// gói tin này sẽ thành "true", khi sử dụng cờ này xong, nhớ trả nó về lại = flase .
char masterData[30] ;
int cnt = 0 ;
float I_load ;

unsigned long timeCnt ; 
// End Toan Code

HardwareTimer pwmtimer(3);
float Power, Power_old, dP, delta = 0, dV, U_pvold;
float U_pv, I_pv , U_out, I_out;
float i = 0;
float  V1 = 0, V2 = 0, I1 = 0;
double I2 = 0;
int  Vinref, Vinrefold, MPPT_STEP;
int j;
unsigned long time1, MaxDuty, Duty_gh;
//static char outstr[15];


void setup()
{
  pinMode(PWMpin, PWM);  // setup the pin as PWM
  pwmWrite(PWMpin, 0);

  time1 = 20;
  MaxDuty = 1440;
  Duty_gh = 600;

  pwmtimer.setPrescaleFactor(1);
  pwmtimer.setPeriod(time1);

 
  
  Serial.begin(9600);
    Serial1.begin(9600);
  
  pinMode(Ipv, INPUT_ANALOG);
  pinMode(Upv, INPUT_ANALOG);
  pinMode(Iout, INPUT_ANALOG);
  pinMode(Uout, INPUT_ANALOG);
  pinMode(PA6, OUTPUT); //DIeu khien loa
  digitalWrite(PA6, LOW);


  myADC.calibrate();
  //myADC.setSampleRate(ADC_SMPR_71_5);//set the Sample Rate
  myADC.setScanMode();              //set the ADC in Scan mode.
  myADC.setContinuous();            //set the ADC in continuous mode.
  myADC.startConversion();


  i = 0;

  V1 = 0; V2 = 0; I1 = 0; Sample = 300;
  for (j = 0; j < Sample; j++)
  {
    V1 += analogRead(Upv);
    I1 += analogRead(Ipv);
  }

  U_pv = V1 * 0.005319 / Sample ;
  //I_pv = I1 * 0.0007326 / Sample;
  I_pv = I1 * 6.8525 / (Sample * 10000); //Rsun=0.12ohm

  for (j = 0; j < 20; j++)
  {
    V2 += analogRead(Uout);
  }
  //U_out=0.008691*V2/10; //R2=10.22K
  U_out = 0.008607 * V2 / 10; //R2=10.33K

  Power_old = U_pv * I_pv;
  Vinref = 100;
  Vinrefold = 0;
  delta = 1;
  pwmWrite(PWMpin, 50);
  MPPT_STEP = 4;

  delay(3000);
  timeCnt = millis() ; 
}

void loop()
{
  do_luong();
  if (U_out < 27.5)
  {
    digitalWrite(PC13, LOW);
    mppt_INCCOND();
    pwmWrite(PWMpin, Vinref);
  }
  if (U_out >= 27.5)
  {
    Vinref = 50;
    pwmWrite(PWMpin, 0);
    digitalWrite(PC13, HIGH);
    //Vinref = 300;
  }

//  giam_sat();
  digitalWrite(PA6, HIGH);
  //delay(300);


  // 1 Biến đo dòng tiêu thụ của mach điều khiển + loa.   I_out
  // Đưa hết giá trị đo được ra biến toàn cục, kiểu dữ liệu float hết

  // Toan code :
  while ( Serial1.available() ) {  // khi có tín hiệu uart nhận được
    char inBuff ;
    inBuff = (char)Serial1.read() ;
    masterData[cnt] = inBuff ;    // cộng dồn từng byte dữ liệu
    cnt++ ;
    Serial.write(inBuff) ;
    if ( inBuff == '\n' ) {       // khi có ký tự '\r' thì tức là báo đã kết thúc lệnh
      masterDataPeriod_FLAG = true ;   // bật cờ Đã nhận được gói tin từ master
      break ;                     // kết thúc
    }
  }

  if ( masterDataPeriod_FLAG == true ) {  // Khi phát hiện có cờ bật lên, bắt đầu kiểm tra nội dung yêu cầu :
    Serial.println("\nCheck cmd : ") ;
    if ( strstr( masterData, "RPD") != NULL ) {   // giải mã ý nghĩa gói tin, nếu là yêu cầu lấy dữ liệu định kì thì :
      Serial.println("Sending response") ;
      sendAllData2Master( U_out, Power , I_pv , I_load ) ;     // gửi toàn bộ giá trị hi ện tại cho Master
    }
    cnt = 0 ;                                     // reset lại cờ, và các biến nhớ
    memset(masterData, 0, sizeof(masterData) ) ;
    masterDataPeriod_FLAG = false ;
    Serial.println("Done !") ;                     // kết thúc quy trình
  }

  // Quản lý lỗi :
  // ( Thầy Tuấn code ), khi nào thấy điện áp thấp, chỉ cần đặt lệnh if tại đây và dùng hàm truyenLowVoltage( U_out ); để báo cho Master là xong !
  /*
      vd:
      if( U_out < 11 ) {
        truyenLowVoltage( U_out ) ;
      }
  */

  if ( (unsigned long) (millis() - timeCnt) > 3000 ){
    timeCnt = millis() ; 
    Serial.println("Now sending data ... ") ; 
    Serial1.println("Hello") ; 
  }
  // End Toan code

}

void do_luong()
{
  V1 = 0; V2 = 0; Sample = 400;
  for (j = 0; j < Sample; j++)
  {
    V1 += analogRead(Upv);
    V2 += analogRead(Uout);
  }
  U_pv =  0.007521 * V1 / Sample ; // Dien ap cua tam pin mat troi
  U_out = 0.007521 * V2 / Sample ;

  I1 = 0; I2 = 0; Sample = 800;
  for (j = 0; j < Sample; j++)
  {
    I1 += analogRead(Ipv);
    delayMicroseconds(3);
    I2 += analogRead(Iout) ;
    delayMicroseconds(3);
  }
  I_pv = I1 * 0.000672 / Sample;    // Dong dien tam pin mat troi
  I_out = (7.326007326 * I2 - 6.7155067155 * I1) / (0.005 * 100000 * Sample);
  /*if (I_out < 0)
  {
    I_out = 0;
  }*/

  dV = U_pv - U_pvold;
  Power = U_pv * I_pv;              // Cong suat phat cua tam pin mat troi
  dP = Power - Power_old;
}

void mppt_INCCOND()
{
  /*V1 = 0; I1 = 0; Sample = 900;
    for (j = 0; j < Sample; j++)
    {
    V1 += analogRead(Upv);
    I1 += analogRead(Ipv);
    }
    U_pv = V1 * 0.007521 / Sample ;   // Dien ap cua tam pin mat troi
    I_pv = I1 * 0.000672 / Sample;    // Dong dien tam pin mat troi

    dV = U_pv - U_pvold;
    Power = U_pv * I_pv;              // Cong suat phat cua tam pin mat troi
    dP = Power - Power_old;*/

  if ((dP > 0 && delta > 0) || (dP < 0 && dV > 0))
  {
    Vinref = Vinref + MPPT_STEP;
  }
  if ((dP > 0 && dV > 0 && delta < 0) || (dP < 0 && dV < 0))
  {
    Vinref = Vinref - MPPT_STEP;
  }

  Power_old = Power;
  U_pvold = U_pv;

  if (Vinref > Duty_gh)
  {
    Vinref = Duty_gh;
  }
  if (Vinref < 10)
  {
    Vinref = 10;
  }

  delta = Vinref - Vinrefold;
  Vinrefold = Vinref;

}
void giam_sat()
{
  Serial.print("Upv : ");
  Serial.println(U_pv, 1);
  Serial.print("Ipv : ");
  Serial.println(I_pv, 2);
  Serial.print("Uout : ");
  Serial.println(U_out, 1);
  Serial.print("Iout : ");
  Serial.println(I_out, 2);
  Serial.print("Vinref : ");
  Serial.println(Vinref);
  Serial.println(I2);
  Serial.print("Power : ");
  Serial.println(Power, 1);         //// Thịnh công tấm pin mặt trời
  Serial.println();

}

/*
   Toan code : thêm bắt đầu từ đây :
*/

void sendAllData2Master( float _U_out, float _Power , float _I_pv , float _I_load ) { //  Ngay tại hàm này, sẽ truyền hết toàn bộ dữ liệu sang master
  Serial1.print(_U_out, 2) ;  // gui U out sang master
  Serial1.print(_Power, 2) ;  // gui Power sang master
  Serial1.print(_I_pv, 2) ;  // gui I pv sang master
  Serial1.print(_I_load, 2) ;  // gui I load sang master
  Serial1.println("OK") ;   // kết thúc gói tin
}


// Hàm truyền khi đo mức điện áp thấp bất thường < 11V :
void truyenLowVoltage( float _U_out ) {   // tham số cần nhập là giá trị điện áp đo được
  Serial1.print("LVW\r\n") ;  // gui lenh : Low Voltage Warning
  /*
     Sau khi khởi lệnh báo Low Voltage Warning, mặc định master sẽ phản hồi lại lệnh "RDP" để lấy dữ liệu định kì.
  */
}


//void serialEvent1() {               // đây là hàm ngắt UART1, khi có gói tin từ master đến, mặc định sẽ bay vào đây để đọc dữ liệu gói tin
//  if ( Serial1.available() ) {
//    char inBuff ;
//    inBuff = Serial1.read() ;
//    masterData[cnt] = inBuff ;
//    cnt++ ;
//    Serial.print(inBuff) ;
//    if( inBuff == '\r' ){
//      masterDataPeriod_FLAG = true ;
//      break ;
//    }
//  }
//}
