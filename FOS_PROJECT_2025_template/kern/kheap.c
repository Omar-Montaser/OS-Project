#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT 2025 - MS1 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code
	//NOTE: Allocation is based on FIRST FIT strategy
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details
	//change this "return" according to your answer

    // Round up size to nearest multiple of PAGE_SIZE
    if (size == 0)
        return NULL;

    uint32 rounded_size = ROUNDUP(size, PAGE_SIZE);
    uint32 total_size = rounded_size + PAGE_SIZE; // +1 page for metadata

    // How many pages do we need
    uint32 num_pages = total_size / PAGE_SIZE;

    uint32 consecutive_free_pages = 0;
    uint32 first_page_va = KERNEL_HEAP_START;

    // First Fit scan: loop over heap virtual addresses
    for (uint32 va = KERNEL_HEAP_START; va < KERNEL_HEAP_MAX; va += PAGE_SIZE)
    {
        uint32* ptr_page_table = NULL;
        struct Frame_Info *frame = get_frame_info(ptr_page_directory, (void*)va, &ptr_page_table);

        if (frame == NULL)
        {
            // Page is free
            if (consecutive_free_pages == 0)
                first_page_va = va;

            consecutive_free_pages++;

            if (consecutive_free_pages == num_pages)
            {
                // We found enough space — allocate here
                uint32 current_va = first_page_va;

                for (uint32 i = 0; i < num_pages; i++, current_va += PAGE_SIZE)
                {
                    struct Frame_Info *new_frame = NULL;
                    allocate_frame(&new_frame);
                    map_frame(ptr_page_directory, new_frame, (void*)current_va, PERM_WRITEABLE | PERM_PRESENT);
                }

                // Store metadata (actual requested size) in first page
                uint32* size_ptr = (uint32*)first_page_va;
                *size_ptr = rounded_size;

                // Return pointer to memory just after the metadata page
                return (void*)(first_page_va + PAGE_SIZE);
            }
        }
        else
        {
            // Page is already used, reset
            consecutive_free_pages = 0;
        }
    }

    // No space found
    return NULL;

}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2025 - MS1 - [1] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

    uint32 va = (uint32)virtual_address;

    if (va < KERNEL_HEAP_START || va >= KERNEL_HEAP_MAX)
        return;

    // Metadata is in the page before this allocation
    uint32 metadata_va = va - PAGE_SIZE;

    uint32* ptr_page_table = NULL;
    struct Frame_Info *metadata_frame = get_frame_info(ptr_page_directory, (void*)metadata_va, &ptr_page_table);

    if (metadata_frame == NULL)
        return;

    // Read the allocation size
    uint32* size_ptr = (uint32*)metadata_va;
    uint32 alloc_size = *size_ptr;

    if (alloc_size == 0 || alloc_size > (KERNEL_HEAP_MAX - KERNEL_HEAP_START))
        return;

    // Total pages to free = metadata + allocation
    uint32 total_size = alloc_size + PAGE_SIZE;

    for (uint32 i = 0; i < total_size; i += PAGE_SIZE)
    {
        unmap_frame(ptr_page_directory, (void*)(metadata_va + i));
    }

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2025 - MS1 - [1] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code
	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details
	//change this "return" according to your answer

	uint32* ptr_page_table = NULL;
	for (uint32 va = KERNEL_HEAP_START; va < KERNEL_HEAP_MAX; va += PAGE_SIZE)
	{
		struct Frame_Info *ptr_frame_info = get_frame_info(ptr_page_directory, (void*)va, &ptr_page_table);
		if (ptr_frame_info != NULL)
		{
			uint32 frame_physical_address = to_physical_address(ptr_frame_info);
			if (frame_physical_address == (physical_address & 0xFFFFF000))
				return va | (physical_address & 0xFFF);
		}
	}
	return 0;

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2025 - MS1 - [1] Kernel Heap] kheap_physical_address()
	//Write your code here, remove the panic and write your code
	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details
	//change this "return" according to your answer

	if (virtual_address < KERNEL_HEAP_START || virtual_address >= KERNEL_HEAP_MAX)
		return 0;

	uint32* ptr_page_table = NULL;
	struct Frame_Info *ptr_frame_info = get_frame_info(ptr_page_directory, (void*)virtual_address, &ptr_page_table);

	if (ptr_frame_info == NULL)
		return 0;

	uint32 page_table_entry = ptr_page_table[PTX(virtual_address)];
	if (!(page_table_entry & PERM_PRESENT))
		return 0;

	uint32 physical_address = to_physical_address(ptr_frame_info);

	return physical_address|=(virtual_address & 0xFFF);

}

void *krealloc(void *virtual_address, uint32 new_size)
{
	panic("krealloc() is not required...!!");
	return NULL;
}
