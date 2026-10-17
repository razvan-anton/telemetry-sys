# Bare-Metal Telemetry System

A dual-node bare-metal telemetry pipeline. An STM32F103 (ARM Cortex-M3) runs a cooperative, non-preemptive scheduler that samples an MPU-6050 IMU over register-level I2C and streams telemetry via an asynchronous UART driver to an ESP32 running FreeRTOS.

---

## Architecture

The system uses a **decoupled producer-consumer model** across two microcontroller nodes:

* **STM32 Node (Scheduler & Sampling):** A SysTick timer drives a cooperative, non-preemptive scheduler executing three periodic tasks (fastest: 50 ms period). Drivers interface directly with memory-mapped registers without vendor HAL/LL libraries.
* **Asynchronous TX Ring Buffer:** Decouples physical wire transmission speeds from task execution. Tasks write payloads into a lockless SPSC ring buffer in RAM and return immediately. The USART TXE interrupt serializes bytes in the background.
* **I2C Master Driver:** Bare-metal register sequencing for MPU-6050 configuration and burst reads.
* **ESP32 Node (Gateway):** Runs FreeRTOS to ingest raw serial streams via UART and route data across tasks with a thread-safe FreeRTOS queue.

---

## Constraints

* **No Vendor HAL:** Written strictly using CMSIS register definitions and direct memory-mapped access (RCC, GPIO, USART3, I2C1).
* **Deterministic 50 ms Loop Deadline:** All drivers use non-blocking buffers or state machines, keeping per-task CPU execution in the tens of microseconds.
* **Static Memory:** Zero dynamic heap allocation (`malloc` / `pvPortMalloc`). Buffers and queues reside statically in `.bss`.
* **115,200 Baud Wire Timing:** Back-to-back ~87 $\mu\text{s}$ frames with zero inter-frame gap, verified on a logic analyzer.

---

## What's Implemented

* **Interrupt-Driven UART RX/TX:** Replaced polling loops (`while (!(SR & TXE))`) with a lockless SPSC ring buffer (power-of-2 mask indexing). Reclaimed >99% of CPU time previously wasted during bit shifting.
* **I2C Burst Reads:** Direct register sequencing (`SB`, `ADDR`, `TXE`, `BTF`). Pre-arms `NACK` + `STOP` before clearing `ADDR` during single-byte reads to prevent slave overruns.
* **Pin Routing Workaround:** Migrated serial output from USART2 to USART3 (PB10/PB11) to bypass unpopulated solder bridges on Nucleo PA2/PA3.
---

## Future Improvements

* **Data Integrity & Framing:** Replace basic checksum with CRC-16 to catch corrupted telemetry frames.
* **Hardware DMA:** Move UART TX and I2C burst transfers to DMA streams to eliminate per-byte ISR overhead.
* **Watchdog Implementation / Reliability:** Add an Independent Watchdog (IWDG) serviced in the main cooperative loop.
* **Network Layer:** Send telemetry over Wi-Fi via a non-blocking UDP socket on the ESP32 to a Linux listener.
* **ESP32 Frame Parser:** 4-stage state machine (HUNT $\to$ HEADER $\to$ PAYLOAD $\to$ VERIFY) parsing `0xAA 0x55` sync bytes to prevent stream desynchronization if bytes drop.
