#ifndef ATOMIC_ORBITAL_H_
#define ATOMIC_ORBITAL_H_

#include <array>

#ifdef HAVE_CUDA
#include "cuda_runtime.h"
#define HOSTDEVICE __host__ __device__
#else 
#define HOSTDEVICE
#endif

class Atomic_Orbital {
 public:
  Atomic_Orbital() = default;
  Atomic_Orbital(int contraction_begin,
      int contraction_end,
      int contraction_index,
      int ao_index,
      int angular_momentum,
      bool is_spherical,
      double pos[3]);

  int contraction_begin;
  int contraction_end;
  int contraction_index;
  int ao_index;
  int angular_momentum;
  int is_spherical;
  double pos[3];

  void evaluate_contraction(const std::array<double, 3>&, double*, double*, double*);
  void evaluate_contraction_with_derivative(const std::array<double, 3>&, double*, double*, double*, double*);

  HOSTDEVICE void evaluate_ao(double*, double*, const double[3]);
  HOSTDEVICE void evaluate_ao_dx(double*, double*, double*, const double[3]);
  HOSTDEVICE void evaluate_ao_dy(double*, double*, double*, const double[3]);
  HOSTDEVICE void evaluate_ao_dz(double*, double*, double*, const double[3]);

 private:

  double calculate_r2(const std::array<double, 3>&);
  //HOSTDEVICE
   void evaluate_spherical_d_shell(double*, double*);
  //HOSTDEVICE
   void evaluate_spherical_f_shell(double*, double*);
  //HOSTDEVICE
   void evaluate_spherical_g_shell(double*, double*);

  HOSTDEVICE void evaluate_s(double*, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_p(double*, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_d(double*, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_f(double*, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_g(double*, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_h(double*, const double&, const double&, const double&, const double&);
  //HOSTDEVICE
   void evaluate_spherical_d(double*, const double&, const double&, const double&, const double&);
  //HOSTDEVICE
   void evaluate_spherical_f(double*, const double&, const double&, const double&, const double&);
  //HOSTDEVICE
   void evaluate_spherical_g(double*, const double&, const double&, const double&, const double&);

  HOSTDEVICE void evaluate_s_dx(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_p_dx(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_d_dx(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_f_dx(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_g_dx(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_h_dx(double*, const double&, const double&, const double&, const double&, const double&);
  //HOSTDEVICE
   void evaluate_spherical_d_dx(double*, const double&, const double&, const double&, const double&, const double&);
  //HOSTDEVICE
   void evaluate_spherical_f_dx(double*, const double&, const double&, const double&, const double&, const double&);
  //HOSTDEVICE
   void evaluate_spherical_g_dx(double*, const double&, const double&, const double&, const double&, const double&);

  HOSTDEVICE void evaluate_s_dy(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_p_dy(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_d_dy(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_f_dy(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_g_dy(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_h_dy(double*, const double&, const double&, const double&, const double&, const double&);
  //HOSTDEVICE
   void evaluate_spherical_d_dy(double*, const double&, const double&, const double&, const double&, const double&);
  //HOSTDEVICE
   void evaluate_spherical_f_dy(double*, const double&, const double&, const double&, const double&, const double&);
  //HOSTDEVICE
   void evaluate_spherical_g_dy(double*, const double&, const double&, const double&, const double&, const double&);

  HOSTDEVICE void evaluate_s_dz(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_p_dz(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_d_dz(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_f_dz(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_g_dz(double*, const double&, const double&, const double&, const double&, const double&);
  HOSTDEVICE void evaluate_cartesian_h_dz(double*, const double&, const double&, const double&, const double&, const double&);
  //HOSTDEVICE
   void evaluate_spherical_d_dz(double*, const double&, const double&, const double&, const double&, const double&);
  //HOSTDEVICE
   void evaluate_spherical_f_dz(double*, const double&, const double&, const double&, const double&, const double&);
  //HOSTDEVICE
   void evaluate_spherical_g_dz(double*, const double&, const double&, const double&, const double&, const double&);
};

#undef HOSTDEVICE

#endif  // ATOMIC_ORBITAL_H_