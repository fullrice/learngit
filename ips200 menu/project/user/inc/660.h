#ifndef _660_H
#define _660_H
typedef struct
{
   volatile float P_DIRE;
	 volatile float AD_DIRE;
	 volatile float D_DIRE;
   volatile float steer_output;
   volatile  int open;


}pdd;

extern pdd my_pdd;
void menu_load(void);
void PDD_location(float offset);
void Camera_pdd_show();
//函数
void sub_key_action(void);
void pdd_sub_menu_main_page(void);
void pdd_ad_dire_adjust(void);
void pdd_d_dire_adjust(void);
void pdd_p_dire_adjust(void);
void front_adjust(void);
void pdd_sub_menu_main(void);
void start_pdd_sub_menu(void);
#endif