
#include "smu.h"

//#include <cpuid.h>
//#include <fcntl.h>
//#include <sys/mman.h>
//#include <unistd.h>
//#include <pci/pci.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
extern "C"{
#include<pci/pci.h>
}
#include <cpuid.h>

Smu::Smu()
{

}

Smu::~Smu()
{

}

u32 Smu::smu_service_req(smu_t smu ,u32 id ,smu_service_args_t *args)
{
    u32 response = 0x0;
    DBG("SMU_SERVICE REQ_ID:0x%x\n", id);
    DBG("SMU_SERVICE REQ: arg0: 0x%x, arg1:0x%x, arg2:0x%x, arg3:0x%x, arg4: 0x%x, arg5: 0x%x\n",  \
        args->arg0, args->arg1, args->arg2, args->arg3, args->arg4, args->arg5);

    /* Clear the response */
    smn_reg_write(smu->nb, smu->rep, 0x0);
    /* Pass arguments */
    smn_reg_write(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 0), args->arg0);
    smn_reg_write(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 1), args->arg1);
    smn_reg_write(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 2), args->arg2);
    smn_reg_write(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 3), args->arg3);
    smn_reg_write(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 4), args->arg4);
    smn_reg_write(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 5), args->arg5);
    /* Send message ID */
    smn_reg_write(smu->nb, smu->msg, id);
    /* Wait until reponse changed */
    while(response == 0x0) {
        response = smn_reg_read(smu->nb, smu->rep);
    }
    /* Read back arguments */
    args->arg0 = smn_reg_read(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 0));
    args->arg1 = smn_reg_read(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 1));
    args->arg2 = smn_reg_read(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 2));
    args->arg3 = smn_reg_read(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 3));
    args->arg4 = smn_reg_read(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 4));
    args->arg5 = smn_reg_read(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 5));

    DBG("SMU_SERVICE REP: REP: 0x%x, arg0: 0x%x, arg1:0x%x, arg2:0x%x, arg3:0x%x, arg4: 0x%x, arg5: 0x%x\n",  \
        response, args->arg0, args->arg1, args->arg2, args->arg3, args->arg4, args->arg5);

    return response;
}

int Smu::smu_service_test(smu_t smu)
{
    u32 response = 0x0;

    /* Clear the response */
    smn_reg_write(smu->nb, smu->rep, 0x0);
    /* Test message with unique argument */
    smn_reg_write(smu->nb, smu->arg_base, 0x47);
    if(smn_reg_read(smu->nb, smu->arg_base) != 0x47){
        printf("PCI Bus is not writeable, check secure boot\n");
        return 0;
    }

    /* Send message ID */
    smn_reg_write(smu->nb, smu->msg, SMU_TEST_MSG);
    /* Wait until reponse changed */
    while(response == 0x0) {
        response = smn_reg_read(smu->nb, smu->rep);
    }

    return response == REP_MSG_OK;
}

smu_t Smu::get_smu(nb_t nb, int smu_type) {
    smu_t smu;

    smu = (smu_t)malloc((sizeof(*smu)));
    smu->nb = nb;

    enum ryzen_family family = cpuid_get_family();

    /* Fill SMU information */
    switch(smu_type){
        case TYPE_MP1:
            if (family == FAM_REMBRANDT || family == FAM_VANGOGH) {
                smu->msg = MP1_C2PMSG_MESSAGE_ADDR_2;
                smu->rep = MP1_C2PMSG_RESPONSE_ADDR_2;
                smu->arg_base = MP1_C2PMSG_ARG_BASE_2;
            } else {
                smu->msg = MP1_C2PMSG_MESSAGE_ADDR_1;
                smu->rep = MP1_C2PMSG_RESPONSE_ADDR_1;
                smu->arg_base = MP1_C2PMSG_ARG_BASE_1;
            }
            break;
        case TYPE_PSMU:
            smu->msg = PSMU_C2PMSG_MESSAGE_ADDR;
            smu->rep = PSMU_C2PMSG_RESPONSE_ADDR;
            smu->arg_base = PSMU_C2PMSG_ARG_BASE;
            break;
        default:
            DBG("Failed to get SMU, unknown SMU_TYPE: %i\n", smu_type);
            goto err;
            break;
    }

    if(smu_service_test(smu)){
        return smu;
    } else {
        DBG("Faild to get SMU, SMU_TYPE: %i\n", smu_type);
        goto err;
    }
err:
    free_smu(smu);
    return NULL;
}

void Smu::free_smu(smu_t smu) {
    free((void *)smu);
}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool mem_obj_obj = true;
int pm_table_fd = 0;
void *phy_map = MAP_FAILED;

pci_obj_t Smu::init_pci_obj(){
    pci_obj_t obj;
    obj = pci_alloc();
    pci_init(obj);
    return obj;
}

nb_t Smu::get_nb(pci_obj_t obj){
    nb_t nb;
    nb = pci_get_dev(obj, 0, 0, 0, 0);
    pci_fill_info(nb, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);
    return nb;
}

void Smu::free_nb(nb_t nb){
    pci_free_dev(nb);
}


void Smu::free_pci_obj(pci_obj_t obj){
    pci_cleanup(obj);
}

u32 Smu::smn_reg_read(nb_t nb, u32 addr)
{
    pci_write_long(nb, NB_PCI_REG_ADDR_ADDR, (addr & (~0x3)));
    return pci_read_long(nb, NB_PCI_REG_DATA_ADDR);
}

void Smu::smn_reg_write(nb_t nb, u32 addr, u32 data)
{
    pci_write_long(nb, NB_PCI_REG_ADDR_ADDR, addr);
    pci_write_long(nb, NB_PCI_REG_DATA_ADDR, data);
}

mem_obj_t Smu::init_mem_obj(u32 physAddr)
{
    int dev_mem_fd;
    int dev_mem_errno, mmap_errno;

    //It is to complicated to check PAT, CONFIG_NONPROMISC_DEVMEM, CONFIG_STRICT_DEVMEM or other dependencies, just try to open /dev/mem
    dev_mem_fd = open("/dev/mem", O_RDONLY);
    dev_mem_errno = errno;
    if (dev_mem_fd > 0){
        phy_map = mmap(NULL, 0x1000, PROT_READ, MAP_SHARED, dev_mem_fd, physAddr);
        mmap_errno = errno;
        close(dev_mem_fd);
    }

    if(phy_map == MAP_FAILED){
        //only complain about mmap errors if we don't have access to alternative smu driver
        pm_table_fd = open("/sys/kernel/ryzen_smu_drv/pm_table", O_RDONLY);
        if (pm_table_fd < 0) {
            printf("failed to get /sys/kernel/ryzen_smu_drv/pm_table: %s\n", strerror(errno));

            //show either fd error or mmap error, depending on which was the problem
            if(dev_mem_errno) {
                printf("failed to get /dev/mem: %s\n", strerror(dev_mem_errno));
            } else {
                printf("failed to map /dev/mem: %s\n", strerror(mmap_errno));
            }

            if(mmap_errno == EPERM || dev_mem_fd < 0) {
                //we are already superuser if memory access is requested because we did successfully do the other smu calls.
                //missing /dev/mem or missing permissions can only mean memory access policy
                printf("If you don't want to change your memory access policy, you need a kernel module for this task.\n");
                printf("We do support usage of this kernel module https://gitlab.com/leogx9r/ryzen_smu\n");
            }
            return NULL;
        }
    }

    return &mem_obj_obj;
}

void Smu::free_mem_obj(mem_obj_t obj)
{
    if(phy_map != MAP_FAILED){
        munmap(phy_map, 0x1000);
    }
    if(pm_table_fd > 0) {
        close(pm_table_fd);
    }
    return;
}

int Smu::copy_pm_table(void *buffer, size_t size)
{
    int read_size;

    if(pm_table_fd > 0){
        lseek(pm_table_fd, 0, SEEK_SET);
        read_size = read(pm_table_fd, buffer, size);
        if(read_size == -1) {
            printf("failed to get pm_table from ryzen_smu kernel module: %s\n", strerror(errno));
            return -1;
        }
        return 0;
    }

    if(phy_map != MAP_FAILED){
        memcpy(buffer, phy_map, size);
        return 0;
    }

    printf("failed to get pm_table from /dev/mem\n");
    return -1;
}

int Smu::compare_pm_table(void *buffer, size_t size)
{
    if(pm_table_fd > 0){
        //we can not compare to physial pm table location because we don't have direct memory access via SMU driver
        return 0;
    }
    return memcmp(buffer, phy_map, size);
}

bool Smu::is_using_smu_driver()
{
    return pm_table_fd > 0;
}



void Smu::getcpuid(unsigned int CPUInfo[4], unsigned int InfoType)
{
    __cpuid(InfoType, CPUInfo[0],CPUInfo[1],CPUInfo[2],CPUInfo[3]);
}

enum ryzen_family Smu::cpuid_get_family()
{
    uint32_t regs[4];
    int family, model;
    char vendor[4 * 4];

    getcpuid(regs, 0);

    /* Hack Alert! Put into str buffer */
    *(uint32_t *) &vendor[0] = regs[1];
    *(uint32_t *) &vendor[4] = regs[3];
    *(uint32_t *) &vendor[8] = regs[2];

    if (strncmp((char *) &vendor, CPUID_VENDOR_AMD , sizeof(CPUID_VENDOR_AMD) - 1)) {
        printf("Not AMD processor, must be kidding\n");
        return FAM_UNKNOWN;
    }

    getcpuid(regs, 1);

    family = ((regs[0] >> 8) & 0xf) + ((regs[0] >> 20) & 0xff);
    model = ((regs[0] >> 4) & 0xf) | ((regs[0] >> 12) & 0xf0);

    switch (family) {
    case 0x17: /* Zen, Zen+, Zen2 */
        switch (model) {
        case 17:
            return FAM_RAVEN;
        case 24:
            return FAM_PICASSO;
        case 32:
            return FAM_DALI;
        case 96:
            return FAM_RENOIR;
        case 104:
            return FAM_LUCIENNE;
        case 144:
            return FAM_VANGOGH;
        default:
            printf("Fam%xh: unsupported model %d\n", family, model);
            break;
        };
        break;

    case 0x19: /* Zen3 */
        switch (model) {
        case 80:
            return FAM_CEZANNE;
        case 64:
        case 68:
            return FAM_REMBRANDT;
        default:
            printf("Fam%xh: unsupported model %d\n", family, model);
            break;
        };
        break;

    default:
        printf("Unsupported family: %xh\n", family);
        break;
    }

    printf("Only Ryzen Mobile Series are supported\n");
    return FAM_UNKNOWN;
}

