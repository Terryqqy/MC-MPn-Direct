#ifndef MCMP_H_
#define MCMP_H_

#include "../qc_monte.h"

#include "mp_functional.h"
#include "mp2_functional.h"
#include "mp3_functional.h"
#include "mp4_functional.h"
#include "../MCF12/mp2_f12.h"

template <class Container>
class MCMP : public QC_monte<std::vector<double>> {
 public:
  MCMP(MPI_info p1, IOPs p2, Molecule p3, Basis_Host p4);
  ~MCMP();

  void monte_energy() override;

 protected:
  std::vector<MP_Functional*> energy_functions;
  std::vector<Accumulator*> cv;

  std::vector<double> emp;
  std::vector<std::vector<double>> control;

  void zero_energies();
  virtual void energy();
};
template class MCMP<std::vector<double>>;

#ifdef HAVE_CUDA
class GPU_MCMP : public MCMP<std::vector<double>> {
 public:
  GPU_MCMP(MPI_info p1, IOPs p2, Molecule p3, Basis_Host p4);

 protected:
  OVPS_Device ovps_device;
  void energy() override;
};
template class MCMP<thrust::device_vector<double>>;
#endif

class Dimer : public MCMP<std::vector<double>> {
 public:
  Dimer(MPI_info p1, IOPs p2, Molecule p3, Basis_Host p4);
  ~Dimer();

 protected:
  Tau* monomer_a_tau;
  Tau* monomer_b_tau;
  std::unordered_map<int, Wavefunction_Type> monomer_a_wavefunctions;
  std::unordered_map<int, Wavefunction_Type> monomer_b_wavefunctions;
  void update_wavefunction() override;
  void energy() override;

  template <class Binary_Op> 
  void local_energy(std::unordered_map<int, Wavefunction_Type>& l_wavefunction, Tau* l_tau, Binary_Op);

  std::vector<double> l_emp;
  std::vector<std::vector<double>> l_control;
};

#endif  // MCMP_H_
