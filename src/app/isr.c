#include "common.h"
#include "include.h"
#include "linearccd.h"
#include <inttypes.h>

/*********** system debug mode/flag ************/
int ccd_debug_print_all_message_flag=0;        // 0: off, 1: on
int ccd_print_flag=0;                          // 0: off, 1: on
int ccd_compressed_print_flag=0;               // 0: off, 1: on
/*********** startup PID values ************/
int32_t speed_array[5]                = {300           , 600         , 900          , 1200    , 0};
int32_t balance_kp_array[5]           = {2684746       , 2880000     , 2725000      , 2781250 , 1}; // 1200 speed can try 2977450
int32_t balance_kd_array[5]           = {90160         , 107560      , 119160       , 119614  , 1}; // 1200 speed can try 128100
int32_t balance_offset_array[5]       = {1130          , 1130        , 1200         , 1200    , 1130};
int32_t speed_kp_array[5]             = {350000        , 297000      , 297000       , 297000  , 350000};
int32_t speed_ki_array[5]             = {10000         , 53000       , 53000        , 53000   , 10000};  // mode 0 : 49500, mode 3 : 60000
int32_t turn_kp_array[5]              = {650000        , 650000      , 500000       , 31000   , 650000};  // 愈細 = 遲入灣 ; 愈大 = 早入灣 , speed 600 : 120500 - speed 900 can try : 49850 ~ 50000 - speed 900: 98800 - speed 1200 can try 36825
int32_t turn_kd_array[5]              = {0             , 0           , 18500        , 11250   , 0}; 
int32_t turn_offset_array[5]          = {0             , 850         , 850          , 850     , 0};  // 愈細 = 中心線靠右 ; 愈大 = 中心線靠左  
float atan_multiply_value_array[5]    = {0.01          , 0.01        , 0.00892080279, 0.015015, 0.01};  // speed 900 can try : 0.00858080279 (larger seems better) - speed 1200 can try 0.01
int32_t left_start_length_array[5]    = {110           , 103         , 25           , 25      , 110};    
int32_t right_start_length_array[5]   = {110           , 103         , 25           , 25      , 110};    
int32_t ccd_mid_pos_array[5]          = {121           , 121         , 121          , 121     , 121};
int32_t run_speed_mode = 4;         /*** vaild input : 0 - 4; Refer mode 0: 300 ; mode 1: 600 ; mode 2: 900 ; mode 3: 1200 ; mode 4: 1500***/
int32_t max_available_mode = 3;
int32_t smooth_interval_jump_time = 500;         /*** Variable for setting mode to mode interval time ***/
int32_t stand_and_dont_move_start_time = 6000;   /*** Variable for setting hold time in start area ***/

/*********** initialize balance PID ************/
int32_t balance_kp = 0;
#define balance_kp_out_of 10000
int32_t balance_kd = 0;
#define balance_kd_out_of 10000
int32_t balance_offset = 0;
extern u32 balance_gyro_offset;
extern u32 turn_gyro_offset;
u32 adjust_lock_counter = 0;
int32_t adjust_balance_offset_flag = 0;
/*********** initialize speed PID ************/
int32_t speed_kp = 0;
#define speed_kp_out_of 10000
int32_t speed_ki = 0;
#define speed_ki_out_of 10000
/*********** initialize turn PID ************/
int32_t turn_kp = 0;
#define turn_kp_out_of 10000
int32_t turn_kd = 0;
#define turn_kd_out_of 10000
int32_t turn_offset = 0;
int32_t gyro_turn = 0;
extern int32_t left_start_length;
extern int32_t right_start_length;
extern int32_t ccd_mid_pos;
extern float atan_multiply_value;
/*********** CCD startup variables ************/
volatile int g_int_ccd_si_counter=0;
extern volatile int g_int_ccd_operation_state; 
int pre_set_si_time=3;                         /*** SI active time: e.g. 3 means 3ms * 3 system loop = 9ms ***/
/*********** CCD related sample result & array ************/
extern char g_char_ar_ccd_current_pixel[256];  /*** ccd sample result ***/
/************* Variables for control PID *************/
volatile int32_t control_omg=0, control_tilt=0;
volatile int32_t start_up_control_tilt=0;
volatile int32_t control_tilt_error=0;
volatile int32_t reference_difference=480;

volatile int32_t motor_command_left=0,motor_command_right=0;
volatile int32_t motor_turn_left=0,motor_turn_right=0;
volatile int32_t motor_command_balance=0;
volatile int32_t speed_error=0;
volatile int speed_offset=10; //adjustable value
volatile int leftDir,rightDir=0;
/************* Variables for speed/position PID *************/
volatile int32_t speed_p,speed_i;
volatile int32_t car_speed;
volatile int32_t motor_command_speed=0;
volatile int32_t motor_command_speed_delta=0;
volatile int32_t speed_control_integral=0;
volatile int32_t control_car_speed=0; //adjustable value, increase car speed
/************* Variables for direction PID *************/
extern int32_t current_dir_arc_value_error;
extern int32_t current_dir_error;
volatile int32_t motor_command_turn_delta=0;
extern int32_t current_edge_middle_distance;
int ccd_distance_value_before_upslope=0;
void temp_ccd_output_debug_message_function(); //temporary
/************* Variables for direction PID : all white encoder hold *************/
volatile int32_t encoder_turn_error=0;
/************* Variables for motor *************/
extern volatile int32_t g_u32encoder_lf;
extern volatile int32_t g_u32encoder_rt;
extern volatile int32_t motor_deadzone_left,motor_deadzone_right;
extern u32 balance_centerpoint_set;
volatile u8 motor_pid_counter=0;  //for the motor command loop
/************* Variables for system *************/
int system_mode=0;
u32 system_loop_tick=0;
int start_up_press_flag=0;
int start_up_press_lock_counter=0;
int startup_smooth_counter = 0;
int mode_selection_start_time_end = 1000;  /*** Variable for setting user press time ***/
int end_of_track_wait_flag=0;
int end_of_track_flag=0;
int track_end_time_counter=0;
int system_already_startup=0;

/************* Variables for end track *************/
int complete_lap_time=0;
int pre_set_lap_time=1;
int this_lap_time_is_count_flag=0;
int track_reset_end_time_counter=0;

void PIT0_IRQHandler(void){
  PIT_Flag_Clear(PIT0);       
}

/****** for encoder testing ******/
void PIT1_IRQHandler(void) 
{   DisableInterrupts;
    printf("\n\fg_u32encoder_lf:%ld",g_u32encoder_lf);
    printf("\n\fg_u32encoder_rt:%ld",g_u32encoder_rt);
    
    g_u32encoder_lf=0;
    g_u32encoder_rt=0;
    
    PIT_Flag_Clear(PIT1);
    EnableInterrupts;
}

void encoder_counter(void){
    u8  n=0;
    n=6;
    if(PORTA_ISFR & (1<<n)){
      PORTA_ISFR  |= (1<<n);
        if(GPIO_GET_1bit(PORTC,5)==1){
          g_u32encoder_lf++;
        }else{
          g_u32encoder_lf--;
        }
    } 
    
    n=7;
    if(PORTA_ISFR & (1<<n)){
      PORTA_ISFR  |= (1<<n);
        if(GPIO_GET_1bit(PORTC,4)==0){
          g_u32encoder_rt++; 
        }else{
         g_u32encoder_rt--; 
        }
    }
}

/****** main system control loop, runs every 1ms, each case runs every 3 ms ******/
void pit3_system_loop(void){
  DisableInterrupts;   
  
  switch (system_mode){
    
    /****** Case 0: get ccd values and calculate turning command from ccd ~410us ******/
    case 0:
      
      //gyro_turn=ad_ave(ADC1,AD6b,ADC_12bit,20)-turn_offset;
      
      if( g_int_ccd_si_counter < pre_set_si_time){
        g_int_ccd_si_counter++;
      }else if(g_int_ccd_operation_state == 0){        
        g_int_ccd_si_counter = 0;         
        ccd_trigger_SI();        
        ccd_sampling(g_char_ar_ccd_current_pixel , 1);
        ccd_recongize_left_right_edge_and_return_dir_error(g_char_ar_ccd_current_pixel);        
                
        /****** ccd dubug ******/
        if(ccd_debug_print_all_message_flag == 1){ // print out all ccd message to UART
          output_algorithm_message_to_UART();  
        }
        
        if(ccd_print_flag == 1){                   // print out 250 pixle raw data 
          ccd_print(g_char_ar_ccd_current_pixel);
        }
        
        if(ccd_compressed_print_flag == 1){        // print out compressed ccd message to UART
          ccd_compressed_print(g_char_ar_ccd_current_pixel);
        }
        
      }
      
      if(motor_pid_counter<33){
        // do nth
        }else{
        //motor_command_turn_delta = ((current_dir_arc_value_error)* turn_kp/turn_kp_out_of + gyro_turn * turn_kd/turn_kd_out_of - motor_turn_left)/33;
         motor_command_turn_delta = ((current_dir_error * turn_kp)/turn_kp_out_of - motor_turn_left)/33; 
      }

      motor_turn_left+=motor_command_turn_delta;
      motor_turn_right-=motor_command_turn_delta;
     
    system_mode=1;
    break;
    
    /****** Case 1: get gyro & accl values + balance pid ~140us ******/
    case 1:
        
      /*                                   
      control_tilt=(ad_ave(ADC1,AD6b,ADC_12bit,20)-balance_offset); // offset
      control_omg=ad_ave(ADC1,AD7b,ADC_12bit,20)-1940;
      motor_command_balance= ((control_tilt)*balance_kp/balance_kp_out_of) - ((control_omg)*balance_kd/balance_kd_out_of);
      */
      
      control_tilt=(ad_ave(ADC1,AD6b,ADC_12bit,20)); // current control tilt
      control_tilt_error = (start_up_control_tilt - control_tilt)-reference_difference;
      control_omg=ad_ave(ADC1,AD7b,ADC_12bit,20)-1940;
      motor_command_balance = ((control_tilt_error)*10/100000) - ((control_omg)*balance_kd/balance_kd_out_of);
      
      
      /*
      printf("\n\f current control_tilt:%ld",control_tilt);
      printf("\n\f start_up_control_tilt:%ld",start_up_control_tilt);
      printf("\n\f control_tilt_error:%ld",control_tilt_error);
      printf("\n\f reference_difference:%ld", reference_difference);
      printf("\n\f motor_command_balance:%ld", motor_command_balance);
      printf("\n\f current control_omg:%ld",control_omg);
      printf("\n\f ======================");
     */
      
    system_mode=2;
    break;
    
    /****** Case 2: output motor ~3.8-5us ******/
    case 2:
  
        if(motor_pid_counter<33){ 
          motor_pid_counter++;
        }else{
          motor_pid_counter=0;
          
          /****** stuff here happens every 33*3ms=99ms, used for calculating and capturing encoder motor PID ******/          
          car_speed=g_u32encoder_lf+g_u32encoder_rt;
          encoder_turn_error+=g_u32encoder_rt-g_u32encoder_lf;
                    
         /************ clears current encoder ************/
          g_u32encoder_lf=g_u32encoder_rt=0;   
          
          speed_error = speed_offset + control_car_speed - car_speed; //optimal speed offset ~10
                      
          speed_p=speed_error*(speed_kp/speed_kp_out_of);
          speed_i=speed_error*(speed_ki/speed_ki_out_of); 
          
          speed_control_integral+=speed_i;
          motor_command_speed_delta=((speed_p+speed_control_integral)-motor_command_speed)/33;
        }
    
        motor_command_speed+=motor_command_speed_delta;

        motor_command_left = motor_command_balance*(-1) - motor_command_speed + motor_turn_left;
        motor_command_right = motor_command_balance*(-1) - motor_command_speed + motor_turn_right;
       
        
        /************ set dir pins on both ************/
          if (motor_command_left>0){
            gpio_set(PORTD,7,0);
            leftDir=1;
          }else{
            gpio_set(PORTD,7,1);
            leftDir=-1;
            motor_command_left=motor_command_left*-1;
          }
          
          if(motor_command_right>0){
            gpio_set(PORTD,9,0);
            rightDir=1;
          }else{
            gpio_set(PORTD,9,1);
            rightDir=-1;
            motor_command_right=motor_command_right*-1;
          }
        /************ saturation & timeout protection ************/   
          if(motor_command_left>8000){
            motor_command_left=8000;
          }
          
          if(motor_command_right>8000){
            motor_command_right=8000;
          }
        /************ excute motor pwm with PID ************/  
        if(end_of_track_flag == 0){
          FTM_PWM_Duty(FTM1, CH0, motor_command_left); 
          FTM_PWM_Duty(FTM1, CH1, motor_command_right);          
        } else if (end_of_track_flag == 1){
          FTM_PWM_Duty(FTM1, CH0, 0); 
          FTM_PWM_Duty(FTM1, CH1, 0); 
        }
          
    system_mode=0; /*** back to the top of system loop  ***/
    break;
  }
  /************ system loop case end here ************/
     
  /************ ticks related handling ************/
    system_loop_tick++;
  
  if(system_loop_tick <= 100){
   start_up_control_tilt = (ad_ave(ADC1,AD6b,ADC_12bit,20));
  }
  //printf("\n\f start_up_control_tilt:%ld",start_up_control_tilt);
        
    if ( system_loop_tick < mode_selection_start_time_end){ /*** Manual speed selection time , < 2000ms ***/
          
        if (gpio_get(PORTE, 8) == 0){ // when 3 press
          if(start_up_press_flag == 0){
            run_speed_mode = run_speed_mode + 1;
            if( run_speed_mode > max_available_mode ){
              run_speed_mode = 0;
            }
            start_up_press_flag = 1;
          }
        }
        
        if(start_up_press_flag == 1){ // lock SW button
          start_up_press_lock_counter++; 
        }
        
        if(start_up_press_lock_counter == 200){ // release SW button
          start_up_press_flag = 0;
          start_up_press_lock_counter = 0;
        }
        
         /*** notify manual selection now operation at the beginning 2000ms***/
        if(system_loop_tick % 200 == 0){
          gpio_turn(PORTE,27);  
        }
        
    } else if( system_loop_tick == mode_selection_start_time_end){ /*** inital startup time , 2000ms ***/
 
          /*** set speed to zero ***/
          control_car_speed = 0;
          
          /*** balance ***/
          balance_kp = balance_kp_array[0];      
          balance_kd = balance_kd_array[0];
          balance_offset = balance_offset_array[0];
          
          /*** speed ***/
          speed_kp =  speed_kp_array[0];    
          speed_ki =  speed_ki_array[0];
           
          /*** turn***/
          turn_kp = turn_kp_array[0]; 
          turn_kd = turn_kd_array[0]; 
          turn_offset = turn_offset_array[0];
          
          /*** vehicle respect to track position ***/
          left_start_length = left_start_length_array[0];
          right_start_length = right_start_length_array[0];
          ccd_mid_pos = ccd_mid_pos_array[0];
          atan_multiply_value = atan_multiply_value_array[0];     
    }
    
    /*** adjust balance offset by SW***/
    if( system_loop_tick <= (mode_selection_start_time_end + stand_and_dont_move_start_time - 4000)){ /*** < (2000 + 6000) ms ***/
       if(adjust_lock_counter % 10 == 0){
          adjust_balance_offset_flag = 0;
       }
      
      if(adjust_balance_offset_flag == 0){
        if (gpio_get(PORTE, 9) == 0){        // when 1 press
           balance_offset_array[0] = balance_offset_array[0] - 1;
           balance_offset_array[1] = balance_offset_array[1] - 1;
           balance_offset_array[2] = balance_offset_array[2] - 1;
           balance_offset_array[3] = balance_offset_array[3] - 1;
           adjust_balance_offset_flag = 1;
           adjust_lock_counter = 0;
        } else if (gpio_get(PORTE, 6) == 0){ // when 4 press
           balance_offset_array[0] = balance_offset_array[0] + 1;
           balance_offset_array[1] = balance_offset_array[1] + 1;
           balance_offset_array[2] = balance_offset_array[2] + 1;
           balance_offset_array[3] = balance_offset_array[3] + 1;
           adjust_balance_offset_flag = 1;
           adjust_lock_counter = 0;
        }
        balance_offset = balance_offset_array[0];
      }
      adjust_lock_counter++;
      
      if(system_loop_tick % 50 == 0){
          gpio_turn(PORTE,27);  
      } 
    }
   
   /*** (4000 + 2000 + 100 * mode )ms ***/    
   if( system_loop_tick == (stand_and_dont_move_start_time + mode_selection_start_time_end + (smooth_interval_jump_time * startup_smooth_counter))){ 
     
      gpio_set(PORTE,27,1);  /*** speed and balace offset selection time end ***/
     
       if(startup_smooth_counter <= run_speed_mode){ 
      
          /*** set speed ***/
          control_car_speed = speed_array[startup_smooth_counter];
            
          /*** balance ***/
          balance_kp = balance_kp_array[startup_smooth_counter];      
          balance_kd = balance_kd_array[startup_smooth_counter];
          balance_offset = balance_offset_array[startup_smooth_counter];
            
          /*** speed ***/
          speed_kp =  speed_kp_array[startup_smooth_counter];    
          speed_ki =  speed_ki_array[startup_smooth_counter];
             
          /*** turn***/
          turn_kp = turn_kp_array[startup_smooth_counter]; 
          turn_kd = turn_kd_array[startup_smooth_counter];
          turn_offset = turn_offset_array[startup_smooth_counter];
            
          /*** vehicle respect to track position ***/
          left_start_length = left_start_length_array[startup_smooth_counter];
          right_start_length = right_start_length_array[startup_smooth_counter];
          ccd_mid_pos = ccd_mid_pos_array[startup_smooth_counter];
          atan_multiply_value = atan_multiply_value_array[startup_smooth_counter];
          
          startup_smooth_counter = startup_smooth_counter + 1;
          system_already_startup = 1;
      } 
   }
   
   /*** dynamic update speed depends on current case***/
   if(system_already_startup == 1){
     if( system_loop_tick % 250 == 0){
            /*** set speed ***/
            control_car_speed = speed_array[run_speed_mode];
              
            /*** balance ***/
            balance_kp = balance_kp_array[run_speed_mode];      
            balance_kd = balance_kd_array[run_speed_mode];
            balance_offset = balance_offset_array[run_speed_mode];
              
            /*** speed ***/
            speed_kp =  speed_kp_array[run_speed_mode];    
            speed_ki =  speed_ki_array[run_speed_mode];
               
            /*** turn***/
            turn_kp = turn_kp_array[run_speed_mode]; 
            turn_kd = turn_kd_array[run_speed_mode];
            turn_offset = turn_offset_array[run_speed_mode];
              
            /*** vehicle respect to track position ***/
            left_start_length = left_start_length_array[run_speed_mode];
            right_start_length = right_start_length_array[run_speed_mode];
            ccd_mid_pos = ccd_mid_pos_array[run_speed_mode];
            atan_multiply_value = atan_multiply_value_array[run_speed_mode];
     }
   }
        /*** mode notification by LED***/
        if(run_speed_mode == 0){
          gpio_set(PORTE,24,1);
          gpio_set(PORTE,25,1);
          gpio_set(PORTE,26,0);
        } else if (run_speed_mode == 1){
          gpio_set(PORTE,24,0);
          gpio_set(PORTE,25,1);
          gpio_set(PORTE,26,1);
        } else if (run_speed_mode == 2){
          gpio_set(PORTE,24,1);
          gpio_set(PORTE,25,0);
          gpio_set(PORTE,26,1);          
        } else if (run_speed_mode == 3){
          gpio_set(PORTE,24,0);
          gpio_set(PORTE,25,1);
          gpio_set(PORTE,26,0);          
        } 
   
    /*** 20000ms trigger end track detection by light sensor ***/
    if( system_loop_tick >= (mode_selection_start_time_end+18000)){ 
      if(gpio_get(PORTB, 21) == 1 && gpio_get(PORTB, 22) == 0 && gpio_get(PORTB, 23) == 1){
          end_of_track_wait_flag = 1;
      }
    }
    
    if(end_of_track_wait_flag == 1){
      if(this_lap_time_is_count_flag == 0){
      complete_lap_time = complete_lap_time + 1;
      this_lap_time_is_count_flag = 1;
      }
      track_reset_end_time_counter++;
    }
    
    if(track_reset_end_time_counter == 2000){
      end_of_track_wait_flag = 0;
      this_lap_time_is_count_flag = 0;
    }
    
    if(complete_lap_time == pre_set_lap_time){
      track_end_time_counter++;
    }
    
    if (track_end_time_counter == 1000){
       //end_of_track_flag = 1;
      run_speed_mode = 0; 
    }
    
     if (track_end_time_counter == 2000){
      run_speed_mode = 4; 
    }
    
    PIT_Flag_Clear(PIT3);
    EnableInterrupts;
}