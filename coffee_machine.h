#ifndef COFFEE_MACHINE_H
#define COFFEE_MACHINE_H

#include "time.h"

#define TIME_TO_MAKE_S 5.0
#define TIME_TO_POUR_S 2.0

struct CoffeeMachine {
  byte coffeeLevel;
  byte waterLevel;
  int hasMug;
  int idle;
  time_t startedMaking;
  int nMaking;
  int nReady;
  time_t startedPouring;
  int pouring;
  int dumping;
} machine;

void machineInit() {
  machine.coffeeLevel = 5;
  machine.waterLevel = 3;
  machine.hasMug = 1;
  machine.idle = 1;
  machine.nMaking = 0;
  machine.nReady = 0;
  machine.pouring = 0;
}

byte machineGetGroundsLevel() {
  return machine.coffeeLevel;
}

byte machineGetWaterLevel() {
  return machine.waterLevel;
}

byte machineGetNCupsReady() {
  return (byte) machine.nReady;
}

int machineHasMug() {
  return machine.hasMug;
}

int machineCanMake() {
  return machine.idle;
}

int machineMake(int n) {
  machine.idle = 0;
  machine.coffeeLevel -= n;
  machine.waterLevel -= n;
  time(&machine.startedMaking);
  machine.nMaking = n;
}

int machineIsReady() {
  return machine.nMaking > 0 && difftime(time(), machine.startedMaking) >= (TIME_TO_MAKE_S * machine.nMaking);
}

int machineGetNMaking() {
  return machine.nMaking;
}

int machineGetNReady() {
  return machine.nReady;
}

void machineMade() {
  machine.nReady = machine.nMaking;
  machine.nMaking = 0;
}

int machineCanPour() {
  return machine.nReady > 0 && !machine.pouring;
}

void machinePour() {
  machine.nReady--;
  time(&machine.startedPouring);
  machine.pouring = 1;
}

void machineDump() {
  machinePour();
  machine.dumping = 1;
}

int machineGetNPouring() {
  return machine.pouring;
}

int machineIsPoured() {
  return machine.pouring && difftime(time(), machine.startedPouring) >= TIME_TO_POUR_S;
}

int machineGetIsDumping() {
  return machine.dumping;
}

void machinePoured() {
  machine.pouring = 0;
  machine.dumping = 0;
  if(machine.nReady == 0) {
    machine.idle = 1;
  }
}

void machineLog() {
  Serial.print("Idle: ");
  Serial.print(machine.idle);
  Serial.print("\nWater Level: ");
  Serial.print(machine.waterLevel);
  Serial.print(", Grounds Level: ");
  Serial.print(machine.coffeeLevel);
  Serial.print("\nSMaking: ");
  Serial.print(machine.startedMaking);
  Serial.print(", NMaking: ");
  Serial.print(machine.nMaking);
  Serial.print("\nNReady: ");
  Serial.print(machine.nReady);
  Serial.print("\nSPouring: ");
  Serial.print(machine.startedPouring);
  Serial.print(", Pouring: ");
  Serial.println(machine.pouring);
}

#endif

