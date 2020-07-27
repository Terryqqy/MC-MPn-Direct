#include <thrust/device_vector.h>
#include "gtest/gtest.h"

#include "../../src/basis/dummy_basis_parser.h"
#include "../../src/basis/basis.h"

namespace {
  std::vector<std::array<double, 3>> create_pos() {
    std::vector<std::array<double, 3>> pos(5);
    for (int i = 0; i < 5; i++) {
      pos[i][0] = i - 2;
      pos[i][1] = i - 2;
      pos[i][2] = i - 2;
    }
    return pos;
  }

  std::vector<double> create_contraction_amplitudes_result() {
    std::vector<double> amp = {
      2.0400914811240242e-14, -3.2300132846920317e-14, 8.3131191032716759e-05, 8.6361291985046149e-03, 3.6157153515072142e-02,
      1.6035067404299949e-12,  6.3630821199921214e-05, 9.4173590343003353e-03, 2.4150463718367900e-02, 1.1277776083265734e-09,
      3.2029654243824013e-04,  8.0272305395771171e-03, 1.3024120378185848e-07, 8.5758954309730960e-04, 6.1750495777277029e-06,
      1.5797071978491031e-04, -2.5010177303378338e-04, 6.2312665009171195e-02, 1.1797284525171936e-01, 9.8275505326898105e-02,
      2.8890484004570741e-03,  9.5980273313993197e-02, 1.2691971129196050e-01, 5.9775960481895568e-02, 1.8854251268197671e-02,
      1.1022150058488693e-01,  6.2479567247585729e-02, 4.5827190181148551e-02, 6.7448902960946050e-02, 5.5244104691114466e-02,
      7.7818733787091530e+00, -5.8858974100643078e+00, 5.6603997097652192e-01, 2.8201336769754959e-01, 1.3714994457481877e-01,
      1.1322210527390615e+01,  1.1007452000846352e+00, 3.0203856019464992e-01, 8.0858731906040621e-02, 4.8210085116246155e+00,
      7.7239183752070772e-01,  1.2382132254845879e-01, 3.2353020987576109e+00, 2.8898370950917501e-01, 1.1468354346860725e+00,
      1.5797071978491031e-04, -2.5010177303378338e-04, 6.2312665009171195e-02, 1.1797284525171936e-01, 9.8275505326898105e-02,
      2.8890484004570741e-03,  9.5980273313993197e-02, 1.2691971129196050e-01, 5.9775960481895568e-02, 1.8854251268197671e-02,
      1.1022150058488693e-01,  6.2479567247585729e-02, 4.5827190181148551e-02, 6.7448902960946050e-02, 5.5244104691114466e-02,
      2.0400914811240242e-14, -3.2300132846920317e-14, 8.3131191032716759e-05, 8.6361291985046149e-03, 3.6157153515072142e-02,
      1.6035067404299949e-12,  6.3630821199921214e-05, 9.4173590343003353e-03, 2.4150463718367900e-02, 1.1277776083265734e-09,
      3.2029654243824013e-04,  8.0272305395771171e-03, 1.3024120378185848e-07, 8.5758954309730960e-04, 6.1750495777277029e-06};
    return amp;
  }

  class HostBasisTest : public testing::Test {
    public:

    HostBasisTest() : basis(5, Dummy_Basis_Parser(true)), pos(create_pos()) { }
   
    Basis<std::vector, std::allocator> basis;
    std::vector<std::array<double, 3>> pos;
  };

  TEST_F(HostBasisTest, BuildContractionTest) {
    basis.build_contractions(pos);
    auto trial_amp = basis.get_contraction_amplitudes();
    auto known_amp = create_contraction_amplitudes_result();
    for (int i = 0, idx = 0; i < 5; i++) {
      for (int j = 0; j < basis.nShells; j++, idx++) {
        ASSERT_FLOAT_EQ(trial_amp[idx], known_amp[idx]) << i << " " << j;
      }
    }
  }

  class DeviceBasisTest : public testing::Test {
    public:

    DeviceBasisTest() : basis(5, Dummy_Basis_Parser(true)), pos(create_pos()) { }
   
    Basis<thrust::device_vector, thrust::device_allocator> basis;
    std::vector<std::array<double, 3>> pos;
  };

  TEST_F(DeviceBasisTest, BuildContractionTest) {
    basis.build_contractions(pos);
    auto trial_amp = basis.get_contraction_amplitudes();
    auto known_amp = create_contraction_amplitudes_result();
    for (int i = 0, idx = 0; i < 5; i++) {
      for (int j = 0; j < basis.nShells; j++, idx++) {
        ASSERT_FLOAT_EQ(trial_amp[idx], known_amp[idx]) << i << " " << j;
      }
    }
  }
}
