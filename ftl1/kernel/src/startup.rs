use alloc::boxed::Box;
use alloc::collections::btree_map::BTreeMap;
use alloc::rc::Rc;
use alloc::string::String;
use alloc::string::ToString;
use alloc::vec::Vec;
use core::str;

use arrayvec::ArrayVec;
use ftl_dtb_parser::Dtb;
use ftl_types::arch::PageAttrs;
use ftl_types::device::BusAddr;
use ftl_types::device::DeviceDesc;
use ftl_types::device::DeviceMatch;
use ftl_types::device::DmaDesc;
use ftl_types::device::IoSpaceDesc;
use ftl_types::environ::SyscallHandler;
use ftl_types::environ::VSyscallPage;
use ftl_types::handle::HandleRight;
use ftl_types::message::CONNECT_MSG;
use ftl_types::message::Cookie;
use ftl_types::message::MessageInfo;
use ftl_types::spec::AppSpec;
use ftl_types::spec::DeviceMatcher;
use ftl_types::spec::EnvType;
use ftl_types::spec::ExportItem;

use crate::arch;
use crate::arch::kernelcall_handler;
use crate::channel::Channel;
use crate::handle::Handle;
use crate::isolation::INKERNEL_ISOLATION;
use crate::process::HandleTable;
use crate::process::Process;
use crate::scheduler::GLOBAL_SCHEDULER;
use crate::shared_ref::SharedRef;
use crate::spinlock::SpinLock;
use crate::thread::Thread;
use crate::vmarea::VmArea;
use crate::vmspace::KERNEL_VMSPACE;

const APP_SPECS: &[&AppSpec] = &[
    &::hello::SPEC,
    &::echo::SPEC,
    &::echo_client::SPEC,
    // &::virtio_blk::SPEC,
    &::virtio_net::SPEC,
    &::tcpip::SPEC,
    &::apiserver::SPEC,
];

#[derive(PartialEq, Eq, PartialOrd, Ord)]
struct ServiceName(&'static str);

struct Server {
    spec: &'static AppSpec,
    their_system_ch: SharedRef<Channel>,
    our_system_ch: SharedRef<Channel>,
}

impl Server {
    fn new(spec: &'static AppSpec) -> Self {
        let (our_system_ch, their_system_ch) = Channel::new().unwrap();
        Self {
            spec,
            their_system_ch,
            our_system_ch,
        }
    }
}

struct ServerMap {
    servers: BTreeMap<ServiceName, Rc<Server>>,
}

impl ServerMap {
    fn new() -> Self {
        let mut servers = BTreeMap::new();
        for app in APP_SPECS {
            let server = Rc::new(Server::new(app));
            for export in app.exports {
                match export {
                    ExportItem::Service { name } => {
                        let service_name = ServiceName(name);
                        assert!(!servers.contains_key(&service_name));
                        servers.insert(service_name, server.clone());
                    }
                }
            }
        }

        Self { servers }
    }

    fn resolve_env(
        &self,
        app: &AppSpec,
        handle_table: &mut HandleTable,
        dtb: &Dtb<'static>,
    ) -> serde_json::Map<String, serde_json::Value> {
        let mut env = serde_json::Map::new();
        for item in app.env {
            let value = match &item.ty {
                EnvType::Service { name: service_name } => {
                    let service_name = ServiceName(service_name);
                    let Some(server) = self.servers.get(&service_name) else {
                        panic!(
                            "service \"{}\" not found for \"{}\"",
                            service_name.0, app.name
                        );
                    };

                    // Create a new channel pair and insert one end to its handle table.
                    let (our_ch, their_ch) = Channel::new().unwrap();
                    let our_id = handle_table
                        .insert(Handle::new(our_ch, HandleRight::READ | HandleRight::WRITE))
                        .unwrap();

                    // Send the connected channel to the server.
                    let mut handles = ArrayVec::new();
                    handles
                        .push(Handle::new(their_ch, HandleRight::READ | HandleRight::WRITE).into());
                    server
                        .our_system_ch
                        .send(
                            MessageInfo::new(
                                CONNECT_MSG,
                                Cookie::new(0),
                                service_name.0.len() as u16,
                                1,
                            ),
                            service_name.0.as_bytes().to_vec(),
                            handles,
                        )
                        .unwrap();

                    serde_json::json!(our_id.as_usize())
                }
                EnvType::Device { matcher } => {
                    let mut matches = Vec::new();
                    for node in dtb.nodes() {
                        let node = node.unwrap();
                        match matcher {
                            DeviceMatcher::DeviceTree { compatible } => {
                                for prop in node.props() {
                                    if prop.name() == b"compatible"
                                        && let Ok(cstr) = prop.as_cstr()
                                        && cstr == compatible.as_bytes()
                                    {
                                        let reg = node.reg().unwrap().unwrap();
                                        let baddr = BusAddr::new(reg.addr as usize);
                                        let size = reg.size as usize;
                                        let iospace = IoSpaceDesc::Mmio { baddr, size };
                                        let dma = DmaDesc::Direct;
                                        let irq = arch::irq_desc_from_devtree(&node);
                                        matches.push(DeviceMatch { iospace, dma, irq });
                                    }
                                }
                            }
                        }
                    }

                    serde_json::json!(DeviceDesc { matches })
                }
            };

            env.insert(item.name.to_string(), value);
        }

        // Get the system channel if exists.
        let server = self
            .servers
            .values()
            .find(|server| server.spec.name == app.name);
        if let Some(server) = server {
            let handle = Handle::new(server.their_system_ch.clone(), HandleRight::ALL);
            let handle_id = handle_table.insert(handle).unwrap();
            env.insert(
                "system_ch".to_string(),
                serde_json::json!(handle_id.as_usize()),
            );
        }

        env
    }
}

struct App {
    _env_json: Vec<u8>,
    _vsyscall: Box<VSyscallPage>,
    _thread: SharedRef<Thread>,
}

impl App {
    fn start(
        app: &'static AppSpec,
        process: SharedRef<Process>,
        env: serde_json::Map<String, serde_json::Value>,
    ) -> Self {
        let env_json = serde_json::to_vec(&env).unwrap();
        let vsyscall = Box::new(VSyscallPage {
            syscall_handler: unsafe {
                core::mem::transmute::<*const _, SyscallHandler>(kernelcall_handler as *const ())
            },
            environ_ptr: env_json.as_ptr() as usize,
            environ_len: env_json.len(),
            name_ptr: app.name.as_ptr() as usize,
            name_len: app.name.len(),
        });
        let arg = &*vsyscall as *const _ as usize;

        // Allocate a user stack.
        const USER_STACK_SIZE: usize = 1024 * 1024;
        let page_attrs = PageAttrs::valid().read().write().execute();
        let stack_vmarea = SharedRef::new(VmArea::new_any(USER_STACK_SIZE)).unwrap();
        let stack_range = process
            .isolation()
            .vmspace()
            .insert_anywhere(USER_STACK_SIZE, stack_vmarea, page_attrs)
            .unwrap();
        let sp = stack_range.end.as_usize();

        let thread = Thread::new(process, app.start as usize, sp, arg).unwrap();
        GLOBAL_SCHEDULER.push(thread.clone());
        Self {
            _env_json: env_json,
            _vsyscall: vsyscall,
            _thread: thread,
        }
    }
}

pub fn init(dtb: Dtb<'static>) {
    let servers = ServerMap::new();

    // In-kernel app instances to keep them alive.
    static APPS: SpinLock<Vec<App>> = SpinLock::new(Vec::new());

    let mut apps = APPS.lock();
    for app in APP_SPECS {
        trace!("starting \"{}\"", app.name);
        let process = SharedRef::new(Process::create(INKERNEL_ISOLATION.clone())).unwrap();

        let env = {
            let mut handle_table = process.handles().lock();

            // Inject KERNEL_VMSPACE at handle id 1.
            let vmspace_handle = Handle::new(KERNEL_VMSPACE.clone(), HandleRight::ALL);
            let vmspace_id = handle_table.insert(vmspace_handle).unwrap();
            assert_eq!(vmspace_id.as_usize(), 1);

            // Inject the current process at handle id 2.
            let process_handle = Handle::new(process.clone(), HandleRight::ALL);
            let process_id = handle_table.insert(process_handle).unwrap();
            assert_eq!(process_id.as_usize(), 2);

            servers.resolve_env(app, &mut handle_table, &dtb)
        };

        apps.push(App::start(app, process, env));
    }

    if apps.is_empty() {
        panic!("no apps running");
    }
}
