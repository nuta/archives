---
title: Google Compute EngineでReseaを動かす
title_en: Running Resea on Google Compute Engine
lang: ja
date: Oct 8, 2020
---

![Google Compute Engine上で動くResea](resea-on-gce.png)

先日virtioデバイスドライバを書いたので，Google Compute Engineで使ってみることにした。
上のスクリーンショットを見ると，きちんとDHCPからIPアドレスを取得できていることがわかる。
[Web APIサーバ](https://resea.org/docs/apps/webapi.html)もきちんと動いた。

## 独自OS on GCE
GCEのドキュメントには [Building custom operating systems](https://cloud.google.com/compute/docs/images/building-custom-os)
というGoogle側で用意していないOSを動かしたい人向けの情報が載っている。GCEでは以下の実行環境を提供している:

- x86_64 CPU。今のところGCEはArm等違うアーキテクチャのインスタンスは提供していない。
- virtio-net (legacy device)
- virtio-scsi (未検証だがたぶんlegacy device)

Reseaは修正の必要なくf1-microインスタンスで一発で動いてくれたので，エミュレータで動くならGCEでもすんなり動くものなんだろう。
ただし，ネットワーク周りにはいくつか落とし穴がある:

- GCEのvirtio-netデバイスはどうやら Legacy Device（virtioの古い仕様に従うデバイス）らしい。
  Modern Deviceではないのは意外だが，[Compute Engine Virtual NIC](https://git.kernel.org/pub/scm/linux/kernel/git/netdev/net-next.git/commit/?id=337d1ccb3dc917c5801f40e71374a63f3e18678b)というものを作っているようなので，virtioではなく独自路線に進んでいくのだろうか。
- ブロードキャストパケットは他のVMには流れない。[そもそも許可していない](https://cloud.google.com/vpc/docs/vpc)らしいので，OSがきちんとパケットを送信できているかパケットキャプチャして確認できない。[Packet Mirroring](https://cloud.google.com/vpc/docs/packet-mirroring)を使えばわかるのかも。
- DHCP DISCOVERのBROADCASTフラグが立っていないとDHCPサーバは返事をしない。

## デプロイの流れ
GCEはISOファイルからの起動はサポートしていないので少し手間がかかる:

1. 生（raw）のハードディスクイメージを作る。
2. ハードディスクイメージをGoogle Cloud Storageへアップロード
3. イメージを登録（`gcloud compute images create`）
4. ディスクを作成（`gcloud beta compute disks create`）
5. VMを作成（`gcloud beta compute instances create`）

## デバッグ
シリアルポートの出力を `gcloud compute instances get-serial-port-output` から見ることができるので便利。
後は勘で頑張る。

## 余談
GCEはKVMらしいので，QEMUで動くならGCEでも動いて当たり前だろうと思っていたら[KVMは使っているがQEMUは使っていないらしい](https://cloud.google.com/blog/products/gcp/7-ways-we-harden-our-kvm-hypervisor-at-google-cloud-security-in-plaintext)。セキュリティのために自分たちでハードウェアエミュレーション部分を作り直すというはかっこいいですね。
