#ifndef QC_MONTE_H_
#define QC_MONTE_H_

#include <chrono>
#include <fstream>
#include <string>
#include <vector>

#include <unordered_map>

#include "qc_mpi.h"
#include "qc_input.h"
#include "qc_geom.h"
#include "basis/qc_basis.h"
#include "weight_function.h"

#include "control_variate.h"
#include "electron_pair_list.h"
#include "electron_list.h"
#include "qc_ovps.h"
#include "qc_random.h"
#include "tau_integrals.h"

#include "MCMP/mcmp.h"
#include "MCMP/qc_mcmp2.h"
#include "MCMP/qc_mcmp3.h"
#include "MCMP/qc_mcmp4.h"
#include "MCF12/mp2_f12.h"

#include "MCGF/qc_mcgf.h"
#include "MCGF/qc_mcgf2.h"
#include "MCGF/qc_mcgf3.h"
#include "MCF12/gf2_f12.h"

class GFStats {
 public:
  std::vector<std::vector<double>> qeps;

  GFStats(bool, int, int, int, int, const std::string&, const std::string&);
  GFStats(bool, int, int, int, int, const std::string&, int);
  GFStats(const GFStats& gf) { throw std::runtime_error("Tried to copy GFStats"); }
  GFStats operator=(const GFStats& gf) { exit(0); }

  ~GFStats();

  void blockIt(const int&);
  void reduce();
  void print(const int& step, const double& time_span);

 private:
  std::vector<std::vector<double>> qepsEx1, qepsEx2, qepsAvg, qepsVar;
  std::vector<std::ofstream*> output_streams;
  bool isMaster;
  double tasks;
};

class QC_monte {
 public:
  QC_monte(MPI_info p0, IOPs p1, Molec p2, Basis p3);
  virtual ~QC_monte();
  virtual void monte_energy() = 0;

 protected:
  MPI_info mpi_info;
  IOPs iops;
  Molec molec;
  Basis basis;
  Electron_Pair_GTO_Weight electron_pair_weight;
  Electron_GTO_Weight electron_weight;

  std::unordered_map<int, Wavefunction> wavefunctions;
  std::unordered_map<int, std::vector<int>> wavefunction_groups;

  Random random;
  OVPs ovps;
  
  Electron_Pair_List* electron_pair_list;
  Electron_List* electron_list;
  Tau* tau;

  int nDeriv;
  int numBand, offBand;
  int iocc1, iocc2, ivir1, ivir2;

  virtual void update_wavefunction();
  void move_walkers();
  static void print_mc_head(std::chrono::system_clock::time_point);
  static void print_mc_tail(double, std::chrono::system_clock::time_point);
};

class Energy : public QC_monte {
 public:
  Energy(MPI_info p1, IOPs p2, Molec p3, Basis p4);
  ~Energy();

  void monte_energy() override;

 protected:
  std::vector<MCMP*> energy_functions;
  std::vector<Accumulator*> cv;

  std::vector<double> emp;
  std::vector<std::vector<double>> control;

  void zero_energies();
  virtual void energy();
};

class Dimer : public Energy {
 public:
  Dimer(MPI_info p1, IOPs p2, Molec p3, Basis p4);
  ~Dimer();

 protected:
  Tau* monomer_a_tau;
  Tau* monomer_b_tau;
  std::unordered_map<int, Wavefunction> monomer_a_wavefunctions;
  std::unordered_map<int, Wavefunction> monomer_b_wavefunctions;
  void update_wavefunction() override;
  void energy() override;

  template <class Binary_Op> 
  void local_energy(std::unordered_map<int, Wavefunction>& l_wavefunction, Tau* l_tau, Binary_Op);

  std::vector<double> l_emp;
  std::vector<std::vector<double>> l_control;
};

class GF : public  QC_monte {
 public:
  void monte_energy() override;

 protected:
  GF(MPI_info p1, IOPs p2, Molec p3, Basis p4) : QC_monte(p1, p2, p3, p4) {}

  std::vector<GFStats> qeps;
  virtual void mc_local_energy(const int& step) = 0;
  virtual int full_print(int& step, int checkNum) = 0;
  std::string genFileName(int, int, int, int, int, int);

  void mcgf2_local_energy_core();
  void mcgf2_local_energy(std::vector<std::vector<double>>&);
  void mcgf2_local_energy_diff(std::vector<std::vector<double>>&);
  void mcgf2_local_energy_full(int);
  void mcgf2_local_energy_full_diff(int);

  void mcgf3_local_energy_core();
  void mcgf3_local_energy(std::vector<std::vector<double>>&);
  void mcgf3_local_energy_diff(std::vector<std::vector<double>>&);
  void mcgf3_local_energy_full(int);
  void mcgf3_local_energy_full_diff(int);

  void mc_gf_statistics(int,
                        std::vector<std::vector<double*>>&,
                        std::vector<double*>&,
                        std::vector<std::vector<double*>>&);
  void mc_gf_full_diffs(int band, std::vector<double> m);

  void mc_gf_copy(std::vector<double>&, double*);

  void mc_gf_full_print(int band, int steps, int checkNum, int order,
      std::vector<double*>& d_ex1,
      std::vector<std::vector<double*>>& d_cov);
};

class Diagonal_GF : public GF {
 public:
  Diagonal_GF(MPI_info p1, IOPs p2, Molec p3, Basis p4);
  void monte_energy() override;

 protected:
  std::vector<MCGF*> energy_functions;
  std::vector<GFStats> qeps;
  void mc_local_energy(const int& step);
  int full_print(int& step, int checkNum);
  std::string genFileName(int, int, int, int, int, int);
};

class GPU_GF2 : public GF {
 protected:
  void mc_local_energy(std::vector<std::vector<double>>&, int);

 public:
  GPU_GF2(MPI_info p1, IOPs p2, Molec p3, Basis p4) : GF(p1, p2, p3, p4) {
    ovps.init_02(iops.iopns[KEYS::ELECTRON_PAIRS], iops.iopns[KEYS::NUM_BAND],
                 iops.iopns[KEYS::OFF_BAND], iops.iopns[KEYS::DIFFS],
                 iops.iopns[KEYS::NBLOCK], NWChem_Movec_Parser(iops, mpi_info, molec), (iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULL) || (iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULLDIFF));

    ovps.alloc_02();
  }
  ~GPU_GF2() {
    ovps.free_tau_02();
    ovps.free_02();
  }
  void monte_energy();
};

class GF2 : public GF {
 public:
  GF2(MPI_info p1, IOPs p2, Molec p3, Basis p4) : GF(p1, p2, p3, p4) {
    ovps.init(1, iops.iopns[KEYS::ELECTRON_PAIRS]);
    ovps.init_02(iops.iopns[KEYS::ELECTRON_PAIRS], iops.iopns[KEYS::NUM_BAND],
                 iops.iopns[KEYS::OFF_BAND], iops.iopns[KEYS::DIFFS],
                 iops.iopns[KEYS::NBLOCK], NWChem_Movec_Parser(iops, mpi_info, molec), (iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULL) || (iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULLDIFF));

    ovps.alloc_02();
    tau->resize(2);

    qeps.reserve(1);
    qeps.emplace_back(mpi_info.sys_master, mpi_info.numtasks, numBand, offBand, nDeriv, iops.sopns[KEYS::JOBNAME], 2);
  }
  ~GF2() {
    ovps.free_tau_02();
    ovps.free_02();
  }
 protected:
  void mc_local_energy(const int& step);
  int full_print(int& step, int checkNum);
};

class GPU_GF3 : public GF {
 protected:
  void mc_local_energy(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, int);

 public:
  GPU_GF3(MPI_info p1, IOPs p2, Molec p3, Basis p4) : GF(p1, p2, p3, p4) {
    ovps.init_03(iops.iopns[KEYS::ELECTRON_PAIRS], iops.iopns[KEYS::NUM_BAND],
                 iops.iopns[KEYS::OFF_BAND], iops.iopns[KEYS::DIFFS],
                 iops.iopns[KEYS::NBLOCK], NWChem_Movec_Parser(iops, mpi_info, molec), (iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULL) || (iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULLDIFF));
    ovps.alloc_03();
  }
  ~GPU_GF3() {
    ovps.free_tau_03();
    ovps.free_03();
  }
  void monte_energy();
};

class GF3 : public GF {
 public:
  GF3(MPI_info p1, IOPs p2, Molec p3, Basis p4) : GF(p1, p2, p3, p4) {
    ovps.init(2, iops.iopns[KEYS::ELECTRON_PAIRS]);
    ovps.init_03(iops.iopns[KEYS::ELECTRON_PAIRS], iops.iopns[KEYS::NUM_BAND],
                 iops.iopns[KEYS::OFF_BAND], iops.iopns[KEYS::DIFFS],
                 iops.iopns[KEYS::NBLOCK], NWChem_Movec_Parser(iops, mpi_info, molec), (iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULL) || (iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULLDIFF));
    ovps.alloc_03();
    tau->resize(2);

    qeps.reserve(2);
    qeps.emplace_back(mpi_info.sys_master, mpi_info.numtasks, numBand, offBand, nDeriv, iops.sopns[KEYS::JOBNAME], 2);
    qeps.emplace_back(mpi_info.sys_master, mpi_info.numtasks, numBand, offBand, nDeriv, iops.sopns[KEYS::JOBNAME], 3);
  }
  ~GF3() {
    ovps.free_tau_03();
    ovps.free_03();
  }

 protected:
  void mc_local_energy(const int& step);
  int full_print(int& step, int checkNum);
};
#endif  // QC_MONTE_H_
