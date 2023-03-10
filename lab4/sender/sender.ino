#include <SPI.h>
#include <LoRa.h>
using namespace std;

#define DeviceNumber 6

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
uint32_t previousMillis = 0, previousMillis2 = 0, previousMillisButton = 0;
int led_status = 0, led2_status = 0;

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
}

void loop()
{
  //Sending package after pushing the button
  if (millis() - previousMillisButton >= interval)
  {
    if (buttonFlag == 0){
      buttonFlag = 1;
      LoRa.beginPacket();
      LoRa.write((const uint8_t*)send_buf_p, 20);
      LoRa.endPacket();
      
      Serial.println("Sent send_buf: ");
      PrintData((buf *)send_buf_p);
      previousMillisButton = millis();
    }
  }
  else
  {
    buttonFlag = 0;
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