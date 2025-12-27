#![no_std]

use ftl::prelude::*;
use ftl::spec::AppSpec;

pub const SPEC: AppSpec = AppSpec {
    name: "hello",
    start: |vsyscall| ftl::start(vsyscall, main),
    env: &[],
    exports: &[],
};

#[derive(serde::Deserialize)]
struct Environ {}

fn main(_env: Environ) {
    info!("Hello, world!");
}
