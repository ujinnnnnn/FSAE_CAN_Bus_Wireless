#ifndef DBC_H
#define DBC_H


#define Inverter_Message_1_ID 0x43
#define Inverter_Message_2_ID 0x143
#define Inverter_Message_3_ID 0x243
#define Inverter_Message_4_ID 0x343
#define Inverter_Message_5_ID 0x443

#define BMS_Message_1_ID 0x123
#define BMS_Message_2_ID 0x124
#define BMS_Message_3_ID 0x1806E5F4
#define BMS_Message_4_ID 0x1806E7F4
#define BMS_Message_5_ID 0x1806E9F4

#define BMS_Cell_Broadcast_ID 0x36

#define TEM_Message_1_ID 0x18EEFF80 // J1939 Address claim
#define TEM_Message_2_ID 0x1839F380 // Thermistor -> BMS broadcast
#define TEM_Message_3_ID 0x1838F380 // Thermistor General Broadcast

#define PLEX_Message_1_ID 0x69
#define PLEX_Message_2_ID 0x70
#define PLEX_Message_3_ID 0x71

#define TEM_MAX_THERMISTORS 80

#endif // dbc.h