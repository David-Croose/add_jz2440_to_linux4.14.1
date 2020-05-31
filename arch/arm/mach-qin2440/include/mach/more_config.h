#ifndef _MORE_CONFIG_H_
#define _MORE_CONFIG_H_

/*
 * nand flash mtd partitions infomation
 */
struct qin2440_mtd_info {
	unsigned int total;
	struct mtd_partition *partition;
};

#endif
