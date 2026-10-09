#ifndef TELEMETRY_H
#define TELEMETRY_H

typedef struct {
    float ax, ay, az;     /* In g (gravity) */
    float gx, gy, gz;     /* In deg/s (DPS) */
    float temp_c;         /* In degrees Celsius */
} mpu_data_t;
_Static_assert(sizeof(mpu_data_t) == 28, "Packet size mismatch");
//extra check in case struct is compiled differently





#endif