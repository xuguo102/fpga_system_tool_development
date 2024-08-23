/*
* Copyright (C) 2013-2022  Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in this
* Software without prior written authorization from Xilinx.
*
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


#include <sys/ioctl.h>         // Include IOCTL calls to access Kernel Mode Driver
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "/home/matthew/vpk120_petalinux/xilinx-vpk120-2023.2/project-spec/meta-user/recipes-apps/fpgapcieapp/files/arm_pcie.h"

/*#define TANSFER_SIZE 5*1024*1024*/
/*#define TANSFER_SIZE 100*1024*/
#define TANSFER_SIZE 1024

/*#define AER_TEST 1*/

struct Config
{
	// NOTE(michiel): Config offsets
	uint32_t pmOffset;
	uint32_t msiOffset;
	uint32_t msixOffset;
	uint32_t pcieCapOffset;
	uint32_t deviceCapOffset;
	uint32_t deviceStatContOffset;
	uint32_t linkCapOffset;
	uint32_t linkStatContOffset;

	// NOTE(michiel): Register values
	uint32_t linkWidthCap;
	uint32_t linkSpeedCap;
	uint32_t linkWidth;
	uint32_t linkSpeed;
	uint32_t linkControl;
	uint32_t pmStatControl;
	uint32_t pmCapabilities;
	uint32_t msiControl;
};

struct RegValueRead
{
    XbmDmaControlReg reg;
    const char *name;
};

static struct RegValueRead gRegReads[] = {
    {Reg_DeviceCS, "Device Control Status"},
    {Reg_DeviceDMACS, "DMA Control Status"},
    {Reg_WriteTlpAddress, "Write Tlp Address"},
    {Reg_WriteTlpSize, "Write Tlp Size"},
    {Reg_WriteTlpCount, "Write Tlp Count"},
    {Reg_WriteTlpPattern, "Write Tlp Pattern"},
    {Reg_ReadTlpPattern, "Read Tlp Pattern"},
    {Reg_ReadTlpAddress, "Read Tlp Address"},
    {Reg_ReadTlpSize, "Read Tlp Size"},
    {Reg_ReadTlpCount, "Read Tlp Count"},
    {Reg_WriteDMAPerf, "Write DMA Perf"},
    {Reg_ReadDMAPerf, "Read DMA Perf"},
    {Reg_ReadComplStatus, "Read Completion Status"},
    {Reg_ComplWithData, "Completion With Data"},
    {Reg_ComplSize, "Completion Size"},
    {Reg_DeviceLinkWidth, "Device Link Width"},
    {Reg_DeviceLinkTlpSize, "Device Link Tlp Size"},
    {Reg_DeviceMiscControl, "Device Misc Control"},
    {Reg_DeviceMSIControl, "Device MSI Control"},
    {Reg_DeviceDirectedLinkChange, "Device Directed Link Change"},
    {Reg_DeviceFCControl, "Device FC Control"},
    {Reg_DeviceFCPostedInfo, "Device FC Posted Info"},
    {Reg_DeviceFCNonPostedInfo, "Device FC Non-Posted Info"},
    {Reg_DeviceFCCompletionInfo, "Device FC Completion Info"},
};

#define DO_READ_REG(i) { \
    struct RegValueRead reader = gRegReads[i]; \
      unsigned int reg_value = reader.reg; \
        if (ioctl(g_devFile, PBE_IOC_RD_BMD_REG, &reg_value) < 0) { \
        printf(" reader.name %s read failed \n", reader.name); \
        return -1; \
    } else { \
            printf(" reader.name %s value 0x%08x \n",reader.name, reg_value); \
    } \
}

//--- read_bmd_regs(): Reads PBE regs and outputs to bmd_regs.txt file
//--- Arguments:  int device file number
//--- Return Value: Returns int SUCCESS or FAILURE
//--- Detailed Description: This module does the following:
//---                       1) Reads all the PBE descriptor registers and outputs those values to a text file
//---                          so that it can be displayed in the GUI under the Read_BMD tab
int read_bmd_regs(int g_devFile) {

    // Switch statement reads PBE descriptor register values sequentially and outputs to log file.  We use a switch
    // statement so we can give actual descriptor register names rather than solely offsets from base.
    printf("*** PBE Register Values ***\n");
    DO_READ_REG(0);
    DO_READ_REG(1);
    DO_READ_REG(2);
    DO_READ_REG(3);
    DO_READ_REG(4);
    DO_READ_REG(5);
    DO_READ_REG(6);
    DO_READ_REG(7);
    DO_READ_REG(8);
    DO_READ_REG(9);
    DO_READ_REG(10);
    DO_READ_REG(11);
    DO_READ_REG(12);
    DO_READ_REG(13);
    DO_READ_REG(14);
    DO_READ_REG(15);
    DO_READ_REG(16);
    DO_READ_REG(17);
    DO_READ_REG(18);
    DO_READ_REG(19);
    printf("*** End PBE Register Space ***\n");
    return 1;
  }

#undef DO_READ_REG

int write_data(int file, uint32_t size, const void *buffer)
{
	return write(file, buffer, size);
}

int read_data(int file, uint32_t size, void *buffer)
{
	return read(file, buffer, size);
}

int main(int argc, char **argv)
{

	int fd = 0;
    uint32_t i = 0;
	bool doRead = false;
	struct Config config;

	uint32_t regValue = 0;
	uint32_t maxPayloadSize = 0;
	uint32_t tlpSizeMax = 0;


	uint32_t writeTLPSize = 0;
	uint32_t writeTLPCount = 0;
	uint32_t readTLPSize = 0;
	uint32_t readTLPCount = 0;
	uint32_t testpattern = 0xfeadbeef;

	uint32_t dmaControlReg = 0;

	uint32_t writeWRRCount = 0; // 1;
	uint32_t readWRRCount = 0; //1;
	uint32_t miscControl = 0;

	uint32_t readEnable = 0;
	uint32_t writeEnable = 0;

	uint32_t dmaControlWrite = 0;
	uint32_t dmaControlRead = 0;

    bool writeError = false;

    uint32_t readData = 0;
    uint32_t writeData = 0;

	int32_t linkWidthMultiplier = 0;
	int32_t trnClks = 0;
	int32_t tempWrMbps = 0;
	int32_t tempRdMbps = 0;

    uint32_t aer_regValue = 0;

	char *gen = 0;

	fd = open("/dev/amdpcie", O_RDWR);
	if (fd <= 0) {
		printf("Could not open file /dev/amdpcie\n");
		return -1;
	}

	if (argc > 1) {
		doRead = true;
        readEnable = true;
		printf("*************** Do Read, argc : %d ****************\n", argc);
	} else {
		doRead = false;
        writeEnable = true;
		printf("*************** Do Write, argc : %d ****************\n", argc);
    }

	maxPayloadSize = 512;
	tlpSizeMax = 512;

#if 0
	if (ioctl(fd, PBE_IOC_RD_CFG_REG, &regValue) < 0) {
		printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_RD_CFG_REG);
		return -1;
	} else {
		maxPayloadSize = (regValue & 0x000000E0) >> 5;
		printf("Max payload size: %d \n", maxPayloadSize);
	}
#endif

	switch (maxPayloadSize) {
		// NOTE(michiel): 128 maxPayloadSize;
		case 0: { tlpSizeMax = 5; maxPayloadSize = 128; } break;
			// NOTE(michiel): 256 maxPayloadSize;
		case 1: { tlpSizeMax = 6; maxPayloadSize = 256; } break;
			// NOTE(michiel): 512 maxPayloadSize;
		case 2: { tlpSizeMax = 7; maxPayloadSize = 512; } break;
			// NOTE(michiel): 1024 maxPayloadSize;
		case 3: { tlpSizeMax = 8; maxPayloadSize = 1024; } break;
			// NOTE(michiel): 2048 maxPayloadSize;
		case 4: { tlpSizeMax = 9; maxPayloadSize = 2048; } break;
			// NOTE(michiel): 4096 maxPayloadSize;
		case 5: { tlpSizeMax = 10; maxPayloadSize = 4096; } break;
		default: { printf("Max payload size is invalid\n"); } break;
	}

	printf("Max payload: %d, tlp max size: %d \n", maxPayloadSize, tlpSizeMax);

#ifdef AER_TEST
    //error injection for MalformTLP
    writeTLPSize = maxPayloadSize/2;
#else
    writeTLPSize = maxPayloadSize/4;
#endif
	writeTLPCount = TANSFER_SIZE/maxPayloadSize;
	readTLPSize = maxPayloadSize/4;
	readTLPCount = TANSFER_SIZE/maxPayloadSize;

	// NOTE(michiel): Setup DMA
	if (ioctl(fd, PBE_IOC_RESET, 0) < 0) {
		printf("IOCTL failed setting reset\n");
		return -1;
	} else {
		printf("DMA Reset\n");
	}

/*#ifdef AER_TEST*/
#if 0
    aer_regValue = 11;
    if (ioctl(fd, PBE_IOC_WRITE_COR_ERRORINJECT, aer_regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WRITE_COR_ERRORINJECT);
    } else {
        printf("Set COR Error Register value : 0x%08x \n", aer_regValue);
    }

    if (ioctl(fd, PBE_IOC_READ_COR_ERRORINJECT, &aer_regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_COR_ERRORINJECT);
    } else {
        printf("Get COR Error Register value : 0x%08x \n", aer_regValue);
    }
#endif

	if (ioctl(fd, PBE_IOC_READ_DMA_CTRL, &dmaControlReg) < 0) {
		printf("IOCTL failed reading DMA Control\n");
		return -1;
	} else {
		printf("DMA Control: 0x%08X \n", dmaControlReg);
	}

	if (ioctl(fd, PBE_IOC_WRITE_WR_PTRN, testpattern) < 0) {
		printf("IOCTL failed setting the write TLP count\n");
		return -1;
	}

	if (ioctl(fd, PBE_IOC_WRITE_WR_COUNT, writeTLPCount) < 0) {
		printf("IOCTL failed setting the write TLP count\n");
		return -1;
	}

	if (ioctl(fd, PBE_IOC_WRITE_WR_LEN, writeTLPSize) < 0) {
		printf("IOCTL failed setting the write TLP size\n");
	}

	if (ioctl(fd, PBE_IOC_WRITE_RD_PTRN, testpattern) < 0) {
		printf("IOCTL failed setting the read TLP count\n");
		return -1;
	}

	if (ioctl(fd, PBE_IOC_WRITE_RD_COUNT, readTLPCount) < 0) {
		printf("IOCTL failed setting the read TLP count\n");
		return -1;
	}

	if (ioctl(fd, PBE_IOC_WRITE_RD_LEN, readTLPSize) < 0) {
		printf("IOCTL failed setting the read TLP size\n");
		return -1;
	}

	if (ioctl(fd, PBE_IOC_READ_WR_COUNT, &regValue) < 0) {
		printf("IOCTL failed reading the write TLP count\n");
		return -1;
	} else {
		printf("Write TLP count: %u, expected  %u \n", regValue, writeTLPCount);
	}

	if (ioctl(fd, PBE_IOC_READ_WR_LEN, &regValue) < 0) {
		printf("IOCTL failed reading the write TLP size\n");
		return -1;
	} else {
		printf("Write TLP size: %u, expected  %u \n", regValue, writeTLPSize);
	}

	if (ioctl(fd, PBE_IOC_READ_RD_COUNT, &regValue) < 0) {
		printf("IOCTL failed reading the read TLP count\n");
		return -1;
	} else {
		printf("Read TLP count: %u, expected  %u \n", regValue, readTLPCount);
	}

#if 0
	writeWRRCount = 0; // 1;
	readWRRCount = 0; //1;
	// 0x01010000 or 0x01010020
	miscControl = (writeWRRCount << 24) | (readWRRCount << 16);

	if (ioctl(fd, PBE_IOC_WRITE_MISC_CTL, miscControl) < 0) {
		printf("IOCTL failed setting misc control\n");
		return -1;
	}

#endif
#ifdef AER_TEST
    aer_regValue = 14;
    if (ioctl(fd, PBE_IOC_WRITE_COR_ERRORINJECT, aer_regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WRITE_COR_ERRORINJECT);
    } else {
        printf("Set COR Error Register value : 0x%08x \n", aer_regValue);
    }

    if (ioctl(fd, PBE_IOC_READ_COR_ERRORINJECT, &aer_regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_COR_ERRORINJECT);
    } else {
        printf("Get COR Error Register value : 0x%08x \n", aer_regValue);
    }
#endif


	dmaControlReg |= (readEnable << 16) | writeEnable;
	if (ioctl(fd, PBE_IOC_WRITE_DMA_CTRL, dmaControlReg) < 0) {
		printf("IOCTL failed setting DMA control\n");
		return -1;
	}

#ifdef AER_TEST
    aer_regValue = 0;
    if (ioctl(fd, PBE_IOC_WRITE_COR_ERRORINJECT, aer_regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_WRITE_COR_ERRORINJECT);
    } else {
        printf("Set COR Error Register value : 0x%08x \n", aer_regValue);
    }

    if (ioctl(fd, PBE_IOC_READ_COR_ERRORINJECT, &aer_regValue) < 0) {
        printf("IOCTL failed reading 0x%03lX \n", PBE_IOC_READ_COR_ERRORINJECT);
    } else {
        printf("Get COR Error Register value : 0x%08x \n", aer_regValue);
    }
#endif

#ifdef AER_TEST
    return 0;
#endif

#if 1
    usleep(100000);
#else
	do {
		usleep(1000);
		if (ioctl(fd, PBE_IOC_READ_DMA_CTRL, &dmaControlReg) < 0) {
			printf("IOCTL failed reading DMA control\n");
			return -1;
		}
		printf("*** dmaControlReg : 0x%08x ***\n", dmaControlReg);
	} while ((readEnable && (dmaControlReg & 0x01010000)) ||
			(writeEnable && (dmaControlReg & 0x00000101)));
#endif

	if (ioctl(fd, PBE_IOC_READ_DMA_CTRL, &regValue) < 0) {
		printf("IOCTL failed reading DMA control\n");
		return -1;
	} else {
		dmaControlReg = regValue;
		dmaControlWrite = dmaControlReg & 0x0000FFFF;
		dmaControlRead = (dmaControlReg & 0xFFFF0000) >> 16;
	}

	read_bmd_regs(fd);

    close(fd);

	return 0;
}

