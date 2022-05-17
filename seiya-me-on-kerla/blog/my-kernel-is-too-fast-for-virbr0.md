---
title: 「なぜか最初のDHCP DISCOVERに返信がこない」バグ
title_en: Why doesn't my OS receive a reply to its *first* DHCP DISCOVER?
date: Apr 30, 2021
---

昨日見つけたちょっと面白いバグ（むしろ仕様）の話。

最近，Rustでフルスクラッチのモノリシックカーネルを書いて遊んでいる。機能が揃ってきたので，以前から気になっていた [Firecracker](https://firecracker-microvm.github.io/) 上でも動くよう色々と修正していた。

FirecrackerはLinux KVMベースのハイパーバイザで，QEMUに比べ余計な機能が削ぎ落とされている（Firecrackerに触発されてQEMUにも [microvm](https://github.com/bonzini/qemu/blob/master/docs/microvm.rst) というマシンモデルが追加されている）。Linuxを満足に動かすことを主目的としているようで，Firecrackerで独自OSをブートさせるには大体以下の修正が必要になる。

- [Linux Boot Protocol](https://www.kernel.org/doc/html/latest/x86/boot.html) への準拠。multiboot（GRUB）とは異なり，直接64-bitモードからカーネルを実行し始める。
- [virtio over MMIO](https://docs.oasis-open.org/virtio/virtio/v1.1/csprd01/virtio-v1.1-csprd01.html#x1-1440002) の実装。PCIがないのでカーネルパラメタ経由でMMIOのアドレスとIRQ番号を渡すようになっている。


使い方はいたってシンプルで，VMごとにfirecrackerを起動すれば良い。

```
$ firecracker --api-sock /tmp/firecracker.sock 
```

QEMUと同じ使い勝手であるが，REST API（またはJSONファイル）経由でVMの設定が行える。

## Firecrackerのネットワーク設定
DHCPを使って楽に任意個のVMを立ち上げたいので，libvirtの提供するDHCPサーバつき仮想ブリッジ（`virbr0`）を使って次のようなネットワークを構築することにした。virbr0の起動とTAPデバイスの作成は以下の通り。

```
# virsh net-start default
# ip tuntap add veth0 mode tap
# ip link set veth0 master virbr0
# ip link set veth0 up
```

以下のような設定を入れるとvirtio-netがVM上に生成される。

```json
{
  "boot-source": {
    "kernel_image_path": "/home/seiya/penguin-kernel.x64.elf",
    "boot_args": "console=ttyS0 pci=off"
  },
  "drives": [],
  "network-interfaces": [
    {
      "iface_id": "eth0",
      "guest_mac": "AA:FC:00:00:00:01",
      "host_dev_name": "veth0"
    }
  ],
  "machine-config": {
    "vcpu_count": 1,
    "mem_size_mib": 1024,
    "ht_enabled": false
  }
}
```

このJSONの `host_dev_name` にTAPデバイスを指定することでVMと通信できるようになる。
[FirecrackerのサンプルLinuxイメージ](https://github.com/firecracker-microvm/firecracker/blob/main/docs/getting-started.md#running-firecracker) 使ってDHCPでIPアドレスを取得できているか確認。

```
# ip link set eth0 up
# udhcpc -f
udhcpc: started, v1.28.4
udhcpc: sending discover
udhcpc: sending select for 192.168.122.132
udhcpc: lease of 192.168.122.132 obtained, lease time 3600
```

きちんと取れている。

## 一番最初のDHCP DISCOVERだけ返信がこない
Linux Boot Protocolの対応を終えると，普通に独自カーネルが立ち上がるようになってきた。しかし，なぜかDHCP OFFERが返ってこない。

再送すると返信が来るようにはなったが，なぜか最初のDHCP DISCOVERへの返信がないのである。そういえば前に[Reseaをさくらのクラウド上で動かしたとき](https://seiya.me/writing-a-microkernel-from-scratch) に同じ現象があったことを思い出した。今回はローカルで再現するので興味本位で原因を探ることにした。

veth0をtcpdumpしてみると正しいパケットが流れているのでvirtioのバグでもTCP/IPプロトコルスタックのバグの可能性は低そう。しかしvirbr0には流れていない。

```
$ sudo tcpdump -i veth0
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on veth0, link-type EN10MB (Ethernet), capture size 262144 bytes
04:09:18.516652 IP6 seiya-nuc > ff02::16: HBH ICMP6, multicast listener report v2, 2 group record(s), length 48
04:09:18.572644 IP6 seiya-nuc > ff02::16: HBH ICMP6, multicast listener report v2, 2 group record(s), length 48
04:09:18.589933 IP 0.0.0.0.bootpc > 255.255.255.255.bootps: BOOTP/DHCP, Request from aa:fc:00:00:00:0a (oui Unknown), length 257
```

```
$ sudo tcpdump -i virbr0
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on virbr0, link-type EN10MB (Ethernet), capture size 262144 bytes
^C
0 packets captured
0 packets received by filter
0 packets dropped by kernel
```

色々と試していると，独自カーネルを起動した時のログメッセージの順序がLinuxで試した場合と若干異なることに気がついた。

```
Apr 29 05:52:14 seiya-nuc systemd-networkd[650]: veth0: Gained carrier
Apr 29 05:52:14 seiya-nuc kernel: virbr0: port 2(veth0) entered blocking state
Apr 29 05:52:14 seiya-nuc kernel: virbr0: port 2(veth0) entered listening state
[...]
Apr 29 05:52:16 seiya-nuc kernel: virbr0: port 2(veth0) entered learning state
Apr 29 05:52:17 seiya-nuc kernel: virbr0: received packet on veth0 with own address as source address (addr:aa:fc:00:00:00:0a, vlan:0)
Apr 29 05:52:18 seiya-nuc systemd-networkd[650]: virbr0: Gained carrier
[...]
Apr 29 05:52:18 seiya-nuc kernel: virbr0: port 2(veth0) entered forwarding state
Apr 29 05:52:18 seiya-nuc kernel: virbr0: topology change detected, propagating
```

`received packet on veth0 with ...` の部分でパケットがvirbr0に流れようとしている。独自カーネルの方だとこのログのように `(veth0) entered forwarding state` になる前に送られている。何か関係がありそう。

## STP LEARNINGステートの罠
原因はSTPだった。Spanning Tree Protocol（STP）はネットワーク上のループを検出するプロトコルで，ネットワーク機器同士が通信しあって利用しないポートを決定することでループを回避する。

Firecrackerがveth0を使い始めると，まずSTPのパケットが行き交ってネットワークの状態を確認する。この間（LISTENING/LEARNINGステート）はパケットがブリッジに流れないようになっているので「なぜか最初のDHCP DISCOVERがvirbr0に流れない」という現象が起きたらしい。試しに `ip link set virbr0 type bridge stp_state 0` を実行してSTPを切ってみると，最初のDHCP DISCOVERの返信が来るようになった。


調べてみると，実は [RedHatのlibvirtトラブルシューティング集](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/virtualization_deployment_and_administration_guide/sect-troubleshooting-common_libvirt_errors_and_troubleshooting) でSTPのことが言及されている:

> **Investigation**
>
> [...] the bridge will not forward network packets from the guest virtual machine onto the bridge until at least that number of forward delay seconds have elapsed [...]
>
> **Solution**
>
> If this is the case, change the forward delay on the bridge to 0, disable STP on the bridge, or both.

forward delayを0にしても，カーネルの起動が速すぎる（ログをみる限り4秒以内）とFORWARDINGステートに入る前にパケットを送れてしまうようだ。

ちなみに，Ciscoのネットワーク機器だと [*PortFast*](https://www.cisco.com/cisco/web/support/JP/docs/SW/LANSWT-Core/CAT6500SWT/CG/004/stp_enha.html#pgfId-1052988) という機能を使って，サーバ等に繋がっているポートで直ちにFORWARDINGステートに移行する設定をできるらしい。

## まとめ
STPが悪いというわけではなく，DHCP DISCOVERの再送をしていないのが悪いのがそもそもの原因。ローカルの仮想ネットワーク上の通信だとしてもUDPパケットが必ず届くと信用しているのが悪かった。

STPの影響でパケットが転送されるまでラグがあるというよく知られているのであろう挙動ではあるが，独自カーネルのDHCPクライアントが立ち上がるのが速いことで気づけたのが少し面白い。

さくらのクラウド上で独自カーネルを動かすと最初のDHCP DISCOVERへの返信がない現象も，もしかしたらSTPが走っている途中だったのかもしれない。
