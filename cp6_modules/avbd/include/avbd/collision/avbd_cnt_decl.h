#pragma once

template<typename CD1, typename CD2>
struct avbd_pcnt_combo;

template<typename CD1, typename CD2>
using avbd_pcnt_combo_t = typename avbd_pcnt_combo<CD1, CD2>::type;

#define SET_AVBD_PCNT_COMBO(cd1, cd2, combo_t)                                                     \
  template<>                                                                                       \
  struct avbd_pcnt_combo<cd1, cd2> {                                                               \
    using type = combo_t;                                                                          \
  };

template<typename CD1, typename CD2>
struct avbd_tcnt_combo;

template<typename CD1, typename CD2>
using avbd_tcnt_combo_t = typename avbd_tcnt_combo<CD1, CD2>::type;

#define SET_AVBD_TCNT_COMBO(cd1, cd2, combo_t)                                                     \
  template<>                                                                                       \
  struct avbd_tcnt_combo<cd1, cd2> {                                                               \
    using type = combo_t;                                                                          \
  };

template<typename CNT_DATA_T>
struct avbd_q_to_cnt_type;

template<typename CNT_DATA_T>
using avbd_q_to_cnt_t = typename avbd_q_to_cnt_type<CNT_DATA_T>::type;

#define SET_AVBD_Q_TO_CNT_TYPE(cnt_data_t, data_t)                                                 \
  template<>                                                                                       \
  struct avbd_q_to_cnt_type<cnt_data_t> {                                                          \
    using type = data_t;                                                                           \
  };

template<typename QD1, typename QD2, typename CNTS, typename AVBD_CNTS>
bool init_avbd_state(const QD1 &qd1, const QD2 &qd2, const CNTS &cnts, AVBD_CNTS &avbd_cnts);

template<typename QD1, typename QD2, typename AVBD_CNTS>
bool update_avbd_vars(const QD1 &qd1, const QD2 &qd2, AVBD_CNTS &avbd_cnts);

template<typename QD1, typename QD2, typename AVBD_CNTS>
bool add_derivs1(QD1 &qd1, const QD2 &qd2, const AVBD_CNTS &avbd_cnts);

template<typename QD1, typename QD2, typename AVBD_CNTS>
bool add_derivs2(const QD1 &qd1, QD2 &qd2, const AVBD_CNTS &avbd_cnts);

template<typename QD1, typename QD2, typename AVBD_CNTS, typename COLOR_T, typename Q_TO_CNT_T>
bool add_derivs1(
  QD1 &qd1,
  const QD2 &qd2,
  const AVBD_CNTS &avbd_tcnts,
  const COLOR_T &color,
  const Q_TO_CNT_T &q1_to_cnt
);

template<typename QD1, typename QD2, typename AVBD_CNTS, typename COLOR_T, typename Q_TO_CNT_T>
bool add_derivs2(
  const QD1 &qd1,
  QD2 &qd2,
  const AVBD_CNTS &avbd_cnts,
  const COLOR_T &color,
  const Q_TO_CNT_T &q2_to_cnt
);

template<typename AVBD_CNTS_T, typename Q_TO_CNT_T>
bool update_q1_to_cnt(const AVBD_CNTS_T &avbd_cnts, Q_TO_CNT_T &q1_to_cnt, unsigned q1_num);

template<typename AVBD_CNTS_T, typename Q_TO_CNT_T>
bool update_q2_to_cnt(const AVBD_CNTS_T &avbd_cnts, Q_TO_CNT_T &q2_to_cnt, unsigned q2_num);
