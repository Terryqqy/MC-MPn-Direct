//
// Created by aedoran on 6/5/18.
#include <thrust/device_vector.h>
#include "gtest/gtest.h"

#include "stochastic_tau.h"

#include "../../src/basis/movec_parser.h"
#include "../../src/basis/dummy_movec_parser.h"
#include "../../src/basis/wavefunction.h"
#include "../../src/qc_ovps.h"


namespace {
  template <class T>
  class ovpsTest : public testing::Test {
   public:
    void SetUp() override {
      int electron_pairs = 10;
      std::vector<std::array<double, 3>> electron_pair_pos(electron_pairs);
      std::shared_ptr<Movec_Parser> movecs(new Dummy_Movec_Parser());

      psi1 = Wavefunction(&electron_pair_pos, movecs);
      psi2 = Wavefunction(&electron_pair_pos, movecs);

      std::shared_ptr<Stochastic_Tau> stochastic_tau(new Stochastic_Tau(movecs));
      stochastic_tau->resize(2);
      stochastic_tau->set({1.0-1.0/exp(2), 1.0-1.0/exp(2)});

      tau = std::shared_ptr<Tau>(stochastic_tau);

      ovps.init(2, electron_pairs);
    }

    Wavefunction psi1;
    Wavefunction psi2;
    std::shared_ptr<Tau> tau;
    OVPS<T> ovps;
  };

  using Implementations = testing::Types<std::vector<double>>; // , thrust::device_vector<double>>;
  TYPED_TEST_SUITE(ovpsTest, Implementations);

  TYPED_TEST(ovpsTest, update) {
  }
}