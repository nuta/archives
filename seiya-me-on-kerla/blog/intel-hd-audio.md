---
title: Intel HD Audioで音を奏でよう
title_en: Playing a sound using Intel HD Audio
lang: ja
date: Nov 11, 2020
---

PCで音を奏でるには，ビープ音を鳴らす仕組みの他にIntel High Definition Audio (以下，HD Audio）という仕様があります。スピーカー以外にもマイクといった音声入力側の制御もカバーしています。サウンドデバイスを気にしたことがないのでよく知らないのですが，現代の主流だそうです。

## 音のデジタル表現
コンピュータは，文字を文字コードという番号を振ることで表現し，画像は原色（RGB/CMYK）の強さ（濃さ？）の組み合わせで表現します。音はどう表現するかというと音は波の一種なので，その波形の各時間間隔での振幅値（例: 整数）で表現します。モノラルなら振幅値が詰まった1次元配列，ステレオならLとRそれぞれの1次元配列の組になります。

時間間隔を決める値がサンプリングレート（単位はHz）であり，また振幅値の表現範囲（8/16/24-bit）を量子化ビット数と呼びます。

## WAVファイル
WAVファイルは非圧縮の音声データを格納するファイルフォーマットとして広く使われています。

以下のようにscipyを使うと簡単に（ステレオ用の）WAVファイルから波形データを取り出すことができます。

```py
# https://docs.scipy.org/doc/scipy/reference/generated/scipy.io.wavfile.read.html
import argparse
from scipy.io import wavfile

parser = argparse.ArgumentParser()
parser.add_argument("wav_file")
args = parser.parse_args()

samplerate, data = wavfile.read(args.wav_file)
length = data.shape[0] / samplerate
print(f"number of channels = {data.shape[1]}")
print(f"samplerate = {samplerate}")
print(f"length = {length}s ({len(data)} bytes)")
print(f"data (L) = {data[:, 0]}")
print(f"data (R) = {data[:, 1]}")
```

スピーカーから音を鳴らすには，この波形データの設定（サンプリングレート・量子化ビット数）と，振幅値（`data (L)`と`data (R)`）をHD Audioに教えてあげる必要があります。

## Intel HD Audioの概要
HD Audioの概要は仕様書の「2. Architecture Overview」で解説されています。HD Audioの世界では以下のコンポーネントが登場します。

- **コントローラ（Controller）:** HD Audioを制御するコンポーネント。OS（サウンドデバイスドライバ）がやり取りするところ。
- **コーデック（Codec）:** コントローラから送られる（DMA転送）音声データを解釈し，コンバータに送るコンポーネント。MP3とかを指す「コーディック」とは別物。
- **コンバータ（Converter）:** 音声データ（デジタル情報）をアナログ情報に変換し，スピーカーなどに出力するするコンポーネント。DAC/ADCを搭載している。
- **リンク（Link）:** コントローラとコーデックとの通信部分。PCIバスみたいなイメージなのだと思う。OSから意識する必要はまずないはず。

### ストリーム（Stream）
仕様書では「ストリームとは論理的または仮想的に作られる，リンクを経由したメモリバッファとコーデックの関係」と定義しています。ざっくりいうと，入力バッファのことです。デバイスをセットアップした後，OSはストリーム（メインメモリ上）に音声データを書き込んでいき，サウンドデバイスはストリーム上のデータを順に読み込んで音を奏でます。

ストリームには1つ以上のチャネル（Channel）が含まれていれおり，例えばステレオスピーカーなら左音声と右音声の2つのチャネルが存在します。

### コーデックのアーキテクチャ
HD Audioはスピーカー以外にもマイクやヘッドセットといったデバイスにも対応しています。ホットプラグ（多分イヤホンの抜き差しとかのことでしょう）にも対応しています。OSはコンピュータに一体どのような機能を持つデバイス（コーデック）が挿さっているのかを確認する必要があります。

HD Audioでは，rootノードから木構造でサウンドデバイスたちの存在や機能を表現します:

```
(root node)
    |
    +--- Audio Function Group (node)
    |      |
    |      +--- Audio Output Converter Widget (node)
    |      +--- Pin Complex Widget (node)
    |
    +--- ...
```

rootノードからには，任意個のFunction Groupという集まりが繋がっています。各Function Groupにはウィジェット（Widget）と呼ばれる各コンポーネントが入っています。

リンクに繋がる各コーデックには初期化時にコーデックアドレス（CAd）が設定され，コーデック内の各ノードにはノードアドレス（NID）が付与されています。OSはこれらを使って特定のノードに命令を送ります。

### Function Group
仕様書にはAudio Function GroupとModem Function Groupの2つが定義されています。今回は音を鳴らしたいのでAudio Function Groupだけに注目します。

Modem Function GroupはたぶんADSLとかで使うあのモデムです。

### ウィジェット（Widget）
QEMUで音を鳴らすのに制御が必要なのは以下のウィジェットです。

- **Audio Output Converter Widget:** 音声データをアナログ情報に変換する部分（DAC）です。アンプが入っていることもあります。音量はここで調節できます。OSは，このウィジェットが読むストリームとそのデータフォーマット（サンプリングレート・量子化ビット数など）を設定する必要があります。
- **Pin Widget** スピーカーに繋がっている部分です。なんだか電源モードの設定等ができるようです。OSは単に電源ONするだけです。

### CORB / RIRB
MMIO上のレジスタはHD Audioコントローラを制御するものです。各ノードを制御するにはCORB（Command Outbound Ring Buffer）とRIRB（Response Inbound Ring Buffer）という仕組みを使います。

CORBとRIRBはリングキューになっていて，CORBに命令を書き込んでその返り値をRIRBから取得します。

CORBに書き込む命令の形式は「7.3. Codec Parameters and Controls」で触れられています。いくつか独特な用語が導入されていますが，verbがプログラム言語でいうところの関数，payloadがその引数のイメージです。

## 音を鳴らすまで
仕様書を読んでHD Audioのイメージがつかめたら音を鳴らしてみましょう。

HD Audioで音を鳴らすにはHD Audioの初期化，繋がっているノードの探索・初期化，そして音声データの転送の大きく3つの実装が必要です。

[サンプルコードはこちらにあります](https://github.com/nuta/resea/blob/master/servers/experimental/hdaudio/hdaudio.c)。QEMUでしか試していませんが，大体の流れは実機でもまあ一緒でしょう。

### とても重要なこと
**スピーカーのボリュームに注意しましょう。気をつけないと突然大音量のノイズが鳴り始めます。** 自分のMacBookだと**音量を数%にしても十分びっくりする音が出ます**。ところで音声関連に関わっているプログラマってどうやって大音量から自衛しているんでしょうか。気になります。

### WAVファイルの準備
まずは再生したい音声ファイルを用意しましょう。WAVファイルが良いです。`ffmpeg -i`でファイル形式をチェックできます。

```
$ ffmpeg -i ~/Music/meow.wav
...
Input #0, wav, from '/Users/seiya/Music/meow.wav':
  Duration: 00:00:04.12, bitrate: 1411 kb/s
    Stream #0:0: Audio: pcm_s16le ([1][0][0][0] / 0x0001), 44100 Hz, stereo, s16, 1411 kb/s
                        ^^^                                ^^^^^     ^^^^^^  ^^^
                         「pcm」のはず                 サンプリングレート   2チャネル  符号付き2バイト整数
```

このWAVファイルは「サンプリングレートが44.1KHz，2チャネル，各振幅値は16ビットの符号付きリトルエンディアン」となっています。デバイスドライバは，この情報をAudio Output Converterノードに教えてあげる必要があります。

**符号付き**整数であることはしっかりチェックしておきましょう。符号なしのデータを入力するとおかしな音が鳴ります。

余談ですが，仕様書には音声データのendiannessについて特に説明していない気がします。*Intel* HD Audioなので暗黙的にリトルエンディアンなのでしょうか。

### HD Audioコントローラの検出
まず，Intel HD Audioデバイスがどこにいるかを探す必要があります。QEMUだと `-device intel-hda,debug=3 -device hda-duplex,debug=3` をコマンドラインオプションで指定するとPCIバス上に出現します。`debug=3`はQEMUのログレベルの設定です。デバッグの際には[DEBUG_OUT](https://github.com/qemu/qemu/blob/1bd5556f6686365e76f7ff67fe67260c449e8345/audio/audio.c#L44)も有効化（コメントアウトを解除）しておくと便利です。

```
(qemu) info pci
...
  Bus  0, device   4, function 0:
    Audio controller: PCI device 8086:2668
      PCI subsystem 1af4:1100
      IRQ 11, pin A
      BAR0: 32 bit memory at 0xfebf0000 [0xfebf3fff].
      id ""
```

BAR0にMMIOのベースアドレスが載っています。ここへのメモリ読み書きでコントロールレジスタ（`GCAP`，`VMIN`，...）にアクセスできます。

PCIバスマスタの設定も忘れずに。忘れるとサウンドデバイスが音声データをメモリから取得することができません。

### HD Audioコントローラの初期化
以下の流れでコントローラを初期化します。

1. `GCTL`のリセットビットをいじってリセットする。QEMU上ならなくても一応動く。
2. CORBの初期化。バッファの物理アドレス（`CORBLBASE`，`CORBHBASE`）の設定，ポインタ（`CORBWP`，`CORBRP`）のリセットをした後にCORBを有効化（`CORBCTL`の`CORBRUN`をセット）する。CORBのサイズは`CORBSIZE`で取得できる。
3. RIRBの初期化。CORBと同じ要領で行う。

### ノードの探索
次にPin ComplexノードとSpeakerノードを探します。ルートノード（`NID = 0`）から繋がっているノードを再帰的に探索していきます。

NIDのベースアドレスとノードの数はGet Parameter verbで取得できるSubordinate Node Countパラメータに記載されています。各ノードの種類もGet Parameter verbを送って取得できます（7.3.4.6. Audio Widget Capabilities）。

詳細はサンプルコードか「7.3 Codec Parameters and Controls」を参考にしてください。Get Parameter verbで取れる各パラメータの詳細は「7.3.4 Parameters」に載っています。

音を鳴らすにはPin Complexノード（`Widget Type == 0x04`）を探す必要があります。見つけたら，そのPin Complexノードに繋がっているAudio Output Converterノードを探します（7.3.3.3. Get Connection List Entryに書いてあるverbを使う）。

### ストリームの構築と音声データの転送
Audio Output Converterに音声データを流す仮想的な通信路（ストリーム）を作ります。

ストリームは複数個作ることができ，どのストリームを使うかはOSの自由です。各ストリームにはBuffer Descriptor Listと呼ばれるLinuxの`iovec`みたいな仕組みが入っています。OSは音声データをメモリ上に用意して，このバッファディスクリプタにそのアドレスと長さを書き込みます。

バッファ上の音声データの中身は「4.5.1. Stream Data In Memory」で解説されています。WAVファイルの中身を（パディングや順序を）ちょっといじって書き込むだけです。楽したい人は[このスクリプトでCに埋め込めます](https://github.com/nuta/resea/blob/master/servers/experimental/hdaudio/wav2c.py)。

後はコントローラが，サンプリングレートの周波数の頻度でリンク経由で各コーデックに送信してくれます。OSはサウンドデバイスがストリームの最後まで再生する前に，次のデータをたまに書き込むだけです。コントローラが面倒を見てくれるおかげで厳格なタスクスケジューリングは必要ないのはとても嬉しいですね。

### Audio Output Converterノードの設定
Audio Output Converterには，ストリームIDとストリーム上の音声データの形式（サンプリング周波数など）をそれぞれChannel/Stream ID verbとStream Format verbで設定します。

### ミュート解除・音量調整
Audio Output ConverterノードのAmplifier Gain Mute verbと，Pin ComplexノードのPower State verbを叩いてミュートを解除できます。**デフォルトで大音量なので注意**。

### 再生開始
**くどいですがスピーカーのボリュームに注意しましょう。気をつけないと突然大音量のノイズが鳴り始めます。自分のMacBookだと音量を数%にしても十分びっくりする音が出ます**。

`REG_SDnCTL`の`RUN`に1をセットすると，バッファディスクリプタに設定された音声データがループ再生されます。

## 音がおかしいときのチェックリスト
- 音声データがデバイスに設定した形式（サンプリングレート・量子化ビット数・チャンネル数）と異なるかもしれません。
- 同じ量子化ビット数でも，符号付きと符号なしの2種類が存在します。**HD Audioでは符号付きのみ受け付ける**ようです。
  - 入力するWAVファイルの中身を予め確認・変換しておきましょう。`ffmpeg -i wavfile`して`Stream #0:0: Audio: pcm_s16le`の部分を見ると[確認できます](https://trac.ffmpeg.org/wiki/audio%20types)。
  - 仕様書の中には音声データの各振幅値が符号付きか否かを明言していないような気がします。自分は[QEMUのソースコード](https://github.com/qemu/qemu/blob/master/hw/audio/hda-codec.c#L104-L108)を読んで初めて知りました。
- 量子化ビット数が24-bitの場合，paddingが必要です（5.3.3.2. Zero Padding Inbound Stream Packets）。

## サンプルコード
[resea/hdaudio.c at master · nuta/resea · GitHub](https://github.com/nuta/resea/blob/master/servers/experimental/hdaudio/hdaudio.c)

## 参考資料
- [Intel HD Audioの仕様書](http://www.intel.co.uk/content/www/us/en/standards/high-definition-audio-specification.html)
- [osdev.org wiki](https://wiki.osdev.org/Intel_High_Definition_Audio)
  - 仕様書を読んでいないと理解は難しい。「Playing Sound」の章はチェックリストとして便利。
