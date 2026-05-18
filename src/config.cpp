#include "config.h"
#include "Motor.h"

// Board v3.5 pin assignment
Motor motorRight(18, 19, 20, -1);
Motor motorLeft (21, 22, 26, -1);

void configureMotors() {
    motorLeft.init();
    motorRight.init();
}
