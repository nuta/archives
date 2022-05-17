---
title: Linuxバイナリ互換機能を自作OSカーネルに実装する
title_en: Implementing Linux binary compatibility in my homebrew OS
lang: ja
date: May 10, 2020
---

せっかくのゴールデンウィークなので，自作OSカーネルの[Resea](https://github.com/nuta/resea)にLinux ABIを実装して**Linuxバイナリを無修正で動作させる**ことにした。具体的にはBusyBoxのシェルから`uname(1)`を実行できるところまでが目標。

![demo](linux-abi-demo.png)

## 先行事例
「他のカーネルの実行可能ファイルをそのまま動かす」という仕組みはそこそこ事例がある。

- [Windows Subsystem for Linux（WSL）](https://github.com/microsoft/WSL)
  - 近年話題のWindows上でLinuxバイナリをシームレスに実行するやつ。WSL2ではLinuxカーネルをそのまま動かしているが，WSL1は[面白い実装をしている](https://channel9.msdn.com/Blogs/Seth-Juarez/Windows-Subsystem-for-Linux-Architectural-Overview)。
  - WSL以前には[Microsoft POSIX subsystem](https://en.wikipedia.org/wiki/Microsoft_POSIX_subsystem)という機能が提供されていたらしい。
- [FreeBSD（Linuxulator）](https://www.freebsd.org/doc/en_US.ISO8859-1/articles/linux-emulation/article.html)
  - 同じUNIXライクなカーネルとはいえ，細かいところが結構違うらしい。
- [NetBSD](https://www.netbsd.org/docs/guide/en/chap-linux.html)
  - LibreOfficeのLinuxビルドを動かす例が載っている。すごい。
- [OSv](https://github.com/cloudius-systems/osv/wiki/OSv-Linux-ABI-Compatibility)
  - Linuxバイナリをそのまま動かせるユニカーネル。ただし，シングルプロセスのみ対応しているので`fork(2)`等は未対応。と言ってもスレッドはあるので問題ないはず。
- [Noah](https://dl.acm.org/doi/10.1145/3381052.3381327)
  - LinuxバイナリをmacOSから動かすソフトウェア。ハイパーバイザの仕組みを使っているがLinuxカーネルが動いているわけではない面白い実装。
- [Geoffrey Lee and Charles Gray. L4/Darwin: Evolving UNIX.](https://ts.data61.csiro.au/publications/papers/Lee_Gray_06.pdf)
  - L4マイクロカーネル上でMac OS XのMach-Oバイナリを動かすという取り組み。[iPhoneのSecure Enclaveで動いているOSはこれの派生らしい](https://www.blackhat.com/docs/us-16/materials/us-16-Mandt-Demystifying-The-Secure-Enclave-Processor.pdf)。
- **Mach**
  - BSDとバイナリ互換があったという話をどこかで読んだ。システムコールが呼ばれたらそれをトラップする仕組みだった気がする。今回Reseaが実装した仕組みに近い。

## Linuxバイナリの動作に必要なもの（Linux ABI）
x86_64のLinuxバイナリが動くには最低限以下の実装が必要。

- LinuxシステムコールやC標準ライブラリ（libc）の基本的な知識。`strace(2)`と`man(1)`を使ってLinuxバイナリの挙動を理解する必要がある。動かない時に原因をlibcを読むなどして調査する力が必要。
- `SYSCALL`命令の対応。名前の通り，ユーザプロセスがシステムコールを呼ぶ時に使う命令。EAXレジスタにシステムコールの種類，RDI等にシステムコールの引数が入っている。自作OSのシステムコールとは別に処理する（フック機構を入れる）必要がある。
  - `SYSCALL`命令が上書きする`RCX`と`R11`以外の汎用レジスタはシステムコールハンドラが保持する必要があるので注意。caller-savedレジスタもきちんと復元しないといけない。
- ユーザプロセスのメモリ空間からの安全なメモリコピー。Linuxカーネルで言うところの`copy_from_user`関数とか。バイナリ互換機能をカーネル内部でなく（マイクロカーネル上の）ユーザプロセスで実装しようとすると，設計によってはちょっと面倒くさいかも。
- プロセス起動時のコマンドライン引数，環境変数，補助ベクタの用意。スタックに置くとメモリ割り当てが減ってちょっと楽。補助ベクタは主にlibcが使う情報で，ELFのプログラムヘッダのアドレスやstack canaryに使う乱数など，雑多な情報が載っている。`AT_NULL`だけでも簡単なプログラムなら動く。
- `FS`レジスタのベースアドレスのセット。スレッド固有ストレージ（TLS）の実現に利用されているようで，`arch_prctl(2)`経由でユーザがセットしてくる。`WRFSBASE`命令で簡単に書き込める。

## ReseaでのABIエミュレーションの仕組み
マイクロカーネルには意地でもLinux-specificな機能を入れたくないので，次のような汎用的なABIエミュレーション機構を導入した。

1. Linuxバイナリがシステムコールを呼ぶ。
2. カーネルのシステムコールハンドラがABIエミュレーションが有効されていることを確認。
3. 呼び出しコンテキスト（プログラムカウンタ，スタックポインタ，汎用レジスタなど）をメッセージに詰め，OSサーバ（Linux ABIを提供するユーザプロセス）にシステムコール処理要求を送信する。
4. OSサーバはメッセージを受けて該当する処理（`write(2)`など）を実行し，新しいコンテキスト
   をメッセージに詰めて返信する。
5. 受け取ったコンテキストを復元してユーザランド処理を続行する。

マイクロカーネルはコンテキストの取得と復元をするだけなのに，これだけで十分ABIエミュレーションができる。`fork(2)`のような少し奇妙な動作をするシステムコールもこの範疇ですっきり実装ができた。復帰先のプログラムカウンタなども指定できるようにしたので，シグナルもこの仕組みを使って実現できそう。

## ソースコードの構成
Linux ABIサーバ（名前はMinLin）の[ソースコードはここ](https://github.com/nuta/resea/tree/master/servers/minlin)。上記のABIエミュレーションの仕組みを使って，カーネル内部ではなく普通のメッセージパッシングをする**ユーザプロセスとして**OSサーバを実装した。

- プロセス関連（`proc.c`）: forkやexecといったプロセスの生成周り
- メモリ管理（`mm.c`）: ページフォルトやユーザプロセスからのメモリコピー処理
- ファイルシステム管理（`fs.c`）: ファイルの読み書きとかの抽象化レイヤ
- 端末関連（`tty.c`）: キーボード入力と画面出力周り
- システムコール処理（`syscall.c`）: 名前の通り

## 実装したシステムコール
これだけ実装するととりあえずBusyboxのシェルが動く。
```
open
read
write
writv
stat
fork
exec
wait4
brk
getpid
arch_prctl: ARCH_SET_FSのみ対応
ioctl（未実装: とりあえず0を返す）
close（未実装: とりあえず0を返す）
rt_sigaction（未実装: とりあえず0を返す）
rt_sigprocmask（未実装: とりあえず0を返す）
set_tid_address（未実装: とりあえず0を返す）
```

## 知見
- Linuxで動かす時と自作OS上で動かす時で挙動が違う（特に異なるシステムコールが呼ばれる場合）と，自作OSのバグの可能性が高い。`.bss`セクションのゼロクリアし忘れとか。
- [System V ABI](https://uclibc.org/docs/psABI-x86_64.pdf)の付録（*Appendix A: Linux Conventions*）にLinuxにおけるシステムコール周りの仕様が載っている。
  - Linuxでは，`-4095 <= x <= -1`を満たすシステムコールの戻り値`x`は**常にエラーを示す**らしい。つまり，その正常終了した時にその範囲の値を返すことはない。この性質はちょっと便利。
- glibcではなくmuslをlibcとして使うと，動作しない時の原因究明がしやすく，スタティックリンクしてもバイナリサイズが小さいのでおすすめ。

## 今後の課題
ソケットや`proc`ファイルシステム，ダイナミックリンク，スレッド対応などなど盛りだくさんの未実装機能が残っているので暇な時間を見つけて実装していきたい。

Linux環境を提供するのは一つのユーザプロセス（OSサーバ）なので，単にこれを複数動かすことでコンテナみたいなことも結構簡単に実現できそうなので試したい。

## おまけ: 開発日記
以下，開発の日記。

### 5/2
Ryzen 9 3900Xとその仲間たちが届いた。PCの組み立てとセットアップで一日を終える。明日から頑張る。6年前のMacBook Proをずっと使ってきたので，何もかもが爆速でびっくり。

### 5/3
まずは以下のHello Worldプログラムが動くレベルが今日の目標。

```x86asm
.code64
.intel_syntax noprefix
.global _start
_start:
    ; write(stdout, "Hello, World from Linux ABI!\n", 29)
	mov rax, 1
	mov rdi, 1
	lea rsi, [rip + msg]
	mov rdx, 29
	syscall

    ; exit(0)
	mov rax, 60
	mov rdi, 0
	syscall

	ud2

.section .rodata
msg:
	.ascii "Hello World from Linux ABI!\n"
```

まず，マイクロカーネルに「ABIエミュレーションが有効化されているタスクからのシステムコールを，メッセージに変換してページャタスク（OSサーバ）に送る」という機構を加えた。具体的には，以下のシステムコール呼び出しコンテキストの取得と復元（ユーザプロセスへの復帰）をメッセージパッシング経由でできるようにした。

```c
struct abi_emu_frame {
    uint64_t fsbase;
    uint64_t gsbase;
    uint64_t rip;
    uint64_t rflags;
    uint64_t rbp;
    uint64_t rax;
    uint64_t rbx;
    uint64_t rdx;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rsp;
} PACKED;
```

次に，他のサーバからELFローダを流用しLinuxバイナリをロードしてみる。ちっちゃいのであまりデバッグせずすぐ動いてくれた。

### 5/4
以下のHello Worldプログラムが動くレベルが今日の目標。昨日のアセンブリプログラムと同じように見えるが，`main`関数が呼ばれる前にC標準ライブラリが行う初期化処理の対応が必要になる。

```c
// musl-gcc -static -o hello hello.c
int main(void) {
    puts("Hello World from Linux ABI!");
}
```

libcには，シンプルですっきりした実装のmuslを採用。とりあえず，muslが何をやるのかstraceで見てみる。

```
$ strace -f ./hello              
execve("./hello", ["./hello"], 0x7ffe2c051b58 /* 45 vars */) = 0
arch_prctl(ARCH_SET_FS, 0x4055d8)       = 0
set_tid_address(0x4057f0)               = 27010
ioctl(1, TIOCGWINSZ, {ws_row=88, ws_col=85, ws_xpixel=0, ws_ypixel=0}) = 0
writev(1, [{iov_base="Hello World from Linux ABI!", iov_len=27}, {iov_base="\n", iov!) = 28
exit_group(0)                           = ?
```

`arch_prctl(2)`でFSレジスタのベースアドレスをセットしている。TLSに使うっぽい。
`set_tid_address(2)`はとりあえず無視で良さそう。`ioctl(2)`でやっているのは，出力先が
ttyか否かをチェックしているらしい。`writev(2)`で文字列を出力して`exit_group(2)`でプロセス終了という流れ。

Reseaで動かしてみるととなぜかmmapを呼んでくる。muslのコードを読んでみるとTLSの初期化時に条件次第でmmapしているっぽい。
結局.bssセクションがゼロクリアされていないのが原因だった。具体的にはELFローダが`p_memsz > p_filesz`のケースの対処をしていなかった。

### 5/5
BusyBoxの `echo(1)` を動かすのとファイルシステム周りの設計が目標。まず最初にglibcを静的リンクしたやつが動くか試す。

色々実装した挙句，XMMレジスタを触るところで無効命令例外が出てしまう。十中八九カーネルのバグだが，今はLinux ABI実装に専念したいので修正は後回しにしてmuslをglibcの代わりに使うようことに。ELFローダのバグを一つ潰したら`echo(1)`は動くようになった。

### 5/6
ファイルシステム周りの実装をしてBusyBoxの`cat(1)`が動くことが目標。FATファイルシステムサーバがすでにあるので，そこに繋がるようにするだけでちゃんと動いた。

昨日，XMMレジスタを触る部分が動かないのでmuslに代えたが，muslの方でも使っているようで無効命令例外が出るようになってしまった。調べてみると，Extended Control RegisterというレジスタをいじってSSEを有効化する必要があるらしい。

すぐに目標達成してしまったので，独自Linuxディストリビューションのビルドシステムの整備をした。当初はAlpine Linuxをそのまま使うつもりだったが，ASLRの実装が必要なようで面倒くさい。そこで，Resea用にちっこいLinuxディストリビューションを作ることにした。とりあえずBusyboxをDockerコンテナ内でビルドできるようにしてひとまず終わり。

BusyBoxは実行ファイルへシンボリックリンクしてやると，リンク元のファイル名（つまり`argv[0]`）で実行するコマンドを指定できる便利機能があるのでそれが使いたい。しかし，FATファイルシステムにはシンボリックリンク機能がないらしい。コマンドの数だけBusyBoxの実行ファイルを置くわけにいかないので，FATの代わりにtarballをファイルシステムとして使う`tarfs`ファイルシステムサーバをデバッグ用に実装した。

まだ時間があるので`fork(2)`を実装してみる。プロセスのコピーを作り，戻り値が親プロセスと子プロセスで異なるという面倒くさいことこの上ないシステムコールなので敬遠していたが，実装してみると案外すんなり動いた。ELFローダ・ページフォルトハンドラのバグが大体取れたようでBusyBoxも安定して動くようになってきた。

### 5/7
プロセスの生成機能（fork/exec）が今日の目標。具体的には`/busybox sh -c "uname & wait"`が動くことが目標。
execveしたプロセスのページテーブルをクリアするのを忘れて謎のバグに悩まされたが，
既存のexecveをする関数を少しリファクタリングしたら動いた。

### 5/8
ttyを実装してBusyBoxのシェルを動かすのが今日の目標。既存のキーボードドライバとディスプレイドライバに繋げるだけのシンプルなやつ。そういえば`read(2)`のような「後でプロセスを起こす」処理のことを考えていないことに気づく。面倒くさいので，とりあえず動く`FIXME`コメントだらけのコードを書く。Busyboxのシェルが動くようになった。

### 5/9
リファクタリングの日。ファイルシステムまわりの設計の見直しや，システムコール処理でブロックしない（OSサーバのイベントループを止めない）よう「あとでシステムコールをやり直す」仕組みを作った。

### 5/10
リファクタリングとmasterへのマージが今日の目標。気になるところを直してマージ。FIXMEやTODOがたくさん残っているが，とりあえず動くのでよし。
