#include <SPI.h>
#include <LoRa.h>
using namespace std;

#define DeviceNumber 9

// Functions prototypes
uint16_t Calculate_CRC(void *pointer);
void PrintData(struct buf *send_buf_p);
uint16_t Check_CRC(void *pointer);
int NotDevice(struct buf *buf_p);

//Variables to use LEDs
const int button = 5; //Just a pin number
const int led2 = 6; //This one is built in
const int led = 7;
const uint32_t interval = 1000; //This variable defines how many ms LED's are turn on after receiving last signal
uint32_t previousMillis = 0, previousMillis2 = 0;
uint32_t previousMillisMiddle7 = 0;
const int deviceNumberMiddle7 = 7;
int isCommunicationStatusPrinted = 0;
int led_status = 0, led2_status = 0;
const uint32_t timeout = 10000; 

// Protocol definition
struct buf {
  uint8_t device_number;
  uint8_t repeat_number;
  uint8_t data[16];
  uint16_t CRC;
} send_buf, receive_buf; //send_buf is the data waiting to be sent by the device. receive_buf is going to store received data. It should make the code faster and easier to use.
void *receive_buf_p, *receive_buf_p_0; //wskazniki na bufory
buf *send_buf_p; //skaznik na send bufor
int packetSize, i;

int buttonFlag = 0;

void setup() 
{
  //Initialization of pins 
  pinMode(button, INPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  pinMode(led2, OUTPUT);
  digitalWrite(led2, LOW);
  Serial.begin(9600);
  Serial.println("LoRa Transiver");
  if (!LoRa.begin(8681E5))   //Initialize communication at a given frequency (868.1MHz at this case) and checks is there was any problem with it. 
  {
    Serial.println("Starting LoRa failed!");
    while (1)
    {
      //If the communication initialization fails, LEDs will start blinking 
      digitalWrite(led, HIGH);
      digitalWrite(led2, HIGH);
      delay(500);
      digitalWrite(led, LOW);
      digitalWrite(led2, LOW);
      delay(500);
    }
  }

  // Writing data to protocol that is going to be send
  send_buf_p = &send_buf;
  send_buf_p->device_number = DeviceNumber;
  send_buf_p->repeat_number = 0;
  for (i = 0; i < 16; i++) 
  {
    send_buf_p->data[i] = i;
  }
  send_buf_p->CRC = Calculate_CRC(send_buf_p);

  // Void pointers which are use to efficiently read data
  receive_buf_p = &receive_buf;
  receive_buf_p_0 = receive_buf_p;

  previousMillisMiddle7 = 0;
}

void loop()
{
  //Receiving package
  packetSize = LoRa.parsePacket(); //Returns expected number of bytes to be received
  if (packetSize)
  {
    while (LoRa.available()) //Returns numbers of bytes to be read
    {
      *(uint8_t *)receive_buf_p = LoRa.read(); //this reads 1 byte at a time
      (uint32_t *)receive_buf_p++;
    }
    receive_buf_p = receive_buf_p_0; //ustawiamy receive_buf_p z powrotem na pierwszy przeczytany bajt
    
    Serial.println("Received_buf: "); 
      PrintData((buf *)receive_buf_p_0);
      
      if(Check_CRC(receive_buf_p)!=0) //Checks is the CRC in readed data is correct
      { 
        Serial.print("CHECK CRC ERROR  ");
        Serial.println(Check_CRC(receive_buf_p));
        digitalWrite(led2, HIGH);
        led2_status = 1;
        previousMillis2 = millis();
      }
      else //If the CRC is correct
      {
        if(receive_buf.device_number == deviceNumberMiddle7){
          previousMillisMiddle7 = millis();
          isCommunicationStatusPrinted = 0;
        }
        
        //Turn on the LED if the last data byte != 0
        if (receive_buf.data[15] != 0)
        {
          digitalWrite(led, HIGH);
          led_status = 1;
          previousMillis = millis();
        }
      }
  }
  
  if (millis() - previousMillisMiddle7 >= timeout)
    { 
      previousMillis = millis();
      digitalWrite(led, LOW);
      led_status = 0;
      if(isCommunicationStatusPrinted == 0){
        Serial.println("No communication with device 7"); 
        isCommunicationStatusPrinted = 1;
      }
      
    }
}  

//Calculates simple CRC-16-CCITT
uint16_t Calculate_CRC(void *pointer)
{
  uint8_t x;
  uint16_t crc = 0xFFFF;
  int length1 = 18;
  while (length1--){
    x = crc >> 8 ^ *(uint8_t *)pointer++;
    x ^= x>>4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
  }
  return crc;
}  

//Returns diffrence beetween readed CRC and calculated CRC 
uint16_t Check_CRC(void *pointer)
{
  return (Calculate_CRC(pointer) - *(uint16_t *)(pointer+18));
}

//Returns 1 if the data hasn't been on the device

//As you can see, this function prints protocol
void PrintData(struct buf *send_buf_p)
{
  Serial.print("Device number: ");
  Serial.print(send_buf_p->device_number);
  Serial.print("; repeat_number: ");
  Serial.print(send_buf_p->repeat_number);
  Serial.print("; data: ");
  for (i = 0; i < 16; i++)
  {
    Serial.print(send_buf_p->data[i]);
    Serial.print(" ");
  }
  Serial.print("; CRC:   ");
  Serial.println(send_buf_p->CRC);
  
}