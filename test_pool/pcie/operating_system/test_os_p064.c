/** @file
 * Copyright (c) 2016-2018, 2021-2023, Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_pcie.h"
#include "val/include/bsa_acs_memory.h"

#define TEST_NUM   (ACS_PCIE_TEST_NUM_BASE + 64)
#define TEST_RULE  "PCI_MSI_2"
#define TEST_DESC  "Check MSI(X) vectors uniqueness       "

/**
    @brief   Returns MSI(X) status of the device

    @param   dev_index    index of PCI device

    @return  0    device does not support MSI(X)
    @return  1    device supports MSI(X)
**/
static uint32_t check_msi_status(uint32_t dev_index)
{
  uint32_t data;

  data = val_peripheral_get_info (ANY_FLAGS, dev_index);

  if ((data & PER_FLAG_MSI_ENABLED) &&
      val_peripheral_get_info (ANY_GSIV, dev_index)) {
    return 1;
  }

  return 0;
}

/**
    @brief   Compare two lists of MSI(X) vectors

    @param   list_one    pointer to a first list of MSI(X) vectors
    @param   list_two    pointer to a second list of MSI(X) vectors

    @return  0    no vectors duplicates are found
    @return  1    lists contain at leas one common MSI(X) vector
**/
static
uint32_t
check_list_duplicates (PERIPHERAL_VECTOR_LIST *list_one, PERIPHERAL_VECTOR_LIST *list_two)
{
  PERIPHERAL_VECTOR_LIST *flist_node;
  PERIPHERAL_VECTOR_LIST *slist_node;

  uint32_t fcount = 0;
  uint32_t scount = 0;
  uint32_t irq_start1, irq_end1;
  uint32_t irq_start2, irq_end2;

  flist_node = list_one;
  slist_node = list_two;

  while (flist_node != NULL) {
    while (slist_node != NULL) {
      irq_start1 = flist_node->vector.vector_irq_base;
      irq_end1 = flist_node->vector.vector_irq_base + flist_node->vector.vector_n_irqs - 1;
      irq_start2 = slist_node->vector.vector_irq_base;
      irq_end2 = slist_node->vector.vector_irq_base + slist_node->vector.vector_n_irqs - 1;
      if (!(irq_end1 < irq_start2 || irq_start1 > irq_end2))
        return 1;
      slist_node = slist_node->next;
      scount++;
    }
    slist_node = list_two;
    flist_node = flist_node->next;
    fcount++;
    scount = 0;
  }

  return 0;
}

/**
    @brief   Free memory allocated for a list of MSI(X) vectors

    @param   list    pointer to a list of MSI(X) vectors
**/
static
void
clean_msi_list (PERIPHERAL_VECTOR_LIST *list)
{
  PERIPHERAL_VECTOR_LIST *next_node;
  PERIPHERAL_VECTOR_LIST *current_node;

  current_node = list;
  while (current_node != NULL) {
    next_node = current_node->next;
    val_memory_free (current_node);
    current_node = next_node;
  }
}

static
void
payload (void)
{

  uint32_t count = val_peripheral_get_info (NUM_ALL, 0);
  uint32_t index = val_hart_get_index_mpid (val_hart_get_mpid());
  uint8_t status;
  PERIPHERAL_VECTOR_LIST *current_dev_mvec;
  PERIPHERAL_VECTOR_LIST *next_dev_mvec;
  uint64_t current_dev_bdf;
  uint64_t next_dev_bdf;
  uint32_t count_next;
  uint32_t test_skip = 1;

  if (!count) {
     val_print(ACS_PRINT_DEBUG, "\n       No peripherals found. Skipping test", 0);
     val_set_status (index, RESULT_SKIP(TEST_NUM, 3));
     return;
  }

  status = 0;
  current_dev_mvec = NULL;
  next_dev_mvec = NULL;

  /*
    Pull each discovered PCI device and its list of MSI(X) vectors.
    Compare this list with MSI(X) vector lists of other discovered
    PCI devices and find duplicates exist.
  */
  while (count > 0 && !status) {
    count_next = count - 1;
    if (check_msi_status (count - 1)) {
      /* Get BDF of a device */
      current_dev_bdf = val_peripheral_get_info (ANY_BDF, count - 1);
      if (current_dev_bdf) {
        val_print (ACS_PRINT_DEBUG, "\n       Checking PCI device with BDF 0x%X", current_dev_bdf);
        /* Read MSI(X) vectors */
        if (val_get_msi_vectors (current_dev_bdf, &current_dev_mvec)) {

          /* Pull other PCI devices left in the devices list */
          while (count_next > 0 && !status) {
            if (check_msi_status (count_next - 1)) {
              /* Get BDF of a device */
              next_dev_bdf = val_peripheral_get_info (ANY_BDF, count_next - 1);
              /* Read MSI(X) vectors */
              if (val_get_msi_vectors (next_dev_bdf, &next_dev_mvec)) {
                test_skip = 0;
                /* Compare two lists of MSI(X) vectors */
                if (check_list_duplicates (current_dev_mvec, next_dev_mvec)) {
                  val_print (ACS_STATUS_ERR, "\n       Allocated MSIs are not unique", 0);
                  val_set_status (index, RESULT_FAIL(TEST_NUM, 02));
                  status = 1;
                }
                clean_msi_list (next_dev_mvec);
              }
            }
            count_next--;
          }

          clean_msi_list (current_dev_mvec);
        }
      } else {
        val_print (ACS_PRINT_DEBUG, "\n       Invalid BDF 0x%x", current_dev_bdf);
      }
    }
    count--;
  }

  if (test_skip) {
    val_print(ACS_PRINT_ERR, "\n       No MSI vectors found ", 0);;
    val_set_status (index, RESULT_SKIP(TEST_NUM, 01));
  } else  if (!status) {
    val_set_status (index, RESULT_PASS(TEST_NUM, 01));
  }
}

uint32_t
os_p064_entry(uint32_t num_hart)
{
  uint32_t status = ACS_STATUS_FAIL;

  num_hart = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_hart);
  if (status != ACS_STATUS_SKIP) {
      val_run_test_payload(TEST_NUM, num_hart, payload, 0);
  }

  /* get the result from all HART and check for failure */
  status = val_check_for_error(TEST_NUM, num_hart, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
