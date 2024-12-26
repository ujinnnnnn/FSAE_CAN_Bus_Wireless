# FSAE CAN-BUS Wireless Module front-end

## Background
In an FSAE EV car, devices communicate and broadcast important information through the CAN bus. Typically a user PC can monitor the communication and extract critical information about the car using a wired connection, this project aims to implement a wireless system that parses information from the CAN bus and transmit them wirelessly to the user PC for monitoring.


## Setup 
1. E32-TTL-100
   - UART --> LoRa module
2. Raspberry Pi Zero
   - plan to use zero but current developed using Pi 5
   - Python 3.11.2
