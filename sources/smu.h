#ifndef SMU_H
#define SMU_H

#include "stdint.h"

#include <QDebug>

typedef struct pci_dev *nb_t;
typedef struct pci_access *pci_obj_t;
typedef bool *mem_obj_t;

#define u32 uint32_t
#define DBG //qDebug
#define C2PMSG_ARGx_ADDR(y, x)          (y + 4 * x)

#define CPUID_VENDOR_AMD          "AuthenticAMD"

#define NB_PCI_REG_ADDR_ADDR 0xB8
#define NB_PCI_REG_DATA_ADDR 0xBC

#define MP1_C2PMSG_MESSAGE_ADDR_1        0x3B10528
#define MP1_C2PMSG_RESPONSE_ADDR_1       0x3B10564
#define MP1_C2PMSG_ARG_BASE_1            0x3B10998

/* For Vangogh and Rembrandt */
#define MP1_C2PMSG_MESSAGE_ADDR_2        0x3B10528
#define MP1_C2PMSG_RESPONSE_ADDR_2       0x3B10578
#define MP1_C2PMSG_ARG_BASE_2            0x3B10998

#define PSMU_C2PMSG_MESSAGE_ADDR          0x3B10a20
#define PSMU_C2PMSG_RESPONSE_ADDR         0x3B10a80
#define PSMU_C2PMSG_ARG_BASE              0x3B10a88

#define REP_MSG_OK                    0x1
#define REP_MSG_Failed                0xFF
#define REP_MSG_UnknownCmd            0xFE
#define REP_MSG_CmdRejectedPrereq     0xFD
#define REP_MSG_CmdRejectedBusy       0xFC

/*
* All the SMU have the same TestMessage as for now
* Correct me if they don't
*/
#define SMU_TEST_MSG 0x1

typedef struct _smu_t {
    nb_t nb;
    u32 msg;
    u32 rep;
    u32 arg_base;
} *smu_t;

typedef struct _smu_service_args_t {
        u32 arg0;
        u32 arg1;
        u32 arg2;
        u32 arg3;
        u32 arg4;
        u32 arg5;
} smu_service_args_t;

enum ryzen_family {
        FAM_UNKNOWN = -1,
        FAM_RAVEN = 0,
        FAM_PICASSO,
        FAM_RENOIR,
        FAM_CEZANNE,
        FAM_DALI,
        FAM_LUCIENNE,
        FAM_VANGOGH,
        FAM_REMBRANDT,
        FAM_END
};

enum SMU_TYPE{
    TYPE_MP1,
    TYPE_PSMU,
    TYPE_COUNT,
};

class Smu
{
public:
    Smu();
    ~Smu();

    u32 smu_service_req(smu_t smu ,u32 id ,smu_service_args_t *args);
    int smu_service_test(smu_t smu);
    smu_t get_smu(nb_t nb, int smu_type);
    void free_smu(smu_t smu);

    pci_obj_t init_pci_obj();
    nb_t get_nb(pci_obj_t obj);
    void free_nb(nb_t nb);
    void free_pci_obj(pci_obj_t obj);
    u32 smn_reg_read(nb_t nb, u32 addr);
    void smn_reg_write(nb_t nb, u32 addr, u32 data);
    mem_obj_t init_mem_obj(u32 physAddr);
    void free_mem_obj(mem_obj_t obj);
    int copy_pm_table(void *buffer, size_t size);
    int compare_pm_table(void *buffer, size_t size);
    bool is_using_smu_driver();

    void getcpuid(unsigned int CPUInfo[4], unsigned int InfoType);
    enum ryzen_family cpuid_get_family();
};

#endif // SMU_H
