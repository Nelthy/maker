#pragma once
#include "ab_link.h"
#include "bin_table.h"
#include "tri_bin_table.h"

bool cull_pcols(std::vector<ab_link> &pcols, const tri_bin_table &phase_links);

bool cull_pcols(std::vector<ab_link> &pcols, const bin_table &phase_links);
