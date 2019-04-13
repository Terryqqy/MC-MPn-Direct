//
// Created by aedoran on 6/8/18.
//

#ifndef MC_MP3_DIRECT_TAU_INTEGRALS_H
#define MC_MP3_DIRECT_TAU_INTEGRALS_H

#include <algorithm>
#include "basis/qc_basis.h"
#include "qc_random.h"

class Tau {
 public:
  virtual void resize(int dimm, Basis& basis) = 0;
  virtual void new_tau(Random& random) = 0;
  virtual std::vector<double> get_exp_tau(int, int) = 0;
  virtual double get_gfn_tau(int, int, int ,int) = 0;
  virtual double get_wgt(int) = 0;
  virtual double get_tau(int) = 0;
 protected:

};

 class Stochastic_Tau : public Tau {
 public:
  Stochastic_Tau() = default;
  Stochastic_Tau(const Stochastic_Tau& tau) {
    throw std::runtime_error("copy constructor for stochastic tau class called");
  }
  ~Stochastic_Tau() = default;

  void resize(int dimm, Basis& basis) override {
    iocc1 = basis.iocc1;
    iocc2 = basis.iocc2;
    ivir1 = basis.ivir1;
    ivir2 = basis.ivir2;

    evals.resize(ivir2);
    std::copy(basis.nw_en, basis.nw_en + ivir2, evals.begin());

    tau.resize(dimm);
    wgt.resize(dimm);

    exp_tau.resize(dimm);
    for (auto &it : exp_tau) {
      it.resize(ivir2);
    }

    scratch.resize(ivir2);
    lambda = 2.0 * (evals[ivir1] - evals[iocc2 - 1]);
  }
  void new_tau(Random& random) override {
    // generate new tau point and weights
    for (auto i = 0; i < tau.size(); i++) {
      double p = random.uniform();
      tau[i] = -log(1.0 - p) / lambda;
      wgt[i] = 1.0 / (lambda * (1.0 - p));

      for (auto im = iocc1; im < iocc2; im++) {
        exp_tau[i][im] = exp(evals[im] * tau[i]);
      }
      for (auto am = ivir1; am < ivir2; am++) {
        exp_tau[i][am] = exp(-evals[am] * tau[i]);
      }
    }
  }
  std::vector<double> get_exp_tau(int stop, int start) override {
    if (start == stop) {
      return exp_tau[start];
    } else {
      std::fill(scratch.begin(), scratch.end(), 1.0);
      for (auto it = start; it <= stop; it++) {
        std::transform(scratch.begin(), scratch.end(), exp_tau[it].begin(), scratch.begin(), std::multiplies<>());
      }
      return scratch;
    }
  }
  double get_gfn_tau(int stop, int start, int offset, int conjugate) override {
    double s(1.0);
    if (start == stop) {
      s = exp_tau[start][iocc2 + offset];
    } else {
      for (int it = start; it <= stop; it++) {
        s *= exp_tau[it][iocc2 + offset];
      }
    }
    if (conjugate && offset < 0) {
      s = 1.0 / s;
    } else if (!conjugate && offset >= 0) {
      s = 1.0 / s;
    }
    return s;
  }
  double get_wgt(int dimm) override {
    return std::accumulate(wgt.begin(), wgt.begin()+dimm, 1.0, std::multiplies<>());
  }
  double get_tau(int index) override {
    return tau[index];
  }

 private:
  int iocc1, iocc2, ivir1, ivir2;
  double lambda;
  std::vector<double> evals;
  std::vector<double> tau;
  std::vector<double> wgt;
  std::vector<double> scratch;
  std::vector<std::vector<double>> exp_tau;
};

/*
void OVPs::init_tau_02(Basis& basis) {
  std::array<double, 21> xx = {
      459.528454529921248195023509,
      0.002176143805986910199912,
      75.647524700428292021570087,
      0.013219203192174486943822,
      27.635855710538834273393149,
      0.036184875564343521592292,
      13.821771900816584022209099,
      0.072349623997261858221464,
      8.124825510985218102177896,
      0.123079566280893559770959,
      5.238489369094648573366158,
      0.190894727380696543894700,
      3.574116946388957050118051,
      0.279789389938773946919781,
      2.529798344872996818111233,
      0.395288423690625334572246,
      1.834438449215696431693345,
      0.545125948721552511244681,
      1.349829280916060136874535,
      0.740834425610734315092998,
      1.000000000000000000000000};

  for (uint it = 0; it < xx.size(); it++) {
    for (int jt = 0; jt < iocc2; jt++) {
      double en = basis.nw_en[jt];
      ovps.t_save_val1[it * ivir2 + jt] = exp(en * xx[it]);
    }
    for (int jt = ivir1; jt < ivir2; jt++) {
      double en = basis.nw_en[jt];
      ovps.t_save_val1[it * ivir2 + jt] = exp(-en * xx[it]);
    }
    for (int jt = 0; jt < numBand; ++jt) {
      double en = basis.nw_en[iocc2 - offBand + jt];
      ovps.tg_save_val1[it * numBand + jt] = exp(en * xx[it]);
      ovps.tgc_save_val1[it * numBand + jt] = exp(-en * xx[it]);
    }
  }
}
void OVPs::init_tau_03(Basis& basis) {
  std::array<double, 21> xx = {
      459.528454529921248195023509,
      0.002176143805986910199912,
      75.647524700428292021570087,
      0.013219203192174486943822,
      27.635855710538834273393149,
      0.036184875564343521592292,
      13.821771900816584022209099,
      0.072349623997261858221464,
      8.124825510985218102177896,
      0.123079566280893559770959,
      5.238489369094648573366158,
      0.190894727380696543894700,
      3.574116946388957050118051,
      0.279789389938773946919781,
      2.529798344872996818111233,
      0.395288423690625334572246,
      1.834438449215696431693345,
      0.545125948721552511244681,
      1.349829280916060136874535,
      0.740834425610734315092998,
      1.000000000000000000000000};

  for (uint it = 0; it < xx.size(); it++) {
    for (int jt = 0; jt < iocc2; jt++) {
      double en = basis.nw_en[jt];
      ovps.t_save_val1[it * ivir2 + jt] = exp(en * xx[it]);
      ovps.t_save_val2[it * ivir2 + jt] = exp(en * xx[it]);
    }
    for (int jt = ivir1; jt < ivir2; jt++) {
      double en = basis.nw_en[jt];
      ovps.t_save_val1[it * ivir2 + jt] = exp(-en * xx[it]);
      ovps.t_save_val2[it * ivir2 + jt] = exp(-en * xx[it]);
    }
    for (int jt = 0; jt < numBand; ++jt) {
      double en = basis.nw_en[iocc2 - offBand + jt];
      ovps.tg_save_val1[it * numBand + jt] = exp(en * xx[it]);
      ovps.tgc_save_val1[it * numBand + jt] = exp(-en * xx[it]);
      ovps.tg_save_val2[it * numBand + jt] = exp(en * xx[it]);
      ovps.tgc_save_val2[it * numBand + jt] = exp(-en * xx[it]);
    }
  }
}
*/

#endif //MC_MP3_DIRECT_TAU_INTEGRALS_H
