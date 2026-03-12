# MoVIER

This repository contains the software stack for **MoVIER** **R1** and **R2** base module capable of autonomous mapping and navigation. The project implements sensor fusion, motor control, and SLAM using extended Kalman filter.

---

## 📖 Table of Contents
* [Project Overview](#-project-overview)
* [System Architecture](#-system-architecture)
* [Design](#-design)
* [SLAM Testing](#-slam-testing)
* [Benchmarks](#-benchmarks)
* [The Team](#-the-team)

---

## 🚀 Project Overview
The robot utilizes a two-wheeled differential steering system with 2 passive casters for stability. By processing data from an onboard LIDAR, IMU and wheel encoders, the robot constructs a real-time map of its environment while simultaneously tracking its own position.
* **Differential Drive Control:** Precise velocity commands ($v$, $\omega$) translated to left/right motor PWM.
* **SLAM Integration:** Compatible with Cartographer for high-accuracy 2D mapping.
* **Navigation Stack:** Autonomous path planning and obstacle avoidance via the ROS Navigation Stack (Nav2).
* **Simulation Support:** Includes Gazebo and RViz configurations for testing in virtual environments.

---

## 🏗 System Architecture

* **Middleware:** ROS2 Humble
* **SLAM:** Cartographer
* **Navigation:** Nav2 (Navigation 2 Stack)
* **Simulation:** Gazebo & RViz2
* **Sensors:** YD G2 Lidar, BNO085 IMU, Wheel encoders

---

## 🤖 Design

### CAD Models

<table border="0">
  <tr>
    <td align="center">
      <img src="./documentation/design/base.png" width="100%" />
      <br />
      <b>MoVIER Base Module</b>
    </td>
    <td align="center">
      <img src="./documentation/design/bot.png" width="100%" />
      <br />
      <b>MoVIER R1 Bot</b>
    </td>
  </tr>
</table>

### Prototypes

* **ProtoV1**
<img src="./documentation/ProtoV1.jpeg" alt="ProtoV1" width="300"/>

* **ProtoV2**
<img src="./documentation/ProtoV2.jpeg" alt="ProtoV2" width="300"/>

---

## 🧭 SLAM Testing

<table border="0">
  <tr>
    <td align="center">
      <img src="./documentation/slam/mapping.gif" width="100%" />
      <br />
      <b>Mapping</b>
    </td>
    <td align="center">
      <img src="./documentation/slam/localization.gif" width="100%" />
      <br />
      <b>Localization</b>
    </td>
  </tr>
</table>

---

## 🛠 Benchmarks

* **PID Algorithms Comparison**
<img src="./documentation/benchmarks/PID_algorithms.jpeg" width="500"/>

* **Absolute Position Error**
<p align="center">
  <img src="./documentation/benchmarks/APE_wrt_ground_truth.jpeg" alt="APE_wrt_ground_truth" width="45%" />
  <img src="./documentation/benchmarks/APE_wrt_time.jpeg" alt="APE_wrt_time" width="45%" />
</p>

* **Cartographer Algorithms Comparison**
<img src="./documentation/benchmarks/Cartographer_Optimization_Algorithms.jpeg" width="500"/>

* **Path Tracking Over Continuous Laps**
<img src="./documentation/benchmarks/state_estimation.jpeg" width="500"/>

---

## 👥 The Team
Developed with passion for the **India Innovates 2026** National Innovation Summit at Bharat Mandapam, New Delhi.

* **Divyanshu Pandey**
    * *Mail:* [Link](mailto:dnokia3310@gmail.com)
    * *LinkedIn:* [Link](https://www.linkedin.com/in/divyanshu006/)
* **Rahul Rajak**
    * *Mail:* [Link](rahulrajak5629@gmail.com)
    * *LinkedIn:* [Link](https://www.linkedin.com/in/rahul-rajak-964638261/)
* **Shubh Khandelwal**
    * *Mail:* [Link](mailto:shubh4664@gmail.com)
    * *LinkedIn:* [Link](https://www.linkedin.com/in/shubh--khandelwal/)
* **Ayush Kumar**
    * *Mail:* [Link](mailto:ayushle6@gmail.com)
    * *LinkedIn:* [Link](https://www.linkedin.com/in/ayush-kumar-a44632283/)
* **Raj Rajeshwar Gupta**
    * *Mail:* [Link](mailto:rajrajeshwargupta745@gmail.com)
    * *LinkedIn:* [Link](https://www.linkedin.com/in/raj-rajeshwer-gupta-15511a220/)