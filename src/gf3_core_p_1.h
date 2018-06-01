  TIDX_CONTROL {
    TIDY_CONTROL {
      double en3 = 0;
      int index = tidy * mc_pair_num + tidx;
      int jkIndex, ijIndex, ikIndex;

      if(tidx != tidy) {
        double en[10];
        for(int tidz = 0; tidz < 10; tidz++) {
          en[tidz] = 0;
        }
        for(int tidz = 0; tidz < mc_pair_num; tidz++) {
          if(tidx != tidz && tidy != tidz) {
            ikIndex = tidx * mc_pair_num + tidz;
            jkIndex = tidy * mc_pair_num + tidz;
            en[0] += ovps.os_35[jkIndex] * ovps.os_46[jkIndex] * ovps.vs_15[ikIndex] * ovps.vs_26[ikIndex] * ovps.rv[tidz];
            en[1] += ovps.os_35[jkIndex] * ovps.os_46[jkIndex] * ovps.vs_16[ikIndex] * ovps.vs_25[ikIndex] * ovps.rv[tidz];
            en[2] += ovps.os_35[jkIndex] * ovps.os_16[ikIndex] * ovps.vs_35[jkIndex] * ovps.vs_16[ikIndex] * ovps.rv[tidz];
            en[3] += ovps.os_35[jkIndex] * ovps.os_16[ikIndex] * ovps.vs_35[jkIndex] * ovps.vs_26[ikIndex] * ovps.rv[tidz];
            en[4] += ovps.os_35[jkIndex] * ovps.os_16[ikIndex] * ovps.vs_36[jkIndex] * ovps.vs_15[ikIndex] * ovps.rv[tidz];
            en[5] += ovps.os_35[jkIndex] * ovps.os_16[ikIndex] * ovps.vs_36[jkIndex] * ovps.vs_25[ikIndex] * ovps.rv[tidz];
            en[6] += ovps.os_45[jkIndex] * ovps.os_16[ikIndex] * ovps.vs_35[jkIndex] * ovps.vs_16[ikIndex] * ovps.rv[tidz];  // 34
            en[7] += ovps.os_45[jkIndex] * ovps.os_16[ikIndex] * ovps.vs_35[jkIndex] * ovps.vs_26[ikIndex] * ovps.rv[tidz];  // 34
            en[8] += ovps.os_45[jkIndex] * ovps.os_16[ikIndex] * ovps.vs_36[jkIndex] * ovps.vs_15[ikIndex] * ovps.rv[tidz];  // 34
            en[9] += ovps.os_45[jkIndex] * ovps.os_16[ikIndex] * ovps.vs_36[jkIndex] * ovps.vs_25[ikIndex] * ovps.rv[tidz];  // 34
          }
        }
        ijIndex = tidx * mc_pair_num + tidy;
        en3 +=  2.00 * en[0] * ovps.os_13[ijIndex];
        en3 += -1.00 * en[1] * ovps.os_13[ijIndex];
        en3 +=  4.00 * en[2] * ovps.vs_24[ijIndex];
        en3 += -2.00 * en[3] * ovps.vs_14[ijIndex];
        en3 += -2.00 * en[4] * ovps.vs_24[ijIndex];
        en3 +=  1.00 * en[5] * ovps.vs_14[ijIndex];
        en3 += -2.00 * en[6] * ovps.vs_23[ijIndex];  // 34
        en3 +=  1.00 * en[7] * ovps.vs_13[ijIndex];  // 34
        en3 +=  1.00 * en[8] * ovps.vs_23[ijIndex];  // 34
        en3 += -2.00 * en[9] * ovps.vs_13[ijIndex];  // 34
        en3 = en3 * ovps.rv[tidx] * ovps.rv[tidy];
      }
      ovps.en3_1pCore[index] = en3;
    }
  }
