/**
 * @file Segmentation.h
 * @author Sina Karvandi (sina@hyperdbg.org)
 * @brief Functions for handling memory segmentations
 * @details
 * @version 0.9
 * @date 2024-06-03
 *
 * @copyright This project is released under the GNU Public License v3.
 *
 */
#pragma once

//////////////////////////////////////////////////
//				     Constants		      		//
//////////////////////////////////////////////////

/**
 * @brief Whether the hypervisor should use the default OS's
 * GDT as the host GDT in VMCS or not
 *
 */
#define USE_DEFAULT_OS_GDT_AS_HOST_GDT FALSE

/**
 * @brief Maximum number of entries in GDT
 *
 */
#define HOST_GDT_DESCRIPTOR_COUNT 4

//////////////////////////////////////////////////
//					Functions					//
//////////////////////////////////////////////////

_Success_(return)
BOOLEAN
SegmentGetDescriptor(_In_ PUCHAR GdtBase, _In_ UINT16 Selector, _Out_ PVMX_SEGMENT_SELECTOR SegmentSelector);
