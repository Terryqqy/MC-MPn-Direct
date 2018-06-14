//
// Created by aedoran on 6/1/18.
//
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

#ifdef HAVE_MPII
#include "mpi.h"
#endif

#include "../qc_monte.h"

void GF::energy() {}

void GF2::monte_energy() {
  int checkNum = 1;
  int print_mat;

  //clock
  std::chrono::high_resolution_clock::time_point step_start, step_end, mc_start, mc_end;
  std::chrono::duration<double> time_span;

  //start clock and print out time
  if (mpi_info.sys_master) {
    step_start = std::chrono::high_resolution_clock::now();
    mc_start = std::chrono::high_resolution_clock::now();
    print_mc_head(mc_start);
  }

  mc_local_energy(qeps[0].qeps, 0);
  for (auto step = 1; step <= iops.iopns[KEYS::MC_TRIAL]; step++) {
    move_walkers();

    //calculate energies
    mc_local_energy(qeps[0].qeps, step);
    for (auto order = 0; order < qeps.size(); order++) {
      qeps[order].blockIt(step);
    }

//print every 128 steps
    if (0 == step % 128) {
      //Reduce variables across all threads
#ifdef HAVE_MPI
      MPI_Barrier(MPI_COMM_WORLD);
#endif
      for (auto order = 0; order < qeps.size(); order++) {
        qeps[order].reduce();
      }

      if (mpi_info.sys_master) {  //print out results
        step_end = std::chrono::high_resolution_clock::now();
        time_span = std::chrono::duration_cast<std::chrono::duration<double>>(step_end - step_start);
        step_start = std::chrono::high_resolution_clock::now();
        for (auto order = 0; order < qeps.size(); order++) {
          qeps[order].print(step, time_span.count());
        }
      }
    }

    if ((iops.iopns[KEYS::TASK] == TASKS::GFFULL) || (iops.iopns[KEYS::TASK] == TASKS::GFFULLDIFF)) {
      mc_end = std::chrono::high_resolution_clock::now();
      time_span = std::chrono::duration_cast<std::chrono::duration<double>>(mc_end - mc_start);
      print_mat = (time_span.count() > checkNum * 900);
#ifdef UES_MPI
      MPI_Barrier(MPI_COMM_WORLD);
      MPI_Bcast(&print_mat, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
      if (print_mat) {
        for (auto band = 0; band < numBand; band++) {
          mc_gf2_full_print(band, step, checkNum % 2);
        }
        checkNum++;
#ifdef HAVE_MPI
        MPI_Barrier(MPI_COMM_WORLD);
#endif
      }
    }
  }

  if (iops.iopns[KEYS::TASK] == TASKS::GFFULL || iops.iopns[KEYS::TASK] == TASKS::GFFULLDIFF) {
    for (auto band = 0; band < numBand; band++) {
      mc_gf2_full_print(band, iops.iopns[KEYS::MC_TRIAL], 2);
      std::cout.flush();
    }
#ifdef HAVE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif
  }

  //close streams and print out time
  if (mpi_info.sys_master) {
    mc_end = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(mc_end - mc_start);
    print_mc_tail(time_span.count(), mc_end);
  }
}

void GF2::mc_local_energy(std::vector<std::vector<double>> &qeps2, int step) {
  update_wavefunction();

  if (iops.iopns[KEYS::TASK] == TASKS::GF || iops.iopns[KEYS::TASK] == TASKS::GFDIFF) {
    for (auto &it : qeps2) {
      std::fill(it.begin(), it.end(), 0.00);
    }
  } else if (iops.iopns[KEYS::TASK] == TASKS::GFFULL || iops.iopns[KEYS::TASK] == TASKS::GFFULLDIFF) {
    ovps.zero_energy_arrays_02();
  }

  tau.new_tau(random);
  ovps.update_ovps_02(el_pair_list.data(), tau);
  mcgf2_local_energy_core();
  for (int band = 0; band < numBand; band++) {
    if (iops.iopns[KEYS::TASK] == TASKS::GF) {
      mcgf2_local_energy(qeps2[band], band);
    } else if (iops.iopns[KEYS::TASK] == TASKS::GFDIFF) {
      mcgf2_local_energy_diff(qeps2[band], band);
    } else if (iops.iopns[KEYS::TASK] == TASKS::GFFULL) {
      mcgf2_local_energy_full(band);
    } else if (iops.iopns[KEYS::TASK] == TASKS::GFFULLDIFF) {
      mcgf2_local_energy_full_diff(band);
      if (step > 0) {
        mc_gf2_statistics(band, step);
      }
    }
  }

  if (step > 0 && (iops.iopns[KEYS::TASK] == TASKS::GFFULLDIFF || iops.iopns[KEYS::TASK] == TASKS::GFFULL)) {
    mc_gf_statistics(step, qeps2, ovps.d_ovps.en2, ovps.d_ovps.en2Ex1, ovps.d_ovps.en2Ex2);
  }
}

void GF3::monte_energy() {
  int checkNum = 1;
  int i, band, print_mat;

  //clock
  std::chrono::high_resolution_clock::time_point step_start, step_end, mc_start, mc_end;
  std::chrono::duration<double> time_span;

  //start clock and print out time
  if (mpi_info.sys_master) {
    step_start = std::chrono::high_resolution_clock::now();
    mc_start = std::chrono::high_resolution_clock::now();
    print_mc_head(mc_start);
  }

  mc_local_energy(qeps[0].qeps, qeps[1].qeps, 0);
  for (i = 1; i <= iops.iopns[KEYS::MC_TRIAL]; i++) {
    move_walkers();

    //calculate energies
    mc_local_energy(qeps[0].qeps, qeps[1].qeps, i);
    qeps[0].blockIt(i);
    qeps[1].blockIt(i);

//print every 128 steps
    if (0 == i % 128) {
      //Reduce variables across all threads
#ifdef HAVE_MPI
      MPI_Barrier(MPI_COMM_WORLD);
#endif
      qeps[0].reduce();
      qeps[1].reduce();

      if (mpi_info.sys_master) {  //print out results
        step_end = std::chrono::high_resolution_clock::now();
        time_span = std::chrono::duration_cast<std::chrono::duration<double>>(step_end - step_start);
        step_start = std::chrono::high_resolution_clock::now();
        qeps[0].print(i, time_span.count());
        qeps[1].print(i, time_span.count());
      }
    }

    if ((iops.iopns[KEYS::TASK] == TASKS::GFFULL) || (iops.iopns[KEYS::TASK] == TASKS::GFFULLDIFF)) {
      mc_end = std::chrono::high_resolution_clock::now();
      time_span = std::chrono::duration_cast<std::chrono::duration<double>>(mc_end - mc_start);
      print_mat = (time_span.count() > checkNum * 900);
#ifdef HAVE_MPI
      MPI_Barrier(MPI_COMM_WORLD);
      MPI_Bcast(&print_mat, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
      if (print_mat) {
        for (band = 0; band < numBand; band++) {
          mc_gf2_full_print(band, i, checkNum % 2);
          mc_gf3_full_print(band, i, checkNum % 2);
        }
        checkNum++;
#ifdef HAVE_MPI
        MPI_Barrier(MPI_COMM_WORLD);
#endif
      }
    }
  }

  if (iops.iopns[KEYS::TASK] == TASKS::GFFULL || iops.iopns[KEYS::TASK] == TASKS::GFFULLDIFF) {
    for (i = 0; i < numBand; i++) {
      mc_gf2_full_print(i, iops.iopns[KEYS::MC_TRIAL], 2);
      std::cout.flush();
      mc_gf3_full_print(i, iops.iopns[KEYS::MC_TRIAL], 2);
      std::cout.flush();
    }
#ifdef HAVE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif
  }

  //close streams and print out time
  if (mpi_info.sys_master) {
    mc_end = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(mc_end - mc_start);
    print_mc_tail(time_span.count(), mc_end);
  }
}

void GF3::mc_local_energy(std::vector<std::vector<double>> &qeps2, std::vector<std::vector<double>> &qeps3, int step) {
  update_wavefunction();

  if (iops.iopns[KEYS::TASK] == TASKS::GF || iops.iopns[KEYS::TASK] == TASKS::GFDIFF) {
    for (auto &it : qeps2) {
      std::fill(it.begin(), it.end(), 0.00);
    }
    for (auto &it : qeps3) {
      std::fill(it.begin(), it.end(), 0.00);
    }
  } else if (iops.iopns[KEYS::TASK] == TASKS::GFFULL || iops.iopns[KEYS::TASK] == TASKS::GFFULLDIFF) {
    ovps.zero_energy_arrays_03();
  }

  tau.new_tau(random);

  ovps.update_ovps_03(el_pair_list.data(), tau);
  mcgf2_local_energy_core();
  mcgf3_local_energy_core();

  for (int band = 0; band < numBand; band++) {
    if (iops.iopns[KEYS::TASK] == TASKS::GF) {
      mcgf2_local_energy(qeps2[band], band);
    } else if (iops.iopns[KEYS::TASK] == TASKS::GFDIFF) {
      mcgf2_local_energy_diff(qeps2[band], band);
    } else if (iops.iopns[KEYS::TASK] == TASKS::GFFULL) {
      mcgf2_local_energy_full(band);
    } else if (iops.iopns[KEYS::TASK] == TASKS::GFFULLDIFF) {
      mcgf2_local_energy_full_diff(band);
      if (step > 0) {
        mc_gf2_statistics(band, step);
      }
    }

    if (iops.iopns[KEYS::TASK] == TASKS::GF) {
      mcgf3_local_energy(qeps3[band], band);
    } else if (iops.iopns[KEYS::TASK] == TASKS::GFDIFF) {
      mcgf3_local_energy_diff(qeps3[band], band);
    } else if (iops.iopns[KEYS::TASK] == TASKS::GFFULL) {
      mcgf3_local_energy_full(band);
    } else if (iops.iopns[KEYS::TASK] == TASKS::GFFULLDIFF) {
      mcgf3_local_energy_full_diff(band);
      if (step > 0) {
        mc_gf3_statistics(band, step);
      }
    }
  }

  if (step > 0 && (iops.iopns[KEYS::TASK] == TASKS::GFFULLDIFF || iops.iopns[KEYS::TASK] == TASKS::GFFULL)) {
    mc_gf_statistics(step, qeps2, ovps.d_ovps.en2, ovps.d_ovps.en2Ex1, ovps.d_ovps.en2Ex2);
    mc_gf_statistics(step, qeps3, ovps.d_ovps.en3, ovps.d_ovps.en3Ex1, ovps.d_ovps.en3Ex2);
  }
}
