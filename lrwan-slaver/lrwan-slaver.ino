#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <string.h>


// Datalogger 1 : 
static const PROGMEM u1_t NWKSKEY[16] = { 0x58, 0x3c, 0x92, 0x69, 0x90, 0x8c, 0xcc, 0x6c, 0x21, 0x17, 0x34, 0x22, 0x47, 0x8f, 0x33, 0x83 };
static const u1_t PROGMEM APPSKEY[16] = { 0xbb, 0x0d, 0x82, 0xf7, 0xe4, 0x90, 0x47, 0xaf, 0x01, 0x84, 0x44, 0x4e, 0x55, 0xc2, 0x9b, 0x43 };
static const u4_t DEVADDR = 0x0167ebf6 ; 

//
//// Datalogger 2 : 
//static const PROGMEM u1_t NWKSKEY[16] = { 0xf5, 0x2a, 0xee, 0x35, 0x12, 0x01, 0x76, 0xee, 0x0e, 0xec, 0x12, 0xde, 0x89, 0xe1, 0x77, 0x9b };
//static const u1_t PROGMEM APPSKEY[16] = { 0xeb, 0x8d, 0xbb, 0xaa, 0x2c, 0xdd, 0xfe, 0x60, 0x20, 0x6b, 0xf8, 0xe7, 0xf5, 0xef, 0x4a, 0x0e };
//static const u4_t DEVADDR = 0x00cb10b9 ; 

void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }


uint8_t mydata[20] = "";
static osjob_t sendjob;
void do_send(osjob_t* j) ;
void getMasterData() ;
uint8_t sendCommand(const char* ATcommand, const char* expected_answer, unsigned int timeout) ;
char masterData[20] ; 
int cnt2reset = 0 ; 
const int times2Reset = 30 ; 

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 10;

SoftwareSerial debug( 3, 4 )  ; // rx tx 

#define   REQ_DATA_PERIODLY "\r\nRDP"

#define   WARNING_LV1 0x01 
#define   WARNING_LV2 0x02 
#define   WARNING_LV3 0x03 
#define   WARNING_LV4 0x04 
#define   WARNING_LV5 0x05 
#define   STOP_WARNING 0x00 

#define   REQ_WARNING_LV1 "\r\nRWL1"
#define   REQ_WARNING_LV2 "\r\nRWL2"
#define   REQ_WARNING_LV3 "\r\nRWL3"
#define   REQ_WARNING_LV4 "\r\nRWL4"
#define   REQ_WARNING_LV5 "\r\nRWL5"
#define   REQ_STOP_WARNING "\r\nRSW"


void(* resetFunc) (void) = 0;//cài đặt hàm reset 

// Hàm này để giao tiếp UART với chip STM32
uint8_t sendCommand(const char* ATcommand, const char* expected_answer, unsigned int timeout) {
    uint8_t x=0,  answer=0;
    char response[20];
    unsigned long previous;
    memset(response, 0 , 20 );    // Initialize the string
    debug.println("\nSending command to Master:  ") ; 
    debug.println(ATcommand) ;

    while( Serial.available() > 0) Serial.read();    // Clean the input buffer
    Serial.println(ATcommand);    // Send the AT command 
    x = 0;
    previous = millis();
    // this loop waits for the answer
    do{
        if(Serial.available() != 0){    
            // if there are data in the UART input buffer, reads it and checks for the asnwer
            response[x] = Serial.read();      
//            Serial.print(response[x]);
            x++;
            // check if the desired answer  is in the response of the module
            if (strstr(response, expected_answer) != NULL){
                answer = 1;
            }
        }
         // Waits for the asnwer with time out
    }while((answer == 0) && ((millis() - previous) < timeout));

    return answer;
}


// Hàm này kế thừa chức năng giao tiếp trên để thực hiện trao đổi dữ liệu, tương tác ...  
// Như phía dưới là hàm để lấy dữ liệu từ STM32
void getMasterData(){
  if ( cnt2reset >= times2Reset ) {
    cnt2reset = 0 ; 
    resetFunc(); 
  }
    debug.println("\nStart getting data from Master :") ;
    memset(masterData, 0, 20);    // Initialize the string
    int x = 0; 
    bool done = false ;
    unsigned long timeOutRes ; 

    sendCommand( REQ_DATA_PERIODLY, "OK", 1000) ; 

    timeOutRes = millis() ; 
    
    while ( done == false && ((unsigned long) (millis() - timeOutRes) < 3000 ) ){
      if( Serial.available() ){
          char input = Serial.read();
          masterData[x] = input ; 
          // debug.print(masterData[x]) ;  
          if(  input  ==  '\n') done = true ; 
          // debug.print(x) ;  
          x++ ;
          
      }
    }
    
    if ( done == false ) {
      debug.println("\nGetting data failed !") ;
      Serial.println("Failed !") ; 
    }else {
      debug.println("\nGetting data done !") ; 
      debug.println(masterData) ;  
      Serial.println("Success !") ; 
    }
    
    cnt2reset++ ; 
}


const lmic_pinmap lmic_pins = {
  .nss = 10,                       // chip select on feather (rf95module) CS
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 8,                       // reset pin
  .dio = {2, 7, 9},               // assumes external jumpers [feather_lora_jumper]
                                  // DIO1 is on JP1-1: is io1 - we connect to GPO6
                                  // DIO1 is on JP5-3: is D2 - we connect to GPO5
};


void onEvent (ev_t ev) {
    debug.print(os_getTime());
    debug.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            debug.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            debug.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            debug.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            debug.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            debug.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            debug.println(F("EV_JOINED"));
            break;
        case EV_RFU1:
            debug.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            debug.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            debug.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            debug.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              debug.println(F("Received ack"));
            if (LMIC.dataLen) {
              debug.println(F("Received "));
              debug.println(LMIC.dataLen);
              debug.println(F(" bytes of payload"));
              if (LMIC.dataLen == 1) {
                uint8_t result = LMIC.frame[LMIC.dataBeg + 0];
                if (result == WARNING_LV1)  {
                    sendCommand((char*)REQ_WARNING_LV1, (char*)"OK", 2000) ; 
                }else if (result == WARNING_LV2)  {
                    sendCommand((char*)REQ_WARNING_LV2, (char*)"OK", 2000) ; 
                }else if (result == WARNING_LV3)  {
                    sendCommand((char*)REQ_WARNING_LV3, (char*)"OK", 2000) ; 
                }else if (result == WARNING_LV4)  {
                    sendCommand((char*)REQ_WARNING_LV4, (char*)"OK", 2000) ; 
                }else if (result == WARNING_LV5)  {
                    sendCommand((char*)REQ_WARNING_LV5, (char*)"OK", 2000) ; 
                }else if (result == STOP_WARNING)  {
                    sendCommand((char*)REQ_STOP_WARNING, (char*)"OK",2000) ; 
                }                                                   
              }
            }
            // Schedule next transmission
            // Get data periodly : 
            getMasterData() ; 


            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            debug.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            debug.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            debug.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            debug.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            debug.println(F("EV_LINK_ALIVE"));
            break;
         default:
            debug.println(F("Unknown event"));
            break;
    }
}


void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        debug.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        strcpy((char*)mydata, masterData) ;
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        debug.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}


void setup() {
    Serial.begin(9600);
    
    debug.begin(9600) ;
    debug.println(F("Starting"));

    Serial.println("\r\nLoRaWAN restarted :");

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    #ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif

    LMIC_setupChannel(0, 921200000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 921400000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 921600000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 921800000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 922000000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 922200000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 922400000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 922600000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 924800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    getMasterData() ; // get Master data before first uplink 

    // Start job
    do_send(&sendjob);
}


void loop() {
    os_runloop_once();
}
