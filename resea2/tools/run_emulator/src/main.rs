use ansi_term::{Color, Style};
use phf::phf_map;
use std::{
    path::PathBuf,
    process::{Command, Stdio},
};
use structopt::StructOpt;

#[derive(Debug)]
struct Arch {
    qemu: &'static str,
    args: &'static [&'static str],
}

const DEFAULT_QEMU_ARGS: &[&str] = &[
    "-no-reboot",
    "-serial",
    "mon:stdio",
    "-d",
    "guest_errors,unimp",
];

const ARCHS: phf::Map<&'static str, Arch> = phf_map! {
    "riscv64-qemu" => Arch {
        qemu: "qemu-system-riscv64",
        args: &[
            "-machine", "virt", "-bios", "none", "-m", "1024",
        ],
    }
};

/// A basic example
#[derive(StructOpt, Debug)]
#[structopt()]
struct Opts {
    #[structopt(long)]
    arch: String,
    #[structopt(long)]
    machine: String,
    #[structopt(name = "KERNEL_IMAGE")]
    kernel_image: PathBuf,
    #[structopt(long)]
    qemu: Option<String>,
    #[structopt(long)]
    gui: bool,
    #[structopt(long)]
    kvm: bool,
    #[structopt(long)]
    gdb: bool,
}

fn main() {
    let opts = Opts::from_args();
    let arch_key = format!("{}-{}", opts.arch, opts.machine);
    let arch = match ARCHS.get(&arch_key) {
        Some(arch) => arch,
        None => panic!("unknown arch/machine: {}", arch_key),
    };

    let mut qemu_program = arch.qemu;
    let mut qemu_args = Vec::new();
    qemu_args.extend(DEFAULT_QEMU_ARGS);
    qemu_args.extend(arch.args);

    if !opts.gui {
        qemu_args.extend(["-nographic"]);
    }

    if opts.kvm {
        qemu_args.extend(["-accel", "kvm"]);
    }

    if opts.gdb {
        qemu_args.extend(["-S", "-s"]);
    }

    if let Some(ref exec) = opts.qemu {
        qemu_program = exec;
    }

    println!(
        "\n{}",
        Style::new().bold().paint(format!(
            ">>> emulator: Booting {} ({})",
            opts.kernel_image.display(),
            qemu_program
        ))
    );

    if opts.gdb {
        println!(
            "{}",
            Color::Yellow
                .bold()
                .paint(">>> emulator: GDB connection will be available at localhost:1234")
        );
    }

    let mut cmd = Command::new(qemu_program)
        .args(qemu_args)
        .arg("-kernel")
        .arg(opts.kernel_image)
        .stdin(Stdio::inherit())
        .stdout(Stdio::inherit())
        .stderr(Stdio::inherit())
        .spawn()
        .expect("failed to execute qemu");

    let status = cmd.wait().expect("failed to wait");
    std::process::exit(status.code().unwrap_or(128));
}
