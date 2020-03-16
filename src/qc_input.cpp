// Copyright 2017

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


#include "qc_mpi.h"
#include "qc_input.h"
#include "MCF12/correlation_factors.h"

IOPs::IOPs() {
  /*
   * IOPs constructor
   * initializes variables to mostly rational values
   *
   * TODO
   *  -read values directly instead of setting them
   */
  bopns[KEYS::SPHERICAL] = true;
  bopns[KEYS::F12_GAMMA] = false;
  bopns[KEYS::F12_BETA] = false;

  dopns[KEYS::MC_DELX] = 0.1;

  iopns[KEYS::ELECTRON_PAIRS] = 16;
  iopns[KEYS::ELECTRONS] = 16;
  iopns[KEYS::MC_TRIAL] = 1024;
  iopns[KEYS::OFF_BAND] = 1;
  iopns[KEYS::NUM_BAND] = 1;
  iopns[KEYS::DIFFS] = 1;
  iopns[KEYS::NBLOCK] = 1;
  iopns[KEYS::ORDER] = 2;
  iopns[KEYS::JOBTYPE] = JOBTYPE::ENERGY;
  iopns[KEYS::SAMPLER] = SAMPLER::DIRECT;
  iopns[KEYS::TAU_INTEGRATION] = TAU_INTEGRATION::STOCHASTIC;
  iopns[KEYS::F12_CORRELATION_FACTOR] = CORRELATION_FACTORS::Slater;

  iopns[KEYS::MP2CV_LEVEL] = 0;
  iopns[KEYS::MP3CV_LEVEL] = 0;
  iopns[KEYS::MP4CV_LEVEL] = 0;

  sopns[KEYS::GEOM] = "geom.xyz";
  sopns[KEYS::BASIS] = "basis.dat";
  sopns[KEYS::MC_BASIS] = "mc_basis.dat";
  sopns[KEYS::MOVECS] = "nwchem.movecs";
  iopns[KEYS::MOVECS] = 0;  // default is binary
}

template <class T>
T string_to_enum(std::string str, const std::vector<std::string>& T_vals) {
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
  auto it = std::find(T_vals.begin(), T_vals.end(), str);

  if (it == T_vals.end()) {
    std::cerr << str << " not reconginzed" << std::endl;
    exit(0);
  }

  return static_cast<T>(std::distance(T_vals.begin(), it));
} 

void IOPs::read(const MPI_info& mpi_info, const std::string& file) {
  /*
   * reads and stores options mcin file provided as command line argument
   *
   * Arguments
   *  -MPI_info mpi_info: see qc_mpi.h
   *  -std::string file: path to mcin file
   *
   * TODO
   *  -should probably read input from a json
   *  -needs input validation
   *  -make key setter by type
   *  -write functions to convert strings to enums
   *  -clean up keys
   */
  KEYS::KEYS keyval;

  bool keySet;
  std::string str;
  std::string key;

  if (mpi_info.sys_master) {
    std::ifstream input(file.c_str());
    if (!input.is_open()) {
      std::cerr << "FILE \"" << file << "\" DOES NOT EXIST" << std::endl;
      std::exit(EXIT_FAILURE);
    }

    keySet = false;
    while (getline(input, str)) {
      std::istringstream ss(str);
      while (ss >> key) {
        if (keySet == false) {  // if key not set, determine key value from key_vals arrays
          keyval = string_to_enum<KEYS::KEYS>(key, KEYS::key_strings);
          keySet = true;
        } else {
          switch (keyval) {
            case KEYS::JOBNAME:
              sopns[keyval] = key;
              keySet = false;
              break;
            case KEYS::JOBTYPE:
              iopns[keyval] = string_to_enum<JOBTYPE::JOBTYPE>(key, JOBTYPE::jobtype_strings);
              keySet = false;
              break;
            case KEYS::SPHERICAL:
              bopns[keyval] = stoi(key, nullptr);
              keySet = false;
              break;
            case KEYS::MC_TRIAL:
              iopns[keyval] = stoi(key, nullptr);
              keySet = false;
              break;
            case KEYS::MC_DELX:
              dopns[keyval] = stod(key, nullptr);
              keySet = false;
              break;
            case KEYS::GEOM:
              sopns[keyval] = key;
              keySet = false;
              break;
            case KEYS::BASIS:
              sopns[keyval] = key;
              keySet = false;
              break;
            case KEYS::MC_BASIS:
              sopns[keyval] = key;
              keySet = false;
              break;
            case KEYS::NBLOCK:
              iopns[keyval] = stoi(key, nullptr);
              if (iopns[keyval] > 20) {
                std::cerr << "NBlock must be less than 20" << std::endl;
                exit(EXIT_FAILURE);
              } else if (iopns[keyval] <= 0) {
                iopns[keyval] = 1;
              }
              keySet = false;
              break;
            case KEYS::MOVECS:
              if (key == "ASCII") {
                iopns[keyval] = 1;
              } else if (key == "END") {
                keySet = false;
              } else {
                sopns[keyval] = key;
              }
              break;
            case KEYS::DEBUG:
              iopns[keyval] = stoi(key, nullptr);
              keySet = false;
              break;
            case KEYS::TASK:
              iopns[keyval] |= (1 << string_to_enum<TASK::TASK>(key, TASK::task_strings));
              keySet = false;
              break;
            case KEYS::NUM_BAND:
              iopns[keyval] = stoi(key, nullptr);
              keySet = false;
              break;
            case KEYS::OFF_BAND:
              iopns[keyval] = stoi(key, nullptr);
              keySet = false;
              break;
            case KEYS::DIFFS:
              iopns[keyval] = stoi(key, nullptr) + 1;
              if (iopns[keyval] <= 0) {
                iopns[keyval] = 1;
              }
              keySet = false;
              break;
            case KEYS::ORDER:
              iopns[keyval] = stoi(key, nullptr);
              keySet = false;
              break;
            case KEYS::SAMPLER:
              iopns[keyval] = string_to_enum<SAMPLER::SAMPLER>(key, SAMPLER::sampler_strings);
              keySet = false;
              break;
            case KEYS::TAU_INTEGRATION:
              iopns[keyval] = string_to_enum<TAU_INTEGRATION::TAU_INTEGRATION>(key, TAU_INTEGRATION::tau_integration_strings);
              keySet = false;
              break;
            case KEYS::F12_CORRELATION_FACTOR:
              iopns[keyval] = string_to_correlation_factors(key);
              keySet = false;
              break;
            case KEYS::F12_GAMMA:
              bopns[keyval] = true;
              dopns[keyval] = stod(key, nullptr);
              keySet = false;
              break;
            case KEYS::F12_BETA:
              bopns[keyval] = true;
              dopns[keyval] = stod(key, nullptr);
              keySet = false;
              break;
            case KEYS::ELECTRONS:
              iopns[keyval] = stoi(key, nullptr);
              keySet = false;
              break;
            case KEYS::ELECTRON_PAIRS:
              iopns[keyval] = stoi(key, nullptr);
              keySet = false;
              break;
            case KEYS::MP2CV_LEVEL:
              iopns[keyval] = stoi(key, nullptr);
              keySet = false;
              break;
            case KEYS::MP3CV_LEVEL:
              iopns[keyval] = stoi(key, nullptr);
              keySet = false;
              break;
            case KEYS::MP4CV_LEVEL:
              iopns[keyval] = stoi(key, nullptr);
              keySet = false;
              break;
            default:
              std::cerr << "KEY \"" << key << "\" NOT RECONGNIZED" << std::endl;
              exit(EXIT_FAILURE);
          }
        }
      }
    }
    input.close();
  }

  if (iopns[KEYS::JOBTYPE] == JOBTYPE::GF || iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULL) {
    iopns[KEYS::DIFFS] = 1;
  }

  MPI_info::barrier();
  MPI_info::broadcast_int(iopns.data(), iopns.size());
  MPI_info::broadcast_double(dopns.data(), dopns.size());
  MPI_info::broadcast_char((char*) bopns.data(), bopns.size());
}

void IOPs::print(const MPI_info& mpi_info, const std::string& file) {
  /*
   * prints state of iops
   *
   * Arguments
   *  -MPI_info mpi_info: see qc_mpi.h
   *  -std::string file: path to mcin file used to set iops values
   *
   * TODO
   *  -should probably read input from a json
   */

  if (mpi_info.sys_master) {
    std::cout << std::endl;
    std::cout << "Input read from " << file << std::endl;
    std::cout << "JOBNAME: " << sopns[KEYS::JOBNAME] << std::endl;
    std::cout << " JOBTYPE: " << JOBTYPE::jobtype_strings[iopns[KEYS::JOBTYPE]] << std::endl;
    std::cout << " MC_TRIAL: " << iopns[KEYS::MC_TRIAL] << std::endl;
    std::cout << " Electron Pairs: " << iopns[KEYS::ELECTRON_PAIRS] << std::endl;
    std::cout << " SAMPLER: " << SAMPLER::sampler_strings[iopns[KEYS::SAMPLER]] << std::endl;
    if (iopns[KEYS::SAMPLER] == SAMPLER::METROPOLIS) {
      std::cout << " MC_DELX: " << dopns[KEYS::MC_DELX] << std::endl;
    }
    std::cout << " TAU_INTEGRATION: " << TAU_INTEGRATION::tau_integration_strings[iopns[KEYS::TAU_INTEGRATION]]  << std::endl;
    std::cout << " SPHERICAL: " << bopns[KEYS::SPHERICAL] << std::endl;

    if (iopns[KEYS::DEBUG] == 1) {
      std::cout << " RNG in debug mode" << std::endl;
    }
    if (iopns[KEYS::JOBTYPE] == JOBTYPE::ENERGY) {
      if (iopns[KEYS::TASK] & TASK::MP2) {
        std::cout << " TASK: MP2   CV_LEVEL " << iopns[KEYS::MP2CV_LEVEL] << "\n";
      }
      if (iopns[KEYS::TASK] & TASK::MP3) {
        std::cout << " TASK: MP3   CV_LEVEL " << iopns[KEYS::MP3CV_LEVEL] << "\n";
      }
      if (iopns[KEYS::TASK] & TASK::MP4) {
        std::cout << " TASK: MP4   CV_LEVEL " << iopns[KEYS::MP4CV_LEVEL] << "\n";
      }
    } else if (iopns[KEYS::JOBTYPE] == JOBTYPE::GF || iopns[KEYS::JOBTYPE] == JOBTYPE::GFDIFF || iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULL || iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULLDIFF) {
      std::cout << "\tDIFFS = ";
      if (iopns[KEYS::DIFFS] > 1) {
        std::cout << iopns[KEYS::DIFFS] - 1 << std::endl;
      } else {
        std::cout << "FALSE" << std::endl;
      }
      std::cout << "\tBands: ";

      for (int i = 0; i < iopns[KEYS::NUM_BAND]; i++) {
        if ((i + 1 - iopns[KEYS::OFF_BAND]) < 0) {
          std::cout << "HOMO" << i - iopns[KEYS::OFF_BAND] + 1;
        } else if ((i + 1 - iopns[KEYS::OFF_BAND]) == 0) {
          std::cout << "HOMO-" << i - iopns[KEYS::OFF_BAND] + 1;
        } else {
          std::cout << "LUMO+" << i - iopns[KEYS::OFF_BAND];
        }

        if (i < iopns[KEYS::NUM_BAND] - 1) {
          std::cout << ", ";
        }
      }
    } else if (iopns[KEYS::JOBTYPE] == JOBTYPE::F12VBX) {
      std::cout << "Number of Electrons Walkers = " << iopns[KEYS::ELECTRONS] << "\n";
      std::cout << "Correlation Factor = " << correlation_factors_to_string(static_cast<CORRELATION_FACTORS::CORRELATION_FACTORS>(iopns[KEYS::F12_CORRELATION_FACTOR])) << "\n";

      std::cout << "F12_GAMMA = ";
      if (bopns[KEYS::F12_GAMMA]) {
        std::cout << "default";
      } else {
        std::cout << dopns[KEYS::F12_GAMMA];
      }
      std::cout << "\n";

      std::cout << "F12_BETA = ";
      if (bopns[KEYS::F12_BETA]) {
        std::cout << "default";
      } else {
        std::cout << dopns[KEYS::F12_BETA];
      }
      std::cout << "\n";
    }
    std::cout << std::endl;
  }
}
