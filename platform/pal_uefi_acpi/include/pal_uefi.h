/** @file
 * Copyright (c) 2016-2023, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __PAL_UEFI_H__
#define __PAL_UEFI_H__

extern VOID* g_bsa_log_file_handle;
extern UINT32 g_print_level;
extern UINT32 g_print_mmio;
extern UINT32 g_curr_module;
extern UINT32 g_enable_module;
extern UINT32 g_pcie_p2p;
extern UINT32 g_pcie_cache_present;

#define ACS_PRINT_ERR   5      /* Only Errors. use this to de-clutter the terminal and focus only on specifics */
#define ACS_PRINT_WARN  4      /* Only warnings & errors. use this to de-clutter the terminal and focus only on specifics */
#define ACS_PRINT_TEST  3      /* Test description and result descriptions. THIS is DEFAULT */
#define ACS_PRINT_DEBUG 2      /* For Debug statements. contains register dumps etc */
#define ACS_PRINT_INFO  1      /* Print all statements. Do not use unless really needed */

#define PCIE_SUCCESS            0x00000000  /* Operation completed successfully */
#define PCIE_NO_MAPPING         0x10000001  /* A mapping to a Function does not exist */
#define PCIE_CAP_NOT_FOUND      0x10000010  /* The specified capability was not found */
#define PCIE_UNKNOWN_RESPONSE   0xFFFFFFFF  /* Function not found or UR response from completer */

#define NOT_IMPLEMENTED         0x4B1D  /* Feature or API by default unimplemented */
#define MEM_OFFSET_SMALL        0x10    /* Memory Offset from BAR base value that can be accesed*/

#define TYPE0_MAX_BARS  6
#define TYPE1_MAX_BARS  2

/* BAR registrer masks */
#define BAR_MIT_MASK    0x1
#define BAR_MDT_MASK    0x3
#define BAR_MT_MASK     0x1
#define BAR_BASE_MASK   0xfffffff

/* BAR register shifts */
#define BAR_MIT_SHIFT   0
#define BAR_MDT_SHIFT   1
#define BAR_MT_SHIFT    3
#define BAR_BASE_SHIFT  4

typedef enum {
  MMIO = 0,
  IO = 1
} BAR_MEM_INDICATOR_TYPE;

typedef enum {
  BITS_32 = 0,
  BITS_64 = 2
} BAR_MEM_DECODE_TYPE;

typedef struct {
  UINT64   Arg0;
  UINT64   Arg1;
  UINT64   Arg2;
  UINT64   Arg3;
  UINT64   Arg4;
  UINT64   Arg5;
  UINT64   Arg6;
  UINT64   Arg7;
} ARM_SMC_ARGS;

#define bsa_print(verbose, string, ...) if(verbose >= g_print_level) \
                                            Print(string, ##__VA_ARGS__)

/**
  Conduits for service calls (SMC vs HVC).
**/
#define CONDUIT_SMC       0
#define CONDUIT_HVC       1
#define CONDUIT_UNKNOWN  -1
#define CONDUIT_NONE     -2

typedef struct {
  UINT32 num_of_hart;
}HART_INFO_HDR;

/**
  @brief  structure instance for HART entry
**/
typedef struct {
  UINT32   hart_num;          ///< HART Index
  UINT64   hart_id;         ///< Hart ID (mhartid) of the hart
  UINT32   acpi_processor_uid;
  UINT32   ext_intc_id;     ///< The unique ID of the external interrupts connected to this hart.
  UINT64   imsic_base;      ///< Physical base address of the Incoming MSI Controller (IMSIC) MMIO region of this hart.
  UINT32   imsic_size;      ///< Size in bytes of the IMSIC MMIO region of this hart.
  UINT8    isa_string[512]; ///< Null-terminated ASCII Instruction Set Architecture (ISA) string for this hart.
}HART_INFO_ENTRY;

typedef struct {
  HART_INFO_HDR    header;
  HART_INFO_ENTRY  hart_info[];
}HART_INFO_TABLE;

VOID     pal_hart_data_cache_ops_by_va(UINT64 addr, UINT32 type);

#define CLEAN_AND_INVALIDATE  0x1
#define CLEAN                 0x2
#define INVALIDATE            0x3

/**
  @brief  IOMMU Info header - Summary of IOMMU subsytem
**/
typedef struct {
  UINT32  num_of_iommu;
} IOMMU_INFO_HDR;

typedef struct {
  UINT32  iommu_num;  ///< info entry index
  UINT8   type;
  UINT16  id;

  /* IOMMU device specific */
  UINT64  hardware_id;
  UINT64  base_address;
  UINT32  flags;

  /* PCIe Root Complex specific */
} IOMMU_INFO_ENTRY;

typedef struct {
  IOMMU_INFO_HDR    header;
  IOMMU_INFO_ENTRY  iommu_info[];
} IOMMU_INFO_TABLE;

typedef struct {
  UINT32   gic_version;
  UINT32   num_gicd;
  UINT32   num_gicc_rd;
  UINT32   num_gicr_rd;
  UINT32   num_its;
  UINT32   num_msi_frame;
  UINT32   num_gich;

  /* RV porting */
  UINT16   supervisor_intr_num;
  UINT16   guest_intr_num;
}GIC_INFO_HDR;

typedef enum {
  ENTRY_TYPE_CPUIF = 0x1000,
  ENTRY_TYPE_GICD,
  ENTRY_TYPE_GICC_GICRD,
  ENTRY_TYPE_GICR_GICRD,
  ENTRY_TYPE_GICITS,
  ENTRY_TYPE_GIC_MSI_FRAME,
  ENTRY_TYPE_GICH,

  /* RV porting */
  ENTRY_TYPE_RINTC,
  ENTRY_TYPE_PLIC
}GIC_INFO_TYPE_e;

/* Interrupt Trigger Type */
typedef enum {
  INTR_TRIGGER_INFO_LEVEL_LOW,
  INTR_TRIGGER_INFO_LEVEL_HIGH,
  INTR_TRIGGER_INFO_EDGE_FALLING,
  INTR_TRIGGER_INFO_EDGE_RISING
}INTR_TRIGGER_INFO_TYPE_e;

/**
  @brief  structure instance for IIC entry
**/
typedef struct {
  UINT32 type;
  UINT64 base;
  UINT32 entry_id;  /* This entry_id is used to tell component ID */
  UINT64 length;  /* This length is only used in case of Re-Distributor Range Address length */
  UINT32 flags;
  UINT32 spi_count;
  UINT32 spi_base;
}GIC_INFO_ENTRY;

/**
  @brief  IIC Information Table
**/
typedef struct {
  GIC_INFO_HDR   header;
  GIC_INFO_ENTRY gic_info[];  ///< Array of Information blocks - instantiated for each IIC type
}GIC_INFO_TABLE;

typedef struct {
  UINT32 s_el1_timer_flag;
  UINT32 ns_el1_timer_flag;
  UINT32 el2_timer_flag;
  UINT32 el2_virt_timer_flag;
  UINT32 s_el1_timer_gsiv;
  UINT32 ns_el1_timer_gsiv;
  UINT32 el2_timer_gsiv;
  UINT32 virtual_timer_flag;
  UINT32 virtual_timer_gsiv;
  UINT32 el2_virt_timer_gsiv;
  UINT32 num_platform_timer;
  UINT32 num_watchdog;
  UINT32 sys_timer_status;

  /* RV porting */
  UINT64 time_base_frequency;
}TIMER_INFO_HDR;

#define TIMER_TYPE_SYS_TIMER 0x2001

/**
  @brief  structure instance for TIMER entry
**/
typedef struct {
  UINT32 type;
  UINT32 timer_count;
  UINT64 block_cntl_base;
  UINT8  frame_num[8];
  UINT64 GtCntBase[8];
  UINT64 GtCntEl0Base[8];
  UINT32 gsiv[8];
  UINT32 virt_gsiv[8];
  UINT32 flags[8];
}TIMER_INFO_GTBLOCK;

typedef struct {
  TIMER_INFO_HDR     header;
  TIMER_INFO_GTBLOCK gt_info[];
}TIMER_INFO_TABLE;

/**
  @brief  Watchdog Info header - Summary of Watchdog subsytem
**/
typedef struct {
  UINT32 num_wd;  ///< number of Watchdogs present in the system
}WD_INFO_HDR;

/**
  @brief  structure instance for Watchdog entry
**/
typedef struct {
  UINT64 wd_ctrl_base;     ///< Watchdog Control Register Frame
  UINT64 wd_refresh_base;  ///< Watchdog Refresh Register Frame
  UINT32 wd_gsiv;          ///< Watchdog Interrupt ID
  UINT32 wd_flags;
}WD_INFO_BLOCK;

/**
  @brief Watchdog Info Table
**/
typedef struct {
  WD_INFO_HDR    header;
  WD_INFO_BLOCK  wd_info[];  ///< Array of Information blocks - instantiated for each WD Controller
}WD_INFO_TABLE;

/**
  @brief PCI Express Info Table
**/
typedef struct {
  UINT64   ecam_base;     ///< ECAM Base address
  UINT32   segment_num;   ///< Segment number of this ECAM
  UINT32   start_bus_num; ///< Start Bus number for this ecam space
  UINT32   end_bus_num;   ///< Last Bus number
}PCIE_INFO_BLOCK;

typedef struct {
  UINT32          num_entries;
  PCIE_INFO_BLOCK block[];
}PCIE_INFO_TABLE;

typedef enum {
  NON_PREFETCH_MEMORY = 0x0,
  PREFETCH_MEMORY = 0x1
}PCIE_MEM_TYPE_INFO_e;

VOID *pal_pci_bdf_to_dev(UINT32 bdf);
VOID pal_pci_read_config_byte(UINT32 bdf, UINT8 offset, UINT8 *data);

/**
  @brief  Instance of SMMU INFO block
**/
typedef struct {
  UINT32 arch_major_rev;  ///< Version 1 or 2 or 3
  UINT64 base;              ///< SMMU Controller base address
}SMMU_INFO_BLOCK;

typedef struct {
  UINT32 segment;
  UINT32 ats_attr;
  UINT32 cca;             //Cache Coherency Attribute
  UINT64 smmu_base;
}IOVIRT_RC_INFO_BLOCK;

typedef struct {
  UINT64 base;
  UINT32 overflow_gsiv;
  UINT32 node_ref;
} IOVIRT_PMCG_INFO_BLOCK;

typedef enum {
  IOVIRT_NODE_ITS_GROUP = 0x00,
  IOVIRT_NODE_NAMED_COMPONENT = 0x01,
  IOVIRT_NODE_PCI_ROOT_COMPLEX = 0x02,
  IOVIRT_NODE_SMMU = 0x03,
  IOVIRT_NODE_SMMU_V3 = 0x04,
  IOVIRT_NODE_PMCG = 0x05
}IOVIRT_NODE_TYPE;

typedef enum {
  IOVIRT_FLAG_DEVID_OVERLAP_SHIFT,
  IOVIRT_FLAG_STRID_OVERLAP_SHIFT,
  IOVIRT_FLAG_SMMU_CTX_INT_SHIFT,
}IOVIRT_FLAG_SHIFT;

typedef struct {
  UINT32 input_base;
  UINT32 id_count;
  UINT32 output_base;
  UINT32 output_ref;
}ID_MAP;

typedef union {
  UINT32 id[4];
  ID_MAP map;
}NODE_DATA_MAP;

#define MAX_NAMED_COMP_LENGTH 256

typedef union {
  CHAR8 name[MAX_NAMED_COMP_LENGTH];
  IOVIRT_RC_INFO_BLOCK rc;
  IOVIRT_PMCG_INFO_BLOCK pmcg;
  UINT32 its_count;
  SMMU_INFO_BLOCK smmu;
}NODE_DATA;

typedef struct {
  UINT32 type;
  UINT32 num_data_map;
  NODE_DATA data;
  UINT32 flags;
  NODE_DATA_MAP data_map[];
}IOVIRT_BLOCK;

typedef struct {
  UINT32 num_blocks;
  UINT32 num_smmus;
  UINT32 num_pci_rcs;
  UINT32 num_named_components;
  UINT32 num_its_groups;
  UINT32 num_pmcgs;
  IOVIRT_BLOCK blocks[];
}IOVIRT_INFO_TABLE;

#define IOVIRT_NEXT_BLOCK(b) (IOVIRT_BLOCK *)((UINT8*)(&b->data_map[0]) + b->num_data_map * sizeof(NODE_DATA_MAP))
#define IOVIRT_CCA_MASK ~((UINT32)0)

/**
  @brief SMMU Info Table
**/
typedef struct {
  UINT32          smmu_num_ctrl;       ///< Number of SMMU Controllers in the system
  SMMU_INFO_BLOCK smmu_block[]; ///< Array of Information blocks - instantiated for each SMMU Controller
}SMMU_INFO_TABLE;

typedef struct {
  UINT32    num_usb;   ///< Number of USB  Controllers
  UINT32    num_sata;  ///< Number of SATA Controllers
  UINT32    num_uart;  ///< Number of UART Controllers
  UINT32    num_all;   ///< Number of all PCI Controllers
}PERIPHERAL_INFO_HDR;

typedef enum {
  PERIPHERAL_TYPE_USB = 0x2000,
  PERIPHERAL_TYPE_SATA,
  PERIPHERAL_TYPE_UART,
  PERIPHERAL_TYPE_OTHER,
  PERIPHERAL_TYPE_NONE
}PER_INFO_TYPE_e;

/**
  @brief  Instance of peripheral info
**/
typedef struct {
  PER_INFO_TYPE_e  type;  ///< PER_INFO_TYPE
  UINT32         bdf;   ///< Bus Device Function
  UINT64         base0; ///< Base Address of the controller
  UINT64         base1; ///< Base Address of the controller
  UINT32         width; ///< Access width
  UINT32         irq;   ///< IRQ to install an ISR
  UINT32         flags;
  UINT32         msi;   ///< MSI Enabled
  UINT32         msix;  ///< MSIX Enabled
  UINT32         max_pasids;
  UINT32         baud_rate;
  UINT32         interface_type;
  UINT32         platform_type;
}PERIPHERAL_INFO_BLOCK;

#define PLATFORM_TYPE_ACPI   0x0
#define PLATFORM_TYPE_DT     0x1

/**
  @brief Peripheral Info Structure
**/
typedef struct {
  PERIPHERAL_INFO_HDR     header;
  PERIPHERAL_INFO_BLOCK   info[]; ///< Array of Information blocks - instantiated for each peripheral
}PERIPHERAL_INFO_TABLE;

/**
  @brief MSI(X) controllers info structure
**/

typedef struct {
  UINT32         vector_upper_addr; ///< Bus Device Function
  UINT32         vector_lower_addr; ///< Base Address of the controller
  UINT32         vector_data;       ///< Base Address of the controller
  UINT32         vector_control;    ///< IRQ to install an ISR
  UINT32         vector_irq_base;   ///< Base IRQ for the vectors in the block
  UINT32         vector_n_irqs;     ///< Number of irq vectors in the block
  UINT32         vector_mapped_irq_base; ///< Mapped IRQ number base for this MSI
}PERIPHERAL_VECTOR_BLOCK;

typedef struct PERIPHERAL_VECTOR_LIST_STRUCT
{
  PERIPHERAL_VECTOR_BLOCK vector;
  struct PERIPHERAL_VECTOR_LIST_STRUCT *next;
}PERIPHERAL_VECTOR_LIST;

UINT32 pal_get_msi_vectors (UINT32 seg, UINT32 bus, UINT32 dev, UINT32 fn, PERIPHERAL_VECTOR_LIST **mvector);

#define LEGACY_PCI_IRQ_CNT 4  // Legacy PCI IRQ A, B, C. and D
#define MAX_IRQ_CNT 0xFFFF    // This value is arbitrary and may have to be adjusted

typedef struct {
  UINT32  irq_list[MAX_IRQ_CNT];
  UINT32  irq_count;
} PERIFERAL_IRQ_LIST;

typedef struct {
  PERIFERAL_IRQ_LIST  legacy_irq_map[LEGACY_PCI_IRQ_CNT];
} PERIPHERAL_IRQ_MAP;

UINT32 pal_pcie_get_root_port_bdf(UINT32 *seg, UINT32 *bus, UINT32 *dev, UINT32 *func);
UINT32 pal_pcie_max_pasid_bits(UINT32 bdf);

/* Memory INFO table */

#define MEM_INFO_TBL_MAX_ENTRY  500 /* Maximum entries to be added in Mem info table*/

typedef enum {
  MEMORY_TYPE_DEVICE = 0x1000,
  MEMORY_TYPE_NORMAL,
  MEMORY_TYPE_RESERVED,
  MEMORY_TYPE_NOT_POPULATED,
  MEMORY_TYPE_LAST_ENTRY
}MEM_INFO_TYPE_e;


typedef struct {
  MEM_INFO_TYPE_e type;
  UINT64        phy_addr;
  UINT64        virt_addr;
  UINT64        size;
  UINT64        flags;  //To Indicate Cacheablility etc..
}MEM_INFO_BLOCK;


typedef struct {
  UINT64  dram_base;
  UINT64  dram_size;
  MEM_INFO_BLOCK  info[];
} MEMORY_INFO_TABLE;


VOID  pal_memory_create_info_table(MEMORY_INFO_TABLE *memoryInfoTable);

VOID    *pal_mem_alloc(UINT32 size);
VOID    *pal_mem_calloc(UINT32 num, UINT32 size);
VOID    *pal_mem_alloc_cacheable(UINT32 bdf, UINT32 size, VOID **pa);
VOID    pal_mem_free_cacheable(UINT32 bdf, UINT32 size, VOID *va, VOID *pa);
VOID    *pal_mem_virt_to_phys(VOID *va);
VOID    *pal_mem_phys_to_virt(UINT64 pa);
UINT64  pal_memory_get_unpopulated_addr(UINT64 *addr, UINT32 instance);

VOID    pal_mem_free(VOID *buffer);
UINT32  pal_hart_get_num();

typedef struct {
  UINT32    num_smbios_structure;
  UINT64    mc_host_if_tbl_addr;  // Management Controller Host Interface (Type 42)
  UINT32    mc_host_if_type;
  UINT64    ipmi_device_info_tbl_addr; // IPMI Device Information (Type 38)
  UINT32    ipmi_device_if_type;
}MNG_INFO_TABLE;

#endif
