#ifndef COFFEE_MESSAGES_H
#define COFFEE_MESSAGES_H

#define CMD_GET 0
#define CMD_MAKE_1 1
#define CMD_MAKE_2 2
#define CMD_MAKE_3 3
#define CMD_POUR 4
#define CMD_DUMP 5

#define CODE_PERFORMING 0x00
#define CODE_COMPLETE 0x01
#define CODE_CUPS_READY 0x40
#define CODE_WATER_LEVEL 0x60
#define CODE_GROUNDS_LEVEL 0x20
#define CODE_ERR_WATER_LOW 0x81
#define CODE_ERR_GROUNDS_LOW 0x82
#define CODE_ERR_NO_MUG 0x83
#define CODE_ERR_WATER_AND_GROUNDS_LOW 0x84
#define CODE_ERR_TOO_MUCH_COFFEE 0x85
#define CODE_ERR_NO_COFFEE 0x86
#define CODE_ERR_CHECKSUM 0xCC
#define CODE_ERR_UNKNOWN 0xFF

#define RX_MESSAGE_LEN 15
#define TX_MESSAGE_LEN 18

#define EOL '\n'

//#define DEBUG

enum RxMessage {
  GET, MAKE_1, MAKE_2, MAKE_3, POUR, DUMP, ERR_UNKNOWN, ERR_BAD_FORMAT, ERR_BAD_CHECKSUM
};

struct RxMessageQueue {
  RxMessage* rxQueue;
  int n;
} msgQueue;

struct MessageBuffer {
  char* buff;
  int used;
  int len;
} rxBuff, txBuff;

void buffInit(struct MessageBuffer *msgBuffer, int initLen) {
  msgBuffer->buff = new char[initLen];
  msgBuffer->used = 0;
  msgBuffer->len = initLen;
}

void buffEnsureAvailable(struct MessageBuffer *msgBuffer, int avail) {
  char* cpy;
  if(msgBuffer->len - msgBuffer->used < avail) {
    cpy = new char[msgBuffer->used + avail];
    memcpy(cpy, msgBuffer->buff, sizeof(char) * msgBuffer->used);
    delete[] msgBuffer->buff;
    msgBuffer->buff = cpy;
    msgBuffer->len = msgBuffer->used + avail;
  }
}

void buffPut(struct MessageBuffer *msgBuffer, char c) {
  msgBuffer->buff[msgBuffer->used++] = c;
}

void buffRewind(struct MessageBuffer *msgBuffer) {
  msgBuffer->used = 0;
}

void messagesInit(int baudRate) {
  msgQueue.rxQueue = new RxMessage[1];
  msgQueue.n = 0;
  buffInit(&rxBuff, RX_MESSAGE_LEN);
  buffInit(&txBuff, TX_MESSAGE_LEN);
  Serial.begin(baudRate);
}

int checksum(char* s, int len) {
  int c = 0, i;
  for(i = 0; i < len; i++) {
      c ^= *(s + i);
  }
  return c;
}

/*
 *  "$BLECOFF,XX*XX\0"
 */
RxMessage readRXMessage(char* buff) {
  int code, cksum, n;
  n = sscanf(buff, "$BLECOFF,%2i*%2x", &code, &cksum);
  if(n != 2) {
    return ERR_BAD_FORMAT;
  }
  if(cksum != checksum(buff + 1, 10)) {
    return ERR_BAD_CHECKSUM;
  }
  switch(code) {
    case CMD_GET: return GET;
    case CMD_MAKE_1: return MAKE_1;
    case CMD_MAKE_2: return MAKE_2;
    case CMD_MAKE_3: return MAKE_3;
    case CMD_POUR: return POUR;
    case CMD_DUMP: return DUMP;
    default: return ERR_UNKNOWN;
  }
}

RxMessage getMakeNMessage(int n) {
  switch(n) {
    case 1: return MAKE_1;
    case 2: return MAKE_2;
    case 3: return MAKE_3;
    default: return ERR_UNKNOWN;
  }
}

RxMessage getPourOrDumpMessage(int n) {
  switch(n) {
    case 0: return POUR;
    case 1: return DUMP;
    default: return ERR_UNKNOWN;
  }
}

/*
 *  "$BLECOFF,XX,XX*XX\n"
 */
void writeTXMessage(char* buff, RxMessage respondTo, byte code) {
  byte codeA = (int) respondTo;
  int ckSum;
  sprintf(buff, "BLECOFF,%02i,%02x", codeA, code);
  ckSum = checksum(buff, 13);
  sprintf(buff, "$BLECOFF,%02i,%02x*%02x\n", codeA, code, ckSum);
}

void messagesSend(RxMessage respondTo, byte code) {
  writeTXMessage(txBuff.buff, respondTo, code);
  Serial.print(txBuff.buff);
}

int messagesSend(RxMessage respondTo, byte code, byte data) {
  messagesSend(respondTo, code | data);
}

RxMessage messagesGet() {
  if(msgQueue.n > 0) {
    msgQueue.n--;
    return msgQueue.rxQueue[0];
  }
  return ERR_BAD_FORMAT;
}

int messagesAvailable() {
  int a, i;
  char c;
  if(msgQueue.n == 0) {
    a = Serial.available();
    if(a > 0) {
      buffEnsureAvailable(&rxBuff, a);
      while(a > 0 && msgQueue.n == 0) {
        c = Serial.read();
        if(c == EOL) {
          buffPut(&rxBuff, 0);
          #ifdef DEBUG
          Serial.print("Command Recieved: ");
          Serial.println(rxBuff.buff);
          #endif
          msgQueue.rxQueue[0] = readRXMessage(rxBuff.buff);
          msgQueue.n = 1;
          buffRewind(&rxBuff);
        } else {
          buffPut(&rxBuff, c);
        }
        a--;
      }
    }
  }
  return msgQueue.n;
}

#endif
