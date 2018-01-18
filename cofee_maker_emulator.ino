#include "coffee_messages.h"
#include "coffee_machine.h"
#include "leds.h"

#define BAUD_RATE 9600
#define CMD_LEN 10
#define CMD_BUFFER_INC_SIZE 10
//#define DEBUG
//#define TEST_MUG

#define N_CUPS 3
#define CUP0_LED_PIN 6
#define CUP1_LED_PIN 7
#define CUP2_LED_PIN 8
#define NO_WATER_LED_PIN 10
#define NO_COFFEE_LED_PIN 11

const int CUP_LED_PINS[N_CUPS] = {CUP0_LED_PIN, CUP1_LED_PIN, CUP2_LED_PIN};

void setup() {
  machineInit();
  messagesInit(BAUD_RATE);
  ledsInit();
}

void updateLedState() {
  int i, nMaking, nReady, nPouring;
  nMaking = machineGetNMaking();
  nReady = machineGetNReady();
  nPouring = machineGetNPouring();
  for(i = 0; i < N_CUPS; i++) {
    if(i < nMaking) {
      ledsSetState(CUP_LED_PINS[i], LED_FLASH);
    } else if(i < nMaking + nReady) {
      ledsSetState(CUP_LED_PINS[i], LED_ON);
    } else if(i < nMaking + nReady + nPouring) {
      ledsSetState(CUP_LED_PINS[i], LED_FLASH);
    } else {
      ledsSetState(CUP_LED_PINS[i], LED_OFF);
    }
  }
  if(machineGetWaterLevel() > 0) {
    ledsSetState(NO_WATER_LED_PIN, LED_OFF);
  } else {
    ledsSetState(NO_WATER_LED_PIN, LED_ON);
  }
  if(machineGetGroundsLevel() > 0) {
    ledsSetState(NO_COFFEE_LED_PIN, LED_OFF);
  } else {
    ledsSetState(NO_COFFEE_LED_PIN, LED_ON);
  }
  ledsUpdate();
}

void loop() {
  RxMessage codeIn;
  int i;
  int sentMessage = false;
  while(messagesAvailable() > 0) {
    codeIn = messagesGet();
    i = 0;
    switch(codeIn) {
      case GET: {
        messagesSend(codeIn, CODE_CUPS_READY, machineGetNCupsReady());
        #ifdef DEBUG
        Serial.print("\n");
        #endif
        messagesSend(codeIn, CODE_WATER_LEVEL, machineGetWaterLevel());
        #ifdef DEBUG
        Serial.print("\n");
        #endif
        messagesSend(codeIn, CODE_GROUNDS_LEVEL, machineGetGroundsLevel());
        break;
      }
      case MAKE_3: i++;
      case MAKE_2: i++;
      case MAKE_1: {
        i++;
        if(machineGetWaterLevel() < i) {
          if(machineGetGroundsLevel() < i) {
            messagesSend(codeIn, CODE_ERR_WATER_AND_GROUNDS_LOW);
          } else {
            messagesSend(codeIn, CODE_ERR_WATER_LOW);
          }
        } else if(machineGetGroundsLevel() < i) {
            messagesSend(codeIn, CODE_ERR_GROUNDS_LOW);
        } else if(!machineCanMake()) {
          messagesSend(codeIn, CODE_ERR_UNKNOWN);
        } else {
          machineMake(i);
          messagesSend(codeIn, CODE_PERFORMING);
        }
        break;
      }
      case POUR: {
        #ifdef TEST_MUG
        if(!machineHasMug()) {
          messagesSend(codeIn, CODE_ERR_NO_MUG);
        } else 
        #endif
        if(!machineCanPour()) {
          messagesSend(codeIn, CODE_ERR_NO_COFFEE);
        } else {
          machinePour();
          messagesSend(codeIn, CODE_PERFORMING);
        }
        break;
      }
      case DUMP: {
        #ifdef TEST_MUG
        if(machineHasMug()) {
          messagesSend(codeIn, CODE_ERR_NO_MUG);
        } else
        #endif
        if(!machineCanPour()) {
          messagesSend(codeIn, CODE_ERR_NO_COFFEE);
        } else {
          machineDump();
          messagesSend(codeIn, CODE_PERFORMING);
        }
        break;
      }
      case ERR_BAD_CHECKSUM: {
        messagesSend(codeIn, CODE_ERR_CHECKSUM);
        break;
      }
      case ERR_BAD_FORMAT:
      case ERR_UNKNOWN:
      default: {
        /* Don't know what to reply to */
        messagesSend(GET, CODE_ERR_UNKNOWN);
      }
    }
    sentMessage = true;
    #ifdef DEBUG
    Serial.print("\n");
    machineLog();
    #endif
  }
  if(machineIsReady()) {
    codeIn = getMakeNMessage(machineGetNMaking());
    machineMade();
    messagesSend(codeIn, CODE_COMPLETE);
    sentMessage = true;
    #ifdef DEBUG
    Serial.print("\n");
    machineLog();
    #endif
  }
  if(machineIsPoured()) {
    codeIn = getPourOrDumpMessage(machineGetIsDumping());
    machinePoured();
    messagesSend(codeIn, CODE_COMPLETE);
    sentMessage = true;
    #ifdef DEBUG
    Serial.print("\n");
    machineLog();
    #endif
  }
  updateLedState();
}


