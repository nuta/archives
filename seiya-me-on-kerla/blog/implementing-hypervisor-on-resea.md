---
title: KVMみたいなハイパーバイザを作った
title_en: Implementing hardware-accelerated hypervisor in Resea
lang: ja
date: Dec 23, 2020
---

ハイパーバイザは仮想的なコンピュータ（仮想マシン）を構築し，複数の様々なOSの並列実行を実現する技術です。VMware ESXiやKVM，Xenが有名ですね。

今回はハードウェア支援機構（Intel VT-x）を使って，独自OSの[Resea](https://github.com/nuta/resea)にLinux KVMのようにアドオン感覚で使えるハイパーバイザを実装しました。Linuxをなんとかブートできる程度の完成度です。

これは[自作OS Advent Calendar 2020](https://adventar.org/calendars/4954)の23日目の記事です。

![デモ: 実機（Intel NUC）のResea上で動くLinux](linux-on-hv.jpg)

## ハイパーバイザを作る
Reseaにハイパーバイザを実装すると何が嬉しいのかというと，Linuxがそのまま動くのでようやくまともにドッグフーディングできるようになります。Linux on Resea on 自宅サーバで遊びながら，Reseaの信頼性や性能を向上していこうという計画です。

Reseaはマイクロカーネルベースです。なので「Linuxを動かしつつ独立した環境で安全性が求められるプログラムを実行したい」といったことを実現できます。「安全性が求められるプログラム」は自宅サーバではまあ必要無いですが面白さはありそうです。

ちなみにマイクロカーネル的アプローチでセキュアなハイパーバイザを作ろうという研究では [NOVA](http://hypervisor.org/) が有名です。関係ないけどドメインがかっこいい。
　
## Intel VT-x入門
ハイパーバイザを作るときに必要なものは「サンドボックスのような」環境です（cf. PopekとGoldbergの仮想化要件）。ゲスト（ハイパーバイザのことを「ホスト」，その上で動くOSのことを「ゲスト」と呼びます）がCPUの状態を変更しても，ハイパーバイザや他のゲストに影響が出てはいけません。

x86 CPUのエミュレータやバイナリトランスレータを作ろうとすると途方もない労力が必要になりますが，Intel VT-xを活用することで比較的楽にハイパーバイザを実現できます。

Intel VT-xは「CPU上でゲストをそのまま実行するが，CPUの動作を変える変更やデバイスとのやり取り（IOポート叩くとか）を試みたら処理を中断してハイパーバイザに処理を移行する」という例外処理に似た挙動を実現してくれます。Virtual Machine Extensions（VMX）と呼ばれることもあります。

また，ゲストがハイパーバイザ等のメモリ領域をいじれてはいけないので，
ホスト物理アドレスとゲスト物理アドレスの2つを考える必要があります。Intel VTでは，Extended Page Table（EPT）と呼ばれる「ゲスト物理アドレスとホスト物理アドレスの対応表」を設定することもできます。ページテーブルと同じような概念ですね。

ざっくりした使い方は以下の通りです。ゲストを起動し（VMEntry），何か起きたら（VMExit）その状況に応じてエミュレーションとVMCSの更新を行って，ゲストを再開する（VMEntry）というループを続けます。

```c
void hypervisor_main(void) {
    // Intel VT-xの有効化。CR4もいじる必要がある。
    asm("VMXON");

    // ゲストの初期状態（RIP, GDT, IDT, CR0, ...）等をセット
    initialize_vmcs();

    // VMEntry: ゲストの実行開始
    asm("VMLAUNCH");
}

// VMExitしてハイパーバイザに処理が移ったら呼ばれる。VMCSでVMExit時のホストの状態
// （例: ハンドラのアドレス）を設定できる。
void vmexit_handler(void) {
    switch (read_exit_reason_from_vmcs()) {
        // VMExitの理由に応じて命令のエミュレーションしたり，VMCSをいじったり
    }

    // VMEntry: ゲストの実行再開
    asm("VMRESUME");
}
```

各ゲストの情報（ゲスト・ホストの各レジスタの値，VMExitの原因，VMEntry時の処理，その他設定）はVirtual Machine Control Structure（VMCS）というメモリ領域で管理され，`VMWRITE/VMREAD`命令で読み書きします。

どのような場合にVMExitするかはVMCSである程度設定できます。代表的なものを挙げると以下の通りです。

- **コントロールレジスタ・MSRの読み書き:** セットして良い値かをチェックしたり，必要に応じてロングモード（64-bitモード）の有効化・無効化をしたり。
- **IOポートの読み書き:** 周辺機器（シリアルポートなど）のエミュレーションが必要。 
- **EPT Violation:** ゲストがアクセスできないゲスト物理アドレスを触ろうとした。ページフォルトみたいなやつ。
- **割り込み:** そのままホスト側の割り込みハンドラで割り込み処理を行える。
- **CPUID命令の実行:** ハイパーバイザがCPUのどういう機能に対応しているのか教える必要がある。
- **HLT命令の実行:** やることがなくなったのでホストに戻る。VMCSのあるフィールドを設定すると，VMEntry時にゲスト側で割り込みを発生させられる。
- **Triple Fault:** どうしようもない。

IRET命令といった他の特権命令や（Triple Fault以外の）例外処理などは，Intel VTの中で普通に動くのでエミュレーションする必要がありません。楽でいいですね。

なお，Intel VT-xは名前にも入っている通りIntel CPUの機能です（ちなみにIntel VT-dとかもあります）。AMDのCPUはAMD Virtualization（AMD-V）という別の仕組みが入っています。

## KVMの仕組み
Kernel-based Virtual Machine（KVM）はIntel VT-xを活用してLinuxをハイパーバイザとして使えるようにするカーネルの機能です。周辺機器などのエミュレーションをカーネル内ではなく，`/dev/kvm` 経由でユーザランドから実装できるようになっています。 

![KVMの概要](kvm-architecture.png)

ユーザランド側はQEMUやFirecrackerなど様々な実装があり，[kvm API](https://www.kernel.org/doc/Documentation/virtual/kvm/api.txt)を使ってゲストを制御できます。kvm APIは少しIntel VTを抽象化して使いやすくしたもので，カーネルが少しMMIO周り等の面倒くさい処理を肩代わりしてくれています。

また，KVMとは別に[Nested VMX](https://www.kernel.org/doc/Documentation/virtual/kvm/nested-vmx.txt)という機能がLinuxに入っています。何をしてくれるのかと言うと，ゲスト上でIntel VTを動かすことを可能にします。ハイパーバイザ on ハイパーバイザです。QEMU上でハイパーバイザ（Reseaとか）の開発・デバッグをする時に便利です。

## hv（Resea版KVM）の仕組み
背景が掴めたところで本題です。KVMのようなOSを[Type-1 ハイパーバイザ](https://www.redhat.com/ja/topics/virtualization/what-is-a-hypervisor)として「も」使えるようにする機能をReseaでも実装しました。名前は *hv* です。

![hvの概要。基本的な仕組みはKVMと大して変わらない](resea-hv-architecture.png)

hvはKVMと同じようにカーネル側（[`kernel/arch/x64/hv.c`](https://github.com/nuta/resea/blob/master/kernel/arch/x64/hv.c)）とユーザランド側（[`servers/hv`](https://github.com/nuta/resea/tree/master/servers/hv)）に分かれています。

ゲストもタスクの1つとして表現され，他のタスクと同じようにスケジューリングされます。
普通のタスクと同じように`task_create`システムコールで起動しますが，起動オプションとして`TASK_HV`を指定することで，ユーザランドではなくIntel VTのゲストとして実行を開始します。

KVMでは`/dev/kvm`を`ioctl(2)`で叩いてゲストの制御をしていますが，Reseaでは[ABIエミュレーション機能](https://seiya.me/implementing-linux-abi)と同じようにメッセージパッシングで実現します。

ゲストがVMExitを起こすと，カーネル内のVMExitハンドラが呼ばれ，ページャタスクに処理要求を送ります。今のところ以下のようなメッセージ/RPCが使われています。Resea独自のIDLですが，大体雰囲気はつかめると思います。

```
namespace hv {
    rpc x64_start(task: task)
        -> (guest_rip: uint64, ept_pml4: paddr, initial_rbx: uint64);

    rpc halt(task: task) -> ();
    rpc ioport_read(task: task, port: uint16, size: size) -> (value: uint32);
    rpc ioport_write(task: task, port: uint16, size: size, value: uint32) -> ();
    rpc guest_page_fault(task: task, gpaddr: gpaddr, frame: hv_frame)
        -> (frame: hv_frame);

    oneway inject_irq(irq_bitmap: uint32);
}
```
## Linuxカーネルを動かそう
`make tinyconfig`をベースに`CONFIG_XEN_PVH=y`と[必要不可欠な設定](https://github.com/linuxboot/book/blob/master/coreboot.u-root.systemboot/README.md#a-few-fundamental-features)を有効化したカーネルを使いました（最終的に使っていたconfigは[ココ](https://gist.github.com/nuta/e76ca295ebeec02b88121a1ae7c73b9e)）。

加えて，[PVH](https://github.com/xen-project/xen/blob/master/docs/misc/pvh.pandoc)というXenがゲストOSを起動する時に使っている仕組みをReseaでも実装しました。これを使うとLinuxをプロテクトモードから直接起動することができます。リアルモードのBIOSコールやUEFIを実装しなくて済むのが嬉しいポイントです。

このページの最後にある開発日記に細かい話は書いてありますが，Linuxは大変几帳面に書かれています。CPUIDの結果がおかしかったり，MPテーブルで適当な値を入れていたりすると「この環境壊れてない？」とちゃんと怒ってきます。カーネルパニックしたり意図しないfallbackをしたりするので，ハイパーバイザもチェックサムを入れる等きちんとしないといけません。これからハイパーバイザを書く人はまず（今のところ）適当に書いてあるReseaでテストしましょう。

少し意外だったのですが，tinyconfigを使っていることもあってかLinuxは謎のMSRやIOポートを叩くことはなく，Reseaでも触っているCPU・周辺デバイスの機能しか使っていませんでした。

## 実装の規模感
全部ひっくりめて3600行程になりました。Reseaカーネルのコアと同じぐらいの分量ですが，まあ「マイクロ」カーネルの体裁は保てていますね。

```
kernel/arch/x64/hv.c                  | 908 ++++++++++++++++++++++++++++++++++
kernel/arch/x64/hv.h                  | 467 +++++++++++++++++
servers/hv/build.mk                   |  18 +
servers/hv/guest.c                    | 174 +++++++
servers/hv/ioport.c                   |  39 ++
servers/hv/main.c                     | 131 +++++
servers/hv/mm.c                       | 429 ++++++++++++++++
servers/hv/pci.c                      | 142 ++++++
servers/hv/virtio_blk.c               | 269 ++++++++++
servers/hv/x64.c                      |  52 ++
...
39 files changed, 3583 insertions(+), 7 deletions(-)
```

エミュレーションを実装したものは以下の通りです。かろうじて動く程度のぽんこつ実装です。
- シリアルポート
- virtio-blk-pci (legacy)
- 割り込みコントローラ (Intel 8259)
- タイマ (Intel 8254)
- Local APIC

## VMXのここが辛いよ

### 仕様が複雑
昨今のIntel CPUには様々な機能が入っていることもあり，それを仮想化しようという[Intel VT-xの仕様](https://software.intel.com/content/www/us/en/develop/download/intel-64-and-ia-32-architectures-sdm-volume-3c-system-programming-guide-part-3.html)も中々の量があります。例えば今回のReseaのハイパーバイザ実装ですが，実装済みVM-Exitのパターン（*exit reason*）は68個中11個だけです。Nested VMXやIntel SGX関連の不要なものがあるとはいえ8割方は未実装のままです。

また，Linuxがブートするとはいえ不十分な実装がそこそこあります。例えばIOポートの読み書きだと`rep outs`のようなストリング命令は非対応です。これはMMIOのエミュレーション（後述）と同じような頑張りが必要です。

ハイパーバイザ実装に限った話ではありませんが，バグ・セキュリティホールがないという自信が一向に湧いてこないですね。仕様の見落としがまだまだある気がします。

### VMCS Exit Reason 33 (invalid guest state)
VMLAUNCH/VMRESUMEでよく起きる「VMCSの設定おかしいよ」エラーのことです。何かがおかしいという事しか分からないのでSDMを読んで原因を一つずつ洗っていく必要があります。辛い。

LinuxのNested VMX機能でテストしている場合は，`kvm_nested_vmenter_failed` というトレースポイントからエラー内容がわかるので便利。

### MMIOの実装をすると簡易CPUエミュレータが出来上がる
「このメモリアドレスにアクセスしようとした」という情報を提供する以上のことをIntel VT-xはしてくれません。ハイパーバイザが命令を解釈する必要があります。具体的には以下の手順を踏みます。

1. ゲストのCR3に入っているページテーブルをたどってRIPの指すゲスト側物理アドレスを取得
2. EPTからRIPの指すホスト側物理アドレスを取得
3. RIPの指す命令のバイト列を取得
4. 命令のデコード
5. （書き込みの場合）ゲストのレジスタから値を取得
6. MMIOエミュレーション
7. （読み込みの場合）ゲストのレジスタに値をセット
8. RIPを次の命令に進める

もはやCPUエミュレータです。Linuxでは [`arch/x86/kvm/emulate.c`](https://elixir.bootlin.com/linux/latest/source/arch/x86/kvm/emulate.c) にCPU命令エミュレーションが詰まっています。ちなみにそのファイルの行数（5926行）でReseaカーネル全体の行数を超しています。ハイパーバイザの道は険しい...

ところで巷のハイパーバイザはどれくらい命令のエミュレーションに対応しているのでしょう？KVM + QEMUといえどMMIO周りの命令はカーネル側でエミュレーションしている訳ですし，KVM側もある程度複雑な実装になるので「ハイパーバイザで安心安全！」とは個人的に思えなくなってきました。ハイパーバイザ向けにfuzzerとか有用そうな気がします。と思ったら[普通にありました](https://scholar.google.com/scholar?hl=en&as_sdt=0%2C5&q=hypervisor+fuzzer&btnG=)。考えることは皆同じ。

## VMXのここが楽しいよ
辛いところもありますが楽しい気持ちの方が勝ちます。

### Linuxの動きが分かる
Linuxが周辺機器（シリアルポート，virtio-blkなど）やCPUの機能をどう使うのかを対応する過程で一緒に学ぶことができます。不思議なプロトコルをWiresharkで眺めている気持ちに似ています。

### インクリメンタルな開発できる
HLTでVMExitするようになる，シリアルポート経由で文字を表示できるようになる，ロングモードに移行できる，...と段階的に動くものが出来上がってくるので定期的に達成感を感じることができます。

### 勘デバッグ力がつく
「何かおかしい」ことしか分からないことがたびたびあるので，勘が身につきます。楽しいですね。

## 最後に
今年頑張ってLinux ABI互換レイヤを実装したというのに本物のLinuxが動く環境が出来上がってしまいました。意図せずWindows Subsystem for Linux（WSL）と同じ道を歩んでいます。

今後はvirtio-netを実装してゲストのLinuxにSSHできるようにしたり，RISC-Vのハイパーバイザ支援機構を試したり，ハイパーバイザ周りでも色々遊んでいきたいですね。

----

## 開発日記

### 1日目
まずはテスト環境探し。QEMUにIntel VT-xのエミュレーションがあると踏んでいたが，どうやらないらしい。LinuxのNested VMX機能が使えそうだが，デスクトップマシンはRyzenなのでAMD SVMが提供されるらしい。しょうがないのでIntel NUC（[NUC6i3SYK](https://ark.intel.com/content/www/us/en/ark/products/89186/intel-nuc-kit-nuc6i3syk.html)）を買った。メモリとストレージ込みで3万円で済んで嬉しい。

Fuchsiaのようにハイパーバイザ用のシステムコールを追加するつもりだったが，Linux ABI互換レイヤと同じようにメッセージパッシングで実現できる気がしてきた。

### 2日目
Linuxカーネルの [Nested VMX](https://www.kernel.org/doc/Documentation/virtual/kvm/nested-vmx.txt) を試す。QEMUに`-enable-kvm -cpu host,vmx`を付けたらVMXON命令が動くようになった。

次に，VMCSの初期状態（GDT, IDT, RIP, RSP, CR0, ...）を埋めていく。明らかにおかしい値についてはカーネルのトレースメッセージ（`kvm_nested_vmenter_failed`）で確認できるので便利。VMLAUNCH命令が成功するようにはなったが，VMCS Exit Reason 33 (invalid guest state) で落ちる。色々試しているとLDTの設定が抜けているのが原因だった。

### 3日目
いったん休憩がてら，ELFローダなどEPT violationが起きた時の処理を実装。カーネルからは分離して，VM-Exitが起きたらユーザランドのサーバに処理を依頼する仕組みを作った。Linux KVMのような使い勝手でハイパーバイザを作れるようになった。

### 4日目
VMLAUNCHすると，なぜかゲストが `0x68` にアクセスしようとしてEPT violation（ページフォルトみたいなやつ）が起きる。VMCSに書いてあるIDTのベースアドレスを `0x1000` に返ると `0x1068` に変わった。0x68 / 8 = 13なので一般保護例外が起きてハンドラを探そうとしているところで落ちている。ゲストのGDTディスクリプタのlimitを`0xffffffff`とすべきところが`0xffff`になっているのが原因だった。

HLT命令でVM-Exitするようになった。嬉しい。

### 5日目
IO命令とVM-Exitからゲストに戻る（VMLAUNCH）処理を実装。シリアルポートが叩けるようになり，以下のようなプログラムが動くようになった。

```
.code32
.global boot
boot:
    // Send "Hi\n" to the serial port.
    mov dx, 0x3f8
    mov bl, 'i'
    mov cl, '\n'

    mov al, 'H'
    out dx, al

    mov al, bl
    out dx, al

    mov al, cl
    out dx, al

    hlt
```

ついでにロングモードにも対応し，Reseaの`boot.S`が通るようになった。VM-Entry controlレジスタのIA-32e mode guestビットをいじるタイミングで少しつまづいた。

### 6日目
MSRの保存・復帰，XSAVE/XSETBV命令を実装してVMCSのコントロールフラグを弄っているとReseaが動くようになった。

```
[kernel] [hv:hv_guest] Booting Resea v0.7.0...
[kernel] [hv:hv_guest] [kernel] WARN: MP table not found
[kernel] [hv:hv_guest] [kernel] new task #1: vm (pager=(null))
[kernel] [hv:hv_guest] [kernel] boot ELF: entry=0000000001014bb8
[kernel] [hv:hv_guest] [kernel] boot ELF: 0000000001000000 -> 0000000000314000 (268KiB)
[kernel] [hv:hv_guest] [kernel] boot ELF: 0000000003000000 -> 0000000000357000 (52KiB)
[kernel] [hv:hv_guest] [kernel] boot ELF: 0000000004000000 -> 0000000000000000 (5196KiB, zeroed)
[kernel] [hv:hv_guest] [kernel] new task #0: (idle) (pager=(null))
[kernel] [hv:hv_guest] [kernel] Booted CPU #0
[kernel] VMExit by CR Access: CR=3, access_type=0
[kernel] [hv:hv_guest] [vm] starting...
[kernel] [hv:hv_guest] [vm] launching hello...
[kernel] [hv:hv_guest] [kernel] new task #2: hello (pager=vm)
[kernel] [hv:hv_guest] [vm] ready
[kernel] VMExit by CR Access: CR=3, access_type=0
[kernel] VMExit by CR Access: CR=3, access_type=0
[kernel] VMExit by CR Access: CR=3, access_type=0
...
[kernel] [hv:hv_guest] [hello] Hello, World!
```

自信がついたのでLinuxカーネルのブートに挑戦。multibootに対応していると思っていたが，していないらしい。Linuxの独自仕様の為にリアルモード対応をするのは面倒くさいなあと思っていると，XenのPVHという仕組みでプロテクトモードからのブートに対応しているらしい。

WRMSR命令でGSBASEを設定する場合の対応等をやっているとCPUID命令でVMExitするところまで進んだ。

### 7日目
PVHで渡すブート情報や，CPUID命令のエミュレーションなどなどを実装するとLinuxのブートメッセージが流れ始めた。

```
[kernel] [hv:hv_guest] 
                       Linux version 5.10.0-rc7+ (seiya@seiya-nuc) (gcc (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0, GNU ld (GNU Binutils for Ubuntu) 2.34) #21 Sat Dec 12 23:
[kernel] [hv:hv_guest] Command line: console=ttyS0 earlyprintk=ttyS0
[kernel] [hv:hv_guest] CPU: vendor_id 'Resea HV!!!!' unknown, using generic init.
[kernel] [hv:hv_guest] CPU: Your system may be unstable.
```

ブートを続けるとVMExitせずハングアップしている。QEMUモニタのレジスタダンプを見るとゲストの情報っぽかったのでaddr2lineしてみる。

```
$ addr2line -e vmlinux.debug.elf ffffffff810027be 
/home/seiya/linux/init/calibrate.c:197
```

`calibrate_delay_converge` 関数で`jiffies`の変化が始まる（タイマが動き始める）まで待つビジーループで止まっている。どうやらタイマは自分でエミュレートする必要があるらしい。

MMIOを実装したくないが，CPUローカルなタイマや割り込み制御機構を担うLocal APICは `0xffe00000` に鎮座している。x2APICという新しめの方式を使うとMSR経由でアクセスできるようになるので問題解決となるはずだった。

悲しいことに，試してみるとLinuxは「途中から」APICへのアクセスをMSR経由に切り替えるようで `0xffe00000` をどうしても触ってしまう。カーネルコンフィグをいじって頑張った挙句，「KVM/Xenのフリをする or IRQ remapping周りの実装をする」の二択になることがわかってきた（[ここでつまづく](https://github.com/torvalds/linux/blob/13cb73490f475f8e7669f9288be0bcfa85399b1f/arch/x86/kernel/apic/apic.c#L1849-L1851)）。x2APICは諦めてMMIOの実装をしようと心に決めた。

MPテーブルを構築してLinuxがLocal APICの存在に気づけるようにしてあげて今日は終わり。

### 8日目
Local APICのエミュレーションのためにMMIOの実装から始める。とりあえずMOV命令のインタプリタを書けば十分な気がする。

EPT violationが起きたらゲストのレジスタ情報をユーザランドで動くハイパーバイザサーバに送って，そちらで命令のエミュレーションをすることにした。カーネル側でも出来なくはないが，ゲストのページテーブルやEPTを触るのが面倒だったのでやめた。

32-bit幅のメモリアクセスを伴うシンプル（SIBは非対応）なMOV命令だけエミュレーションを実装した。触るのはLocal APICとI/O APICぐらいだろうし，OR命令とかで直接触らずにラッパー関数（`native_apic_mem_read()`）経由でアクセスしているので当分は問題ないだろう。

Virtual APIC使えば今日の頑張りは必要なかった気がするが動くものが出来たのでよし。

### 9日目
VMCSをいじっているとタイマ割り込みをゲスト側で発火できるようになった。Linux側でNULL Pointer Dereferenceが出るので何事かと思ったら，PICで割り込みのマスクを実装したら解決した。すると，興味深いカーネルパニックを吐くようになった。

```
[kernel] [hv:hv_guest] Kernel panic - not syncing: Real mode trampoline was not allocated
```

どうやらリアルモードで動かしたいとき用のメモリ領域全てを予約済みとして教えているのが原因だった。一部を利用可能にしてあげると通るようになった。
キーボードコントローラのIOポートアクセスの対応等々をしていると馴染み深いカーネルパニックがようやく出現。

```
[kernel] [hv:hv_guest] 
                       VFS: Cannot open root device "(null)" or unknown-block(0,0): error -6
[kernel] [hv:hv_guest] Please append a correct "root=" boot option; here are the available partitions:
[kernel] [hv:hv_guest] Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(0,0)
[kernel] [hv:hv_guest] CPU: 0 PID: 1 Comm: swapper Tainted: G        W         5.10.0-rc7+ #61
[kernel] [hv:hv_guest] Call Trace:
```

普段は肝が冷えるメッセージだが今日はすごく嬉しい。Linuxカーネルの初期化が通るようになったのだ。わーい。

目標は「Hello Worldするだけの/initをディスクから読み込んで実行できる」ことにした。仮想ディスクをエミュレート出来たらそのまま動きそうな気がする。知見のあるvirtio-blkを実装しよう。

まずはvirtioの前にPCI configuration space周りを実装し，Linuxにvirtio-blkの存在を教える必要がある。[PCI周りのIOポートがきちんと動作しているのかのチェック](https://github.com/torvalds/linux/blob/148842c98a24e508aecb929718818fbf4c2a6ff3/arch/x86/pci/direct.c#L234) が存在するので，そこの対応をするとvirtioのprobeが走るようになった。

```
[kernel] [hv:hv_guest] virtio-pci 0000:00:01.0: virtio_pci: leaving for legacy driver
[kernel] [hv:hv_guest] virtio-pci: probe of 0000:00:01.0 failed with error -12
```

### 10日目
今日はvirtio-blkを作っていく。まずBARのサイズ取得機能などPCI周りでエミュレーションの追加した。するとvirtio-blkのIOポートを叩いてネゴシエーションを始めてくれるようになった。

いくつか些細な落とし穴を直すとディスク読み込み要求を取得できた。ただし，ゲストから指定されたデータ書き込み先のゲスト物理アドレスがまだマップされていないらしい。たぶんゲストがそのページに一度もアクセスしてないんだろう。EPT violation処理ハンドラを呼ぶようにして解決。

気の向くままに実装するとvirtio-blkの処理リクエスト（`struct virtio_blk_req`）が読めるようになった。もうちょっとで終わりそうなので残りは明日に持ち越すことに。

それにしてもシリアルポートが遅い気がする。一文字ずつVMExitするのが効いてるのだろうか。

### 11日目
まずはディスクイメージ作り。

```
$ musl-gcc -static hello.c -o init
$ dd if=/dev/zero of=disk.img bs=1024 count=256
$ fdisk disk.img
$ sudo kpartx -a disk.img
$ sudo mkfs.ext2 /dev/mapper/loop6p1
$ sudo mount /dev/mapper/loop6p1 /mnt
$ sudo mkdir -p /mnt/sbin
$ sudo cp init /mnt/sbin
$ sudo umount /mnt
```

virtio-blkの残りの部分を実装したがゲスト側で割り込みを起こそうとするとVMCS Guest Invalid Stateエラーが出る。タイマ割り込みはうまくいているので何故だろうと思っていたら，Guest Interruptibility Stateフィールドをいじる必要があったらしい。

ルートファイルシステムを認識して `/sbin/init` を起動するようになった。

```
[kernel] [hv:hv_guest] EXT4-fs (vda1): mounted filesystem without journal. Opts: (null)
[kernel] [hv:hv_guest] VFS: Mounted root (ext2 filesystem) on device 254:1.
[kernel] [hv:hv_guest] Freeing unused kernel image (initmem) memory: 564K
[kernel] [hv:hv_guest] Write protecting the kernel read-only data: 8192k
[kernel] [hv:hv_guest] Freeing unused kernel image (text/rodata gap) memory: 2044K
[kernel] [hv:hv_guest] Freeing unused kernel image (rodata/data gap) memory: 1696K
[kernel] [hv:hv_guest] Run /sbin/init as init process
```

virtio-blkデバイスが動くようになったようだ。しかし，Hello Worldは表示されずCR3の読み書きを永遠とするようになった。

```
[kernel] VMExit by CR Access: CR=3, access_type=0
[kernel] VMExit by CR Access: CR=3, access_type=1
[kernel] VMExit by CR Access: CR=3, access_type=0
[kernel] VMExit by CR Access: CR=3, access_type=1
[kernel] VMExit by CR Access: CR=3, access_type=0
[kernel] VMExit by CR Access: CR=3, access_type=1
...
```

VM-Exit元を調べてみると，システムコールハンドラの出入りでページテーブルを切り替えているの原因らしい。どうやら `writev(2)` は呼べているようだ。しかし`init` プロセスの出力が表示されない。何故だろう？

### 12日目
`writev(2)` からデバイスドライバまで辿ってみると，どうやらinitの標準出力は一旦バッファに溜め，割り込みが来てから（転送可能状態になってから）シリアルポートに転送しているらしい。今はシリアルポートから割り込みを送る実装をしていないので，確かに合点がいく。

シリアルポートから送信可能を示す割り込みを送るようにしたり，シリアルポートのステータスレジスタをいじったり，HLTからの復帰処理を直したりしていると，ずっと見たかった表示がようやく出現。

```
[kernel] [hv:hv_guest] Run /sbin/init as init process
[kernel] [hv:hv_guest]  _          _ _          __                  random: fast init done
[kernel] [hv:hv_guest]    
[kernel] [hv:hv_guest] | |__   ___| | | ___    / _|_ __ ___  _ __ ___  
[kernel] [hv:hv_guest] | '_ \ / _ \ | |/ _ \  | |_| '__/ _ \| '_ ` _ \ 
[kernel] [hv:hv_guest] | | | |  __/ | | (_) | |  _| | | (_) | | | | | |
[kernel] [hv:hv_guest] |_| |_|\___|_|_|\___/  |_| |_|  \___/|_| |_| |_|
[kernel] [hv:hv_guest]                                                 
[kernel] [hv:hv_guest]     __   _     _          ___       _ _   
[kernel] [hv:hv_guest]    / /__| |__ (_)_ __    / (_)_ __ (_) |_ 
[kernel] [hv:hv_guest]   / / __| '_ \| | '_ \  / /| | '_ \| | __|
[kernel] [hv:hv_guest]  / /\__ \ |_) | | | | |/ / | | | | | | |_ 
[kernel] [hv:hv_guest] /_/ |___/_.__/|_|_| |_/_/  |_|_| |_|_|\__|
[kernel] [hv:hv_guest]                                           
```

Linuxがブートできるハイパーバイザができた 🎉
