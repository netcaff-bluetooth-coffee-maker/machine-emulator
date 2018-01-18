#ifndef COFFEE_TIME_H
#define COFFEE_TIME_H

typedef unsigned long time_t;

time_t time(unsigned long *t) {
  unsigned long mill = millis();
  if(t != NULL) {
    *t = mill;
  }
  return mill;
}

time_t time() {
  return time(NULL);
}

double difftime(time_t later, time_t earlier) {
  return ((later - earlier) / 1000.0);
}

#endif
