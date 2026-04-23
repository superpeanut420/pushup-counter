# Real-Time Push-Up Counter (STM32 + Ultrasonic Sensor)

## Overview

This project implements a real-time push-up counter using an STM32 Nucleo-F446RE microcontroller and an HC-SR04 ultrasonic sensor. The system measures the distance between the user's chest and a reference surface to detect push-up motion and automatically count repetitions.

The project demonstrates the integration of embedded systems, sensor-based measurement, and real-time signal processing for fitness tracking applications.

---

## Features

* Real-time push-up detection using distance measurement
* Calibration-based rep counting (user-defined up and down positions)
* Threshold-based motion detection logic
* Audible feedback (buzzer) for completed reps
* Countdown timer for session start
* High score tracking (stored in flash memory)

---

## System Architecture

The system consists of:

* **HC-SR04 Ultrasonic Sensor** → measures distance
* **STM32 Nucleo-F446RE** → processes data and runs logic
* **Push Buttons** → calibration, start/reset, high score
* **Buzzer** → feedback for completed reps
* **Display (optional)** → shows rep count and timer

---

## How It Works

1. The ultrasonic sensor continuously measures the distance between the user and the board.
2. The user calibrates:

   * **Top position** (up)
   * **Bottom position** (down)
3. The system stores these values as thresholds.
4. A push-up is counted when:

   * Distance goes below the lower threshold (down position)
   * Then rises above the upper threshold (up position)
5. A buzzer sounds when a valid rep is detected.

---

## Hardware Setup

### Components

* STM32 Nucleo-F446RE
* HC-SR04 Ultrasonic Sensor
* 4 Push Buttons
* Buzzer
* Breadboard / soldered connections

### Pin Configuration

Configured using STM32CubeMX:

* GPIO pins for buttons
* Timer for ultrasonic echo timing
* Output pin for buzzer

---

## Code Structure

* `main.c`

  * Contains core application logic
  * Handles sensor reading, calibration, and rep counting

* `readUltrasonicSensor()`

  * Triggers ultrasonic pulse
  * Measures echo duration
  * Calculates distance

* Rep detection logic

  * Uses threshold comparison
  * Tracks state (up/down)

---

## Getting Started

1. Open project in STM32CubeIDE
2. Connect STM32 Nucleo-F446RE board
3. Build and flash the program
4. Calibrate:

   * Press button for top position
   * Press button for bottom position
5. Start session and perform push-ups

---

## Limitations

* Requires proper calibration for accurate results
* Sensor readings may fluctuate due to movement or clothing
* Prototype board is not optimized for durability
* Not fully user-friendly in current form

---

## Future Improvements

* Custom PCB for compact design
* Improved filtering for sensor noise
* Better user interface and calibration process
* Profile-based tracking and stored workouts

---

## Demo

Demo video: [*(Click here)*](https://drive.google.com/file/d/1alZVjgEafqNX0wpbjw6lo5poTt14o8Tl/view?usp=sharing)

---

## Author

Ian Luquis-Diaz  
Electrical and Computer Engineering  
University of Utah
