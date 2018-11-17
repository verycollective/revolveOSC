


/*
 *  Revolve OSC
 *  -------------
 *  Small project to transmit rotational data from absolute encoder to d3 media server
 *  Data transmission over OSC
 *
 *  Code by Carlos Valente
 *
 *
 */

#include "DEFINITIONS.h"

long angle;
int32_t pos;
static uint8_t state;   // variable for state control
enum states {GET, STREAM, SETUP};

/* Initialize libraries */
EthernetUDP Udp;

// network stuff
byte MAC[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x07, 0x74 };
IPAddress IP(10, 0, 0, 11);
IPAddress TARGET_IP(10, 0, 0, 255); // BROADCAST UNTESTED
int PORT = 8888;

void setup(){

  /* Initialize aux */
  state = GET;
  angle = 0.0L;

  /* Initialize pins */
  // LEDS
  PIOC -> PIO_OER = (1 << LED1) | (1 << LED2) | (1 << LED3);

  /* Initialize encoder stuff */
  // Setup Quadrature Encoder with index
  REG_PMC_PCER0 = (1 << 27); // activate clock for TC0
  // select XC0 as clock source and set capture mode
  //REG_TC0_CMR0 = 5; // continous count
  // or
  REG_TC0_CMR0 = (1 << 0) | (1 << 2) | (1 << 8) | (1 << 10); // reset counter on index
  // activate quadrature encoder and position measure mode, no filters
  REG_TC0_BMR = (1 << 8) | (1 << 9) | (1 << 12);
  // enable the clock (CLKEN=1) and reset the counter (SWTRG=1)
  REG_TC0_CCR0 = 5;

  /* Initialize libraries */
  Ethernet.begin(MAC, IP);
  Udp.begin(PORT);
  Serial.begin(115200);
}

void loop(){
  switch (state) {

    case GET:
      read_data();
      Serial.print(pos);
      Serial.print("\t");
      Serial.println(angle);
    break;

    case STREAM:
      send_osc(angle);
      send_socket(angle);
    break;

    case SETUP:

    state = 1;
    break;

    default:
    // something went wrong and state machine is set to invalid state
    state = GET;
    break;

  }
}

void read_data(){
  pos = REG_TC0_CV0;
  if (pos < 0) { pos += encoder_quad; }
  angle = map(pos, encoder_quad, 0, 360.0, true);
}

void send_osc(long &val){
  OSCMessage msg("/rev/rot");

  msg.add(val);

  Udp.beginPacket(TARGET_IP, PORT);
  msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message

}

void send_socket(long &val){
  // nothing yet, for websockets
}

void reset_home(){

  noInterrupts();

  // get current position

  // delay??

  // if button is no longer pressed reset home position to current

  // if the button is still pressed after a while enter setup mode

  interrupts();

}






float map(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp) {
// nicked from ofx

  if (fabs(inputMin - inputMax) < FLT_EPSILON){
    return outputMin;
  } else {
    float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);

    if( clamp ){
      if(outputMax < outputMin){
        if( outVal < outputMax )outVal = outputMax;
        else if( outVal > outputMin )outVal = outputMin;
      }else{
        if( outVal > outputMax )outVal = outputMax;
        else if( outVal < outputMin )outVal = outputMin;
      }
    }
    return outVal;
  }
}
