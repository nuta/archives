---
title: この個人サイトは自作OSで動いています
date: Dec 23, 2021
lang: ja
---

あなたの予想に反して、このページが見えているでしょうか？このWebサイトは自作OSの[Kerla](https://github.com/nuta/kerla)が提供しています。

これは[自作OS Advent Calendar 2021](https://adventar.org/calendars/6581)の23日目の記事です。

## 自作OS「Kerla」の紹介

[Kerla（かーら）](https://github.com/nuta/kerla)はRustで書かれたLinux ABI互換モノリシックカーネルです。今年の春頃から作り始め、[Dropbear](https://matt.ucc.asn.au/dropbear/dropbear.html)というSSHサーバが動作する程度には基本的なUNIXの機能が実装されています。具体的には、ファイルの読み書きやUDP/TCPソケット、fork/exec、シグナル、擬似端末といったものです。遊んでみたい人は SSHで実際に動いているKerlaにログインできます。SSH接続ごとに専用のVMがFirecracker上で立ち上がるようになっています。

```
$ ssh root@demo.kerla.dev
```

カーネル実装の雰囲気を軽く紹介すると、Kerlaでは以下のようにシステムコールが実装されています。

```rust
/// write(2): ファイルへの書き込みをするシステムコール
pub fn sys_write(&mut self, fd: Fd, uaddr: UserVAddr, len: usize) -> Result<isize> {
    let len = min(len, MAX_READ_WRITE_LEN);

    let opened_file = current_process().get_opened_file_by_fd(fd)?;
    let written_len = opened_file.write(UserBuffer::from_uaddr(uaddr, len))?;

    Ok(written_len as isize)
}
```

見た目がすっきりしている上に、`UserVAddr` や `UserBuffer` といったユーザポインタを安全に扱う型が導入してあり便利そうではないでしょうか。

ビルドシステムもそこそこまともで、Dockerイメージをそのまま動かせるようになっています。例えば以下のように`IMAGE=nuta/helloworld` と指定すると、[このDocker Hubのイメージ](https://hub.docker.com/r/nuta/helloworld)をKerla上で実行します。

```
$ make IMAGE=nuta/helloworld LOG=trace run
...
[   0.029] syscall: execve(439398, 4393b8, 4393c8, 8, 2f2f2f2f2f2f2f2f, 8080808080808080)
[   0.030] syscall: arch_prctl(1002, 4055d8, 0, 20000, 0, ff)
[   0.031] syscall: set_tid_address(4057f0, 4055d8, 0, 20000, 0, ff)
[   0.033] syscall: ioctl(1, 5413, 9ffffeed0, 1, 405040, 9ffffeef7)

 _          _ _                            _     _ _
| |__   ___| | | ___   __      _____  _ __| | __| | |
| '_ \ / _ \ | |/ _ \  \ \ /\ / / _ \| '__| |/ _` | |
| | | |  __/ | | (_) |  \ V  V / (_) | |  | | (_| |_|
|_| |_|\___|_|_|\___/    \_/\_/ \___/|_|  |_|\__,_(_)
```

まだまだ未実装な機能が多いので動作するイメージには限りがありますが、busyboxやcurlといった昔ながらのプログラムはそこそこ動きます。

## 自作OSで個人サイトを動かしたい

せっかく時間をかけてOSを書いてるので、何かしら実際に使いたいと常々考えていました。特に個人サイトを自作OSで提供することは野望でした。自分のOSを多くの人に触れてもらえる機会になりますし、サイトが落ちたとしても誰も困らないので失うものがありません。そして、このページの最後のように「このWebサイトは自作OSで動いてます」と書いてあるとかっこいいのです。

このWebサイトはロードバランサを挟んでDigitalOcean上のVMで動いているKerlaが提供しています。サンフランシスコにデプロイしてあり、自作OSと通信してほしいという思いからCDNはわざと通していません。Kerlaは新機能の実装ばかりで性能のための改善は特に入れていないのですが、案外レスポンスが早く感じませんか？

以降ではクラウド上で自作OSをどう動かすか、デプロイの仕組み、はまったバグなどをご紹介します。

----

## DigitalOceanの解剖

自作OSをクラウド上で動かしたいときに困るのは「メジャーOS以外を考慮していない」問題です。メジャーOSだとクラウド事業者がいい感じに設定してくれているのを、自作OSでは自分でやる必要があります。

今回は個人サイトを提供したいので、どうやって自作OSのTCP 80に繋げられるかが重要です。ということで、UbuntuのDropletを建ててネットワーク周りの構成を解明していきます。

## 2つのNIC

まずはどんなNICが刺さっているかを`lspci`で見てみます。

```
root@ubuntu-digitalocean:~# lspci -v -x
00:03.0 Ethernet controller: Red Hat, Inc. Virtio network device
	Subsystem: Red Hat, Inc. Virtio network device
	Physical Slot: 3
	Flags: bus master, fast devsel, latency 0, IRQ 10
	I/O ports at c0c0 [size=32]
	Memory at fd012000 (32-bit, non-prefetchable) [size=4K]
	Capabilities: [40] MSI-X: Enable+ Count=3 Masked-
	kerla driver in use: virtio-pci
00: f4 1a 00 10 07 05 10 00 00 00 00 02 00 00 00 00
10: c1 c0 00 00 00 20 01 fd 00 00 00 00 00 00 00 00
20: 00 00 00 00 00 00 00 00 00 00 00 00 f4 1a 01 00
30: 00 00 00 00 40 00 00 00 00 00 00 00 0b 01 00 00

00:04.0 Ethernet controller: Red Hat, Inc. Virtio network device
	Subsystem: Red Hat, Inc. Virtio network device
	Physical Slot: 4
	Flags: bus master, fast devsel, latency 0, IRQ 11
	I/O ports at c0e0 [size=32]
	Memory at fd013000 (32-bit, non-prefetchable) [size=4K]
	Capabilities: [40] MSI-X: Enable+ Count=3 Masked-
	kerla driver in use: virtio-pci
00: f4 1a 00 10 07 05 10 00 00 00 00 02 00 00 00 00
10: e1 c0 00 00 00 30 01 fd 00 00 00 00 00 00 00 00
20: 00 00 00 00 00 00 00 00 00 00 00 00 f4 1a 01 00
30: 00 00 00 00 40 00 00 00 00 00 00 00 0b 01 00 00
```

準仮想化デバイスのvirtio-netが2つ刺さっています。それぞれどんなIPアドレスが設定されているかみてみましょう。

```
root@ubuntu-digitalocean:~# ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP group default qlen 1000
    link/ether f6:71:04:9d:cb:0c brd ff:ff:ff:ff:ff:ff
    inet 147.182.229.125/20 brd 147.182.239.255 scope global eth0
       valid_lft forever preferred_lft forever
    inet 10.48.0.5/16 brd 10.48.255.255 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::f471:4ff:fe9d:cb0c/64 scope link
       valid_lft forever preferred_lft forever
3: eth1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP group default qlen 1000
    link/ether 72:a3:af:7a:d1:06 brd ff:ff:ff:ff:ff:ff
    inet 10.124.0.2/20 brd 10.124.15.255 scope global eth1
       valid_lft forever preferred_lft forever
    inet6 fe80::70a3:afff:fe7a:d106/64 scope link
       valid_lft forever preferred_lft forever
```

たぶん以下のような感じです。グローバルIPアドレスとプライベートネットワークに別々のNICをアタッチしているようです。

- `147.182.229.125/20 (eth0)`: グローバルIPアドレス
- `10.48.0.5/16 (eth0)`: DigitalOcean内部で使っていそうなプライベートIPアドレス
- `10.124.0.2/20 (eth1)`: プライベートネットワーク に繋がっているプライベートIPアドレス

## DHCPは使っていない？

Kerlaはカーネル内のDHCPクライアントでIPアドレスを自動設定します。しかし、Dropletの中でDHCPを使っている気配がありませんし、DHCP DISCOVERを送っても誰も返事をしてくれません。悲しいことに、FreeBSDのインスタンスを調べるとこんなことが書いてありました。

```
root@freebsd-s-1vcpu-1gb-sfo3-01:~ # cat /etc/network/interfaces
# Generated by the DigitalOcean provisioning process on 2021-12-03T13:09:56Z
# See 'man interfaces' on a Debian/Ubuntu systems.
# The network configuration was generated from http://169.254.169.254/metadata/v1.json.
# You may also find the it on the locally attached CDROM under 'digitalocean_meta_data.json'
```

メタデータサーバからとってきて設定するプログラムが入っているようです。メタデータサーバはEC2とかにもあるインスタンス固有の情報を返してくれる便利なサーバで、以下のようなJSONを返してくれます。

```
root@ubuntu-digitalocean:~# curl -s http://169.254.169.254/metadata/v1.json | jq .interfaces
{
  "private": [
    {
      "ipv4": {
        "ip_address": "10.124.0.2",
        "netmask": "255.255.240.0",
        "gateway": "0.0.0.0"
      },
      "mac": "72:a3:af:7a:d1:06",
      "type": "private"
    }
  ],
  "public": [
    {
      "ipv4": {
        "ip_address": "147.182.229.125",
        "netmask": "255.255.240.0",
        "gateway": "147.182.224.1"
      },
      "anchor_ipv4": {
        "ip_address": "10.48.0.5",
        "netmask": "255.255.0.0",
        "gateway": "10.48.0.1"
      },
      "mac": "f6:71:04:9d:cb:0c",
      "type": "public"
    }
  ]
}
```

「じゃあメタデータサーバに接続する用のIPアドレス等はどうするんだ」という話になる訳ですが、代わりに[Config Drive](https://cloudinit.readthedocs.io/en/latest/topics/datasources/configdrive.html)という仮想ディスク（これもvirtioデバイス）から読み取っているようです。

後々発見したDigitalOceanのトリビアですが、OSの種類が「Unknown」のカスタムイメージでDropletを作成するとDHCPサーバが応答するようになります。UbuntuやFreeBSDならcloud-initに対応しているから要らないだろうという判断なのでしょう。

## ロードバランサはどこから？

次に、ロードバランサがどのIPアドレスに向けてリクエストを送ってくるかを確認します。単に`tcpdump`をするだけです。

```
root@ubuntu-digitalocean:~# tcpdump -nni eth1 port 80
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on eth1, link-type EN10MB (Ethernet), capture size 262144 bytes
12:40:28.768375 IP 10.124.0.3.57400 > 10.124.0.2.80: Flags [S] ...
12:40:28.768437 IP 10.124.0.2.80 > 10.124.0.3.57400: Flags [S.] ...
12:40:28.769244 IP 10.124.0.3.57400 > 10.124.0.2.80: Flags [.] ...
```

プライベートネットワーク経由で通信しています。`10.124.0.3`がロードバランサに割り当てられているようです。

## カスタムイメージを動かそう

当たり前ですがDigitalOceanはKerlaを正式サポートしていないので、自分でOSイメージを用意する必要があります。[ドキュメント](https://docs.digitalocean.com/products/images/custom-images/)によるとqcow2形式が使えるようです。

[ドキュメント](https://docs.digitalocean.com/products/images/custom-images/)には「ext3/ext4にしておくこと」だとか「cloud-initをインストールしておくこと」だとかをrequirementsとして列挙していますが、従わなくても動きます。必須事項というよりは推奨くらいのニュアンスなのでしょう。

「Limited Capacity」だといわれてDropletを作れない時は、他のリージョンにカスタムイメージを追加して試してみましょう。自分の環境だと、NYC3とSFO3は使えました。

## Recovery Console

最後にデバッグ方法の確認です。残念ながらDigitalOceanにはシリアルポートがないようですが、画面をRecovery Consoleから確認することができます。

![リカバリー画面](kerla-in-digitalocean-recovery-console.png)

普通にブートできているようです。ただvirtioの実装が少し合ってなさそうです。

## KerlaのDigitalOcean対応

以上の調査で分かったことは以下の通りです。

- NICは2つ存在。両方virtio-net。デバイスIDが0x1000なのでTransitional Deviceと呼ばれるvirtioの新旧両方の仕様に対応してあるデバイス。PCI上で繋がっている。
- IPアドレス等はConfig DriveにあるJSONファイルに書いてある。
- ロードバランサはプライベートネットワーク経由で通信してくる。今回はロードバランサに向けてHTTPサーバを公開すれば十分なので、`eth1`のみ使えば良さそう。
- カーネルイメージはGRUB + Multiboot2のままで変更なしにブートできる。

## seiya.meの仕組み
以上の研究を踏まえてseiya.meは次の図のように構成されています。

![seiya.meの全体像](seiyame-architecture.svg)

DigitalOcean上では以下のインスタンスが動いています。

- **Ubuntu VM**
  - 雑用サーバ。Kerla監視用のPrometheusとかが建っている。
- **Kerla VM**
  - KerlaとUbuntuのデュアルブート。Kerlaが起動すると、古き良きfork/execスタイルの自作HTTPサーバが立ち上がる。
- **ロードバランサ**
  - TLS終端とKerla VMが1台落ちても大丈夫なようにすることが目的。

## デプロイの流れ

以下の流れで動いています。

1. GitHub ActionsでWebサイトをビルド（自作の静的サイトジェネレータ）してinitramfsとしてKerlaに埋め込み、sftpで雑用サーバにOSイメージをアップロード。KerlaのVMを順番に強制リブート。
2. まずUbuntuが起動する。OSイメージのダウンロードとGRUBの設定等を行いリブート。また、カーネルクラッシュログが残っていたらメールを送る。
3. GRUBがKerlaを起動し、自作HTTPサーバが立ち上がる。
4. ロードバランサが復活を検知して、ユーザリクエストを流すようになる。
5. カーネルクラッシュしたら、boot2dump（後述）がクラッシュログをディスクに保存し、2に戻る。

全てTerraformで書いてあるので、全てが崩壊してもコマンドひとつで復活するようになっています。

----

ここからは、実装の細かい話をしていきます。

## legacy virtioドライバの実装

ここからようやく自作OSの話です。virtioデバイスにはlegacyとmodernの2種類の仕様（というか仕様のバージョン）があります。機能面では大して変わりないですがmodernの方が名前の通り新しく、仕様が綺麗に整理されています。KerlaではFirecrackerがmodernデバイスを実装しているということで、そちらのドライバを実装していました。

lspciのダンプを見る限り、DigitalOceanのvirtio-netドライバはTransitional Deviceと呼ばれるlegacy/modern両方の仕様を実装しているデバイスです。Kerlaのドライバがそのまま動くはずなのですが、どうやらmodern互換デバイスではないようでLinuxカーネルでもlegacyドライバにfallbackしています。

```
root@dev:~# dmesg -t | grep virtio
virtio-pci 0000:00:03.0: virtio_pci: leaving for legacy driver
```

とはいえ手元のQEMU（v6.1.0）だと`-device virtio-net-pci-transitional`を指定してもきちんとmodernデバイスとして動きます。[QEMUを使っていそうな感じはする](https://github.com/digitalocean/go-qemu)のですが、lscpu等を見る限りCPUのモデル名などをDigitalOceanに書き換えていたりします。QEMUにそこそこパッチを当てていて、virtio周りにも手を加えているのかもしれません。

仕方ないのでlegacyドライバを書きました。

## しばらくすると応答しなくなるバグ

CI・CD周りがきちんと動くようになり喜んでいたのも束の間、HTTPリクエストを12回処理した後に特にカーネルパニックする訳でもなく止まってしまうようになりました。毎月500円かかるんですからバグを直してしっかり働いてもらいます。

数日間色んな可能性を片っ端から有ったっても原因が分からず頭を抱えていたところ、成功する時のパターンがあることに気づきました。それは**送信処理時に適当なログメッセージを入れる時**です。つまり、デバッグのために変数の中身を観測しようとすると問題が再現しなくなるのです。詰みました。

## バグの正体

結局のところ、Virtioの使用済みディスクリプタを回収するロジックが原因でした。

Virtioでは、パケットの送信のようなデバイスへの命令を「ディスクリプタ」というフォーマットで指定します。OSはディスクリプタに処理内容を書き込んで、デバイス（ハイパーバイザ）に処理を要求します。デバイスドライバは空きディスクリプタを連結リストで管理しており、`free_head`という変数が先頭を指しています。

![](virtio-gc-bug-1.svg)

パケットの送信等でディスクリプタが必要になると、`free_head`から必要な分だけディスクリプタを取り出して切り取ります。例えば2個必要な場合は以下のようになります。

![](virtio-gc-bug-2.svg)

空きディスクリプタ数が少なくなると、使用済みディスクリプタを回収して連結リストの先頭に差し込んでいきます。

![](virtio-gc-bug-3.svg)

... となるはずだったのですが、回収処理に小さなバグがあり以下のような状態になってしまっていました。

![](virtio-gc-bug-4.svg)

1番の後ろには他の空きディスクリプタが続くべきなのですが、0番を指してしまっています。つまり、**0番と1番が常に空きディスクリプタとして割り当てられる**ことになります。例えば5つディスクリプタを使おうとすると、`0 -> 1 -> 0 -> 1 -> 0`が**常に**返ってくるわけです。

ただし、これは常にバグとして現れません。OSが次のパケットを送信する前にデバイス（ハイパーバイザ）が前のパケット送信処理を実行し終えれば上手く動きます。間に合わなければ、0・1番ディスクリプタが上書きされて違うパケットが送られることになります。加えて、virtio-netのパケット送信にはちょうどディスクリプタを2つだけ使うので、`0 -> 1 -> 0` のようなディスクリプタの被りが起きないのです。おそらくこれが「しばらくすると稀に応答しなくなるバグ」の原因なのでしょう。ローカルのQEMUでは今まで再現したことがないので、てっきりこの部分はバグがなく安定していると思いこんでいました。中途半端に壊れるバグって辛いですよね。

## カーネルパニック

わざわざ自作OSでWebサーバを提供する理由には、自己満足以外に長時間動かした時にしか起きないバグを見つけるという大切な目的があります。メモリリークや稀に起きるデッドロックですね。

ただ、ひとつ問題があります。それは「カーネルパニックのログをどう収集するか」です。Webサーバが反応しなくなったらWebブラウザからDigitalOceanのリカバリー画面を開けば分かりますが面倒です。puppeteerでリカバリー画面のスクリーンショットを撮ってOCRをかけるという手も考えましたが、なんだか面白くないのでカーネル内で頑張ることにしました。

どうやるかというと、カーネルパニックが起きるとそのカーネルが正しく動作するとは信頼できないので、ログをディスクに保存する用の別のOSを起動します。それが[boot2dump](https://github.com/nuta/boot2dump)です。[Resea](https://github.com/nuta/resea)がベースになっています。

Kerlaに依存していないので、どんな自作OSでも使えるようにしてあります。特に[Rustだとライブラリにまとめてある](https://docs.rs/boot2dump/0.0.1/boot2dump/)ので簡単に使えます。位置独立実行形式（PIE）なので、バイナリイメージをobjcopyか何かで埋め込んでおいて、エントリポイント（イメージの先頭）に`JMP`するだけです。

boot2dumpはext4ファイルシステムを解釈して、指定されたメモリバッファをファイルに保存した後、Triple Faultをわざと起こしてリブートさせます。その後Ubuntuが起動し、クラッシュログをメールで送信します。

![Gmailに届いたカーネルパニックログ。ページテーブルをゼロクリアしていないのが原因だった。](kerla-crash-log-email.png)

## メトリクス収集

最後にメトリクスの収集です。カーネルクラッシュログはデバッグに大いに役立ちますが「クラッシュが起きるまで何が起きていたか」を知るには不十分です。例えば、HTTPリクエスト数と比例してじわじわと空きページ数が減っていったのであれば、プロセス解放処理やTCPソケット周りでメモリリークがあるのではないかと当りをつけることができます。

ということで、Prometheusがそのまま読み取れるフォーマットでカーネルの統計情報を出力する `/proc/metrics` という独自機能が[Kerlaには実装されています](https://github.com/nuta/kerla/blob/main/kernel/fs/procfs/metrics.rs)。

後は雑用サーバからscrapeしてGrafana Cloudに送るだけです。次の画像のように上手く動いています。

![メモリリークを直してuptimeが飛躍的に向上した様子](kerla-monitored-by-prometheus.png)

加えて、Grafana Cloudのアラート機能を使って「アクティブなKerla VMがn台以下になったら連絡する」ようなことを実現できるようになりました。これで安心です。

----

## 今後の予定

Linux ABI互換カーネルは実用性とコントリビュートしやすさを考えると良い特徴なのですが、かなり地道な作業で途方に暮れがちです。独自OSだと自由自在に面白いアイデアが試せる（例えば[相対パスという概念の廃止](https://fuchsia.dev/fuchsia-src/concepts/filesystems/dotdot)）のですが、互換OSでは一生「互換性」と向きあうことになります。それはそれで面白いんですが永遠に「Linuxのサブセット」として生きることになります。やっぱり何か新しいことをやっていきたいんです。

というわけで、来年はKerlaにLinux ABI互換以外の特色を加えていこうと考え始めています。具体的には、[Tock](https://www.tockos.org/)や[Hubris](https://hubris.oxide.computer/)のようなRustの力で静的なシステムを安全に実装することを目指すOSを参考にして「安全なカーネルモジュール」を模索してみるだとか、[nebulet](https://github.com/nebulet/nebulet)のようにWebAssemblyをカーネル空間で動かして[オーバーヘッドを考えるとソフトウェアによる分離は遅い](https://microkerneldude.org/2008/03/13/q-what-is-the-difference-between-a-microkernel/)という話を検証するとかを考えています。

## よくありそうな質問

- **Q. どれくらい安定している？**
  - **A.** メモリ不足（つまりメモリリーク）が原因で数日間隔で再起動しています。Grafanaでメトリクスを見る感じ空きページ数に変化がないので、フラグメンテーションが起きている気がします。
- **Q. よくあるカーネルパニックは？**
  - **A.** 今はもうメモリ不足でのパニックしか起きないようになりました。バグをたくさん見つけたいのにバグが現れません。大してテストしていないのに上手く動いているので驚きです。Rustで書いたおかげでしょう。
- **Q. Node.jsやGoのWebアプリケーションは動く？**
  - **A.** 動きません。[スレッド](https://github.com/nuta/kerla/pull/115)や非同期プログラミングに使われている[epoll](https://github.com/nuta/kerla/pull/102)がといった必要な機能がまだ開発中です。
- **Q. 費用は？**
  - **A.** Kerlaサーバ $5 x 2、雑用Ubuntuサーバ $5、ロードバランサ $10 = $25（月額）<br>
  Dropletのスペックは全て1コア・1GBメモリです。
