inno-IAR-2013
===========

This is a library developed by the HKUST Robotics team for the Smart Car Competition 2013. 

Compiles in the IAR IDE for the Freescale k60 MCU on a custom made development board.

The main objective of this repository is to combine libraries from three groups and develop basic derivers for Smart Car 2014.

Important Notice
-----------
1.	Final version cannot include any foul language, by modifying Ambi's comments / printf (by Prof. Tim Woo)
2.	All (new) derivers should integrate under …\GitHub\inno-IAR-2013\src\drivers
3. 	main.c, isr.c and related file should integrate under …\GitHub\inno-IAR-2013\src\app\XXXXXXX_app

XXXXXXX = Camera / Magnetic / Light_Sensor

Task List
-----------
**Camera**

1.Derivers / Libraries / Functions 
- camera.c and camera.h (e.g camera_init(); track_detection(); and related ) (Charmy) 
- LCD.c and LCD.h from STM32 to Freescale (Charmy) 

2.main.c, isr.c, isr.h, include.h, common.h for Camera group (Charmy) 
- Integrate under …\GitHub\inno-IAR-2013\src\app\Camera_app
- Final and/or enhanced 

3.Any modified codes and libraries after inno-IAR.zip from Smart Car 2012

**Electromagnetic**

1.Derivers / Libraries / Functions 
- enhanced adc.c (e.g. find_max(); and related) (Ben)

2.main.c, isr.c, isr.h, include.h, common.h for Magnetic group (Ben)
- Integrate under …\GitHub\inno-IAR-2013\src\app\Magnetic_app
- Final and/or enhanced

3.Any modified codes and libraries after inno-IAR.zip from Smart Car 2012

**Light Sensor**

1.Derivers / Libraries / Functions 
- linearccd.c and linearccd.h, LDR (Louis)
- flash.c and flash.h (John)
- Try 200 Line Encoders, encoder.c and encoder.h (e.g. encoder_init();) (John)
- Gyroscope, Kalman filter (Louis and John)

2.main.c, isr.c, isr.h, include.h, common.h for Light Sensor group
- Integrate under …\GitHub\inno-IAR-2013\src\app\Light_Sensor_app
- Final and/or enhanced 

3.Any modified codes and libraries after inno-IAR.zip from Smart Car 2012

**Basic derivers**
- Pin Test (Yumi)

Conventions
-----------
This library attempts to follow the following conventions:

1.If your function is part of a Hardware Ability:
- have it live inside of a library eventually ( i.e `accl.c`, `accl.h`)

2.Prefix all functions with library name:
- i.e. `accl_tilt();`

3.for global variables, affix g_ to the front of the variable, and label the type with f,u8,ch etc.
- i.e. `g_u8ar_ccd_pixel` would mean it is a global variable with type u8 array called pixel, related to the ccd library,
- this allows code to be self documenting and easier to read

4.With the exception of special deployment scenarios, (i.e. pintest), refrain from defining functions inside of main.c, 
- increase readability. 
- try to place them in their own .h & .c files

