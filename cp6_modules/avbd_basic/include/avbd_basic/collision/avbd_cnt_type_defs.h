#pragma once
#include "avbd/collision/avbd_cnt_decl.h"
#include "avbd_basic/collision/basic_q_to_cnt.h"
#include "collision_basic/circle_cd.h"
#include "collision_basic/poly_cd.h"
#include <avbd_basic/collision/avbd_contact_set.h>

SET_AVBD_PCNT_COMBO(cp6::circle_cd, cp6::circle_cd, cp6::avbd_contact_1p_set)
SET_AVBD_PCNT_COMBO(cp6::circle_cd, cp6::poly_cd, cp6::avbd_contact_1p_set)
SET_AVBD_PCNT_COMBO(cp6::poly_cd, cp6::poly_cd, cp6::avbd_contact_2p_set)

SET_AVBD_TCNT_COMBO(cp6::circle_cd, cp6::circle_cd, cp6::avbd_contact_1p_set)
SET_AVBD_TCNT_COMBO(cp6::circle_cd, cp6::poly_cd, cp6::avbd_contact_1p_set)
SET_AVBD_TCNT_COMBO(cp6::poly_cd, cp6::poly_cd, cp6::avbd_contact_2p_set)

SET_AVBD_Q_TO_CNT_TYPE(cp6::avbd_contact_1p_set, basic_q_to_cnt);
SET_AVBD_Q_TO_CNT_TYPE(cp6::avbd_contact_2p_set, basic_q_to_cnt);
