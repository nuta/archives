#include <hal.h>
#include <resea.h>


void x86_init_vm(void) {

}


void hal_create_vm_space(struct vm_space *vms) {

    WARN("hal_create_vm_space(): is not supported");
}


void hal_remove_vm_space(struct vm_space *vms) {

    WARN("hal_remove_vm_space(): is not supported");
}


void hal_switch_vm_space(struct hal_vm_space *vms) {

    WARN("hal_switch_vm_space(): is not supported");
}


void hal_link_page(struct vm_space *vms, uintptr_t v, paddr_t p, size_t n,
                   page_attrs_t attrs) {

    WARN("hal_link_page(): is not supported");
}


page_attrs_t hal_get_page_attribute(struct vm_space *vms, uintptr_t v) {

    WARN("hal_get_page_attribute(): is not supported");
    return 0;
}


void hal_set_page_attribute(struct vm_space *vms, uintptr_t v, size_t n, page_attrs_t attrs) {

    WARN("hal_set_page_attribute(): is not supported");
}


paddr_t hal_vaddr_to_paddr(struct vm_space *vms, uintptr_t v) {

    return v;
}


uintptr_t hal_paddr_to_vaddr(paddr_t v) {

    return v;
}
