#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <string.h>
#include <CAN.h>

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
String txValue="LOCK";

// See the following for generating UUIDs
// https://www.uuidgenerator.net/
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define ESP_CAN_ID          (unsigned int)(0x18F10110)
#define M50_CAN_ID          (unsigned int)(0x18F00164)
#define ADD_CLAM_CAN_ID     (unsigned int)(0x18EEFF10)
#define LOCK_DATA           (unsigned char)(0xF1)
#define UNLOCK_DATA         (unsigned char)(0xF4)

#define TX_GPIO_NUM   21  // Connects to CTX
#define RX_GPIO_NUM   22  // Connects to CRX

unsigned char    outputbuffer[8]={0xFF,0xFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF};
unsigned char    addClamBuffer[8]={0x00,0x00,0X20,0X04,0X00,0X10,0X0E,0X20};
long             filterMask = 0xffffffff,receivedCanId;
int              dlc =8,recivedCanDlc,cnt=0, timerFlag=0,lockStatusByte=0xFF,lockStatusBits=00;
bool             bleReceivedFlag=0,bleReceivedValue=0,canReceivedFlag=0;
unsigned char   *input_buff_ptr = NULL;
hw_timer_t      * timer = NULL;
std::string rxValue,msg="AADD",cmpStr ;
//==================================================================================//

class MyServerCallbacks: public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer)
    
    {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer)
    {
     deviceConnected = false;
    }
};
//============================================================================//

//========================BLE CALBACK FUNCTION================================//
class MyCallbacks: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
      rxValue = pCharacteristic->getValue();
      
      if (rxValue.length() > 0)
      {
       Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
        {
          Serial.print(rxValue[i]);
        }
        bleReceivedFlag = 1;
        Serial.println();
        Serial.println("*********");
     }
    }
};
//============================================================================//

//=========================TIMER CALBACK FUNCTION=============================//
void IRAM_ATTR onTimer()
{
  timerFlag=1;
}
//============================================================================//

void setup()
  {
    //=====================================BLE SETUP========================= =========//
    Serial.begin(115200);
    // Create the BLE Device
    BLEDevice::init("UART Service");
    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);
    // Create a BLE Characteristic
    pTxCharacteristic = pService->createCharacteristic( CHARACTERISTIC_UUID_TX,BLECharacteristic::PROPERTY_NOTIFY);                    
    pTxCharacteristic->addDescriptor(new BLE2902());
    BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX,BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks());
    // Start the service
    pService->start();
    // Start advertising
    pServer->getAdvertising()->start();
    Serial.println("Waiting a client connection to notify...");
    //=================================================================================//
    //==================================== CAN SETUP===================================//
    Serial.println ("CAN Transmited/Receiver");
    // Set the pins
    CAN.setPins (RX_GPIO_NUM, TX_GPIO_NUM);

    // start the CAN bus at 500 kbps
    if(!CAN.begin (250E3)) {
      Serial.println ("Starting CAN failed!");
      while (1);
    }
    else {
      Serial.println ("CAN1 Initialized");
    }
    //CAN.filterExtended(filterCanId,filterMask);
    CAN.filterExtended(M50_CAN_ID,filterMask);
    // register the receive callback
    CAN.onReceive(onReceive);

    // address claming msg
    CAN.beginExtendedPacket(ADD_CLAM_CAN_ID, dlc, false);
    CAN.write(addClamBuffer,8); 
    CAN.endPacket();
    //==================================================================================//
    //=====================================TIMER SETUP==================================//
    Serial.println("start timer ");
    timer = timerBegin(0, 80, true);  // timer 0, MWDT clock period = 12.5 ns * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns * 80 -> 1000 ns = 1 us, countUp
    timerAttachInterrupt(timer, &onTimer, true); // edge (not level) triggered 
    timerAlarmWrite(timer, 200000, true); // 200000 * 1 us = 1 s, autoreload true
    timerAlarmEnable(timer); // enable
    //==================================================================================//
  }

void loop()
{
  cmpStr=rxValue.substr(0,4);
  if((bleReceivedFlag == 1) && (cmpStr == msg ))
   {
      if((rxValue[rxValue.length()-2] == '0') && (rxValue[rxValue.length()-3]=='0'))
        bleReceivedValue = 0; 

      if((rxValue[rxValue.length()-2] == '1') && (rxValue[rxValue.length()-3]=='0'))
        bleReceivedValue = 1; 
   }
    bleReceivedFlag = 0;

  if(timerFlag == 1)
     { 
      Serial.println ("CAN STARTED");
      canSender(); 
      timerFlag=0;
     }

  if(canReceivedFlag == 1)
   {
    if(receivedCanId == M50_CAN_ID)
    {
      lockStatusByte=input_buff_ptr[0];

      if(lockStatusByte == 0xFD)// Comparing The 0th Byte from the Receievd ID 
        txValue = "LOCK";
      if(lockStatusByte == 0xFC)  
        txValue = "UNLOCK";
      if(lockStatusByte == 0xFF) 
        txValue = "ERROR";

     }
   }
     
   if (deviceConnected)
   {
      pTxCharacteristic->setValue(txValue.c_str());
      pTxCharacteristic->notify();
      delay(10);
   }
   if(!deviceConnected && oldDeviceConnected)
   {
//       delay(500); // give the bluetooth stack the chance to get things ready
       pServer->startAdvertising(); // restart advertising
       Serial.println("start advertising");
       oldDeviceConnected = deviceConnected;
    }
   // connecting
    if (deviceConnected && !oldDeviceConnected)
   {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
   }
}
//============================================================================//

//==========================CAN SENDER FUNCTION===============================//
void canSender() {
  Serial.print ("Sending packet 1 ... ");

  if(bleReceivedValue == 1)
  {
    Serial.println ("CAN SENDING LOCK STATUS");
    outputbuffer[0]=LOCK_DATA;
    CAN.beginExtendedPacket(ESP_CAN_ID, dlc, false);
    CAN.write(outputbuffer,8); 
    CAN.endPacket();
  }
  else if(bleReceivedValue == 0)
  {
    Serial.println ("CAN SENDING UNLOCK STATUS");
    outputbuffer[0]=UNLOCK_DATA;
    CAN.beginExtendedPacket(ESP_CAN_ID, dlc, false);
    CAN.write(outputbuffer,8); 
    CAN.endPacket();
  }

  Serial.println ("done");
}
//============================================================================//

//==========================CAN RECEIVER FUNCTION=============================//
void onReceive(int packetSize) {
  // received a packet
  unsigned char input_buffer[8];
  Serial.print("Received ");
  canReceivedFlag=1;

  if (CAN.packetExtended()) {
    Serial.print("extended ");
  }
   
  if (CAN.packetRtr()) {
    // Remote transmission request, packet contains no data
    Serial.print("RTR ");
  }

  Serial.print("packet with id 0x");
  Serial.print(CAN.packetId(), HEX);
  receivedCanId = CAN.packetId();

  if (CAN.packetRtr()) {
    Serial.print(" and requested length ");
    Serial.println(CAN.packetDlc());
  } else {
    Serial.print(" and length ");
    Serial.println(packetSize);
    recivedCanDlc = CAN.packetDlc();
    // only print packet data for non-RTR packets
    int i=0;
    input_buff_ptr = (unsigned char*)malloc(CAN.available());
    if(input_buff_ptr != NULL)
    {
    while (CAN.available()) {
     // Serial.print((char)CAN.read(),HEX);
       input_buff_ptr[i]=CAN.read();
       i++;
    }
    }

    Serial.println();
  }
 
  Serial.println();
}
//==================================================================================//

