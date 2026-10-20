#pragma once
#include "ab_link.h"
#include "bin_table.h"
#include "collision/collision_combo_decl.h"
#include "collision_basic/circle_cd.h"
#include "collision_basic/contact_set.h"
#include "collision_basic/poly_cd.h"
#include "tri_bin_table.h"


SET_TCNT_COMBO(cp6::circle_cd, cp6::circle_cd, cp6::contact_1p_set)
SET_TCNT_COMBO(cp6::circle_cd, cp6::poly_cd, cp6::contact_1p_set)
SET_TCNT_COMBO(cp6::poly_cd, cp6::poly_cd, cp6::contact_2p_set)

SET_PCNT_COMBO(cp6::circle_cd, cp6::circle_cd, cp6::contact_1p_set)
SET_PCNT_COMBO(cp6::circle_cd, cp6::poly_cd, cp6::contact_1p_set)
SET_PCNT_COMBO(cp6::poly_cd, cp6::poly_cd, cp6::contact_2p_set)

SET_PHASE_LINKS_COMBO(cp6::circle_cd, cp6::circle_cd, bin_table)
SET_PHASE_LINKS_COMBO(cp6::circle_cd, cp6::poly_cd, bin_table)
SET_PHASE_LINKS_COMBO(cp6::poly_cd, cp6::poly_cd, bin_table)
SET_PHASE_LINKS_SELF(cp6::circle_cd, tri_bin_table);
SET_PHASE_LINKS_SELF(cp6::poly_cd, tri_bin_table);

struct sap_pcols;
SET_PCOL_COMBO(cp6::circle_cd, cp6::circle_cd, std::vector<ab_link>);
SET_PCOL_COMBO(cp6::circle_cd, cp6::poly_cd, std::vector<ab_link>);
SET_PCOL_COMBO(cp6::poly_cd, cp6::poly_cd, sap_pcols);
