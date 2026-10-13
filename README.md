# AUTOSAR-NvM-Manager_-Job-Queue-State-Machine

# Exercise 17: AUTOSAR NvM Manager – Job Queue and State Machine

## 1. Project Overview

This project implements a simplified AUTOSAR-style Non-Volatile Memory (NvM) Manager using an external M24C04 EEPROM and an NXP S32K144 microcontroller.

The application demonstrates how multiple memory-write requests can be queued, processed sequentially through a state machine, and verified by reading the stored data back from the EEPROM.

The project is developed in Embedded C using NXP S32 Design Studio and the S32K1xx SDK.

## 2. Objectives

- Implement a FIFO queue for EEPROM write requests.
- Develop an NvM state machine to manage write operations.
- Interface with an external M24C04 EEPROM using I2C.
- Process multiple write requests sequentially.
- Monitor state transitions and write results through UART.
- Verify EEPROM data using read-back comparison.

## 3. Hardware Requirements

- NXP S32K144 Evaluation Board
- M24C04 external EEPROM
- I2C communication interface
- UART connection for serial monitoring
- USB connection for programming and debugging

## 4. Software Requirements

- NXP S32 Design Studio
- S32K1xx SDK
- Embedded C
- PuTTY or another serial terminal

## 5. System Architecture

The application is organized into the following modules:

| File | Responsibility |
|---|---|
| `main.c` | Initializes peripherals, submits write requests, monitors states, and verifies stored data |
| `nvm.c` | Implements the NvM state machine and coordinates write operations |
| `queue.c` | Implements FIFO enqueue and dequeue operations |
| `eeprom.c` | Implements I2C-based EEPROM read and write operations |
| `nvm.h` | Defines NvM states, data structures, constants, and function declarations |
| `eeprom.h` | Declares EEPROM functions and status types |

## 6. NvM State Machine

The state machine processes each queued write request through the following sequence:

`IDLE → WRITE_PENDING → WRITE_BUSY → DONE → IDLE`

- **IDLE:** Checks for a queued request.
- **WRITE_PENDING:** Starts the EEPROM write operation.
- **WRITE_BUSY:** Waits for the configured EEPROM write-cycle delay.
- **DONE:** Records the job result and returns to IDLE for the next request.

The state machine is advanced by repeated calls to `NvM_MainFunction()`.

## 7. FIFO Job Queue

The FIFO queue stores write requests until the NvM Manager processes them.

Each request contains:
- EEPROM block ID
- Data buffer
- Data length

The queue uses `queueHead`, `queueTail`, and `queueCount` to manage insertion, removal, and the number of queued requests.

The configured queue capacity is eight requests.

## 8. EEPROM Interface

The EEPROM driver communicates with the external M24C04 using the LPI2C peripheral.

The implementation includes:
- EEPROM device-address selection based on the memory address.
- Address and data validation.
- Page-boundary checks for write operations.
- I2C-based write and read operations.
- Write-cycle delay handling.
- Read-back data verification.

The current write-cycle handling uses a fixed delay rather than I2C ACK polling.

## 9. Testing and Verification

The application submits three write requests for EEPROM blocks 1, 2, and 3.

Each block stores four bytes of test data.

After processing the queued requests, the application reads the data back from the EEPROM and compares each byte with the original test data.

UART output reports:
- Successful or failed queue requests
- NvM state transitions
- EEPROM write results
- Read-back values
- PASS or FAIL verification results

## 10. Expected Result

When the hardware connections and peripheral configuration are correct, the application should process the three write requests sequentially and report PASS for each block whose read-back data matches the original test data.

Actual results should be confirmed using the serial output from the hardware.

## 11. Build and Execution

1. Import the project into NXP S32 Design Studio.
2. Verify the clock, pin multiplexing, LPUART, and LPI2C configurations.
3. Build the project.
4. Flash the program onto the S32K144 evaluation board.
5. Open the configured UART COM port using PuTTY.
6. Observe the queue processing, state transitions, and EEPROM verification results.

## 12. Conclusion

This exercise demonstrates FIFO job scheduling, state-machine-based memory management, I2C peripheral communication, and non-volatile data verification in an embedded microcontroller application.

The modular design separates queue management, NvM job processing, and EEPROM hardware access, making the implementation easier to understand, test, and maintain.
