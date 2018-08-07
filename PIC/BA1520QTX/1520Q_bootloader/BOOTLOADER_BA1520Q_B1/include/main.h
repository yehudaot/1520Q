#ifndef MAIN_H_
#define MAIN_H_

#define MAIN_ADDRESS 0x2700

#define ATOMIC_SET(var, val)    do { \
                                      disable_interrupts(GLOBAL); \
                                      var = val; \
                                      enable_interrupts(GLOBAL); \
                                } while(0);

#endif /* MAIN_H_ */
