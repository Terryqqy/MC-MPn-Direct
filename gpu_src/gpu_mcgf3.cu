#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <thrust/execution_policy.h>
#include <thrust/reduce.h>
#include "cublas_v2.h"

#include "../cublasStatus_t_getErrorString.h"

#include "../qc_monte.h"
__global__ void gf3_core_c(OVPS_ARRAY ovps, int mc_pair_num) {
  int tidx = blockIdx.x * blockDim.x + threadIdx.x;
  int tidy = blockIdx.y * blockDim.y + threadIdx.y;
#define TIDX_CONTROL if (tidx < mc_pair_num)
#define TIDY_CONTROL if (tidy < mc_pair_num)
#include "gf3_core_c.h"
#undef TIDX_CONTROL
#undef TIDY_CONTROL
}
__global__ void gf3_core_p_1(OVPS_ARRAY ovps, int mc_pair_num) {
  int tidx = blockIdx.x * blockDim.x + threadIdx.x;
  int tidy = blockIdx.y * blockDim.y + threadIdx.y;
#define TIDX_CONTROL if (tidx < mc_pair_num)
#define TIDY_CONTROL if (tidy < mc_pair_num)
#include "gf3_core_p_1.h"
#undef TIDX_CONTROL
#undef TIDY_CONTROL
}
__global__ void gf3_core_p_2(OVPS_ARRAY ovps, int mc_pair_num) {
  int tidx = blockIdx.x * blockDim.x + threadIdx.x;
  int tidy = blockIdx.y * blockDim.y + threadIdx.y;
#define TIDX_CONTROL if (tidx < mc_pair_num)
#define TIDY_CONTROL if (tidy < mc_pair_num)
#include "gf3_core_p_2.h"
#undef TIDX_CONTROL
#undef TIDY_CONTROL
}
__global__ void gf3_core_p_12(OVPS_ARRAY ovps, int mc_pair_num) {
  int tidx = blockIdx.x * blockDim.x + threadIdx.x;
  int tidy = blockIdx.y * blockDim.y + threadIdx.y;
#define TIDX_CONTROL if (tidx < mc_pair_num)
#define TIDY_CONTROL if (tidy < mc_pair_num)
#include "gf3_core_p_12.h"
#undef TIDX_CONTROL
#undef TIDY_CONTROL
}
__global__ void gf3_core_m_1(OVPS_ARRAY ovps, int mc_pair_num) {
  int tidx = blockIdx.x * blockDim.x + threadIdx.x;
  int tidy = blockIdx.y * blockDim.y + threadIdx.y;
#define TIDX_CONTROL if (tidx < mc_pair_num)
#define TIDY_CONTROL if (tidy < mc_pair_num)
#include "gf3_core_m_1.h"
#undef TIDX_CONTROL
#undef TIDY_CONTROL
}
__global__ void gf3_core_m_2(OVPS_ARRAY ovps, int mc_pair_num) {
  int tidx = blockIdx.x * blockDim.x + threadIdx.x;
  int tidy = blockIdx.y * blockDim.y + threadIdx.y;
#define TIDX_CONTROL if (tidx < mc_pair_num)
#define TIDY_CONTROL if (tidy < mc_pair_num)
#include "gf3_core_m_2.h"
#undef TIDX_CONTROL
#undef TIDY_CONTROL
}
__global__ void gf3_core_m_12(OVPS_ARRAY ovps, int mc_pair_num) {
  int tidx = blockIdx.x * blockDim.x + threadIdx.x;
  int tidy = blockIdx.y * blockDim.y + threadIdx.y;
#define TIDX_CONTROL if (tidx < mc_pair_num)
#define TIDY_CONTROL if (tidy < mc_pair_num)
#include "gf3_core_m_12.h"
#undef TIDX_CONTROL
#undef TIDY_CONTROL
}

void GF::mcgf3_local_energy_core() {
  double alpha, beta;
  cublasHandle_t handle;
  cublasStatusAssert(cublasCreate(&handle), __FILE__, __LINE__);

  dim3 blockSize(128, 1, 1);
  dim3 gridSize((iops.iopns[KEYS::MC_NPAIR] + 127) / 128, iops.iopns[KEYS::MC_NPAIR], 1);

  gf3_core_c<<<gridSize, blockSize>>>(ovps.d_ovps, iops.iopns[KEYS::MC_NPAIR]);
  cudaError_t_Assert(cudaPeekAtLastError(), __FILE__, __LINE__);
  gf3_core_p_1<<<gridSize, blockSize>>>(ovps.d_ovps, iops.iopns[KEYS::MC_NPAIR]);
  cudaError_t_Assert(cudaPeekAtLastError(), __FILE__, __LINE__);
  gf3_core_p_2<<<gridSize, blockSize>>>(ovps.d_ovps, iops.iopns[KEYS::MC_NPAIR]);
  cudaError_t_Assert(cudaPeekAtLastError(), __FILE__, __LINE__);
  gf3_core_p_12<<<gridSize, blockSize>>>(ovps.d_ovps, iops.iopns[KEYS::MC_NPAIR]);
  cudaError_t_Assert(cudaPeekAtLastError(), __FILE__, __LINE__);
  gf3_core_m_1<<<gridSize, blockSize>>>(ovps.d_ovps, iops.iopns[KEYS::MC_NPAIR]);
  cudaError_t_Assert(cudaPeekAtLastError(), __FILE__, __LINE__);
  gf3_core_m_2<<<gridSize, blockSize>>>(ovps.d_ovps, iops.iopns[KEYS::MC_NPAIR]);
  cudaError_t_Assert(cudaPeekAtLastError(), __FILE__, __LINE__);
  gf3_core_m_12<<<gridSize, blockSize>>>(ovps.d_ovps, iops.iopns[KEYS::MC_NPAIR]);
  cudaError_t_Assert(cudaPeekAtLastError(), __FILE__, __LINE__);

  alpha = 1.00;
  beta = 0.00;
  cublasStatusAssert(cublasDgemv(handle, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_12cCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.one, 1,
                                 &beta, ovps.d_ovps.en3c12, 1),
                     __FILE__, __LINE__);

  alpha = 1.00;
  beta = 0.00;
  cublasStatusAssert(cublasDgemv(handle, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_22cCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.one, 1,
                                 &beta, ovps.d_ovps.en3c22, 1),
                     __FILE__, __LINE__);

  cudaError_t_Assert(cudaThreadSynchronize(), __FILE__, __LINE__);
  cublasStatusAssert(cublasDestroy(handle), __FILE__, __LINE__);
}
void GF::mcgf3_local_energy(std::vector<double>& egf3, int band) {
  int nsamp;
  double en3 = 0;
  double en3t = 0;
  cublasHandle_t handle;
  cublasStatusAssert(cublasCreate(&handle), __FILE__, __LINE__);

  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3c12, 1,
                                ovps.d_ovps.ps_12c + band * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3 = en3 + en3t;
  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3c22, 1,
                                ovps.d_ovps.ps_22c + band * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3 = en3 + en3t;

  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3_1pCore, 1,
                                ovps.d_ovps.ps_24 + band * iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3 = en3 + en3t * tau.get_gfn_tau(0, 0, band - offBand, false);
  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3_2pCore, 1,
                                ovps.d_ovps.ps_24 + band * iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3 = en3 + en3t * tau.get_gfn_tau(1, 1, band - offBand, false);
  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3_12pCore, 1,
                                ovps.d_ovps.ps_24 + band * iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3 = en3 + en3t * tau.get_gfn_tau(1, 0, band - offBand, false);

  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3_1mCore, 1,
                                ovps.d_ovps.ps_24 + band * iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3 = en3 + en3t * tau.get_gfn_tau(0, 0, band - offBand, true);
  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3_2mCore, 1,
                                ovps.d_ovps.ps_24 + band * iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3 = en3 + en3t * tau.get_gfn_tau(1, 1, band - offBand, true);
  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3_12mCore, 1,
                                ovps.d_ovps.ps_24 + band * iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3 = en3 + en3t * tau.get_gfn_tau(1, 0, band - offBand, true);

  nsamp = iops.iopns[KEYS::MC_NPAIR] * (iops.iopns[KEYS::MC_NPAIR] - 1) * (iops.iopns[KEYS::MC_NPAIR] - 2);
  en3 = en3 * tau.get_wgt(2) / static_cast<double>(nsamp);
  egf3.front() += en3;
  cublasStatusAssert(cublasDestroy(handle), __FILE__, __LINE__);
}
void GF::mcgf3_local_energy_diff(std::vector<double>& egf3, int band) {
  int ip, dp;
  int nsamp;
  double en3t;
  std::array<double, 7> en3;

  cublasHandle_t handle;
  cublasStatusAssert(cublasCreate(&handle), __FILE__, __LINE__);

  std::fill(en3.begin(), en3.end(), 0);

  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3c12, 1,
                                ovps.d_ovps.ps_12c + band * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3[0] = en3[0] + en3t;
  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3c22, 1,
                                ovps.d_ovps.ps_22c + band * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3[0] = en3[0] + en3t;

  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3_1pCore, 1,
                                ovps.d_ovps.ps_24 + band * iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3[1] = en3[1] + en3t * tau.get_gfn_tau(0, 0, band - offBand, false);
  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3_2pCore, 1,
                                ovps.d_ovps.ps_24 + band * iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3[2] = en3[2] + en3t * tau.get_gfn_tau(1, 1, band - offBand, false);
  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3_12pCore, 1,
                                ovps.d_ovps.ps_24 + band * iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3[3] = en3[3] + en3t * tau.get_gfn_tau(1, 0, band - offBand, false);

  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3_1mCore, 1,
                                ovps.d_ovps.ps_24 + band * iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3[4] = en3[4] + en3t * tau.get_gfn_tau(0, 0, band - offBand, true);
  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3_2mCore, 1,
                                ovps.d_ovps.ps_24 + band * iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3[5] = en3[5] + en3t * tau.get_gfn_tau(1, 1, band - offBand, true);
  cublasStatusAssert(cublasDdot(handle, iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR],
                                ovps.d_ovps.en3_12mCore, 1,
                                ovps.d_ovps.ps_24 + band * iops.iopns[KEYS::MC_NPAIR] * iops.iopns[KEYS::MC_NPAIR], 1,
                                &en3t),
                     __FILE__, __LINE__);
  en3[6] = en3[6] + en3t * tau.get_gfn_tau(1, 0, band - offBand, true);

  nsamp = iops.iopns[KEYS::MC_NPAIR] * (iops.iopns[KEYS::MC_NPAIR] - 1) * (iops.iopns[KEYS::MC_NPAIR] - 2);
  for (auto& it : en3) {
    it = it * tau.get_wgt(2) / static_cast<double>(nsamp);
  }

  for (ip = 0; ip < iops.iopns[KEYS::DIFFS]; ip++) {
    if (ip == 0) {
      for (dp = 0; dp < 3; dp++) {
        egf3[ip] += en3[dp + 1] + en3[dp + 4];
      }
      egf3[ip] += en3[0];
    } else if (ip % 2 == 1) {
      for (dp = 0; dp < 3; dp++) {
        egf3[ip] += en3[dp + 1] - en3[dp + 4];
      }
    } else if (ip % 2 == 0) {
      for (dp = 0; dp < 3; dp++) {
        egf3[ip] += en3[dp + 1] + en3[dp + 4];
      }
    }
    auto xx1 = tau.get_tau(0);
    auto xx2 = tau.get_tau(1);
    en3[1] = en3[1] * xx1;
    en3[2] = en3[2] * xx2;
    en3[3] = en3[3] * xx1 * xx2;
    en3[4] = en3[4] * xx1;
    en3[5] = en3[5] * xx2;
    en3[6] = en3[6] * xx1 * xx2;
  }
  cublasStatusAssert(cublasDestroy(handle), __FILE__, __LINE__);
}
void GF::mcgf3_local_energy_full(int band) {
  int nsamp = iops.iopns[KEYS::MC_NPAIR] * (iops.iopns[KEYS::MC_NPAIR] - 1) * (iops.iopns[KEYS::MC_NPAIR] - 2);
  //	int offset = (ivir2-iocc1) * (iocc2-iocc1-offBand+band) + (iocc2-iocc1-offBand+band);
  //	double en3 = 0;
  double alpha, beta;
  cublasHandle_t handle;
  cublasStatusAssert(cublasCreate(&handle), __FILE__, __LINE__);

  // ent = contraction_exp en3_1p . psi2
  alpha = tau.get_gfn_tau(0, 0, band - offBand, false) * tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_1pCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]),
                     __FILE__, __LINE__);

  // ent = contraction_exp en3_2p . psi2 + ent
  alpha = tau.get_gfn_tau(1, 1, band - offBand, false) * tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 1.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_2pCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]),
                     __FILE__, __LINE__);

  // ent = contraction_exp en3_12p . psi2 + ent
  alpha = tau.get_gfn_tau(1, 0, band - offBand, false) * tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 1.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_12pCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]),
                     __FILE__, __LINE__);

  // ent = contraction_exp en3_1m . psi2 + ent
  alpha = tau.get_gfn_tau(0, 0, band - offBand, true) * tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 1.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_1mCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]),
                     __FILE__, __LINE__);

  // ent = contraction_exp en3_2m . psi2 + ent
  alpha = tau.get_gfn_tau(1, 1, band - offBand, true) * tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 1.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_2mCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]),
                     __FILE__, __LINE__);

  // ent = contraction_exp en3_12m . psi2 + ent
  alpha = tau.get_gfn_tau(1, 0, band - offBand, true) * tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 1.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_12mCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]),
                     __FILE__, __LINE__);

  // en3 = Transpose[psi2] . ent + en3
  alpha = 1.00;
  beta = 1.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_N,
                                 ivir2 - iocc1, ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.en3[band][0], ivir2 - iocc1),
                     __FILE__, __LINE__);
  // en3c = contraction_exp en3_12c . IdentityVector
  //	contraction_exp = tau.get_wgt(2) / static_cast<double>(nsamp);
  //	beta  = 0.00;
  //	cublasDgemv(handle, CUBLAS_OP_N,
  //			iops.iopns[KEYS::MC_NPAIR], iops.iopns[KEYS::MC_NPAIR], &contraction_exp,
  //			ovps.d_ovps.en3_12cCore, iops.iopns[KEYS::MC_NPAIR],
  //			ovps.d_ovps.one, 1,
  //			&beta, ovps.d_ovps.en3c, 1), __FILE__, __LINE__);

  // ent = diag[enc12] . psi1
  cublasDdgmm(handle, CUBLAS_SIDE_LEFT,
              iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1,
              ovps.d_ovps.psi1, iops.iopns[KEYS::MC_NPAIR],
              ovps.d_ovps.en3c12, 1,
              ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]);

  // en3 = Transpose[psi2] . ent + en3
  //	contraction_exp = 1.00;
  alpha = tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 1.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_N,
                                 ivir2 - iocc1, ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.en3[band][0], ivir2 - iocc1),
                     __FILE__, __LINE__);

  // en3c = contraction_exp en3_22c . IdentityVector
  //	contraction_exp = tau.get_wgt(2) / static_cast<double>(nsamp);
  //	beta  = 0.00;
  //	cublasDgemv(handle, CUBLAS_OP_N,
  //			iops.iopns[KEYS::MC_NPAIR], iops.iopns[KEYS::MC_NPAIR], &contraction_exp,
  //			ovps.d_ovps.en3_22cCore, iops.iopns[KEYS::MC_NPAIR],
  //			ovps.d_ovps.one, 1,
  //			&beta, ovps.d_ovps.en3c, 1), __FILE__, __LINE__);

  // ent = diag[en3c22] . psi2
  cublasDdgmm(handle, CUBLAS_SIDE_LEFT,
              iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1,
              ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
              ovps.d_ovps.en3c22, 1,
              ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]);

  // en3 = Transpose[psi2] . ent + en3
  alpha = tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 1.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_N,
                                 ivir2 - iocc1, ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.en3[band][0], ivir2 - iocc1),
                     __FILE__, __LINE__);

  /*
	cudaMemcpy(&en3, ovps.d_ovps.en3 + offset, sizeof(double), cudaMemcpyDeviceToHost);
	cudaThreadSynchronize();
	egf3.front() += en3;
*/

  cublasStatusAssert(cublasDestroy(handle), __FILE__, __LINE__);
}
void GF::mcgf3_local_energy_full_diff(int band) {
  int nsamp = iops.iopns[KEYS::MC_NPAIR] * (iops.iopns[KEYS::MC_NPAIR] - 1) * (iops.iopns[KEYS::MC_NPAIR] - 2);
  double alpha, beta;
  cublasHandle_t handle;
  cublasStatusAssert(cublasCreate(&handle), __FILE__, __LINE__);

  // ent = contraction_exp en3_1pCore . psi2
  alpha = tau.get_gfn_tau(0, 0, band - offBand, false) * tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_1pCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]),
                     __FILE__, __LINE__);

  // en3_1p = Tranpsose[psi2] . ent
  alpha = 1.00;
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_N,
                                 ivir2 - iocc1, ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.en3_1p, ivir2 - iocc1),
                     __FILE__, __LINE__);

  // ent = contraction_exp en3_2pCore . psi2
  alpha = tau.get_gfn_tau(1, 1, band - offBand, false) * tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_2pCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]),
                     __FILE__, __LINE__);

  // en3_2p = Tranpsose[psi2] . ent
  alpha = 1.00;
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_N,
                                 ivir2 - iocc1, ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.en3_2p, ivir2 - iocc1),
                     __FILE__, __LINE__);

  // ent = contraction_exp en3_12pCore . psi2
  alpha = tau.get_gfn_tau(1, 0, band - offBand, false) * tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_12pCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]),
                     __FILE__, __LINE__);

  // en3_12p = Tranpsose[psi2] . ent
  alpha = 1.00;
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_N,
                                 ivir2 - iocc1, ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.en3_12p, ivir2 - iocc1),
                     __FILE__, __LINE__);

  // ent = contraction_exp en3_1mCore . psi2
  alpha = tau.get_gfn_tau(0, 0, band - offBand, true) * tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_1mCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]),
                     __FILE__, __LINE__);

  // en3_1m = Tranpsose[psi2] . ent
  alpha = 1.00;
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_N,
                                 ivir2 - iocc1, ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.en3_1m, ivir2 - iocc1),
                     __FILE__, __LINE__);

  // ent = contraction_exp en3_2mCore . psi2
  alpha = tau.get_gfn_tau(1, 1, band - offBand, true) * tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_2mCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]),
                     __FILE__, __LINE__);

  // en3_2m = Tranpsose[psi2] . ent
  alpha = 1.00;
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_N,
                                 ivir2 - iocc1, ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.en3_2m, ivir2 - iocc1),
                     __FILE__, __LINE__);

  // ent = contraction_exp en3_12mCore . psi2
  alpha = tau.get_gfn_tau(1, 0, band - offBand, true) * tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
                                 iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.en3_12mCore, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]),
                     __FILE__, __LINE__);

  // en3_12m = Tranpsose[psi2] . ent
  alpha = 1.00;
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_N,
                                 ivir2 - iocc1, ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.en3_12m, ivir2 - iocc1),
                     __FILE__, __LINE__);

  // en3c = contraction_exp en3_12cCore . IdentityVector
  //	contraction_exp = tau.get_wgt(2) / static_cast<double>(nsamp);
  //	beta  = 0.00;
  //	cublasDgemv(handle, CUBLAS_OP_N,
  //			iops.iopns[KEYS::MC_NPAIR], iops.iopns[KEYS::MC_NPAIR], &contraction_exp,
  //			ovps.d_ovps.en3_12cCore, iops.iopns[KEYS::MC_NPAIR],
  //			ovps.d_ovps.one, 1,
  //			&beta, ovps.d_ovps.en3c, 1), __FILE__, __LINE__);

  // ent = diag[enc12] . psi1
  cublasDdgmm(handle, CUBLAS_SIDE_LEFT,
              iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1,
              ovps.d_ovps.psi1, iops.iopns[KEYS::MC_NPAIR],
              ovps.d_ovps.en3c12, 1,
              ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]);

  // en3_c = Transpose[psi2] . ent
  //contraction_exp = 1.00;
  alpha = tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 0.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_N,
                                 ivir2 - iocc1, ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.en3_c, ivir2 - iocc1),
                     __FILE__, __LINE__);

  // en3c = contraction_exp en3_22cCore . IdentityVector
  //	contraction_exp = tau.get_wgt(2) / static_cast<double>(nsamp);
  //	beta  = 0.00;
  //	cublasDgemv(handle, CUBLAS_OP_N,
  //			iops.iopns[KEYS::MC_NPAIR], iops.iopns[KEYS::MC_NPAIR], &contraction_exp,
  //			ovps.d_ovps.en3_22cCore, iops.iopns[KEYS::MC_NPAIR],
  //			ovps.d_ovps.one, 1,
  //			&beta, ovps.d_ovps.en3c, 1), __FILE__, __LINE__);

  // ent = diag[en3c22] . psi2
  cublasDdgmm(handle, CUBLAS_SIDE_LEFT,
              iops.iopns[KEYS::MC_NPAIR], ivir2 - iocc1,
              ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
              ovps.d_ovps.en3c22, 1,
              ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR]);

  // en3_c = Transpose[psi2] . ent + en3_c
  //contraction_exp = 1.00;
  alpha = tau.get_wgt(2) / static_cast<double>(nsamp);
  beta = 1.00;
  cublasStatusAssert(cublasDgemm(handle, CUBLAS_OP_T, CUBLAS_OP_N,
                                 ivir2 - iocc1, ivir2 - iocc1, iops.iopns[KEYS::MC_NPAIR], &alpha,
                                 ovps.d_ovps.psi2, iops.iopns[KEYS::MC_NPAIR],
                                 ovps.d_ovps.ent, iops.iopns[KEYS::MC_NPAIR],
                                 &beta, ovps.d_ovps.en3_c, ivir2 - iocc1),
                     __FILE__, __LINE__);

  cudaThreadSynchronize();
  cublasStatusAssert(cublasDestroy(handle), __FILE__, __LINE__);
}

