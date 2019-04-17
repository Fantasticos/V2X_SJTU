#ifndef _V2X_API_H_
#define _V2X_API_H_

#include <v2x_msg_bsm.h>
#include <v2x_msg_map.h>
#include <v2x_msg_spat.h>
#include <v2x_msg_rsa.h>
#include <v2x_msg_tim.h>
#include <v2x_msg_srm.h>
#include <v2x_msg_ssm.h>
#include <v2x_msg_rtcm.h>

typedef void (*bsm_recv_cb) (v2x_msg_bsm_t* user_bsm);
typedef void (*map_recv_cb) (v2x_msg_map_t* user_map);
typedef void (*spat_recv_cb) (v2x_msg_spat_t* user_spat);
typedef void (*rsa_recv_cb) (v2x_msg_rsa_t* user_rsa);
typedef void (*tim_recv_cb) (v2x_msg_tim_t* user_tim);
typedef void (*srm_recv_cb) (v2x_msg_srm_t* user_srm);
typedef void (*ssm_recv_cb) (v2x_msg_ssm_t* user_ssm);
typedef void (*rtcm_recv_cb) (v2x_msg_rtcm_t* user_rtcm);

int mde_v2x_init(const char* log_cfg_path, const char* moudle_name);

int mde_v2x_register_bsm(bsm_recv_cb cb);

int mde_v2x_send_bsm(v2x_msg_bsm_t* user_bsm);

int mde_v2x_register_map(map_recv_cb cb);

int mde_v2x_send_map(v2x_msg_map_t* user_map);

void mde_v2x_msg_map_free(v2x_msg_map_t* map);

int mde_v2x_register_spat(spat_recv_cb cb);

int mde_v2x_send_spat(v2x_msg_spat_t* spat);

void mde_v2x_msg_spat_free(v2x_msg_spat_t* spat);

int mde_v2x_register_rsa(rsa_recv_cb cb);

int mde_v2x_send_rsa(v2x_msg_rsa_t* rsa);

int mde_v2x_register_tim(tim_recv_cb cb);

int mde_v2x_send_tim(v2x_msg_tim_t* tim);

void mde_v2x_msg_tim_free(v2x_msg_tim_t* tim);

int mde_v2x_register_srm(srm_recv_cb cb);

int mde_v2x_send_srm(v2x_msg_srm_t* srm);

int mde_v2x_register_ssm(ssm_recv_cb cb);

int mde_v2x_send_ssm(v2x_msg_ssm_t* ssm);

int mde_v2x_register_rtcm(rtcm_recv_cb cb);

int mde_v2x_send_rtcm(v2x_msg_rtcm_t* rtcm);

#endif
