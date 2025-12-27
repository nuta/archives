use ftl_types::environ::VSyscallPage;

pub fn set_vsyscall_page(_vsyscall: *const VSyscallPage) {
    todo!()
}

pub fn get_vsyscall_page() -> &'static VSyscallPage {
    todo!()
}
