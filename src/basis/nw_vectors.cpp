#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "nw_vectors.h"
#include "qc_basis.h"

NWChem_Movec_Parser::NWChem_Movec_Parser(IOPs& iops, MPI_info& mpi_info, Molec& molec, KEYS::KEYS source) {
  if (mpi_info.sys_master) {
    if (0 == iops.iopns[source]) {
      parse_binary_movecs(iops.sopns[source]);
    } else {
      parse_binary_movecs(iops.sopns[source]);
    }

    std::cout << "nw_vectors: nbf " << n_basis_functions << "\n";
    std::cout << "nw_vectors: nmo " << n_molecular_orbitals << "\n";
    n_occupied_orbitals = std::count_if(occupancy.begin(), occupancy.end(), [](double x){return x > 0.0;});
  }

  broadcast();


  //orbital_check();
  iocc1 = 0;
  if (iops.bopns[KEYS::FREEZE_CORE]) {
    for (auto &atom : molec.atoms) {
      if (atom.znum > 3 && atom.znum < 10 && !atom.is_ghost) {
        iocc1 += 1;
      }
    }
  }
  iocc2 = n_occupied_orbitals;
  ivir1 = n_occupied_orbitals;
  ivir2 = n_molecular_orbitals;

  std::cout << iocc1 << "\t";
  std::cout << iocc2 << "\t";
  std::cout << ivir1 << "\t";
  std::cout << ivir2 << "\n";

  if (mpi_info.sys_master) {
    if (iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GF ||
        iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFDIFF ||
        iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULL ||
        iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULLDIFF) {
      log_orbital_energies(iops.sopns[KEYS::JOBNAME]);
    }
  }
}

void verfiy(int qc_nbf, int nw_nbf) {
  if (qc_nbf != nw_nbf) {
    std::cerr << "You might use the different basis sets or geometry" << std::endl;
    exit(EXIT_FAILURE);
  }
}

void NWChem_Movec_Parser::read(std::ifstream& input, char* v, bool set_null=false) {
  int size;
  input.read((char*)&size, 4);
  input.read(v, size);
  if (set_null) {
    v[size] = '\0';
  }
  input.read((char*)&size, 4);
}

void NWChem_Movec_Parser::log_orbital_energies(std::string jobname) {
  std::ofstream output(jobname + ".orbital_energies");

  // print out data
  output << "# File contains orbitral eigenvalue energies\n";
  output << "# Indexing is 0 based.\n";
  output << "# Last occupied orbitals is " << iocc1 - 1 << "\n";
  for (auto it = 0; it < ivir2; it++) {
    output << orbital_energies[it] << std::endl;
  }
}

void NWChem_Movec_Parser::broadcast() {
  MPI_info::barrier();
  MPI_info::broadcast_int(&n_basis_functions, 1);
  MPI_info::broadcast_int(&n_molecular_orbitals, 1);
  MPI_info::broadcast_int(&n_occupied_orbitals, 1);
  resize();
  MPI_info::broadcast_vector_double(orbital_energies);
  MPI_info::broadcast_vector_double(movecs);
}

void NWChem_Movec_Parser::resize() {
  occupancy.resize(n_basis_functions);
  orbital_energies.resize(n_basis_functions);
  movecs.resize(n_basis_functions * n_molecular_orbitals);
}

void NWChem_Movec_Parser::parse_binary_movecs(std::string filename) {
  long long title_length;
  long long basis_title_length;
  char ignoreChar[256];

  std::ifstream input;

  std::cout << "Reading binary MOVECS from " << filename << std::endl;
  input.open(filename, std::ios::binary);
  if (!input.is_open()) {
    std::cerr << "no movecs file" << std::endl;
    exit(EXIT_FAILURE);
  }

  //get calcaultion info
  read(input, ignoreChar, true);

  //calcualtion type
  read(input, ignoreChar, true);

  //title length
  read(input, (char*) &title_length);

  //title
  read(input, ignoreChar, true);

  //basis name length
  read(input, (char*) &basis_title_length);

  //basis name
  read(input, ignoreChar, true);

  //nwsets
  read(input, (char*) &nw_nsets);

  //nw_nbf
  read(input, (char*) &n_basis_functions);

  //nw_nmo
  if (nw_nsets > 1) {
    std::cerr << "nw_nsets > 1" << std::endl;
    std::cerr << "Code only supports nw_nset==1" << std::endl;
    exit(EXIT_FAILURE);
  } else {
    read(input, (char*) &n_molecular_orbitals);
  }

  resize();

  read(input, (char*) occupancy.data());
  read(input, (char*) orbital_energies.data());
  for (int i = 0; i < n_molecular_orbitals; i++) {
    read(input, (char*) (movecs.data() + i * n_basis_functions));
  }
}

void NWChem_Movec_Parser::parse_ascii_movecs(std::string filename) {
  long long title_length;
  long long basis_title_length;
  std::string scftype20;
  std::string title;
  std::string basis_name;

  std::ifstream input;

  std::cout << "Reading ascii MOVECS from " << filename << std::endl;
  input.open(filename);
  if (!input.is_open()) {
    std::cerr << "no movecs file" << std::endl;
    exit(EXIT_FAILURE);
  }

  input.ignore(1000, '\n');  // #
  input.ignore(1000, '\n');  // skip convergence info
  input.ignore(1000, '\n');  // skip convergence info
  input.ignore(1000, '\n');  // space
  input.ignore(1000, '\n');  // scftype20
  input.ignore(1000, '\n');  // date lentit
  input >> scftype20;
  input >> title_length;
  input.ignore(1000, '\n');
  std::getline(input, title);

  input >> basis_title_length;
  input.ignore(1000, '\n');
  std::getline(input, basis_name);

  input >> nw_nsets >> n_basis_functions;

  if (nw_nsets > 1) {
    std::cerr << "nw_nsets > 1" << std::endl;
    std::cerr << "Code only supports nw_nset==1" << std::endl;
    exit(EXIT_FAILURE);
  } else {
    input >> n_molecular_orbitals;
  }

  resize();

  for (int i = 0; i < n_molecular_orbitals; i++) {
    input >> occupancy[i];
  }
  for (int i = 0; i < n_molecular_orbitals; i++) {
    input >> orbital_energies[i];
  }

  for (int i = 0, index = 0; i < n_molecular_orbitals; i++) {
    for (int j = 0; j < n_basis_functions; j++) {
      input >> movecs[index++];
    }
  }
}

/*
void Basis::nw_vectors_read(IOPs& iops, MPI_info& mpi_info, Molec& molec) {
  int i, j;
  long long titleLength;
  long long basisTitleLength;
  int ignoreInt;
  char ignoreChar[256];
  double* occ;
  int nw_icore, nw_iocc, nw_nsets;
  std::string scftype20;
  std::string title;
  std::string basis_name;

  std::ifstream input;

  int readmode = iops.iopns[KEYS::MOVECS];
  if (mpi_info.sys_master) {
    if (readmode == 0) {
      std::cout << "Reading binary MOVECS from " << iops.sopns[KEYS::MOVECS] << std::endl;
      input.open(iops.sopns[KEYS::MOVECS].c_str(), std::ios::binary);
      if (!input.is_open()) {
        std::cerr << "no movecs file" << std::endl;
        exit(EXIT_FAILURE);
      }

      //get calcaultion info
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n", ignoreInt); //debug
      input.read(ignoreChar, ignoreInt);
      ignoreChar[ignoreInt] = '\0';
      //			std::cout << ignoreChar << std::endl; //debug
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n\n", ignoreInt); //debug

      //calcualtion type
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n", ignoreInt); //debug
      input.read(ignoreChar, ignoreInt);
      ignoreChar[ignoreInt] = '\0';
      //			std::cout << ignoreChar << std::endl; //debug
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n\n", ignoreInt);//debug

      //title length
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n", ignoreInt); //debug
      input.read((char*)&titleLength, ignoreInt);
      //			std::cout << "title Length: " << titleLength << std::endl; //debug
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n\n", ignoreInt); //debug

      //title
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n", ignoreInt); //debug
      input.read(ignoreChar, ignoreInt);
      ignoreChar[ignoreInt] = '\0';
      //			std::cout << ignoreChar << std::endl; //debug
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n\n", ignoreInt); //debug

      //basis name length
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n", ignoreInt); //debug
      input.read((char*)&basisTitleLength, ignoreInt);
      //			std::cout << "basis title Length: " << basisTitleLength << std::endl; //debug
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n\n", ignoreInt); //debug

      //basis name
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n", ignoreInt); //debug
      input.read(ignoreChar, ignoreInt);
      ignoreChar[ignoreInt] = '\0';
      //			std::cout << ignoreChar << std::endl; //debug
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n\n", ignoreInt); //debug

      //nwsets
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n", ignoreInt); //debug
      input.read((char*)&nw_nsets, ignoreInt);
      //			std::cout << "nw sets: " << nw_nsets << std::endl; //debug
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n\n", ignoreInt); //debug

      //nw_nbf
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n", ignoreInt); //debug
      input.read((char*)&nw_nbf, ignoreInt);
      //			std::cout << "nbf: " << nw_nbf << std::endl; //debug
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n\n", ignoreInt); //debug

      //nw_nmo
      if (nw_nsets > 1) {
        std::cerr << "nw_nsets > 1" << std::endl;
        std::cerr << "Code only supports nw_nset==1" << std::endl;
        std::cerr << "Please contact Alex" << std::endl;
        exit(EXIT_FAILURE);
      } else {
        input.read((char*)&ignoreInt, 4);
        //				printf("%#08x\n", ignoreInt); //debug
        input.read((char*)&nw_nmo, ignoreInt);
        //				std::cout << "nw_nmo: " << nw_nmo << std::endl; //debug
        input.read((char*)&ignoreInt, 4);
        //				printf("%#08x\n\n", ignoreInt); //debug
      }

    } else {
      std::cout << "Reading ascii MOVECS from " << iops.sopns[KEYS::MOVECS] << std::endl;
      input.open(iops.sopns[KEYS::MOVECS].c_str());
      if (!input.is_open()) {
        std::cerr << "no movecs file" << std::endl;
        exit(EXIT_FAILURE);
      }
      input.ignore(1000, '\n');  // #
      input.ignore(1000, '\n');  // skip convergence info
      input.ignore(1000, '\n');  // skip convergence info
      input.ignore(1000, '\n');  // space
      input.ignore(1000, '\n');  // scftype20
      input.ignore(1000, '\n');  // date lentit
      input >> scftype20;
      input >> titleLength;
      input.ignore(1000, '\n');
      std::getline(input, title);

      input >> basisTitleLength;
      input.ignore(1000, '\n');
      std::getline(input, basis_name);

      input >> nw_nsets >> nw_nbf;

      nw_nmo = 0;

      for (i = 0; i < nw_nsets; i++) {
        input >> nw_nmo;
      }
    }
    std::cout << "nw_vectors: nbf " << nw_nbf << std::endl;
    std::cout << "nw_vectors: nmo ";
    for (i = 0; i < nw_nsets; i++) {
      std::cout << nw_nmo << " ";
    }
    std::cout << std::endl;
    std::cout.flush();
  }

#ifdef HAVE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Bcast(&nw_nsets, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&nw_nbf, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&nw_nmo, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif

  occ = new double[nw_nbf];
  nw_en = new double[nw_nbf];
  h_basis.nw_co = new double[nw_nbf * nw_nmo];
//nw_co = new double*[nw_nbf];
//for(i=0;i<nw_nbf;i++) {
//	nw_co[i] = new double[nw_nmo];
//}

  if (mpi_info.sys_master) {
    if (readmode == 0) {
      //occupancy information
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\t%i\n", ignoreInt, ignoreInt); //debug
      input.read((char*)occ, ignoreInt);
      //			std::cout << "occ:"; //debug
      //			for(i=0;i<nw_nbf;i++) { //debug
      //				std::cout << "\t" << occ[i] << std::endl; //debug
      //			} //debug
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n\n", ignoreInt); //debug

      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\t%i\n", ignoreInt, ignoreInt); //debug
      input.read((char*)nw_en, ignoreInt);
      //			std::cout << "nw_en:"; //debug
      //			for(i=0;i<nw_nbf;i++) { //debug
      //				std::cout << "\t" << nw_en[i] << std::endl; //debug
      //			} //debug
      input.read((char*)&ignoreInt, 4);
      //			printf("%#08x\n\n", ignoreInt); //debug

      int index = 0;
      for (i = 0; i < nw_nmo; i++) {
        double temp[nw_nbf];
        input.read((char*)&ignoreInt, 4);
        //				printf("%#08x\t%i\n", ignoreInt, ignoreInt); //debug
        input.read((char*)temp, ignoreInt);
        //				std::cout << "nw_co:"; //debug
        for (j = 0; j < nw_nbf; j++) {
          h_basis.nw_co[index] = temp[j];
          index++;
          //					std::cout << "\t" << temp[j] << std::endl; //debug
        }
        input.read((char*)&ignoreInt, 4);
        //				printf("%#08x\n\n", ignoreInt); //debug
      }
    } else {
      for (i = 0; i < nw_nbf; i++) {
        input >> occ[i];
      }
      for (i = 0; i < nw_nbf; i++) {
        input >> nw_en[i];
      }

      int index = 0;
      for (i = 0; i < nw_nmo; i++) {
        for (j = 0; j < nw_nbf; j++) {
          input >> h_basis.nw_co[index];
          index++;
        }
      }
    }
    input.close();

    nw_iocc = 0;
    for (i = 0; i < nw_nbf; i++) {
      if (occ[i] > 0.000) {
        nw_iocc = i;
      }
    }
    delete[] occ;
  }

#ifdef HAVE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Bcast(&nw_iocc, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&nw_icore, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(nw_en, nw_nbf, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(h_basis.nw_co, nw_nbf * nw_nmo, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif

  //orbital_check();
  nw_icore = 0;
  if (iops.bopns[KEYS::FREEZE_CORE]) {
    for (i = 0; i < molec.atoms.size(); i++) {
      if (molec.atoms[i].znum > 3 && molec.atoms[i].znum < 10 && !molec.atoms[i].is_ghost) {
        nw_icore = nw_icore + 1;
      }
    }
  }
  iocc1 = nw_icore;
  iocc2 = nw_iocc + 1;
  ivir1 = nw_iocc + 1;
  ivir2 = nw_nmo;

  if (qc_nbf != nw_nbf) {
    std::cerr << "You might use the different basis sets or geometry" << std::endl;
    exit(EXIT_FAILURE);
  }

  // print orbital energies to <JOBNAME>.orbital_energies
  if (mpi_info.sys_master && (
         iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GF ||
         iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFDIFF ||
         iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULL ||
         iops.iopns[KEYS::JOBTYPE] == JOBTYPE::GFFULLDIFF)) {
    std::stringstream ss;
    std::string str;
    std::ofstream output;

    // construct file name
    ss << iops.sopns[KEYS::JOBNAME] << ".orbital_energies";
    ss >> str;

    // open ofstream
    output.open(str.c_str());

    // print out data
    output << "# File contains energies for orbital " << iocc1 << " to orbital " << ivir2 << std::endl;
    output << "# 0 == first orbital" << std::endl;
    for (auto it = iocc1; it < ivir2; it++) {
      output << nw_en[it] << std::endl;
    }

    // close ofstream
    output.close();
  }
}
*/
