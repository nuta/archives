---
title: Rust製GUIスタック「Kazari」
title_en: Kazari - A GUI stack written in Rust
lang: ja
date: Mar 11, 2021
---

グラフィカルユーザインタフェース（GUI）をResea上に実装をした話。

かねてから，綺麗な画面をどうやって高速に描画するのか気になっていた。Webアプリケーションを除けばターミナル上で動くアプリをずっと書いていたので，実際にGUIの世界に触れることはなかった。ReseaにはGUIは必要ないと思っていたが，せっかくなので勉強がてらGUIを書いてみることにした。

GUIスタック自体はライブラリとして分けておくと便利そうだったので別プロジェクト [Kazari](https://github.com/nuta/kazari) として開発した。語感が好み。最近Reseaで対応を始めているRustで書いた。`no_std`（ただし`alloc`クレートには依存）環境で動く。移植性に気をつけたので，Resea以外の環境でもしっかり動いてくれるはず。

## 画面に描画されるまでを学ぶ
GUI周りはてんで知識がない。OpenGL，EGL，Skia，X11，...と「名前は聞いたことはある（何も知らない）」ものだらけなので，まずは大雑把な仕組みから掴んでいくことにした。参考になった文献はこちら。

- [Life of Pixel](https://bit.ly/lifeofapixel) by Steve Kobes
  - Google ChromeがWebページをどういう過程を経て描画するかを解説したスライド。
- [Wayland Protocol](https://wayland-book.com) by Drew DeVault
  - ウィンドウサーバの通信プロトコル「Wayland Protocol」の解説。X Window Systemの代替になるらしいとは聞いていた。
  - 各ウィンドウ，マウスカーソル等の画面の登場人物たちをどう抽象化するのか理解できた。
  - プロトコル自体はかなりシンプルで直感的な印象。というかX Window Systemは中々複雑なことしているのによく動いてるな。
- [weston-terminal](https://github.com/wayland-project/weston/blob/master/clients/terminal.c)
  - Wayland compositorのリファレンス実装「Weston」に入っている端末エミュレータアプリ。アプリケーション（クライアント）側で何をしているか，cairoのような2Dグラフィックスライブラリの役割は何かを理解できた。

## Waylandプロトコル入門
Waylandプロトコルは，クライアント（アプリケーション側）とサーバ（*compositor*側）で構成される。通信はWaylandプロトコル独自のバイナリプロトコルで以下のフィールドを含んだメッセージを互いに送り合う。

- 対象のオブジェクトID（ウィンドウ，キーボード，メモリバッファ等）
- リクエスト・イベントのopcode（プログラム言語でいうところのメソッド名）
- 引数（整数，文字列，オブジェクトID，ファイルハンドル等）

メッセージには「ウィンドウのタイトルを変更する」だとか「キーボードが押された」とか「ウィンドウの内容を変更した」など様々な要求・イベントが存在する。詳しくは[プロトコルの仕様](https://wayland.freedesktop.org/docs/html/apa.html)に載っている。

ウィンドウの描画はクライアントの責任になっており，サーバは「この共有メモリ上にRGBA8888形式でウィンドウの内容が入っている」くらいのことしか知らない。クライアントたちのメモリバッファ（画像）を重ね合わせて画面を描画するのがサーバの仕事。

きちんと理解したい人は [こちらのwebページ](https://wayland-book.com/) を読むのがおすすめ。

## Rust向けwayland-scannerの実装
Waylandには [XML形式のプロトコル定義](https://github.com/wayland-project/wayland/blob/master/protocol/wayland.xml) が存在する。C言語だと `wayland-scanner` というプログラムで，Wayland Protocolのグルーコードを自動生成してくれる。

Rustでもいくつか実装が散見されるが，自分で作った方が融通がききそうだったのでPythonでサクッと作った。生成されたコードは [ここに置いてある](https://docs.rs/kazari/0.0.1/kazari/wl/protocols/common/index.html)。

## `<canvas>`をRustから描画する
初っ端からResea上でGUIを作ろうにも，ディスプレイドライバ等を用意する必要があるのでWebブラウザ上で動くものを作ることにした。描画したフレームバッファを `CanvasRenderingContext2D.putImageData()` API経由でビットマップ画像として`<canvas>`に写すという単純な仕組み。

ビルド周りは[wasm-pack](https://github.com/rustwasm/wasm-pack)を使ってみた。大変よくできていて驚いた。とりあえず[embedded-graphics](https://github.com/embedded-graphics/embedded-graphics)というマイコンでも動く2Dグラフィックスライブラリのデモ画面を描画してみた。

![Chromeで動くembedded-graphics](embedded-graphics-demo.png)

Webブラウザ上で作るメリットの一つとして，DevToolsのパフォーマンスタブがかなり便利だった。何に時間を割いているのかRustで書いた部分もきちんと表示してくれる。スクリーンショットでどういう描画をしている時なのかも分かりやすい。
![Chrome DevToolsのパフォーマンスタブ。pixel_mutの実装がまずいことが分かる。](kazari-devtools-performance-tab.png)

## 動けマウスカーソル
まずはカーソルの画像を作る。ファイル形式ははXPMを採用。Xで使われているビットマップ画像形式で，ASCIIベースのかなり単純な形式であるのと，`pixmap(1)`という画像エディタがあるので使い勝手がよい。

![pixmap(1)で描くマウスカーソル](pixmap.png)

マウスカーソルを描いていると「マウスカーソルってどんな形だったっけ...?」と悩むようになってきた。いまいちしっくりくる形にならない。難しい。

カーソルができたので動かしてみる。マウスカーソルを動かすには，マウス入力の対応と重ね合わせ処理の2つの処理を実装する必要がある。

マウス入力はWebブラウザ上なので `keyup` と `keydown` イベントを読むだけで済む。

マウス入力を受け取ると，Kazariはカーソルの新しい座標を計算して移動前後の領域を再描画する必要がある。再描画といっても，ウィンドウやカーソルの画像を必要に応じて画面のバッファにメモリコピーするだけで済む。クライアントに再びウィンドウの描画を依頼する必要はない。

![動くマウスカーソル](kazari-moving-cursor.mp4)

ボタン等を実装するためには，クライアントにカーソルの位置・マウスボタンの状態を送る機能（`wl_pointer`）が必要になる。今は必要ないので後回し。

## ウィンドウを作る
Waylandプロトコルには「ボタンを作る」といったリッチなAPIは存在しない。クライアントは「このメモリバッファにRGBA8888形式でウィンドウの内容が入っているからよろしく」とか「ウィンドウのこの部分を更新した」といったサーバに伝えるだけ。言い換えれば，クライアントはウィンドウの「画像」を共有メモリを介して共有し，変更点を随時サーバに伝えている。

上の動画で登場しているウィンドウ（赤い四角形が入っているだけのやつ）は，クライアント側で2Dグラフィックライブラリ（`embedded-graphics`）を使って描いた。ただし，タイトルバーはサーバ側で描いている。拡大するとこんな感じ。

![ウィンドウの描画](kazari-window.png)

ウィンドウを描画するだけだとつまらないので，マウスのドラッグに対応してウィンドウを移動できるようにしてみた。

![ウィンドウの移動](kazari-moving-a-window.mp4)

アクティブウィンドウになると色が変わる変更を入れるだけでもGUIらしさが増して良い。

## キーボード入力
最後にキーボード入力の実装。基本的には，アクティブウィンドウのクライアントにキーボードイベントを送るだけ...ではあるが面倒な問題が一つある。「キーマップ」の存在である。

Waylandプロトコルでは [xkbcommon](https://xkbcommon.org/) を使うか，実装依存のキーコードをクライアントに送るかの二択になっている。とりあえず，ASCIIコードを送るだけにしてみた。

![キーボード入力](kazari-keyboard-input.mp4)

## まとめ
ウィンドウの描画やキーボード入力といった基礎の基礎部分ができるようになり，とりあえず満足。見栄えが（味はあるが）みすぼらしいので，アウトラインフォントの対応など2Dグラフィックライブラリ部分の拡充を進めたい。