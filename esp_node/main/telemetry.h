#ifndef TELEMETRY_H
#define TELEMETRY_H

typedef struct {
    float ax, ay, az;     /* In g (gravity) */
    float gx, gy, gz;     /* In deg/s (DPS) */
    float temp_c;         /* In degrees Celsius */
} mpu_data_t;





#endif