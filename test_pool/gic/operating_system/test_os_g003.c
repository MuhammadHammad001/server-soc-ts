/** @file
 * Copyright (c) 2016-2021, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#include "val/include/bsa_acs_val.h"
#include "val/include/val_interface.h"

#include "val/include/bsa_acs_gic.h"
#include "val/include/bsa_acs_pcie.h"

#define TEST_NUM   (ACS_GIC_TEST_NUM_BASE + 3)
#define TEST_RULE  "B_GIC_03"
#define TEST_DESC  "If PCIe, GICv3 then ITS, LPI          "

static
void
payload()
{

  uint32_t data;
  uint32_t num_ecam = 0;
  uint32_t gic_ver = 0;
  uint32_t index = val_hart_get_index_mpid(val_hart_get_mpid());

  num_ecam = val_pcie_get_info(PCIE_INFO_NUM_ECAM, 0);
  gic_ver  = val_gic_get_info(GIC_INFO_VERSION);

  if ((num_ecam != 0) && (gic_ver >= 3))
  {
    data = val_gic_get_info(GIC_INFO_NUM_ITS);
    if (data == 0)
    {
        val_print(ACS_PRINT_ERR, "\n       GICv3 and PCIe : ITS Not Present", 0);
        val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
        return;
    }

    data = VAL_EXTRACT_BITS(val_mmio_read(val_get_gicd_base() + GICD_TYPER), 17, 17);
    if (data == 0)
    {
        val_print(ACS_PRINT_ERR, "\n       GICv3 and PCIe : LPI Not Supported", 0);
        val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
        return;
    }

    val_set_status(index, RESULT_PASS(TEST_NUM, 1));
    return;
  }

  /* If PCIe is not present or Gic version is older then GICv3,
     just Skip the test */
  val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
}

uint32_t
os_g003_entry(uint32_t num_hart)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_hart = 1;  //This IIC test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_hart);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_hart, payload, 0);

  /* get the result from all HART and check for failure */
  status = val_check_for_error(TEST_NUM, num_hart, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
