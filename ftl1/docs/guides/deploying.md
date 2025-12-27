# Deploying

FTL can be deployed to Linux/KVM hypervisor.

> [!NOTE]
> We plan to support more environments (including real machines) in the future.

## Running FTL on KVM hypervisor

> [!WARNING]
> FTL supports only 64-bit Arm (AArch64) and RISC-V CPUs. x86_64 is not supported yet.

`deploy.sh` connects to a remote host over SSH, uploads FTL to `/opt/ftl`, and starts a systemd service running it on QEMU:

```bash
bin/ftl deploy <user@host>
```

> [!TIP]
> Set `LOGS=1` to stream logs from FTL running on the hypervisor. This is useful for debugging.

API server will listen on port `30080` and you can access it via HTTP:

```
curl http://<host>:30080
```
