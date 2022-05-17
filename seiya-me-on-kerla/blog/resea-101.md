---
title: 入門 Reseaオペレーティングシステム
title_en: Introduction to Resea operating system
date: Jul 21, 2021
lang: ja
---

これはマイクロカーネルベースのオペレーティングシステム「Resea（りーせあ）」の紹介記事です。非常にシンプルな作りをしているOSなので，この記事を読みさえすれば今日から立派なResea開発者です。

## Reseaの特徴

- シンプルで美しく，それでいて柔軟性の高い独自マイクロカーネルがベースです。カーネルがやることは，コンテキストスイッチとメッセージパッシングくらいです。
- おもちゃレベルのTCP/IPプロトコルスタック，FATファイルシステム，ハイパーバイザ（Resea上でLinuxがブートできる！）など基本的なOSの機能がユーザランドで実装されています。
- 移植性が高いです。どれぐらい高いかというと，週末の二日間だけで Raspberry Pi 3B+ 上でカーネル・ユーザランドの両方が動くところまで持っていくことができました。
- 大部分はC言語で書かれています。ただし，ユーザランドでは今後Rustをどんどん導入していく予定です。

## Reseaの設計思想

Reseaはマイクロカーネルをベースに，様々なユーザランドで動くタスクたち（サーバ）でOSの機能を実現しています。タスク間はメッセージパッシングで繋がっており，「メッセージを受信したら必要な処理をして返信する」というイベント駆動で実装されています。マイクロサービスみたいなものです。

マイクロカーネルベースのOSは古くからあり，特に珍しいものではありません。では **Reseaが何を目指しているかというと「ハックしやすいこと」です**。

具体的には，OSの仕組みを理解しやすく，機能拡張しやすいことを目標としています。この目標を達成すべくReseaに取り入れられている設計思想をかっこよく代弁してくれている名文句が以下の2つです:

> I conclude that there are two ways of constructing a software design: One way is to make it so simple that there are *obviously* no deficiencies and the other way is to make it so complicated that there are no *obvious* deficiencies.
> 
> The first method is far more difficult.
> 
> -- C.A.R Hoare, Turing award lecture ([PDF](https://www.cs.fsu.edu/~engelen/courses/COP4610/hoare.pdf))

ざっくりいうと「ソフトウェアの設計手法には『シンプルさを追求した**明らかに**欠陥がないもの』と『**明らかな**欠陥がない複雑なもの』の2通りがあり，前者ははるかに難しい」という話です。

Reseaはシンプルさを追求しています。「プロセス」と「スレッド」は「タスク」というひとつの概念にまとめています。タスク間通信には，単純で簡素なメッセージパッシングを使います。「ページャ」というシンプルな仕組みにより，ページフォルトの処理からABIエミュレーション（他OSカーネル用のバイナリを「そのまま」動かす機能）や，ハイパーバイザまで複雑な機能を柔軟に実現しています。OSの基礎部分は一日あれば内部構造を十分理解できるコード量にまとまっています。多少は便利な拡張機能が入るかもしれませんが，基礎部分（カーネルとResea標準ライブラリのよく使う部分）は既に完成状態にあります。

2つめの名文句はGolangのことわざです:

> Clear is better than clever.
>
> -- Rob Pike, Gopherfest Nov 2015 ([YouTube](https://www.youtube.com/watch?v=PAAkCSZUG1c&t=875s))

ざっくりいうと「言語の機能を駆使した短いコード（clever）より，分かりやすいコード（clear）をGoでは書こう」という話です。Reseaの実装でもこの思想を取り入れています。

Reseaはカーネル含め，ほとんど全ての部分がシングルスレッド・イベント駆動で書かれています。また，タスク間通信は同期的処理です。粒度の細かいロックを頑張って使い分ける必要はありません。そもそもロックはカーネルに一個あるだけです。

理想論をいうと「いい感じの非同期システムコールとマルチスレッドを駆使した高速処理」がみんなの夢ですが，Reseaではそれを諦め，性能を極める実装（cleverな実装）より単純な実装（clearな実装）を重視しています。計算機の能力を極限まで引き出したい時がもしあれば，その時に手段を考えて頑張りましょうというスタンスです。

この実装方針のおかげでReseaのプログラムは処理の流れを追いやすく，デッドロックとは大体無縁です。OSという複雑なソフトウェアを快適に開発できます。

## Reseaの全体像

![Reseaの全体像](resea-architecture-ja.png)

上の図では，分かりやすさ優先でアプリケーション，サーバ，デバイスドライバに分類されていますが，違いはありません。みな普通のユーザランドで動くプログラムです。仮想マシンもカーネルは少し特殊な扱いをしますが，タスクのひとつです。

Reseaのブート時には，カーネルイメージに埋め込まれたVMサーバが最初に起動します。vmサーバは自身に埋め込んであるファイルシステム（Linuxでいうinitramfs）から，他のサーバを展開し起動します。どのサーバを有効化するかはビルド設定で決められます。

## Reseaカーネル
Reseaカーネルは，MINIX3やseL4，HelenOSなど他のマイクロカーネルに比べてシンプルな作りになっており，次下のような特徴を持っています。

- カーネルのコア部分（`kernel/*.[ch]`）は1500行ほどしかありません。システムコールの数は（今のところ）16です。カーネルは既に完成状態にあり，新機能の予定は今のところありません。最近のOSの新機能はユーザランド側で実装されています。
- カーネル内部で動的なメモリ割り当ては存在しません。物理メモリの割り当てはユーザランド（vmサーバ内）で行っています。
- マルチプロセッサに対応しています。ただし[Big Kernel Lock](https://ja.wikipedia.org/wiki/%E3%82%B8%E3%83%A3%E3%82%A4%E3%82%A2%E3%83%B3%E3%83%88%E3%83%AD%E3%83%83%E3%82%AF)があるため，カーネルのコードを実行できるCPUコアは一度に一つだけです。おかげでカーネル開発が格段に楽になります。細かいロックは高速なプログラムを書く上での定石ですが，実はBig Kernel Lockのままでも[十分性能がでる](https://dl.acm.org/doi/10.1145/2797022.2797042)はずです。試してませんが。

## タスク
他のカーネルでいう「プロセス」に相当する機能です。各タスクは独立した仮想アドレス空間を持っています。ファイルといった概念はReseaカーネル内にはないので，ファイルテーブルのようなものは[カーネルには存在しません](https://github.com/nuta/resea/blob/dc57ce154ccaa2bf2017695062467dbda0b858f7/kernel/task.h#L48-L95)。

一つ特徴的なのが「ページャタスク」という概念です。ページフォルト（メモリアクセス）や実行時エラー（ゼロ除算など）が起きた時には，カーネルはタスクの実行を中断し，登録されたページャと呼ばれるタスクにメッセージを送ります。ページャタスクは実行ファイルの読み込みなど「タスクの実行に必要な処理」を担っており，メッセージを返信することで呼び出し元のタスクの処理を再開させます。

例えば，ページフォルトが起きると以下の流れでメモリページが用意され，タスクの実行を再開します:

1. タスクAの中でページフォルトが起きる
2. [カーネルのページフォルト例外ハンドラ](https://github.com/nuta/resea/blob/dc57ce154ccaa2bf2017695062467dbda0b858f7/kernel/arch/x64/trap.S#L189)にCPUの処理が飛ぶ
3. タスクAのページャタスク（ここではvmサーバとする）にページフォルト処理要求メッセージを送る（[ソースコード](https://github.com/nuta/resea/blob/dc57ce154ccaa2bf2017695062467dbda0b858f7/kernel/task.c#L366-L367)）
4. タスクAがメッセージ受信待ち状態に入る
5. vmサーバがファイルを読み出し等を行い，タスクAのページテーブルを更新し，メッセージをタスクAに返信する（[ソースコード](https://github.com/nuta/resea/blob/dc57ce154ccaa2bf2017695062467dbda0b858f7/servers/vm/main.c#L188-L216)）
6. タスクAの処理が[ココ](https://github.com/nuta/resea/blob/dc57ce154ccaa2bf2017695062467dbda0b858f7/kernel/task.c#L368)から再開する。ページフォルトが起きた箇所から[処理を復帰する](https://github.com/nuta/resea/blob/dc57ce154ccaa2bf2017695062467dbda0b858f7/kernel/arch/x64/trap.S#L284)

Linux KVMに似たハイパーバイザ機能や，ABIエミュレーション（LinuxバイナリをResea上でそのまま動かす）といった機能はこの仕組みを活用して実現されています。

ちなみに，大半のタスクのページャタスクはvmサーバです。

## IPC（メッセージパッシング）
他のカーネルと違い，Reseaのメッセージパッシング（IPC）は非常に簡素です。ファイルディスクリプタを送るような機能はありません。「ユーザランドで実現できることはそっちで頑張る」という思想から来ています。メッセージには以下の情報が入っています。

- **送信元のタスクID:** カーネルが書き込むので信頼できる
- **メッセージタイプ:** ユーザが好きに設定
- **固定長（256バイト）のバイト列:** ユーザが好きに設定

チャネルやソケットのような「コネクション」の概念はありません。宛先はタスクIDを直で指定します。柔軟性には欠けますが，実際に使ってみるとシンプルで分かりやすく，Reseaのアプリケーションだとこれで十分です。

細かい話をすると，IPCは「Inter-Process Communication」の略なのでReseaだと「Inter-Task Communication」と呼ぶべきなのですが，他のマイクロカーネルたちは皆「IPC」という言葉を使っているので合わせています。

### メッセージの送受信

メッセージの送信操作は全て同期的です。つまり，宛先のタスクが受信するまでブロックします。同期的にすることで，カーネルはメッセージキューを持つ必要がなく，また決定的に動きやすくなりデバッグしやすいのです。とはいえ，非同期にメッセージを送る必要がある場合（後述）があるので，カーネルではなく標準ライブラリ側で非同期メッセージパッシングAPIを提供しています。

C言語では次のようにメッセージを送受信できます。シンプルですね。

```c
struct message m;
// メッセージの種類とデータをセットする（共にユーザが自由に決められる）
// 後述する「IPCスタブ」を使ってもう少し便利にメッセージを構築できる
m.type = 1;
memcpy(&m.raw, message_data, sizeof(m.raw)); 

// メッセージをdestタスクに送信
// 宛先タスクが受信するまでブロックする
ipc_send(dest, &m);

// destタスクからのメッセージを待つ
// 送信して必要なくなった変数 m を再利用している
ipc_recv(dest, &m);

// ここでは ipc_send/ipc_recv を使って送受信操作を別々にやっているが，
// 通常は送信した後に受信もしてくれる ipc_call APIを使う
```

### 通知の送受信
メッセージパッシングに加え，Reseaは通知（notifications）と呼ばれるIPCを提供しています。32ビット長のビットマップのOR操作しかできませんが，メッセージとは違い非同期に通知を送信できます。何の役に立つかというと，非同期メッセージパッシングをカーネルではなく標準ライブラリ側で実現できます（後述）。他にはデバイスドライバにハードウェア割り込みを知らせるために使われています。

使い方はこんな感じです。

```c
// 送信側: 通知を送信する。ブロックしない。
ipc_notify(server_task, 1 << 0 /* ビットフラグ */);
// 既に受信している通知があるので，宛先のタスクは
// 3 = (1 << 0) | (1 << 1) を受信することになる
ipc_notify(server_task, 1 << 1 /* ビットフラグ */);

// 受信側: 通知がきたら，NOTIFICATIONS_MSGとして
//        メッセージと同じのように受け取れる
struct message m;
ipc_recv(client_task, &m);
if (m.type == NOTIFICATIONS_MSG) {
    // 3 が表示される
    INFO("notifications: %d", m.notifications.data);
}
```

通知の受信も `ipc_recv` APIを使います。通知のデータは `NOTIFICATIONS_MSG` タイプの擬似的なメッセージとして返ります。受信するとビットマップがクリアされて0になります。カーネルはメッセージより通知を優先します。

### IPCの細かい話 その1（Notify & Pullパターン)
メッセージパッシングは同期的処理です。大半のケースで上手く動くのですが，次のような双方向にメッセージを送りたい時に困ります。

```
               送信パケットを送信
TCP/IPサーバ  ------------------>    NICドライバ
             <------------------      サーバ
　            受信したパケットを送信
```

TCP/IPサーバが送信パケットのメッセージをNICドライバに送ろうする時に，NICドライバが逆に受信したパケットを送ろうしてしまうと，お互いが相手が受信状態になるまで待つ状態が永遠に続いてしまいます。

そこで，Reseaでは **「双方向に同期的なメッセージを送信してはならない」** というルールを敷いています。代わりに，サーバ→クライアントの通信は非同期IPCで行います。

```
                       2. 非同期メッセージが
                         あることを通知
TCP/IPサーバ           ------------------>    NICドライバ
                     <------------------      サーバ
1. 送信パケットを      3. 非同期メッセージを要求
非同期メッセージ
キューに追加            ------------------>    
                      4. 送信パケットを送信
```

非同期メッセージパッシングでは，前述した通知（Notifcations）APIを利用します。サーバ側の標準ライブラリ内には非同期メッセージ用のキューが入っており（[実装](https://github.com/nuta/resea/blob/dc57ce154ccaa2bf2017695062467dbda0b858f7/libs/resea/async.c)），クライアントは非同期メッセージがあるという旨の通知を受け取ると，メッセージをサーバからもらいます。サーバが通知の送信（notify）をして，クライアントがメッセージをとってくる（pull）ので，「Notify & Pullパターン」と呼んでいます。


### IPCの細かい話 その2（`ipc_call / ipc_reply` API）
上記のNotify & Pullパターンですが，クライアントが「3. 非同期メッセージを要求」した後に受信状態に（意図的に）入っていない場合，サーバはクライアントが受信状態になるまで永遠に待ってしまいます。

サーバ側がハングアップするのは避けたいので，ノンブロッキング（非同期とは別の概念）な送信API（`ipc_reply`）が存在します。サーバはクライアントにメッセージを送る時に`ipc_reply`で使うことで，クライアントが受信状態に入っていない場合は待たずに失敗扱いします。

クライアントはサーバを呼び出す時に常に `ipc_call` APIを利用します。このAPIは，サーバにメッセージを送信後，即座に受信状態に入ることを保証しているので，`ipc_reply`で送られる返信をきちんと受け取れるわけです。

### IPCの細かい話 その3（Out-of-Lineペイロード）
各メッセージは256バイトの固定長です。それ以上のデータを送るにはどうするのかというと，2つ方法があります。

- Out-of-Lineペイロードを使う
- そこそこ最近入った共有メモリ機能を使う（[使用例](https://github.com/nuta/resea/blob/main/servers/apps/test/shm_test.c)）

現在はもっぱら前者のOut-of-Line（OoL）ペイロードが活用されています。これはメモリ管理を司るvmサーバに代わりにデータコピーを行ってもらう機能です。アプリケーションは，次のようにデータへのポインタとその長さを指定するだけです。

```c
uint8_t very_large_data[] = { /* すごく長いデータ */ };
struct message m;
m.type = MSG_OOL;                     // 「OoLが含まれている」フラグを立てる
m.ool_ptr = very_large_data;          // OoLポインタ
m.ool_len = sizeof(very_large_data);  // OoLデータ長
ipc_send(dest_task, &m);
```

 `m.ool_ptr` の値は送信元タスクのアドレス空間のメモリアドレスなので，受信側で新しくmallocされた領域を指すように標準ライブラリがよしなに対応してくれます。

### IPCの細かい話 その4（エラー処理）
メッセージ受信API（`ipc_recv`, `ipc_call`, `ipc_replyrecv`）はエラーコード（`error_t`）を返却します。エラーの生成元は2通りあります:

- カーネル: メッセージの送受信自体に失敗した。
- メッセージの送信者（タスク）: メッセージの処理に失敗して `ipc_send_err`, `ipc_reply_err` API経由でエラーを出した。

それぞれ，REST APIでいうところのネットワークエラー（例: Destination Unreachable）と，HTTPエラーコード（例: 403 Forbidden）のような関係です。Reseaでは一緒くたにすることで，次のようにエラー処理をシンプルにしています。

```c
error_t err = ipc_call(dest_task, &m);
if (err != OK) {
    // 宛先タスクがエラーを返してきたか，そもそもメッセージの送受信操作に失敗した
    WARN("something went wrong: %s", err2str(err));
}
```

### IPCスタブ
Reseaでは様々な種類のメッセージが使われています。メッセージデータの構造体定義など（IPCスタブ）を，C言語とRustでいちいちで書いていくのは億劫なので自動生成しています。ちょうど [gRPC](https://grpc.io/) みたいなやつです。

Reseaでは，次のような見た目の独自インタフェース定義言語を導入しています。

```
// ファイルシステムサーバで使われるメッセージ
namespace fs {
    rpc open(path: str) -> (handle: handle);
    rpc close(handle: handle) -> ();
    rpc read(handle: handle, offset: offset, len: size) -> (data: bytes);
}
```

ちなみに `str` と `bytes` というのがOoLペイロードです。`str`はC文字列（`'\0'`で終わる），`bytes`は任意のバイト列を指します。IPCスタブを使うと，次のように書きやすく読みやすいコードを実現できます:

```c
// ファイルの先頭から128バイト読み込むリクエスト
struct mesage m;
m.type = FS_READ_MSG;
m.fs_read.handle = handle;
m.fs_read.offset = 0;
m.fs_read.len = 128;

// リクエストを送り，返信を待つ（変数 m は返信で上書きされる）
ipc_call(fs_server, &m);

// 読んだデータを表示する
INFO("file data: %s", m.fs_read_reply.data);
```

## ユーザランドプログラミングの雰囲気
[公式ドキュメント](https://resea.org/docs/userspace/index.html)にユーザランドプログラミングの解説が載っています。ここでは，どんな雰囲気かを軽く紹介します。

### サーバ
TCP/IPやファイルシステムなど，OSの機能を提供するタスクのことを「サーバ」と呼んでいます。次のように「メッセージを待つ→メッセージの種類に応じた処理を行う→返信を行う→次のメッセージを待つ...」という書き方をしています。

```c
void main(void) {
    // サーバをサービスディスカバリに登録する
    ipc_serve("rtc");

    while (true) {
        // 次のメッセージが来るまで待つ
        // IPC_ANYは「任意のタスクからのメッセージを受信」する設定
        struct message m;
        ipc_recv(IPC_ANY, &m);

        switch (m.type) {
            case RTC_READ_MSG:
                // 返信メッセージを構築する
                m.type = RTC_READ_REPLY_MSG;
                m.rtc_read_reply.year = 2021;
                m.rtc_read_reply.month = 7;
                m.rtc_read_reply.day = 20;

                // 返信する
                ipc_reply(m.src, &m);
                break;
            default:
                WARN("unknown message %d", m.type);
        }
    }
}
```


### アプリケーション
アプリケーション（クライアント側）の雰囲気はこんな感じです:

```c
void print_date(void) {
    // 時刻デバイスドライバのタスクIDを検索する（サービスディスカバリ）
    task_t rtc_driver = ipc_lookup("rtc");

    // RTC_READ_MSG メッセージを送信して，返信が来るまで待つ
    // 返信は m に上書きされる
    struct message m;
    m.type = RTC_READ_MSG;
    ipc_call(rtc_driver, &m);

    ASSERT(m.type == RTC_READ_REPLY_MSG);
    INFO("today: %d-%d-%d",
        m.rtc_read_reply.year,
        m.rtc_read_reply.month,
        m.rtc_read_reply.day,
    );
}
```

クライアント側は，サーバのタスクIDを検索した後，リクエストメッセージを送って返信を待つ，いわゆる [RPC](https://e-words.jp/w/RPC.html) でサーバと通信します。REST APIを叩くWebアプリケーションを書くような気分です。

## Reseaを動かしてみる
筆者が開発環境の快適さこだわるタイプなので開発環境の構築は簡単です。LLVMのツールチェーン，Python，QEMU，makeがあれば十分です。Ubuntuだと20.04以降が必要です。
```
macOSの場合: brew install llvm python qemu 
Ubuntuの場合: apt install llvm clang lld python3 qemu-system make
```

次にリポジトリをダウンロードしてpipパッケージをいくつかインストールします。
```
git clone https://github.com/nuta/resea
cd resea
pip3 install --user -r tools/requirements.txt
```

次にビルド設定を行います。どのサーバ・アプリを有効化する等を設定できます。
```
make menuconfig
```

準備ができたので，ビルドしてQEMU上で実行してみましょう。
```
make run
```

やる気があれば，[実機（Intel NUC）](https://seiya.me/implementing-hypervisor-on-resea) や [Google Compute Engine](https://seiya.me/resea-on-google-compute-engine) 上で動かすこともできます。

## Reseaの今後

たまに興味を持ってくれた方がコントリビュートをしてくれますが，ほとんど全て筆者が一人で書いています。フルタイムで働くようになりReseaの開発スピードは亀並みになりましたが，半年に一回は面白い新機能を書き上げられるようぼちぼち頑張っています。

今月から「実用的なOSにする」をテーマに，[Waylandプロトコル](https://ja.wikipedia.org/wiki/Wayland)に影響を受けたGUIや[Windows Subsystem for Linux](hhttps://ja.wikipedia.org/wiki/Windows_Subsystem_for_Linux)もどきを今年中に実装する予定です。順調に進めば，来年（2022年）にはMINIX3やseL4のように「信頼性のあるOSにする」がテーマになることでしょう。
