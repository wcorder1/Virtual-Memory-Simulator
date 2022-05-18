#include <stdio.h>
#include <stdlib.h>
#include "vm.h"
#include "API.h"
#include "list.h"


struct Node * PFN_queue = NULL;
int fifo()
{
		int PFN;

		// Head of PFN_queue is used
		PFN = PFN_queue->data;

		// Remove head of PFN queue and rotates PFN to tail
		PFN_queue = list_remove_head(PFN_queue);
		PFN_queue = list_insert_tail(PFN_queue, PFN);

		return PFN;
}

int lru()
{
		int PFN;

		// Head of PFN_queue is used
		PFN = PFN_queue->data;
		
		// Remove head of PFN queue and rotates PFN to tail
		PFN_queue = list_remove_head(PFN_queue);
		PFN_queue = list_insert_tail(PFN_queue, PFN);

		return PFN;
}


int p = 0;
int clock() 
{
	int PFN = p % MAX_PFN;

	while (true) {

		if (clockArray[PFN] == 0) {
			p++;
			return PFN;
		} 
		
		else if (clockArray[PFN] == 1) 
		{
			clockArray[PFN] = 0;
			p++;
		}

	}
}

/*========================================================================*/

int find_replacement()
{
		int PFN;
		if(replacementPolicy == ZERO)  PFN = 0;
		else if(replacementPolicy == FIFO)  PFN = fifo();
		else if(replacementPolicy == LRU) PFN = lru();
		else if(replacementPolicy == CLOCK) PFN = clock();

		return PFN;
}

int pagefault_handler(int pid, int VPN, char type)
{
		int PFN;

		// find a free PFN.
		PFN = get_freeframe();

		if (PFN >= 0) {
			PFN_queue = list_insert_tail(PFN_queue, PFN); // Adds PFN to doubly linked list
		}

		// no free frame available. find a victim using page replacement. 
		else if (PFN < 0) {
				PFN = find_replacement();

		// Sets victim to [pid][VPN] of logical address occupying the found PFN
				IPTE victim = read_IPTE(PFN);

		// If victim is dirty then it's swapped out, returning changes to the original logical memory address
				if (read_PTE(victim.pid, victim.VPN).dirty) {
					swap_out(victim.pid, victim.VPN, PFN); // Moves value from physical memory to logical memory
				}

		// Changes page table entry of victim to invalid since logical memory is no longer linked to correct physical memory
				PTE victim_pte;
				victim_pte.valid = false;
				write_PTE(victim.pid, victim.VPN, victim_pte);
		}

		// New page table entry for logical memory being moved, sets valid to true and defines PFN
		PTE new_pte;
		new_pte.valid = true;
		new_pte.PFN = PFN;
		new_pte.dirty = false;

		IPTE new_ipte; // IPTE [PFE]  =  [pid, VPN]
		new_ipte.pid = pid;
		new_ipte.VPN = VPN;

		// If it's a write operation then the Page Table Entry is dirty
		if (type == 'W') {
			new_pte.dirty = true;
		}

		write_IPTE(PFN, new_ipte); // Updates Inverted Page Table
		write_PTE(pid, VPN, new_pte); // Updates Page Table
		swap_in(pid, VPN, PFN); // Moves value from logical memory to physical memory

		return PFN;
}

int is_page_hit(int pid, int VPN, char type)
{
		/* Read page table entry for (pid, VPN) */
		PTE pte = read_PTE(pid, VPN);

		/* if PTE is valid, it is a page hit. Return physical frame number (PFN) */
		if(pte.valid) {
		/* Mark the page dirty, if it is a write request */
				if(type == 'W') {
						pte.dirty = true;
						write_PTE(pid, VPN, pte);
				}

		/* Need to take care of a page replacement data structure (LRU, CLOCK) for the page hit*/
		/* ---- */

				if (replacementPolicy == LRU) {
		// Removes PFN from queue then places it at tail since it's interacted with
						PFN_queue = list_remove(PFN_queue, pte.PFN);
						PFN_queue = list_insert_tail(PFN_queue, pte.PFN);
				}

				else if (replacementPolicy == CLOCK) {
						if (clockArray[pte.PFN] == 0) {
							clockArray[pte.PFN]++;
						}
				}

				return pte.PFN;
		}
		
		/* PageFault, if the PTE is invalid. Return -1 */
		return -1;
}

int MMU(int pid, int VPN, char type, bool *hit)
{
		int PFN;

		// hit
		PFN = is_page_hit(pid, VPN, type);
		if(PFN >= 0) {
				stats.hitCount++;
				*hit = true;
				return PFN;
		}

		stats.missCount++;
		*hit = false;
				
		// miss -> pagefault
		PFN = pagefault_handler(pid, VPN, type);

		return PFN;
}

